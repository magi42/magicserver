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

#ifndef __MAGICSERVER_MSRVWORKER_H__
#define __MAGICSERVER_MSRVWORKER_H__

#include <magicserver/msrvdef.h>
#include <magicserver/msrvthread.h>
#include <magicserver/msrvserver.h>
#include <magicserver/msrvrequest.h>
#include <magicserver/msrvcontainer.h>

begin_namespace (MSrv);

class Worker;

/*******************************************************************************
 * Request dispenser that passes requests to @ref Worker threads to handle.
 *
 * Maintains a pool of @ref Worker objects that each sit in their own
 * thread, waiting for requests to process.
 ******************************************************************************/
class WorkerPool : public RequestHandler {
  public:
						WorkerPool		(RequestHandler& handler, Log& log, int size=10);
	virtual				~WorkerPool		();

	virtual MSrvResult	process		 	(Request* pRequest);

	bool				isShutdown		() const {return mIsShutdown;}

  private:
	MSrvResult			shutdown		(Request* pRequest);
	RequestHandler&		handler			() {return *mrpHandler;}
	ThreadLock&			getWaitLock		() {return mQueueWaitLock;}
	Queue<Request>&		queue			() {return mRequestQueue;}

	friend class Worker;

  private:
	RequestHandler*		mrpHandler;     /**< Handler of worker requests.         */
	Worker**			mpWorkers;      /**< Pool of workers.                    */
	int      			mWorkerCount;   /**< Number of workers in the pool.      */
	Queue<Request>		mRequestQueue;  /**< Requests dispensed to workers.      */
	ThreadLock          mQueueLock;     /**< For locking the request queue.      */
	ThreadLock			mQueueWaitLock; /**< Lock for waiting the queue.         */
	bool                mIsShutdown;    /**< Is the worker pool being shut down? */
	Log&				mrLog;
};

/*******************************************************************************
 * Worker thread that processes requests dispensed by a @ref WorkerPool.
 *
 * Calls a user-defined @ref RequestHandler for actually processing
 * the requests.
 ******************************************************************************/
class Worker : public Thread {
  public:
					Worker (WorkerPool* pool);

	virtual void*	execute		();

  private:
	static void*	startThread	(void* pParam);

	WorkerPool*		mpPool; /**< Owner pool. */
};

end_namespace (MSrv);

#endif

