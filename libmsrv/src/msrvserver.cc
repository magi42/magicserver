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

#include <magicserver/msrvserver.h>
#include <magicserver/msrvrequest.h>
#include <magicserver/msrverror.h>
#include <magicserver/msrvlog.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Default constructor.
 *
 * \note Newly created ServerListener does not have an associated log. You
 *       need to attach it to a log before operating with it.
 ******************************************************************************/
ServerListener::ServerListener (
	RequestHandler& rHandler,
	Log*            rpLog)
		: Listener (rpLog)
{
	mSocket              = 0;
	mProtocol            = TCP;
	mrpHandler           = &rHandler;
	mrpConnectionFactory = NULL;
	mRequestMask         = Request::NewConnection | Request::StreamData |
                           Request::Datagram | Request::ConnectionLost |
		                   Request::Shutdown;

	/* Ignore signals when a host closes connection unexpectedly. */
	signal (SIGPIPE, SIG_IGN);

	/* Initialize request handler.               */
	/* It can make some settings on this object. */
	mrpHandler->init (*this);
}

/*******************************************************************************
 * Destructor
 ******************************************************************************/
ServerListener::~ServerListener ()
{
}

/*******************************************************************************
 * Initializes the server for listening a socket
 ******************************************************************************/
MSrvResult ServerListener::bind (
	int           portno,
	protocol_type protocol,
	uint          flags)
{
	int result   = 0;
	int socktype = 0;

	/* Translate the protocol type. */
	if (protocol == ServerListener::TCP)
		socktype = SOCK_STREAM;
	else if (protocol == ServerListener::UDP)
		socktype = SOCK_DGRAM;

	/* Create the server listening socket. */
	int sockfd = socket (PF_INET, socktype, 0);
	if (sockfd <= 0) {
		log().message ("SERVER", Log::Critical, MSRVERR_SOCKET_FAILED,
						"Creating socket failed with error %d; %s.",
						errno, strerror (errno));
		return MSRVERR_SOCKET_FAILED;
	}

	/* Set socket options. */
	int reuseaddr = ! (flags & BINDF_NOREUSE);
	if (reuseaddr) {
		result = setsockopt (sockfd,
							 SOL_SOCKET,
							 SO_REUSEADDR,
							 (void*) &reuseaddr,
							 sizeof (reuseaddr));
		if (result < 0) {
			log().message ("SERVER", Log::Critical, MSRVERR_SET_SOCKET_OPTIONS_FAILED,
						   "Setting socket options failed with error %d; %s.",
						   errno, strerror (errno));
			::close (sockfd);
			return MSRVERR_SET_SOCKET_OPTIONS_FAILED;
		}
	}
	
	log().message ("SERVER", Log::Info, 0,
					"Binding to %s port %d...",
					(protocol==ServerListener::TCP)? "TCP":"UDP",
					portno);

	/* Define IP address to bind to. */
	struct sockaddr_in myaddr;
	myaddr.sin_family      = AF_INET;
	myaddr.sin_port        = htons (portno);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	
	/* Bind to the address. */
	result = ::bind (sockfd, (sockaddr*) &myaddr, sizeof (myaddr));
	if (result < 0) {
		log().message ("SERVER", Log::Critical, MSRVERR_BIND_FAILED,
						"Bind failed with error %d; %s",
						errno, strerror (errno));
		::close (sockfd);
		return MSRVERR_BIND_FAILED;
	}

	/* On TCP ports, set the listening queue length. */
	if (protocol == ServerListener::TCP) {
		result = ::listen (sockfd, 10);
		if (result) {
			log().message ("SERVER", Log::Critical, MSRVERR_LISTEN_FAILED,
						   "Listen failed with error %d; %s",
							errno, strerror (errno));
			::close (sockfd);
			return MSRVERR_LISTEN_FAILED;
		}
	}

	/* Store the server socket. */
	mThreadLock.lock ();
	mSocket   = sockfd;
	mProtocol = protocol;
	mDescriptors.add (new Descriptor (mSocket, NULL));
	mThreadLock.unlock ();
	
	return 0;
}

/*******************************************************************************
 * \fn void ServerListener::setRequestMask (uint mask)
 *
 * Sets request mask that controls which requests will be generated
 * and sent to the request handler.
 *
 * By default all other requests are enabled, except Request::Timeout.
 *
 * See @ref Request for the list of request types.
 ******************************************************************************/

/*******************************************************************************
 * \fn uint ServerListener::requestMask () const
 *
 * Retrieves the request mask that controls which requests will be
 * generated and sent to the request handler.
 *
 * By default all other requests are enabled, except Request::Timeout.
 *
 * See @ref Request for the list of request types.
 ******************************************************************************/

/*******************************************************************************
 * Starts listening a TCP server socket.
 ******************************************************************************/
MSrvResult ServerListener::accept ()
{
	struct sockaddr_in clientAddr;
	int                clientAddrLen = sizeof (clientAddr);

	memset (&clientAddr, 0, sizeof (clientAddr));

	/* Accept a connection. */
	int clientsocket = ::accept (mSocket,
								 (sockaddr*) &clientAddr,
								 (socklen_t*) &clientAddrLen);
	if (clientsocket < 0) {
		log().message ("SERVER", Log::Critical, MSRVERR_ACCEPT_FAILED,
					   "Accept failed with error %d; %s.",
					   errno, strerror (errno));
		return MSRVERR_ACCEPT_FAILED;
	}
	
	log().message ("SERVER", Log::Info, 0,
				   "Accepted connection from %d.%d.%d.%d",
				   ((clientAddr.sin_addr.s_addr) & 0xff),
				   ((clientAddr.sin_addr.s_addr >> 8) & 0xff),
				   ((clientAddr.sin_addr.s_addr >> 16) & 0xff),
				   ((clientAddr.sin_addr.s_addr >> 24) & 0xff));

	/* Create new connection object, with a factory, if available */
	Connection* pNewConn = NULL;
	if (mrpConnectionFactory)
		pNewConn = mrpConnectionFactory->create (clientsocket, clientAddr, *this);
	else
		pNewConn = new Connection (clientsocket, clientAddr, *this);

	/* Start listening to the client socket. */
	mDescriptors.add (new Descriptor (clientsocket, pNewConn));

	/* Tell the request handler about the new connection. */
	if (mRequestMask & Request::NewConnection) {
		Request* pRequest = new NewConnectionRequest (clientsocket,
													  *pNewConn,
													  *this);
		getHandler()->process (pRequest);
	}

	return 0;
}

/*******************************************************************************
 * Handles an event on a descriptor (socket).
 *
 * The meaning of the event depends on the socket type.
 *
 * Generates a request and calls request handler with
 * RequestHandler::process().
 ******************************************************************************/
MSrvResult ServerListener::descriptorEvent (
	int   fd,              /**< Descriptor.                                   */
	void* pDescriptorData) /**< Ptr to data associated with the descriptor.   */
{
	char buffer[MSRV_READ_BUFFER_LEN];

	if (fd == mSocket && mProtocol == TCP) {
		/* It's the TCP server socket; accept a new connection. */
		accept ();

	} else {
		/* Data has become data available in the socket.               */
		/* The socket can be a TCP client socket or UDP server socket. */

		/* Read all data available from the socket.           */
		char* dynbuffer = NULL;
		int   dynpos    = 0;
		int   readcount = 0;
		do {
			/* Read a block of data from the socket. */
			readcount = ::read (fd, buffer, MSRV_READ_BUFFER_LEN);

			if (readcount < 0) {
				/* Error. */
				log().message ("SERVER", Log::Warning, MSRVERR_READ_FAILED,
							   "Read failed with error %d; %s.",
							   errno, strerror (errno));
				break;
				
			} else if (readcount > 0) {
				/* Allocate enough space in the target buffer. */
				if (dynbuffer)
					dynbuffer = (char*) realloc (dynbuffer, dynpos + readcount);
				else
					dynbuffer = (char*) malloc (readcount);

				/* Copy the new block to the target buffer. */
				memcpy (dynbuffer + dynpos, buffer, readcount);
				
				dynpos += readcount;
			} else {
				/* No (more) data. */
				break;
			}

			/* Terminate when there is no more data available. */
		} while (readcount == MSRV_READ_BUFFER_LEN);

		if (dynpos > 0) {
			/* Put the data into a request object. */
			DataRequest* pRequest = NULL;
			if (mProtocol == TCP)
				if (mRequestMask & Request::StreamData)
					pRequest = new StreamDataRequest (fd,
													  *static_cast <Connection*> (pDescriptorData),
													  *this);
				else
					pRequest = NULL;
			else
				if (mRequestMask & Request::Datagram)
					pRequest = new DatagramRequest (fd, *this);
				else
					pRequest = NULL;

			if (pRequest) {
				pRequest->setData (dynbuffer, dynpos);
				
				/* Send the request to handler. */
				getHandler()->process (pRequest);

				/* The handler must have destroyed the request object. */
			}

		} else if (readcount == 0 && dynpos == 0) {
			/* No data was available from the socket.     */
			/* This must imply that the socket is closed. */
			
			log().message ("SERVER", Log::Info, 0,
						   "Connection lost. Closing the connection.");


			/* Send a ConnectionLost request to handler. */
			if (mRequestMask & Request::ConnectionLost) {
				Connection* pConn = static_cast <Connection*> (pDescriptorData);

				Request* pRequest = new ConnectionLostRequest (fd,
															   *pConn,
															   *this);
				getHandler()->process (pRequest);
			}

			/* Remove the descriptor from Listener. */
			removeDescriptor (fd);

			/* Note:                                                        */
			/* The associated Connection object will be removed from the    */
			/* Listener and  destroyed by the desctructor of the Request.   */
			/* This is because we can't destroy it here because the Request */
			/* may need the Connection object.                              */
			/* See Connection::close for notes.                             */
		}
	}

	return 0;
}

/*******************************************************************************
 * Handle Listener timeout event
 ******************************************************************************/
MSrvResult ServerListener::timeoutEvent ()
{
	MSrvResult result = 0;
	
	if (mRequestMask & Request::Timeout) {
		Request* pRequest = new TimeoutRequest (*this);
		result = getHandler()->process (pRequest);
	}

	return result;
}

/*******************************************************************************
 * Performs shutdown of the ServerListener.
 *
 * Sends a @ref ShutdownRequest to associated @ref RequestHandler.
 ******************************************************************************/
MSrvResult ServerListener::shutdown ()
{
	int result = 0;
	
	/* Inform the user application about the shutdown. */
	if (mRequestMask & Request::Shutdown)
		result = getHandler()->process (new ShutdownRequest (*this));

	/* Close all client sockets still open. */
	for (ConnIter conn_i (*this); !conn_i.exhausted (); conn_i.next())
		close (&conn_i.get());

	/* Close server socket. */
	::close (mSocket);

	/* Remove it from the Listener. */
	removeDescriptor (mSocket);

	return result;
}

/*******************************************************************************
 * Closes a client connection
 *
 * The Connection object will be destroyed during this call and will
 * be invalid afterwards.
 *
 * @return 0 if successful, otherwise an error code.
 ******************************************************************************/
MSrvResult ServerListener::close (Connection* pConn) {
	if (!pConn)
		return MSRVERR_NULL_ARGUMENT;

	mThreadLock.lock ();

	pConn->close ();

	mThreadLock.lock ();

	return 0;
}

/*******************************************************************************
 * Constructor for connection iterator.
 ******************************************************************************/
ServerListener::ConnIter::ConnIter (ServerListener& server)
		: rServer (server), mDescIter (server.mDescriptors)
{
	while (!mDescIter.exhausted() && mDescIter.get().mFd == rServer.mSocket)
		mDescIter.next ();
}

/*******************************************************************************
 * Retrieves current object.
 ******************************************************************************/
Connection&	ServerListener::ConnIter::get ()
{
	return *static_cast <Connection*> (mDescIter.get ().mpData);
}

/*******************************************************************************
 * Retrieves current object.
 ******************************************************************************/
void ServerListener::ConnIter::next		()
{
	do {
		mDescIter.next();
	} while (!mDescIter.exhausted() && mDescIter.get().mFd == rServer.mSocket);
}

/*******************************************************************************
 * Is iterator past last item?
 ******************************************************************************/
bool ServerListener::ConnIter::exhausted ()
{
	return mDescIter.exhausted();
}

/*******************************************************************************
 * Creates a TCP connection data object.
 *
 * Inheriting this object is useful for attaching application-specific
 * data to connections.
 ******************************************************************************/
Connection::Connection (
	int                       socket,   /**< Client socket.                   */
	const struct sockaddr_in& rAddr,    /**< Client address.                  */
	Listener&                 pListener /** < Listener the connection belongs.*/)
{
	mSocket     = socket;
	mrpListener = &pListener;

	mpAddress = (sockaddr_in*) malloc (sizeof (sockaddr_in));
	memcpy (mpAddress, &rAddr, sizeof (sockaddr_in));
}

/*******************************************************************************
 * Destroys and closes the connection
 ******************************************************************************/
Connection::~Connection ()
{
	close ();

	if (mpAddress)
		free (mpAddress);
}

/*******************************************************************************
 * \fn int Connection::socket () const
 *
 * Returns the socket descriptor associated with the connection.
 ******************************************************************************/

/*******************************************************************************
 * Returns the IP address of the host as a byte quartet
 ******************************************************************************/
int Connection::ipAddress () const
{
	return mpAddress->sin_addr.s_addr;
}

/*******************************************************************************
 * \fn const sockaddr_in& Connection::address () const
 *
 * Returns the low-level address structure of the connection
 ******************************************************************************/

/*******************************************************************************
 * \fn ThreadLock& Connection::threadLock ()
 *
 * Returns the thread lock that protects access to the connection object.
 ******************************************************************************/

/*******************************************************************************
 * Closes the connection
 *
 * This will generate a warning in the log if the connection has been
 * lost prior to this call.
 ******************************************************************************/
MSrvResult Connection::close ()
{
	int result = 0;

	if (!mSocket)
		return MSRVERR_CONNECTION_NO_SOCKET;

	if (!mrpListener)
		result = MSRVERR_NO_LISTENER; /* Not fatal. */
	else {
		/* Remove socket from listener. */
		result = mrpListener->removeDescriptor (mSocket);

		/* NOTE: Removing will fail if the connection was lost. */
		if (result < 0)
			mrpListener->log().message ("SERVER", Log::Warning, 0,
										"Removing descriptor failed with error %d.",
										result);
	}

	/* Close the socket. */
	::close (mSocket);
	mSocket = 0;

	mrpListener->log().message ("SERVER", Log::Info, 0,
								"Connection closed.");

	return result;
}

/*******************************************************************************
 * \fn Connection* ConnectionFactory::create (int socket, const struct sockaddr_in& rAddr, Listener& pListener) = 0
 *
 * Creates a connection with the given parameters.
 *
 * This method must be implemented by inheritor of ConnectionFactory
 * to create the specific type of Connection object.
 *
 * The parameters should be passed as such to the constructor of the
 * inherited object, which should passes them directly to the
 * constructor of the Connection object.
 ******************************************************************************/

end_namespace (MSrv);

