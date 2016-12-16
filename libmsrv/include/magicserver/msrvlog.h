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

#ifndef __MAGICSERVER_MSRVLOG_H__
#define __MAGICSERVER_MSRVLOG_H__

#include <stdio.h>
#include <magicserver/msrvdef.h>
#include <magicserver/msrvthread.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Abstract log
 ******************************************************************************/
class Log {
  public:
	/** Fatality of a log event. Compatible with syslog(). */
	enum fatality {Emergency = 0, /**< System is unusable.                 */
				   Alert     = 1, /**< Action must be taken immediately.   */
				   Critical  = 2, /**< Critical conditions.                */
				   Error     = 3, /**< Error conditions.                   */
				   Warning   = 4, /**< Warning conditions.                 */
				   Notice    = 5, /**< Normal but insignificant condition. */
				   Info      = 6, /**< Informational.                      */
				   Debug     = 7  /**< Debug-level messages.               */};

	virtual 			~Log		() {}

	virtual MSrvResult	open		() = 0;
	virtual void		close		() = 0;
	virtual MSrvResult	message		(const char* modulename, int fatality, int errno, const char* message, ...) = 0;
	
  protected:
	virtual MSrvResult	write		(const char* data, int len) = 0;
	
  private:
};

/*******************************************************************************
 * Text-based log associated with a file stream
 *
 * The log format is as follows:
 *
 * \code <modulename> <fatality> <errno>: <message> \endcode
 *
 * For example:
 *
 * \code MYMODULE WARNING 1234: This is a simple warning. \endcode
 ******************************************************************************/
class LogFile : public Log {
  public:
						LogFile		(FILE* stream = stdout);
						LogFile		(const char* filename);
	virtual 			~LogFile	();

	MSrvResult			setFilename	(const char* filename);

	virtual MSrvResult	open		();
	virtual void		close		();

	virtual MSrvResult	message		(const char* modulename, int fatality, int errno, const char* message, ...);
	
  protected:
	virtual MSrvResult	write		(const char* data, int len);

  private:
	const char* mpFilename;
	FILE*		mpLogStream;
	ThreadLock  mThreadLock;
};

/*******************************************************************************
 * Dummy log
 ******************************************************************************/
class DummyLog : public Log {
  public:
	virtual 			~DummyLog	() {}
	virtual MSrvResult	open		() {return 0;}
	virtual void		close		() {;}
	virtual MSrvResult	message		(const char* modulename, int fatality, int errno, const char* message, ...) {return 0;}

  protected:
	virtual MSrvResult	write		(const char* data, int len) {return 0;}
};

end_namespace (MSrv);

#endif
