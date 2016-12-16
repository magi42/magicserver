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

#ifndef __MSRVSAMPLEMAIN_H__
#define __MSRVSAMPLEMAIN_H__

#include <magicserver/msrvserver.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MSRVTEST_PORTNO 1234

/*******************************************************************************
 * Application exit values
 ******************************************************************************/
#define MSRVTEST_RETVAL_INIT_FAILED		1
#define MSRVTEST_RETVAL_EXEC_FAILED		2

/*******************************************************************************
 * Application arguments
 ******************************************************************************/
struct TestArgs {
	TestArgs () {
		daemonize = false;
		logfile   = NULL;
		portno    = MSRVTEST_PORTNO;
		udp       = false;
	}
	
	bool        daemonize; /**< Should the server detach from tty?            */
	const char* logfile;   /**< Log file to write to or - for standard output.*/
	int         portno;    /**< Port number to listen to.                     */
	bool        udp;       /**< Should UDP be used instead of TCP?            */
};

/*******************************************************************************
 * User has to implement this function to use this library.
 ******************************************************************************/
extern int serverMain (const TestArgs& args);

#endif



