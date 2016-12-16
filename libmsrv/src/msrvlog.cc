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

#include <magicserver/msrverror.h>
#include <magicserver/msrvlog.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdexcept>
#include <time.h>

begin_namespace (MSrv);

/*******************************************************************************
 * \fn MSrvResult Log::open()=0
 *
 * Opens the log file.
 ******************************************************************************/

/*******************************************************************************
 * \fn void Log::close()=0
 *
 * Closes the log file.
 ******************************************************************************/

/*******************************************************************************
 * \fn MSrvResult Log::message (const char* modulename, int fatality, int errno, const char* message, ...)=0;
 *
 * Writes a message to the log.
 ******************************************************************************/

/*******************************************************************************
 * \fn MSrvResult Log::write (const char* data, int len)=0;
 *
 * Writes to the log.
 ******************************************************************************/

/*******************************************************************************
 * Opens log to the given already open stream.
 *
 * \par Example:
 * \code
 *   LogFile log (stdout);
 *  log.message ("TEST", Log::Info, 0, "Log opened.");
 * \endcode
 ******************************************************************************/
LogFile::LogFile (
	FILE* stream /**< Stream to write the log to. */)
{
	mpLogStream = stream;
}

/*******************************************************************************
 * Opens a log file with the given name.
 *
 * The log will be appended to existing log file, if it
 * exists. Otherwise, the log file will be created.
 ******************************************************************************/
LogFile::LogFile (
	const char* filename /**< File to write the log to. */)
{
	mpLogStream = NULL;

	if (!filename)
		mpFilename = NULL; /* A problematic situation. */
	else
		mpFilename = strdup (filename);

	int result = open ();
	if (result < 0) {
		char buffer[1024];
		sprintf (buffer, "Opening log file failed with error %d.", -result);
		throw std::runtime_error (buffer);
	}
}

/*******************************************************************************
 * Closes the log file
 ******************************************************************************/
LogFile::~LogFile ()
{
	mThreadLock.lock ();

	close ();

	delete mpFilename;

	mThreadLock.unlock ();
}

/*******************************************************************************
 * Sets the file name
 *
 * Does not reopen the file with the new file name. You must \ref close() and
 * \ref open() the file.
 *
 * \return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult LogFile::setFilename (const char* filename)
{
	if (!filename)
		return MSRVERR_LOG_NO_FILENAME;

	mThreadLock.lock ();
	if (mpFilename)
		delete mpFilename;

	mpFilename = strdup (filename);

	mThreadLock.unlock ();
	return 0;
}

/*******************************************************************************
 * Opens the log file
 *
 * The log will be appended to existing log file, if it
 * exists. Otherwise, the log file will be created.
 *
 * \return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult LogFile::open ()
{
	int result = 0;

	mThreadLock.lock ();

	if (!mpFilename)
		result = MSRVERR_LOG_NO_FILENAME;
	
	if (!result && mpLogStream)
		result = MSRVERR_LOG_ALREADY_OPEN;

	if (!result) {
		if (! strcmp (mpFilename, "-")) {
			/* Open log to standard output. */
			mpLogStream = stdout;
			
		} else {
			/* Open the file. */
			FILE* logStream = fopen (mpFilename, "a");
			if (logStream == NULL)
				result = MSRVERR_LOG_OPEN_FAILED;
			else
				mpLogStream = logStream;
		}
	}

	mThreadLock.unlock ();
	return result;
}

/*******************************************************************************
 * Closes the log file
 ******************************************************************************/
void LogFile::close ()
{
	mThreadLock.lock ();

	/* Close the file, if necessary. */
	if (mpLogStream && mpLogStream != stdout) {
		fclose (mpLogStream);
		mpLogStream = 0;
	}

	mThreadLock.unlock ();
}

/*******************************************************************************
 * Writes a block of data to log stream
 *
 * \return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult LogFile::write (const char* data, int len)
{
	mThreadLock.lock ();

	if (! mpLogStream)
		return MSRVERR_LOG_NOT_OPEN;

	if (!data)
		return MSRVERR_NULL_ARGUMENT;

	int written = fwrite (data, 1, len, mpLogStream);
	if (written < len)
		return MSRVERR_SHORT_WRITE;

	mThreadLock.unlock ();
	return 0;
}

/*******************************************************************************
 * Writes a log message
 *
 * \par Example:
 * \code
 * 		mpLog->message ("SERVER",
 *                      Log::Critical,
 *                      MSRVERR_ACCEPT_FAILED,
 *                      "Accept failed with error %d; %s.",
 *                      errno, strerror (errno));
 * \endcode
 *
 * \return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult LogFile::message (
	const char* modulename,  /**< An identifier of the module writing to log. */
	int         fatality,    /**< Severity of the event.                      */
	int         errno,       /**< Possible message number.                    */
	const char* message,     /**< Message (as a format string for printf).    */
	...)
{
	static const char* fatalities[8] = {"EMERGENCY",
										"ALERT",
										"CRITICAL",
										"ERROR",
										"WARNING",
										"NOTICE",
										"INFO",
										"DEBUG"};

	mThreadLock.lock ();

	if (! mpLogStream)
		return MSRVERR_LOG_NOT_OPEN;

	if (!message)
		return MSRVERR_NULL_ARGUMENT;

	if (fatality < 0 || fatality > Debug)
		return MSRVERR_LOG_INVALID_FATALITY;

	/* Ensure that the error code is positive. */
	if (errno < 0)
		errno = -errno;

	/* Write current time. */
	time_t currentTime = time (NULL);
	struct tm* pTimeTm = localtime (&currentTime);
	int written = fprintf (mpLogStream,
						   "%04d/%02d/%02d %02d:%02d:%02d ",
						   1900+pTimeTm->tm_year,
						   pTimeTm->tm_mon,
						   pTimeTm->tm_mday,
						   pTimeTm->tm_hour,
						   pTimeTm->tm_min,
						   pTimeTm->tm_sec);
	if (written <= 0)
		return MSRVERR_LOG_WRITE_FAILED;

	/* Write log line header. */
		written = fprintf (mpLogStream,
						   "%s %s %d: ",
						   modulename,
						   fatalities[fatality],
						   errno);
	if (written <= 0)
		return MSRVERR_LOG_WRITE_FAILED;

	/* Open ellipsis list. */
	va_list ap;
	va_start (ap, message);

	/* Write the message with optional ellipsis. */
	written = vfprintf (mpLogStream, message, ap);
	if (written < 0)
		return MSRVERR_LOG_WRITE_FAILED;

	/* Close the ellipsis handling. */
	va_end (ap);
	
	/* Write the ending newline. */
	written = fprintf (mpLogStream, "\n");
	if (written <= 0)
		return MSRVERR_LOG_WRITE_FAILED;

	fflush (mpLogStream);
	
	mThreadLock.unlock ();
	return 0;
}


end_namespace (MSrv);
