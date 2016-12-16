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

#ifndef __MSRVSAMPLEHANDLER_H__
#define __MSRVSAMPLEHANDLER_H__

#include <magicserver/msrvrequest.h>

/*******************************************************************************
 * Sample request handler
 ******************************************************************************/
class MyHandler : public MSrv::RequestHandler {
  public:
						MyHandler		() {;}
	virtual				~MyHandler		() {;}

	virtual MSrvResult	init		(MSrv::ServerListener& rListener);
	
	virtual MSrvResult	process 	(MSrv::NewConnectionRequest&  rRequest);
	virtual MSrvResult	process 	(MSrv::ConnectionLostRequest& rRequest);
	virtual MSrvResult	process 	(MSrv::StreamDataRequest&     rRequest);
	virtual MSrvResult	process 	(MSrv::DatagramRequest&       rRequest);
	virtual MSrvResult	process 	(MSrv::ShutdownRequest&       rRequest);
	virtual MSrvResult	process 	(MSrv::TimeoutRequest&        rRequest);

  protected:
	MSrvResult			processData	(MSrv::DataRequest& rRequest);
};

#endif



