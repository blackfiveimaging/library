#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <dirent.h>
#include <sys/stat.h>

#include "support/dirtreewalker.h"

using namespace std;


int main(int argc,char **argv)
{
	try
	{
		if(argc>1)
		{
			DirTreeWalker toplevel(argv[1]);
			DirTreeWalker *dir=toplevel.NextDirectory();
			while(dir)
			{
				vector<string> strlist;
				const char *file;

				while((file=dir->NextFile()))
					strlist.push_back(string(file));

				sort(strlist.begin(),strlist.end());

				vector<string>::const_iterator it;
				for(it=strlist.begin(); it!=strlist.end(); ++it)
					cerr << "File: " << *it << endl;

				dir=dir->NextDirectory();
			}

			cerr << endl << " -----   High level - files   ----- " << endl;
			DirTree_Files dtf(argv[1]);
			const char *fn;
			while((fn=dtf.Next()))
			{
				cerr << fn << endl;
			}

			cerr << endl << " -----   High level - directories   ----- " << endl;
			DirTree_Dirs dtd(argv[1]);
			while((fn=dtd.Next()))
			{
				cerr << fn << endl;
			}

		}
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}


#if 0
int main(int argc,char **argv)
{
	try
	{
		if(argc>1)
		{
			DirTreeWalker toplevel(argv[1]);
			DirTreeWalker *dir=toplevel.NextDirectory();
			while(dir)
			{
				const char *file;
				while((file=dir->NextFile()))
				{
					cerr << "Got file: " << file << endl;
				}
				dir=dir->NextDirectory();
			}
		}
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}
#endif

