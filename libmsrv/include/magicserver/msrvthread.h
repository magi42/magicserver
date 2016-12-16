/***************************************************************************
 *   This file is part of the MagiCServer++ library.                       *
 *                                                                         *
 *   Copyright (C) 2003 Marko Grönroos <magi@iki.fi>                       *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Library General Public            *
 *  License as published by the Free Software Foundation; either           *
 *  version 2 of the License, or (at your option) any later version.       *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Library General Public License for more details.                       *
 *                                                                         *
 *  You should have received a copy of the GNU Library General Public      *
 *  License along with this library; see the file COPYING.LIB.  If         *
 *  not, write to the Free Software Foundation, Inc., 59 Temple Place      *
 *  - Suite 330, Boston, MA 02111-1307, USA.                               *
 *                                                                         *
 ***************************************************************************/

#ifndef __MAGICSERVER_MSRVTHREAD_H__
#define __MAGICSERVER_MSRVTHREAD_H__

/* This definition is needed to enable pthread_mutexattr_settype in pthread.h */
#define _XOPEN_SOURCE 600

#include <pthread.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Thread lock
 *
 * This is a trivial thread lock wrapper. Also condition variables are
 * supported.
 ******************************************************************************/
class ThreadLock {
  public:
	ThreadLock ();
	~ThreadLock ();

	void		lock		();
	void		unlock		();

	MSrvResult	wait		(double seconds=0.0);
	MSrvResult	signal		();
	MSrvResult	broadcast	();
	
  private:
	void	initCond	();
	
	pthread_mutex_t	mThreadLock; /**< Thread lock (mutex)                        */
	pthread_cond_t	mThreadCond; /**< Conditional variable for signalling.       */
	bool            mCondInited; /**< Has condition been initialized?            */
};

/*******************************************************************************
 * Thread object.
 *
 * A thread is a single sequential flow of control within a program.
 * Threads are somewhat like processes, except that they run in same
 * context and share their system resources (such as memory). On
 * multi-processor systems, threads can execute simultaneously on
 * different processors while accessing same memory.
 *
 * This Thread class is used by inheriting it and reimplementing the
 * @ref execute() method with application-specific functionality.
 *
 * To actually start the concurrent execution of a thread, call @ref
 * start(). When @ref start() is called, the method returns
 * immediately in the calling thread, but the new thread will starting
 * executing in the @ref execute() method concurrently.
 *
 * To wait for termination of a thread, call @ref join(). This must
 * always be done, either in the thread that originally started the
 * thread or in some other thread.
 *
 * To guard data from simultaneous access by multiple threads, you
 * need to use thread locks from the @ref ThreadLock class.
 *
 * \see ThreadLock
 ******************************************************************************/
class Thread {
  public:
						Thread		();
	virtual				~Thread		();

	MSrvResult			start		();
	MSrvResult			join		(void**);

	virtual void*		execute		() = 0;

  protected:
	inline int			threadId	() const {return mThreadId;}
	
  private:
	static void*		entryPoint	(void*);

	pthread_t			mThreadId;
	pthread_attr_t		mThreadAttr;
	bool                mHasJoined;
};

end_namespace (MSrv);

#endif
