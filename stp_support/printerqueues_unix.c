#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBCUPS
#include <cups/cups.h>
#endif

struct PrintSystem
{
	const char *print_command;
	const char *queue_select;
	const char *raw_flag;
	const char *key_file;
	const char *scan_command;
	const char *extra_opts;
};

static struct PrintSystem printsystems[] =
{
  { "lp -s", "-d", "-oraw", "/usr/sbin/cupsd",
    "/usr/bin/lpstat -v | grep -i '^device for ' | awk '{print $3}' | sed 's/://'" },
  { "lp -s", "-d", "-oraw", "/usr/bin/lp",
    "/usr/bin/lpstat -v | grep -i '^device for ' | awk '{print $3}' | sed 's/://'" },
  { "lpr", "-P", "-l", "/etc/lpc",
    "/etc/lpc status | grep '^...*:' | sed 's/:.*//'" },
  { "lpr", "-P", "-l", "/usr/bsd/lpc",
    "/usr/bsd/lpc status | grep '^...*:' | sed 's/:.*//'" },
  { "lpr", "-P", "-l", "/usr/etc/lpc",
    "/usr/etc/lpc status | grep '^...*:' | sed 's/:.*//'" },
  { "lpr", "-P", "-l", "/usr/libexec/lpc",
    "/usr/libexec/lpc status | grep '^...*:' | sed 's/:.*//'" },
  { "lpr", "-P", "-l", "/usr/sbin/lpc",
    "/usr/sbin/lpc status | grep '^...*:' | sed 's/:.*//'" },
};


struct printernode
{
	void (*Dispose)(struct printernode *pn);
	struct printernode *next,*prev;
	struct pqprivate *head;
	char *name;
};


enum pqoutputmode
{
	PQMODE_COMMAND,
	PQMODE_CUSTOMCOMMAND,
	PQMODE_FILE	
};


struct pqprivate
{
	void (*Dispose)(struct pqprivate *pp);
	char *currentqueue;
	int printsystem;
	struct printernode *first;
	int cancelled;
	char *printcommand;
	char *extopts;

	enum pqoutputmode mode;
	char *customcommand;
	char *(*getfilecallback)(void *userdata);
	void *userdata;

	enum pqinfo_datatype datatype;	// Used to set the raw flag in the print command
	int pipefd[2];
	int outputfd;
	int childpid;	
};


void pn_dispose(struct printernode *pn)
{
	if(pn->next)
		pn->next->prev=pn->prev;
	if(pn->prev)
		pn->prev->next=pn->next;
	else
		pn->head->first=pn->next;
	if(pn->name)
		free(pn->name);
	free(pn);
}


struct printernode *printernode_create(struct pqprivate *head,const char *name)
{
	struct printernode *pn=(struct printernode *)malloc(sizeof(struct printernode));
	if(pn)
	{
		struct printernode *p=head->first;
		while(p)
		{
			if(strcmp(p->name,name)==0)
			{
				free(pn);
				return(p);
			}
			p=p->next;
		}
		p=head->first;
		pn->next=pn->prev=NULL;
		pn->Dispose=pn_dispose;
		pn->head=head;
		if(p)
		{
			while(p->next)
				p=p->next;
			p->next=pn;
			pn->prev=p;
		}
		else
			head->first=pn;

		pn->name=strdup(name);
	}
	return(pn);
}


static void pqp_dispose(struct pqprivate *pp)
{
	while(pp->first)
		pp->first->Dispose(pp->first);
	if(pp->currentqueue)
		free(pp->currentqueue);
	if(pp->printcommand)
		free(pp->printcommand);
	if(pp->customcommand)
		free(pp->customcommand);
	if(pp->extopts)
		free(pp->extopts);
	free(pp);
}


static bool pqp_identifyprintsystem(struct pqprivate *pq)
{
#ifdef HAVE_LIBCUPS
	pq->printsystem=0;
	return(true);
#else
	int count=sizeof(printsystems)/sizeof(struct PrintSystem);
	int i;
	for(i=0;i<count;++i)
	{
		if (!access(printsystems[i].key_file, R_OK))
		{
			pq->printsystem=i;
			return(true);
		}	
	}
#endif
	return(false);
}


static void pqp_buildqueuelist(struct pqprivate *pp)
{
	// If libcups is available we use it to identify the print queues, to avoid the
	// increasingly messy environment variable issues on localized systems.
#ifdef HAVE_LIBCUPS
	cups_dest_t *dests;
	int c,i;
	pqp_identifyprintsystem(pp);

	c=cupsGetDests(&dests);
	for(i=0;i<c;++i)
	{
		printernode_create(pp,dests[i].name);
	}
	cupsFreeDests(c,dests);
	if(!c)
#endif
	{
	FILE *pfile;
	char buf[256];

	const char *old_lang=getenv("LANG");
	const char *old_lcmess=getenv("LC_MESSAGES");
	const char *old_lcall=getenv("LC_ALL");
	setenv("LANG","C",1);
	setenv("LC_ALL","C",1);
	setenv("LC_MESSAGES","C",1);

	if(pqp_identifyprintsystem(pp))
	{
		if ((pfile = popen(printsystems[pp->printsystem].scan_command, "r")))
		{
			while (fgets(buf, sizeof(buf), pfile) != NULL)
			{
				int i;
				for(i=strlen(buf)-1;i>0;--i)
				{
					if(buf[i]=='\n')
						buf[i]=0;
					if(buf[i]=='\r')
						buf[i]=0;
				}
				printernode_create(pp,buf);
			}
			pclose(pfile);
		}

		if(old_lang)
			setenv("LANG",old_lang,1);
		else
			unsetenv("LANG");

		if(old_lcall)
			setenv("LC_ALL",old_lcall,1);
		else
			unsetenv("LC_ALL");

		if(old_lcmess)
			setenv("LC_MESSAGES",old_lcmess,1);
		else
			unsetenv("LC_MESSAGES");
		}
	}
	printernode_create(pp,PRINTERQUEUE_CUSTOMCOMMAND);
	printernode_create(pp,PRINTERQUEUE_SAVETOFILE);
}


static struct pqprivate *pqprivate_create()
{
	struct pqprivate *pp=(struct pqprivate *)malloc(sizeof(struct pqprivate));
	if(pp)
	{
		pp->Dispose=pqp_dispose;
		pp->first=NULL;
		pp->currentqueue=NULL;
		pp->printcommand=NULL;
		pp->customcommand=NULL;
		pp->extopts=strdup("");
		pp->getfilecallback=NULL;
		pp->datatype=PQINFO_RAW;
		pqp_buildqueuelist(pp);
	}
	return(pp);
}


/* pqinfo member functions */


static void dispose(struct pqinfo *pq)
{
	if(pq->priv)
		pq->priv->Dispose(pq->priv);
	free(pq);
}


static int getprintercount(struct pqinfo *pq)
{
	int count=0;
	struct printernode *p=pq->priv->first;
	while(p)
	{
		++count;
		p=p->next;
	}
	return(count);
}


static char *getprintername(struct pqinfo *pq,int index)
{
	struct printernode *p=pq->priv->first;
	while(--index>=0 && p)
	{
		p=p->next;
	}
	if(p)
		return(strdup(p->name));
	return(NULL);
}


static void setprinterqueue(struct pqinfo *pq,const char *queue)
{
	if(queue)
	{
		if(pq->priv->currentqueue)
			free(pq->priv->currentqueue);
		pq->priv->currentqueue=strdup(queue);
	}
	if(strcmp(queue,PRINTERQUEUE_CUSTOMCOMMAND)==0)
		pq->priv->mode=PQMODE_CUSTOMCOMMAND;
	else if(strcmp(queue,PRINTERQUEUE_SAVETOFILE)==0)
		pq->priv->mode=PQMODE_FILE;
	else
		pq->priv->mode=PQMODE_COMMAND;
}


static const char *getprinterqueue(struct pqinfo *pq)
{
	return(pq->priv->currentqueue);
}


static void setcustomcommand(struct pqinfo *pq,const char *command)
{
	if(pq->priv->customcommand)
		free(pq->priv->customcommand);
	if(command)
		pq->priv->customcommand=strdup(command);
	else
		pq->priv->customcommand=NULL;
}


static const char *getcustomcommand(struct pqinfo *pq)
{
	return(pq->priv->customcommand);
}


static void setextopts(struct pqinfo *pq,const char *opts)
{
	if(pq->priv->extopts)
		free(pq->priv->extopts);
	if(opts)
		pq->priv->extopts=strdup(opts);
	else
		pq->priv->extopts=strdup("");
}


static void setgetfilenamecallback(struct pqinfo *pq,char *(*func)(void *userdata),void *userdata)
{
	pq->priv->getfilecallback=func;
	pq->priv->userdata=userdata;
}


static char *getdriver(struct pqinfo *pq)
{
	if(strcmp(pq->priv->currentqueue,PRINTERQUEUE_SAVETOFILE)==0)
		return(NULL);
	if(strcmp(pq->priv->currentqueue,PRINTERQUEUE_CUSTOMCOMMAND)==0)
		return(NULL);
#ifdef HAVE_LIBCUPS
	char *result=NULL;
	const char *ppdname=cupsGetPPD(pq->priv->currentqueue);
	if(ppdname)
	{
		const char *cmd="cat %s | grep -i 'StpDriverName' | awk '{print $2}' | sed 's/\"//g'";
		int len=strlen(cmd)+strlen(ppdname)+2;
		char *cmd2=(char *)malloc(len);
		FILE *pfile;
		
		sprintf(cmd2,cmd,ppdname);
		
		if((pfile = popen(cmd2, "r")))
		{
			char buf[256];
			int i;
			if(fgets(buf, sizeof(buf), pfile))
			{
				for(i=strlen(buf)-1;i>0;--i)
				{
					if(buf[i]=='\n')
						buf[i]=0;
					if(buf[i]=='\r')
						buf[i]=0;
				}
				if(strlen(buf)>1)
					result=strdup(buf);
			}
			pclose(pfile);
		}
		free(cmd2);
	}
	if(!result)
		result=strdup("ps2");

	return(result);
#else
	return(NULL);
#endif
}

static char *getppd(struct pqinfo *pq)
{
	char *result=NULL;
#ifdef HAVE_LIBCUPS
	const char *ppdname=cupsGetPPD(pq->priv->currentqueue);
	if(ppdname)
		result=strdup(ppdname);
#endif
	return(result);
}


/************ Job Spooling ************/

static int aborted=0;

static void sighandler(int sig)
{
	switch(sig)
	{
		case SIGPIPE:
			fprintf(stderr,"PrintQueue: Received SIGPIPE\n");
			aborted=1;
			break;
		default:
			break;
	}
}


static void setdatatype(struct pqinfo *pq,enum pqinfo_datatype type)
{
	pq->priv->datatype=type;
}


static char *buildprintcommand(struct pqinfo *pq)
{
	int len=strlen(printsystems[pq->priv->printsystem].print_command);
	len+=strlen(printsystems[pq->priv->printsystem].queue_select);
	len+=strlen(printsystems[pq->priv->printsystem].raw_flag);
	len+=strlen(pq->priv->currentqueue);
	len+=strlen(pq->priv->extopts)+2;

#if 0
	const char *driver=getdriver(pq);
	if(driver)
	{
		if(strcmp("ps",driver)==0 || strcmp("ps2",driver)==0)
			rawmode=0;
	}
	else
	{
		fprintf(stderr,"***\n*** WARNING: driver is null - this shouldn't happen - please report this bug!\n***\n");
		rawmode=0;
	}
#endif

	char *cmd=(char *)malloc(len+15);
	sprintf(cmd,"%s %s %s %s %s",
		printsystems[pq->priv->printsystem].print_command,
		pq->priv->currentqueue ? printsystems[pq->priv->printsystem].queue_select : "",
		pq->priv->currentqueue ? pq->priv->currentqueue : "",
		pq->priv->datatype==PQINFO_RAW ? printsystems[pq->priv->printsystem].raw_flag : "",
		pq->priv->extopts ? pq->priv->extopts : "");
#if 0
	free((void *)driver);
#endif
	return(cmd);
}


static int initialisejob(struct pqinfo *pq)
{
	if(pq->priv->mode==PQMODE_FILE)
	{
		char *fn=NULL;
		aborted=0;
		pq->priv->cancelled=0;

		if(pq->priv->getfilecallback)
			fn=pq->priv->getfilecallback(pq->priv->userdata);

		if(!fn)
			return(0);

		pq->priv->outputfd=-1;
		if(fn)
			pq->priv->outputfd=open(fn,O_CREAT|O_RDWR,0644);
		free(fn);

		if(pq->priv->outputfd<0)
			return(0);
		return(1);
	}

	// FIXME - bail out if the customcommand isn't set.
	if(pq->priv->mode==PQMODE_CUSTOMCOMMAND)
	{
		if(pq->priv->customcommand)
			pq->priv->printcommand=strdup(pq->priv->customcommand);
		else
			return(0);
	}
	else
		pq->priv->printcommand=buildprintcommand(pq);
	
	signal(SIGPIPE,&sighandler);
	aborted=0;
	pq->priv->cancelled=0;
	// FIXME - alternate runs seem to result in the SIGPIPE signal being received
	// immediately.  Why?
	// Workaroud for now - clear aborted and cancelled flags in the InitialisePage function.
	// Race condition, though - not a permanent fix. 

	fprintf(stderr,"Print command: %s\n",pq->priv->printcommand);

	if(pipe(pq->priv->pipefd))
		return(0);
	pq->priv->outputfd=pq->priv->pipefd[1];

	pq->priv->childpid=fork();
	if(pq->priv->childpid==-1)
	{
//		fprintf(stderr,"PrinterQueue: Failed to create child process\n");
		return(0);
	}
	if(pq->priv->childpid==0)
	{
//		printf("Child process: %d\n",pq->priv->childpid);
		dup2(pq->priv->pipefd[0],0);
		close(pq->priv->pipefd[0]);
		close(pq->priv->pipefd[1]);
		execl("/bin/sh", "/bin/sh", "-c", pq->priv->printcommand, NULL);
		exit(0);
	}
	return(1);
}


static void initialisepage(struct pqinfo *pq)
{
	// FIXME - ugly workaround to the phantom SIGPIPE signals being received...
	aborted=0;
	pq->priv->cancelled=0;
}


static void endpage(struct pqinfo *pq)
{
}


static void endjob(struct pqinfo *pq)
{
	if(pq->priv->mode==PQMODE_FILE)
	{
		close(pq->priv->outputfd);
	}
	else
	{
//		fprintf(stderr,"In endjob()\n");
		if(pq->priv->cancelled)
		{
//			fprintf(stderr,"Job cancelled - sending term signal");
			// Might as well not do this, since it doesn't seem to work anyway.
			// FIXME - figure out the job ID and cancel it using lprm.
//			kill(pq->priv->childpid,SIGTERM);
		}
		close(pq->priv->pipefd[0]);
		close(pq->priv->pipefd[1]);
		free(pq->priv->printcommand);
		pq->priv->printcommand=NULL;
		int st;
		wait(&st);
	}
}


static void canceljob(struct pqinfo *pq)
{
	pq->priv->cancelled=1;
}


static int writedata(struct pqinfo *pq,const char *data,int bytecount)
{
//	fprintf(stderr,"%d\n",bytecount);
	int remain=bytecount;
	while(remain)
	{
		int written=write(pq->priv->outputfd,data,remain);
		if(written<0)
		{
			perror("error code");
			aborted=1;
		}
		data+=written;
		remain-=written;
		if(remain)
			usleep(500);
	}
	return(1-aborted);
}


struct pqinfo *pqinfo_create()
{
	struct pqinfo *pq=(struct pqinfo *)malloc(sizeof(struct pqinfo));
	if(pq)
	{
		pq->Dispose=dispose;
		pq->GetPrinterCount=getprintercount;
		pq->GetPrinterName=getprintername;
		pq->SetPrinterQueue=setprinterqueue;
		pq->GetPrinterQueue=getprinterqueue;
		pq->GetDriver=getdriver;
		pq->GetPPD=getppd;
		pq->SetDataType=setdatatype;
		pq->InitialiseJob=initialisejob;
		pq->InitialisePage=initialisepage;
		pq->EndPage=endpage;
		pq->EndJob=endjob;
		pq->CancelJob=canceljob;
		pq->WriteData=writedata;
		pq->SetCustomCommand=setcustomcommand;
		pq->GetCustomCommand=getcustomcommand;
		pq->SetExtendedOptions=setextopts;
		pq->SetGetFilenameCallback=setgetfilenamecallback;

		if(!(pq->priv=pqprivate_create()))
		{
			pq->Dispose(pq);
			return(NULL);
		}
	}
	return(pq);
}

