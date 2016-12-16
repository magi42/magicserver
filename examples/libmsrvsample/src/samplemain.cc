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
 * Parses command-line arguments
 ******************************************************************************/
int parse_cmd (int       argc,
			   char*     argv[],
			   TestArgs& args)
{
	int arg = 1;
	while (arg < argc) {
		if (!strcmp (argv[arg], "-d"))
			args.daemonize = true;
		else if (!strcmp (argv[arg], "-udp"))
			args.udp = true;
		else if (!strcmp (argv[arg], "-l") && arg < argc-1)
			args.logfile = argv[++arg];
		else if (!strcmp (argv[arg], "-p") && arg < argc-1)
			args.portno = atoi (argv[++arg]);
		else {
			fprintf (stderr, "Invalid command line argument '%s'\n",
					 argv[arg]);
			fprintf (stderr, "Usage: %s [-d] [-udp] [-l <logfile>] [-p <portno>]\n",
					 argv[0]);
			return 1;
		}

		arg++;
	}

	return 0;
}

/*******************************************************************************
 * Main program
 ******************************************************************************/
int main (int   argc,
		  char* argv[])
{
	int        exitValue = 0;
	TestArgs   args;

	/* Parse command-line arguments. */
	int result = parse_cmd (argc, argv, args);
	if (result)
		return result;

	try {
		/* Initialize and run the server. */
		exitValue = serverMain (args);
	} catch (std::runtime_error& e) {
		fprintf (stderr, "Exception caught at main level: %s\n",
				 (const char*) e.what ());
	}
	
	return exitValue;
}
