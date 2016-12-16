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

#include <magicserver/msrvrequest.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Creates a request
 ******************************************************************************/
Request::Request (
	int             socket,    /**< Socket from which the request originated. */
	int             reqt,      /**< Request type.                             */
	ServerListener& rListener  /**< Listener that originated the request.     */)
{
	mSocket          = socket;
	mRequestType     = reqt;
	mpServerListener = &rListener;
}

/*******************************************************************************
 * \fn Request::~Request ()
 *
 * Destroys the request.
 ******************************************************************************/

/*******************************************************************************
 * \fn int Request::socket () const
 *
 * Returns the socket associated with the request.
 *
 * The semantics of the socket depend on the exact class. It can be
 * either a listening server socket or a client connection socket.
 ******************************************************************************/

/*******************************************************************************
 * \fn ServerListener& Request::serverListener ()
 *
 * Returns a reference to the @ref ServerListener that originated the request.
 ******************************************************************************/

/*******************************************************************************
 * \fn int Request::getType () const
 *
 * Returns the numeric request type identifier associated with the class.
 *
 * This identifier is an easy and quick way to determine the identity
 * of a request, without needing to use slow and laborius dynamic_cast.
 ******************************************************************************/

/*******************************************************************************
 * Constructor for a connection request.
 ******************************************************************************/
ConnectionRequest::ConnectionRequest (
	int             socket,    /**< Socket from which the request originated. */
	int             requesttype,
	Connection&     rConn,    /**< Connection associated with the socket. */
	ServerListener& rListener /**< Listener that manages the socket.      */)
		: Request (socket, requesttype, rListener),
		  mrpConn (&rConn)
{
}

/*******************************************************************************
 * \fn Connection& ConnectionRequest::connection ()
 *
 * Returns a reference to the client connection that originated the
 * request.
 ******************************************************************************/

/*******************************************************************************
 * Constructor for a new connection request.
 ******************************************************************************/
NewConnectionRequest::NewConnectionRequest (
	int             socket,    /**< Socket from which the request originated. */
	Connection&     rConn,     /**< Connection associated with the socket. */
	ServerListener& rListener)
		:  Request (socket, Request::NewConnection, rListener),
		   ConnectionRequest (socket, Request::NewConnection, rConn, rListener)
{
}

/*******************************************************************************
 * Constructor for a data request.
 ******************************************************************************/
DataRequest::DataRequest (
	int             socket,      /**< Socket from which the request originated. */
	int             requesttype,
	ServerListener& rListener)
		: Request (socket, requesttype, rListener)
{
	mpData   = 0;
	mDataLen = 0;
}

/*******************************************************************************
 * \fn DataRequest::~DataRequest ()
 *
 * Destroys the request and the associated data buffer.
 ******************************************************************************/

/*******************************************************************************
 * Sets the data of the request.
 *
 * \note The request object takes ownership of the data buffer.
 ******************************************************************************/
void DataRequest::setData (
	char* pData,
	int   len)
{
	delete mpData;
	mpData   = pData;
	mDataLen = len;
}

/*******************************************************************************
 * Constructor for a stream data request.
 ******************************************************************************/
StreamDataRequest::StreamDataRequest (
	int             socket,   /**< Socket the data arrived from.          */
	Connection&     rConn,    /**< Connection associated with the socket. */
	ServerListener& rListener /**< Listener that manages the socket.      */)
		: Request (socket, Request::StreamData, rListener),
		  DataRequest (socket, Request::StreamData, rListener),
		  ConnectionRequest (socket, Request::StreamData, rConn, rListener)
{
}

/*******************************************************************************
 * Constructor for a datagram request.
 ******************************************************************************/
DatagramRequest::DatagramRequest (
	int             socket,   /**< Socket the data arrived from.          */
	ServerListener& rListener /**< Listener that manages the socket.      */)
		:  Request (socket, Request::Datagram, rListener),
		   DataRequest (socket, Request::Datagram, rListener)
{
}

/*******************************************************************************
 * Constructor for a connection lost request.
 ******************************************************************************/
ConnectionLostRequest::ConnectionLostRequest (
	int             socket,    /**< Socket from which the request originated. */
	Connection&     rConn,
	ServerListener& rListener /**< Listener that manages the socket.      */)
		:  Request (socket, Request::ConnectionLost, rListener),
		   ConnectionRequest (socket, Request::ConnectionLost, rConn, rListener)
{
}

/*******************************************************************************
 * Destructor for a connection lost request.
 *
 * This will finally destroy the associated Connection object.
 ******************************************************************************/
ConnectionLostRequest::~ConnectionLostRequest ()
{
	/* The pointer is not a reference in this case, but we are really */
	/* allowed to destroy the object.                                 */
	delete mrpConn;
}

/*******************************************************************************
 * \fn char* DataRequest::getData () const = 0
 *
 * Returns data buffer containing the request data.
 ******************************************************************************/

/*******************************************************************************
 * \fn long DataRequest::dataLen () const = 0
 *
 * Returns length of the data buffer.
 ******************************************************************************/

/*******************************************************************************
 * Initializes the request handler with a listener.
 *
 * A request handler can implement this method to initialize itself or
 * to configure given ServerListener.
 *
 * You can, for example, the request mask here with 
 * @ref ServerListener::setRequestMask().
 ******************************************************************************/
MSrvResult RequestHandler::init (ServerListener& rListener)
{
	return 0;
}

/*******************************************************************************
 * Handles a client request.
 *
 * The default implementation of this method acts as a switchboard to
 * call a request type specific handler.
 *
 * The user-defined request handler can reimplement either this or
 * some of the already separated methods. If this generic handler
 * method is reimplemented, the message-specific handler methods will
 * not be called.
 *
 * Reimplementing this method is useful for building chained request
 * handlers, such as @ref WorkerPool. Using the default switchboard
 * solution is also slightly slower (extra method invocation, casting).
 *
 * A reimplementation has the responsibility of destroying the request
 * object after processing.
 *
 * @return Should return 0 if successful, otherwise a negative error code.
 *
 * @param pRequest Request object.
 ******************************************************************************/
MSrvResult RequestHandler::process (Request* pRequest)
{
	MSrvResult result = 0;
	
	switch (pRequest->getType ()) {

	  case Request::NewConnection: {
		  result = process (*dynamic_cast<NewConnectionRequest*> (pRequest));
	  } break;
		
	  case Request::StreamData: {
		  result = process (*dynamic_cast<StreamDataRequest*> (pRequest));
	  } break;

	  case Request::Datagram: {
		  result = process (*dynamic_cast<DatagramRequest*> (pRequest));
	  } break;
	  
	  case Request::ConnectionLost: {
		  result = process (*dynamic_cast<ConnectionLostRequest*> (pRequest));
	  } break;

	  case Request::Shutdown: {
		  result = process (*dynamic_cast<ShutdownRequest*> (pRequest));
	  } break;
		
	  case Request::Timeout: {
		  result = process (*dynamic_cast<TimeoutRequest*> (pRequest));
	  } break;
		
	  default: {
		  pRequest->serverListener().log().message ("REQUEST", Log::Warning, 0,
													"Unhandled request type '%d'.",
													pRequest->getType ());
	  }
	}

	/* It is our responsibility to destroy the Request object. */
	delete pRequest;

	return result;
}

/*******************************************************************************
 * Notifies of a new client connection.
 *
 * @return Should return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult RequestHandler::process (NewConnectionRequest&  pRequest)
{
	return 0;
}

/*******************************************************************************
 * Notifies of a lost client connection.
 *
 * @return Should return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult RequestHandler::process (ConnectionLostRequest&  pRequest)
{
	return 0;
}

/*******************************************************************************
 * Notifies of an arrived stream data request.
 *
 * @return Should return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult RequestHandler::process (StreamDataRequest&  pRequest)
{
	return 0;
}
/*******************************************************************************
 * Notifies of an arrived datagram request.
 *
 * @return Should return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult RequestHandler::process (DatagramRequest&  pRequest)
{
	return 0;
}
/*******************************************************************************
 * Notifies of shutdown.
 *
 * @return Should return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult RequestHandler::process (ShutdownRequest&  pRequest)
{
	return 0;
}

/*******************************************************************************
 * Notifies of a timeout event on ServerListener.
 *
 * @return Should return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult RequestHandler::process (TimeoutRequest&  pRequest)
{
	return 0;
}

end_namespace (MSrv);
