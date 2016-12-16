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

#include <magicserver/msrvworker.h>
#include <magicserver/msrverror.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Creates a worker pool of given size.
 ******************************************************************************/
WorkerPool::WorkerPool (RequestHandler& rHandler, Log& log, int size)
		: mrLog (log)
{
	mrpHandler       = &rHandler;
	mIsShutdown      = false;
	mWorkerCount     = size;
	mpWorkers        = new Worker* [size];

	/* Create the workers. */
	for (int i=0; i<mWorkerCount; ++i) {
		mpWorkers[i] = new Worker (this);
		mpWorkers[i]->start ();
	}

	mrLog.message ("WORKER", Log::Info, 0,
				   "Started %d worker threads successfully.",
				   mWorkerCount);

	/* Nothing may be initialized after this, as the workers may */
	/* access the object. */
}

/*******************************************************************************
 * Destroys worker pool.
 *
 * Shuts down all the workers violently if they not already
 * shut down.
 ******************************************************************************/
WorkerPool::~WorkerPool ()
{
	/* Ensure that the children have been shutdown before destroying them.  */
	/* This may happen if the workerpool was not properly shut down by the  */
	/* Listener by sending it a Shutdown request.                           */
	if (!mIsShutdown)
		shutdown (NULL);
	
	/* Destroy the workers. */
	for (int i=0; i<mWorkerCount; ++i)
		delete mpWorkers[i];
	
	delete mpWorkers;
}

/*******************************************************************************
 * Process a request
 ******************************************************************************/
MSrvResult WorkerPool::process (Request* pRequest)
{
	switch (pRequest->getType ()) {
	  case Request::Shutdown:
		  return shutdown (pRequest);
		  break;

	  default:
		/* Put the request in queue. */
		mRequestQueue.push (pRequest);
		
		/* Awaken one worker. */
		getWaitLock().signal ();
	}
	
	return 0;
}

/*******************************************************************************
 * Orders all worker threads to shut down.
 *
 * After shutting down the request handler threads, the optional final
 * request will be passed to the request handler and processed,
 * exceptionally in the main thread.
 ******************************************************************************/
MSrvResult WorkerPool::shutdown (Request* pRequest /**< Final request. May be NULL. */)
{
	if (mIsShutdown)
		return MSRVERR_REPEAT_SHUTDOWN_REQUEST;
	
	/* Go to shutdown state. */
	mIsShutdown = true;

	mrLog.message ("WORKER", Log::Info, 0,
				   "Shutting down worker threads...");

	/* Awaken all workers. */
	getWaitLock().broadcast ();

	/* Join all the workers except one. */
	for (int i=1; i<mWorkerCount; ++i)
		mpWorkers[i]->join (NULL);

	mrLog.message ("WORKER", Log::Info, 0,
				   "All %d worker threads joined successfully.",
				   mWorkerCount);

	mrLog.message ("WORKER", Log::Info, 0,
				   "Handling shutdown request in main thread...");

	/* Send the shutdown notification. */

	/* It would have been nicer to send it to one of the threads,  */
	/* as we might in future have separate handler in each thread. */
	/* This has to be reconsidered later.                          */
	if (pRequest)
		mrpHandler->process (pRequest);

	return 0;
}

/*******************************************************************************
 * Creates a worker thread.
 *
 * The worker thread must be started with @ref start().
 ******************************************************************************/
Worker::Worker (WorkerPool* pPool)
{
	mpPool = pPool;
}

/*******************************************************************************
 * Executes a worker thread.
 *
 * The execution is continued until the WorkerPool goes to shutdown
 * state. The request queue will be processed before exiting.
 ******************************************************************************/
void* Worker::execute ()
{
	while (1) {
		if (mpPool->isShutdown ())
			break;

		/* Wait for a signal from the worker pool, indicating either */
		/* that there is a new request to process, or the server is  */
		/* shutting down.                                            */
		mpPool->getWaitLock ().wait ();

		/* Process requests until queue is empty.                   */

		/* Pull the topmost request from the request queue. */
		while (Request* pRequest = mpPool->queue ().pull ()) {
			/* NOTICE that as the shutdown flag is not checked here,    */
			/* we WILL empty the queue before letting server shut down. */
			/* I guess we could have an "immediate" flag to leave       */
			/* the queue unclean.                                       */

			/* Invoke the request handler to handle the request. */
			if (pRequest)
				mpPool->handler ().process (pRequest);
		}
	}

	return NULL;
}

end_namespace (MSrv);
