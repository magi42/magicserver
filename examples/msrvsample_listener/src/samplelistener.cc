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

#include <magicserver/msrvserver.h>
#include <magicserver/msrvworker.h>
#include <magicserver/msrvlog.h>

#include <msrvsamplehandler.h>
#include <msrvsamplemain.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdexcept>

using namespace MSrv;

/*******************************************************************************
 * Initializes and runs the server
 ******************************************************************************/
int serverMain (const TestArgs& args)
{
	MSrvResult msrvResult = 0;
	int        exitValue  = 0;

	/* Open log to file or standard output. */
	LogFile log (args.logfile? args.logfile : "-");
	log.message ("SMPLLIST", Log::Info, 0, "Log opened.");
	
	if (args.daemonize)
		if (daemon (0, 0) < 0) {
			log.message ("SMPLLIST", Log::Critical, 0,
						 "Daemonization failed with error %d; %s.",
						 errno, strerror (errno));
		}
	
	/* Create transaction handler. */
	MyHandler myHandler;
	
	/* Create and configure server object. */
	ServerListener myServer (myHandler, &log);
	
	/* Create a server socket and bind it to an address. */
	msrvResult = myServer.bind (args.portno,
								args.udp? ServerListener::UDP : ServerListener::TCP,
								0);
	if (msrvResult < 0) {
		log.message ("SMPLLIST", Log::Critical, 0,
					 "Server initialization failed with error %d.",
					 -msrvResult);
		exitValue = MSRVTEST_RETVAL_INIT_FAILED;
	}
	
	if (msrvResult >= 0) {
		/* Enter the listener loop. */
		msrvResult = myServer.listen ();
		if (msrvResult < 0) {
			log.message ("SMPLLIST", Log::Critical, 0,
						 "Server execution failed with error %d.",
						 -msrvResult);
			return MSRVTEST_RETVAL_EXEC_FAILED;
		}
	}
	
	/* Server has stopped. */
	
	log.message ("SMPLLIST", Log::Info, 0,
				 "Server stopped. Closing log and exiting.");

	return exitValue;
}
