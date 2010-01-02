#ifndef REFCOUNTUI_H
#define REFCOUNTUI_H

#include <gtk/gtkmain.h>

#include "../support/refcount.h"
#include "../support/thread.h"
#include "../support/debug.h"

// A UI-safe variant of the reference count class.
// It differs from the regular RefCount class in two ways:
//
// 1.  If the mutex is held by a subthread, instead of waiting, we spin the GTK event loop
//     which gives idle/timeout handlers a chance to run.  This is important because the subthread
//     that holds the mutex may be waiting for an idle/timeout handler to complete before releasing.
//
// 2.  If asked to unref the object from a thread other than the one that created it, a timeout handler
//     is invoked to do the actual unreferencing, thus ensuring that object deletion happens from
//     the main thread.
//

class RefCountUI : public RefCount
{
	public:
	RefCountUI() : RefCount()
	{
		threadid=Thread::GetThreadID();
	}
	virtual ~RefCountUI()
	{
		Debug[TRACE] << "In RefCountUI Destructor" << std::endl;
	}
	virtual void ObtainRefMutex()
	{
		if(threadid==Thread::GetThreadID())
		{
			if(!RefCount::refmutex.AttemptMutex())
			{
				Debug[TRACE] << "Pumping gtk_main_iteration until we can obtain the mutex" << std::endl;

				while(!RefCount::refmutex.AttemptMutex())
				{
					gtk_main_iteration_do(TRUE);
				}
			}
		}
		else
			RefCount::refmutex.ObtainMutex();
	}
	virtual void UnRef()
	{
		if(threadid==Thread::GetThreadID())
		{
			Debug[TRACE] << "RefCountUI: Referencing from same thread as creation - unreferencing directly..." << std::endl;
			RefCount::UnRef();
			Debug[TRACE] << "RefCountUI: UnReferenced" << std::endl;
		}
		else
		{
			Debug[TRACE] << "RefCountUI: Unreferencing from a different thread - deferring..." << std::endl;
			g_timeout_add(1,unreffunc,this);
		}
	}
	protected:
	static gboolean unreffunc(gpointer ud)
	{
		RefCountUI *rc=(RefCountUI *)ud;
		rc->RefCount::UnRef();
		return(FALSE);
	}
	
	ThreadID threadid;
};

#endif

