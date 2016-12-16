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

#ifndef __MAGICSERVER_MSRVREQUEST_H__
#define __MAGICSERVER_MSRVREQUEST_H__

#include <magicserver/msrvdef.h>
#include <magicserver/msrvserver.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Request from client.
 ******************************************************************************/
class Request {
  public:
	/** Type identifier of a request, also acts as a request mask. */
	enum requesttype {NewConnection  = 0x0001,
					  StreamData     = 0x0002,
					  Datagram       = 0x0004,
					  ConnectionLost = 0x0008,
					  Shutdown       = 0x0010,
					  Timeout        = 0x0020};

	virtual			~Request		() {}

	int				socket			() const {return mSocket;}
	ServerListener&	serverListener	() {return *mpServerListener;}
	int				getType			() const {return mRequestType;}

  protected:
					Request			(int socket, int requesttype, ServerListener& rListener);

  private:
	
	int				mRequestType;
	int				mSocket;			/**< Socket to read request data from. */
	ServerListener* mpServerListener;
};

/*******************************************************************************
 * An abstract request associated with a connection.
 ******************************************************************************/
class ConnectionRequest : virtual public Request {
  public:
	Connection&		connection				() {return *mrpConn;}

  protected:
					ConnectionRequest		(int socket,
											 int requesttype,
											 Connection& rConn,
											 ServerListener& rListener);

  protected:
	Connection*		mrpConn; /**< Reference to the originating client connection. */
};

/*******************************************************************************
 * A new client connection.
 ******************************************************************************/
class NewConnectionRequest : public ConnectionRequest {
  public:
	NewConnectionRequest	(int socket, Connection& rConn, ServerListener& rListener);
};

/*******************************************************************************
 * Data has arrived on socket.
 ******************************************************************************/
class DataRequest : virtual public Request {
  public:
	virtual			~DataRequest	() {delete mpData;}

	void			setData			(char* data, int len);
	virtual char*	getData			() const {return mpData;}
	virtual long	dataLen			() const {return mDataLen;}

  protected:
					DataRequest		(int socket,
									 int requesttype,
									 ServerListener& rListener);
  private:
	char*			mpData;
	int				mDataLen;
};

/*******************************************************************************
 * Data has arrived on a connection-based (TCP) socket.
 ******************************************************************************/
class StreamDataRequest : public DataRequest, public ConnectionRequest {
  public:
				StreamDataRequest		(int socket, Connection& rConn, ServerListener& rListener);
};

/*******************************************************************************
 * An UDP datagram has arrived on socket.
 ******************************************************************************/
class DatagramRequest : public DataRequest {
  public:
	DatagramRequest		(int socket, ServerListener& rListener);
};

/*******************************************************************************
 * Client connection has been lost.
 ******************************************************************************/
class ConnectionLostRequest : public ConnectionRequest {
  public:
				ConnectionLostRequest	(int socket, Connection& rConn, ServerListener& rListener);
				~ConnectionLostRequest	();
};

/*******************************************************************************
 * @ref Listener has requested shutdown.
 ******************************************************************************/
class ShutdownRequest : public Request {
  public:
	ShutdownRequest	(ServerListener& rListener) : Request (-1, Request::Shutdown, rListener) {;}
};

/*******************************************************************************
 * Timeout in @ref Listener.
 ******************************************************************************/
class TimeoutRequest : public Request {
  public:
	TimeoutRequest	(ServerListener& rListener) : Request (-1, Request::Timeout, rListener) {;}

  private:
};

/*******************************************************************************
 * Interface for handling client requests and other server notifications.
 *
 * Requests are generated by a ServerListener, when executing in @ref
 * Listener::listen().
 ******************************************************************************/
class RequestHandler {
  public:
	virtual ~RequestHandler		() {}
	virtual MSrvResult init		(ServerListener& rListener);
	virtual MSrvResult process 	(Request* pRequest);
	virtual MSrvResult process 	(NewConnectionRequest&  pRequest);
	virtual MSrvResult process 	(ConnectionLostRequest& pRequest);
	virtual MSrvResult process 	(StreamDataRequest&     pRequest);
	virtual MSrvResult process 	(DatagramRequest&       pRequest);
	virtual MSrvResult process 	(ShutdownRequest&       pRequest);
	virtual MSrvResult process 	(TimeoutRequest&        pRequest);
};

end_namespace (MSrv);

#endif
