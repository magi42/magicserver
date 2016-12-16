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

#ifndef __MAGICSERVER_MSRVSERVER_H__
#define __MAGICSERVER_MSRVSERVER_H__

#include <magicserver/msrvdef.h>
#include <magicserver/msrvlistener.h>

/*******************************************************************************
 * Predeclarations
 ******************************************************************************/
struct sockaddr_in;

namespace MSrv {
	class IRequest;
	class Request;
	class RequestHandler;
	class Log;
	class Connection;
	class ConnectionFactory;
};

begin_namespace (MSrv);

/*******************************************************************************
 * ServerListener object, capable of accepting connections and data.
 *
 * For TCP servers, ServerListener distinguishes between the server
 * socket that listens for incoming connections and established
 * connection sockets.
 *
 * For UDP servers, ServerListener listens the UDP server socket,
 * receives any data sent to it and forwards the @ref Request to a
 * @ref RequestHandler.
 *
 * \image html flowcharts-listener2.png
 ******************************************************************************/
class ServerListener : public Listener {
  public:
						ServerListener	(RequestHandler& rHandler, Log* rpLog=NULL);
	virtual	     		~ServerListener	();

	enum protocol_type {TCP=0, UDP=1};

	enum bindflags     {BINDF_NOREUSE=0x00000001};

	virtual MSrvResult  bind					(int portno, protocol_type protocol, uint flags);

	void				setHandler				(RequestHandler& handler) {mrpHandler = &handler;}
	void				setRequestMask			(uint mask) {mRequestMask = mask;}
	uint				requestMask				() const {return mRequestMask;}
	void				setConnectionFactory	(ConnectionFactory& factory) {mrpConnectionFactory = &factory;}

	MSrvResult			close					(Connection* pConn);

	/** Connection iterator */
	class ConnIter {
	  public:
					ConnIter	(ServerListener& server);
		Connection&	get			();
		void		next		();
		bool		exhausted	();
		
	  private:
		ServerListener&				rServer;
		Array<Descriptor>::Iterator mDescIter;
	};

  protected:
	RequestHandler*		getHandler		() {return mrpHandler;}
	virtual MSrvResult	descriptorEvent	(int fd, void* data);
	virtual MSrvResult	timeoutEvent	();
	virtual MSrvResult	shutdown		();

  private:
	virtual MSrvResult	accept			();

	int					mSocket;    /**< The server socket.           */
	int					mProtocol;  /**< Protocol, either TCP or UDP. */
	RequestHandler*		mrpHandler;
	ConnectionFactory*	mrpConnectionFactory;
	uint				mRequestMask;
};

/*******************************************************************************
 * Connection object
 ******************************************************************************/
class Connection {
  public:
						Connection	(int socket, const struct sockaddr_in& rAddr, Listener& pListener);
	virtual				~Connection ();

	int					socket		() const {return mSocket;}
	int					ipAddress	() const;
	const sockaddr_in&	address		() const {return *mpAddress;}
	virtual	MSrvResult	close		();
	ThreadLock&			threadLock	() {return mThreadLock;}
	
  private:
	Listener*		mrpListener;
	int				mSocket;
	sockaddr_in*	mpAddress;
	ThreadLock		mThreadLock;
};

/*******************************************************************************
 * Factory to create @ref Connection objects.
 *
 * This class can be inherited to allow @ref ServerListener to create
 * user-defined Connection objects. The factory must be set with @ref
 * ServerListener::setConnectionFactory().
 *
 * The inheritor must reimplement the virtual @ref create() method.
 ******************************************************************************/
class ConnectionFactory {
  public:
	virtual ~ConnectionFactory	() {}
	virtual Connection*	create	(int socket, const struct sockaddr_in& rAddr, Listener& pListener) = 0;
};

end_namespace (MSrv);

#endif

