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

#include <magicserver/msrvdef.h>
#include <magicserver/msrverror.h>
#include <magicserver/msrvthread.h>

#include <sys/time.h>
#include <errno.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Creates a mutually exclusive (mutex) thread lock.
 *
 * The lock will be a recursive; it may be locked multiple times by the same
 * thread, after which it must be unlocked equal number of times.
 ******************************************************************************/
ThreadLock::ThreadLock ()
{
	/* Create and initialize mutex attributes. */
	pthread_mutexattr_t attribs;
	pthread_mutexattr_init (&attribs);
	pthread_mutexattr_settype (&attribs, PTHREAD_MUTEX_RECURSIVE_NP);

	/* Initialize the mutex. */
	pthread_mutex_init (&mThreadLock, &attribs);
	/* Question: is it ok to have the attributes temporary? */

	/* Conditional variables are not initialized in creation. */
	mCondInited = false;
}

/*******************************************************************************
 * Destroys a thread lock object.
 *
 * \note This must NOT be done when the thread is locked! Also, no
 * threads must be waiting in @ref wait().
 ******************************************************************************/
ThreadLock::~ThreadLock ()
{
	if (mCondInited) {
		pthread_cond_destroy (&mThreadCond);
	}

	/* Should we do something else here? Probably not. */
}

/*******************************************************************************
 * Locks the thread.
 *
 * Same thread may lock the thread lock multiple times. Unlocking it
 * requires exact number of corresponding \ref unlock() calls.
 ******************************************************************************/
void ThreadLock::lock	()
{
	pthread_mutex_lock (&mThreadLock);
}

/*******************************************************************************
 * Unlocks the thread.
 ******************************************************************************/
void ThreadLock::unlock	()
{
	pthread_mutex_unlock (&mThreadLock);
}

/*******************************************************************************
 * Starts waiting for a signal.
 *
 * The thread execution is suspended and does not consume any CPU time
 * until the condition variable is signaled.
 *
 * @return 0 if successful, otherwise a negative error code.
 * MSRVERR_TIMEOUT is returned on timeout.
 ******************************************************************************/
MSrvResult ThreadLock::wait (double seconds)
{
	int result = 0;

	/* We must enter lock before calling wait. */
	lock ();

	/* Initialize condition variable, if not yet initialized. */
	if (!mCondInited)
		initCond ();

	/* If no timeout. */
	if (seconds <= 0.0) {
		/* Wait indefinitely. */
		pthread_cond_wait (&mThreadCond, &mThreadLock);
	}
	else {
		/* Timed wait. */

		/* Get current time. */
		struct timeval  now;
		gettimeofday (&now, NULL);

		/* Determine timeout time. */
		struct timespec timeout;
		timeout.tv_sec  = now.tv_sec + long (seconds);
		timeout.tv_nsec = long ((now.tv_usec + (seconds - double (long (seconds))) * 1000000)*1000);

		/* Start waiting. */
		int tresult = pthread_cond_timedwait (&mThreadCond, &mThreadLock, &timeout);

		/* Timedout is the only error the timedwait can return. */
		if (tresult == ETIMEDOUT)
			result = MSRVERR_TIMEOUT;
	}

	unlock ();

	return result;
}

/*******************************************************************************
 * Restarts one of the threads that are waiting.
 *
 * If no threads are waiting with @ref wait(), nothing happens. If
 * several threads are waiting, exactly one is restarted, but it is
 * not specified which.
 ******************************************************************************/
MSrvResult ThreadLock::signal ()
{
	lock ();
	/* Initialize condition variable, if not yet initialized. */
	if (!mCondInited)
		initCond ();
	unlock ();

	pthread_cond_signal (&mThreadCond); /* Never returns error. */

	return 0;
}

/*******************************************************************************
 * Restarts all the threads that are waiting.
 *
 * Nothing happens if no threads are waiting with @ref wait().
 ******************************************************************************/
MSrvResult ThreadLock::broadcast ()
{
	lock ();
	/* Initialize condition variable, if not yet initialized. */
	if (!mCondInited)
		initCond ();
	unlock ();

	pthread_cond_broadcast (&mThreadCond); /* Never returns error. */

	return 0;
}

/*******************************************************************************
 * Initializes condition variable
 ******************************************************************************/
void ThreadLock::initCond ()
{
	if (!mCondInited) {
		pthread_condattr_t condattr; /* Condition variable attributes. */
		
		pthread_cond_init (&mThreadCond, &condattr); /* Never returns error. */
		
		mCondInited = true;
	}
}

/*******************************************************************************
 * Creates a thread object.
 *
 * To actually start the concurrent execution of a thread, call @ref
 * start().
 ******************************************************************************/
Thread::Thread	()
{
	mThreadId  = 0;
	mHasJoined = true; /* This is turned to false when the thread is started. */

	pthread_attr_init (&mThreadAttr);
}

/*******************************************************************************
 * Destroys the thread.
 *
 * The thread must be joined with @ref join() before destroying the
 * thread object.
 *
 * If the thread has been started with @ref start(), the caller
 * (or some other thread) must call call @ref join() to wait for
 * termination of the thread before the thread object has been called.
 *
 * Thread may not destroy itself from @ref execute().
 ******************************************************************************/
Thread::~Thread	()
{
	/* NOTE: Don't try to force joining here -- if the thread has not */
	/*       exited, joining here forcefully would hang it.           */
	//if (!mHasJoined)
	//		join (NULL);
	
	/* I'm not sure if this has to be stored this long. */
	pthread_attr_destroy (&mThreadAttr);
}

/*******************************************************************************
 * Starts the execution of the thread.
 *
 * The method returns immediately in the calling thread. Execution of
 * the thread starts in the @ref execute() method, which must be
 * reimplemented.
 *
 * \see execute(), join()
 *
 * @return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult Thread::start ()
{
	int result = pthread_create (&mThreadId, &mThreadAttr, Thread::entryPoint, this);

	if (result < 0)
		return MSRVERR_THREAD_CREATE_FAILED;

	mHasJoined = false;
	return 0;
}

/*******************************************************************************
 * \fn void* Thread::execute () = 0
 *
 * This method must be reimplemented by any inheritor to implements
 * the actual program executed in the thread. The method is called
 * after the @ref start() has been called in the creator thread.
 *
 * The execution of the thread stops when the method exits. The method
 * may return a void pointer as a return value. This return value will
 * be available to the thread calling @ref join() as the optional return
 * value parameter.
 ******************************************************************************/

/*******************************************************************************
 * Waits the thread to exit.
 *
 * Suspends the execution of the calling thread until the thread
 * associated with the Thread object terminates either by exiting or
 * by being cancelled.
 *
 * The thread to be joined must be in joinable state: it must not have
 * been detached (not implemented).
 *
 * When a thread terminates, its memory resources (thread descriptor
 * and stack) are not deallocated until another thread performs @ref
 * join() on it. Therefore, @ref join() must be called once for each
 * joinable thread created to avoid memory leaks.
 *
 * At most one thread can wait for the termination of a given
 * thread. Calling @ref join() on a thread on which another thread is
 * waiting for termination returns an error.
 *
 * The thread must be joined with this method before destroying the
 * thread object.
 *
 * @return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult Thread::join (
	void** ppReturnValue /**< Pointer to void* where to store exit value. May be NULL. */
	)
{
	mHasJoined = true; /* Set this true whatever happens. */

	int result = pthread_join (mThreadId, ppReturnValue);

	if (result)
		return MSRVERR_THREAD_JOIN_FAILED;

	return 0;
}

/*******************************************************************************
 *
 ******************************************************************************/
void* Thread::entryPoint (void* pParam)
{
	Thread* pThis = (Thread*) pParam;

	return pThis->execute ();
}

	
end_namespace (MSrv);
