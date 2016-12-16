/***************************************************************************
 *   This file is part of the MagiCServer++ library.                       *
 *                                                                         *
 *   Copyright (C) 2003 Marko Grönroos <magi@iki.fi>                       *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <msrvsamplehandler.h>
#include <magicserver/msrvlog.h>

#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

using namespace MSrv;

/******************************************************************************/
/* Allows handler make settings to listener.                                  */
/******************************************************************************/
MSrvResult MyHandler::init (ServerListener& rListener)
{
	/* Set the request types we want to process.     */
	/* This is actually same as the default setting. */
	rListener.setRequestMask (Request::NewConnection |
							  Request::StreamData |
							  Request::Datagram |
							  Request::ConnectionLost |
							  Request::Shutdown);

	/* Only Request::Timeout is not enabled. */
	
	return 0;
}

/******************************************************************************/
/* Process new connection request.                                            */
/******************************************************************************/
MSrvResult MyHandler::process (NewConnectionRequest& rRequest)
{
	/* Put IP address in string format. */
	char ipaddr [22];
	sprintf (ipaddr, "%d.%d.%d.%d",
			 (rRequest.connection().ipAddress()) & 0xff,
			 (rRequest.connection().ipAddress() >> 8) & 0xff,
			 (rRequest.connection().ipAddress() >> 16) & 0xff,
			 (rRequest.connection().ipAddress() >> 24) & 0xff);

	/* Resolve host name. */
	struct hostent* host = gethostbyaddr (&rRequest.connection().address().sin_addr,
										  sizeof (in_addr),
										  AF_INET);
	
	if (host) {
		/* Send welcome message to the client. */
		char msg[1024];
		snprintf (msg, 1024, "001 Hello, there, \"%s\" (%s)!\n",
				  host->h_name,
				  ipaddr);
		write (rRequest.socket(), msg, strlen (msg));
	}
	else {
		rRequest.serverListener().log().message ("SAMPLE", Log::Info, 0,
												 "Resolving address %s failed with error %d; %s.",
												 ipaddr,
												 h_errno,
												 hstrerror (h_errno));
		const char* msg = "001 Hello, there!\n";
		write (rRequest.socket(), msg, strlen (msg));
	}
	
	return 0;
}

/******************************************************************************/
/* Process connection lost request.                                           */
/******************************************************************************/
MSrvResult MyHandler::process (ConnectionLostRequest& rRequest)
{
	rRequest.serverListener().log().message ("SAMPLE", Log::Info, 0,
											  "Connection lost");

	return 0;
}

/******************************************************************************/
/* Process stream data request.                                               */
/******************************************************************************/
MSrvResult MyHandler::process (StreamDataRequest& rRequest)
{
	/* Use common handler for all data request types. */
	return processData (rRequest);
}

/******************************************************************************/
/* Process datagram request.                                                  */
/******************************************************************************/
MSrvResult MyHandler::process (DatagramRequest& rRequest)
{
	/* Use common handler for all data request types. */
	return processData (rRequest);
}

/******************************************************************************/
/* Process data request (either stream data or datagram).                     */
/******************************************************************************/
MSrvResult MyHandler::processData (DataRequest& rRequest)
{
	char*      data   = rRequest.getData ();
	MSrvResult result = 0;
	
	/* Cut at newline. */
	for (int i=0; i<rRequest.dataLen(); ++i)
		if (data[i] < '\x20') {
			data[i] = 0x00;
			break;
		}
	
	/* Shutdown command. */
	if (!strcmp (data, "shutdown"))
		rRequest.serverListener().startShutdown ();
	
	/* Quit command (only on TCP server). */
	else if  (rRequest.getType () == Request::StreamData &&
			  !strcmp (data, "quit")) {
		StreamDataRequest& rSDRequest = dynamic_cast<StreamDataRequest&> (rRequest);
		
		/* Send bye message to the client. */
		const char* msg = "002 Bye, there!\n";
		write (rSDRequest.socket(), msg, strlen (msg));
		
		result = rSDRequest.connection().close ();
	}
	else {
		/* Format and send a response. */
		char msg[1024];
		snprintf (msg, 1024, "004 Well well well, '%s' to you too!\n",
				  data);
		write (rRequest.socket(), msg, strlen (msg));

		/* Relay the message to all other clients. */
		snprintf (msg, 1024, "005 Someone else said: '%s'.\n",
				  data);

		/* Iterate through connections in the listener. */
		for (ServerListener::ConnIter serv_i (rRequest.serverListener());
			 !serv_i.exhausted ();
			 serv_i.next()) {
			if (serv_i.get().socket() != rRequest.socket())
				write (serv_i.get().socket(), msg, strlen (msg));
		}
	}
	
	rRequest.serverListener().log().message ("SAMPLE", Log::Info, 0,
											 "Received message '%s'.",
											 data);

	return result;
}

/******************************************************************************/
/* Process shutdown request.                                                  */
/******************************************************************************/
MSrvResult MyHandler::process (ShutdownRequest& rRequest)
{
	rRequest.serverListener().log().message ("SAMPLE", Log::Info, 0,
											 "Server is shutting down.");

	/* Send shutdown message to all connected clients. */
	const char* msg = "003 Server shutting down immediately! (bye bye)\n";
	
	/* Iterate through connections in the listener. */
	for (ServerListener::ConnIter serv_i (rRequest.serverListener());
		 !serv_i.exhausted ();
		 serv_i.next()) {
		/* Write shutdown message to the connection. */
		write (serv_i.get().socket(), msg, strlen (msg));
	}

	return 0;
}

/******************************************************************************/
/* Process timeout request.                                                   */
/******************************************************************************/
MSrvResult MyHandler::process (TimeoutRequest& rRequest)
{
	rRequest.serverListener().log().message ("SAMPLE", Log::Info, 0,
											 "Listener notified about a routine timeout.");

	/* We could do something interesting here if we wanted. */
	return 0;
}
