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

#ifndef __MAGICSERVER_MSRVLISTENER_H__
#define __MAGICSERVER_MSRVLISTENER_H__

#include <magicserver/msrvdef.h>
#include <magicserver/msrvlog.h>
#include <magicserver/msrvthread.h>
#include <magicserver/msrvcontainer.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Stores a descriptor (socket handle) and associated data.
 ******************************************************************************/
struct Descriptor {
  public:
	Descriptor (int fd, void* pData) : mFd (fd), mpData (pData) {;}
	
	int   mFd;     /**< Descriptor.                          */
	void* mpData;  /**< Data associated with the descriptor. */
};

/*******************************************************************************
 * Notifies of status changes on a set of descriptors.
 *
 * Listener has many uses, as the descriptors can be server sockets
 * waiting for new clients, client sockets waiting for requests, or
 * even file destriptors waiting for new data on a file.
 *
 * Most obvious use is for storing TCP connection sockets on a
 * server. You can actually have multiple Listener instances running
 * concurrently, in different threads. This allows you to allocate a
 * pool of listener threads for different tasks. For example, one
 * thread might just wait for new client connections, while others
 * might handle existing connections. They could even move descriptors
 * between them for load balancing.
 *
 * However, most typical use for the Listener is to have a server
 * socket, which is listened for new TCP connections, and numerous
 * connected sockets, which are listened for data. This and UDP server
 * sockets are managed with \ref ServerListener class.
 *
 * The Listener can also generate timeout events, which can be set
 * with @ref setTimeout().
 *
 * Listener can be associated with a @ref Log, to which it writes
 * various messages. The log must be associated immediately after
 * creation of the Listener with @ref setLog().
 ******************************************************************************/
class Listener {
  public:
						Listener			(Log* rpLog=NULL);
	virtual				~Listener			();

	virtual MSrvResult	listen				();

	void				startShutdown		();
	bool				isShutdown			() const {return mShutdownStatus;}
	void				setTimeout			(long seconds, long microseconds);
	void				timeoutLeft			(long& seconds, long& microseconds) const;
	void				setLog				(Log& rLog) {mrpLog = &rLog;}
	Log&				log					() {return *mrpLog;}
	MSrvResult			removeDescriptor	(int fd);
	
  protected:
	virtual MSrvResult	descriptorEvent		(int fd, void* data);
	virtual MSrvResult	timeoutEvent		();
	virtual MSrvResult	shutdown			();

	ThreadLock			mThreadLock;		/**< Thread lock of the Listener object. */
	Array<Descriptor>	mDescriptors;		/**< Array of descriptors listened.      */

  private:
	long				mTimeoutSec;		/**< Timeout in seconds.                 */
	long				mTimeoutUSec;		/**< Timeout in microseconds.            */
	bool				mShutdownStatus;    /**< Is the server in shutdown state?    */
	Log*				mrpLog;             /**< Log to write messages.              */
};

end_namespace (MSrv);

#endif

