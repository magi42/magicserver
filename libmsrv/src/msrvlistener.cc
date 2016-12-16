/******************************************************************************
 *   This file is part of the MagiCServer++ library.                          *
 *                                                                            *
 *   Copyright (C) 2003 Marko Grönroos <magi@iki.fi>                          *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  This library is free software; you can redistribute it and/or             *
 *  modify it under the terms of the GNU Library General Public               *
 *  License as published by the Free Software Foundation; either              *
 *  version 2 of the License, or (at your option) any later version.          *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Library General Public         *
 *  License along with this library; see the file COPYING.LIB.  If            *
 *  not, write to the Free Software Foundation, Inc., 59 Temple Place         *
 *  - Suite 330, Boston, MA 02111-1307, USA.                                  *
 *                                                                            *
 ******************************************************************************/

#include <magicserver/msrvlistener.h>
#include <magicserver/msrverror.h>
#include <magicserver/msrvlog.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

begin_namespace (MSrv);

/*******************************************************************************
 * Dummy log that eats all that is written there.
 *
 * Listener uses this by default.
 ******************************************************************************/
static DummyLog dummyLog;

/*******************************************************************************
 * \fn Descriptor::Descriptor (int fd, void* pData)
 *
 * Creates a descriptor object.
 *
 * @param fd    Low-level descriptor such as a socket or file descriptor.
 * @param pData Pointer to an associated data object, which type is
 *              completely hidden from this class.
 ******************************************************************************/

/*******************************************************************************
 * Default constructor for Listener objects.
 ******************************************************************************/
Listener::Listener (Log* rpLog)
{
	mTimeoutSec     = 1;
	mTimeoutUSec    = 0;
	mShutdownStatus = false;

	if (rpLog)
		mrpLog = rpLog;
	else
		mrpLog = &dummyLog;
}

/*******************************************************************************
 * Destroys a listener object.
 *
 * \note The descriptors must be closed before calling this.
 ******************************************************************************/
Listener::~Listener ()
{
}

/*******************************************************************************
 * Waits for a status change in any of the descriptors.
 *
 * @return 0 if successful, otherwise a negative error code.
 ******************************************************************************/
MSrvResult Listener::listen ()
{
	struct timeval timeout;         /* Timeout for select.                */
	int            errorcount = 0;  /* For counting consecutive failures. */
	fd_set         fdset;           /* Set of descriptors.                */

	mrpLog->message ("LISTENER", Log::Info, 0, "Starting listening...");
	
	while (1) {
		mThreadLock.lock ();

		/* Are we using a timeout? */
		bool usingTimeout = mTimeoutSec>0 || mTimeoutUSec>0;
		
		/* Set select timeout. */
		timeout.tv_sec  = mTimeoutSec;
		timeout.tv_usec = mTimeoutUSec;

		/* Put all watched descriptors to the descriptor set. */
		FD_ZERO (&fdset); /* Empty the FD set.                */
		int maxfd = 0;    /* Highest descriptor in the set.   */
		for (int i=0; i<mDescriptors.length(); ++i) {
			/* Put a descriptor in descriptor set. */
			FD_SET (mDescriptors[i].mFd, &fdset);

			/* Track highest descriptor in the set. */
			if (mDescriptors[i].mFd > maxfd)
				maxfd = mDescriptors[i].mFd;
		}
		mThreadLock.unlock ();
		
		/* Wait for a status change in any of the descriptors. */
		int selectCount = select (maxfd + 1,
								  &fdset,   /* Read events.      */
								  NULL,     /* No write events.  */
								  NULL,     /* Exception events. */
								  usingTimeout? &timeout : NULL);
		if (selectCount < 0) {
			mrpLog->message ("LISTENER", Log::Warning, MSRVERR_SELECT_FAILED,
							 "Select failed with error %d; %s.",
							 errno, strerror (errno));

			/* We don't want to fail completely at first problem, so we try */
			/* again and hope the problem goes away. It probably doesn't.   */
			++errorcount;

		} else if (selectCount == 0) {
			/* Select exited because of timeout. */
			int result = timeoutEvent ();
			
			/* Check if the event caused shutdown. */
			if (result == MSRVERR_SHUTDOWN_EVENT)
				startShutdown ();
			else if (result < 0)
				/* Not much we can do with the error. */;

		} else {
			/* State of some descriptor(s) has changed. */
			mThreadLock.lock ();

			/* Check which descriptors have a status change. */
			bool foundChange = false;
			for (int i=0; i<mDescriptors.length(); ++i)
				/* Check a descriptor for status change. */
				if (FD_ISSET (mDescriptors[i].mFd, &fdset)) {
					/* Status has changed. Handle event. */
					int result = descriptorEvent (mDescriptors[i].mFd,
												  mDescriptors[i].mpData);

					/* Check if the event caused shutdown. */
					if (result == MSRVERR_SHUTDOWN_EVENT)
						startShutdown ();
					else if (result < 0)
						/* TODO: Handle error. */;
					
					foundChange = true;
				}

			/* All is ok again, reset the error count. */
			errorcount = 0;

			mThreadLock.unlock ();
		}

		/* Check if the select has failed too many times consecutively. */
		if (errorcount > MSRV_MAX_SELECT_ERROR_COUNT) {
			mrpLog->message ("LISTENER",
							 Log::Critical,
							 MSRVERR_TOO_MANY_ERRORS,
							 "Too many errors in select.");
			startShutdown ();
		}
		
		if (isShutdown())
			break;
	}

	/* Let an inheritor perform any necessary shutdown tasks. */
	/* It should close all the descriptors too.               */
	shutdown ();

	/* Close any descriptors still open. */
	if (mDescriptors.length() > 0) {
		for (int i=0; i<mDescriptors.length(); ++i)
			::close (mDescriptors[i].mFd);
		
		mrpLog->message ("LISTENER",
						 Log::Warning,
						 0,
						 "%d still open descriptors were closed by Listener.",
						 mDescriptors.length());
	}

	return 0;
}

/*******************************************************************************
 * Initiates shutdown.
 *
 * The listener will not be shut down immediately, as the call may
 * have come from a different thread. Listener can be in wait state,
 * waiting on sockets, in which case it will start shutdown when it
 * awakens.
 ******************************************************************************/
void Listener::startShutdown ()
{
	mShutdownStatus = true;
}

/*******************************************************************************
 * \fn bool Listener::isShutdown () const
 *
 * Tells if the listener is in shutdown state.
 ******************************************************************************/

/*******************************************************************************
 * Sets timeout for listening.
 *
 * During the execution of @ref listen(), after the timeout expires
 * without events in descriptors, @ref timeoutEvent() is called.
 *
 * This should not be very small, preferably more than a second.
 *
 * If the timeout is set to 0s 0us, it is disabled. This is not
 * recommended as then the Listener will not start shut down before
 * one of the descriptors has an event.
 ******************************************************************************/
void Listener::setTimeout (
	long seconds,
	long microseconds)
{
	mTimeoutSec  = seconds;
	mTimeoutUSec = microseconds;
}

/*******************************************************************************
 * Returns the amount of timeout counter left since the last event.
 *
 * The values will be non-zero only if the last event raised by the
 * Listener was not timeoutEvent.
 ******************************************************************************/
void Listener::timeoutLeft (
 	long& seconds,
	long& microseconds) const
{
	/* TODO: Return the values. */
}

/*******************************************************************************
 * \fn void Listener::setLog (Log& rLog)
 *
 * Sets the log to which the Listener should print its messages.
 ******************************************************************************/

/*******************************************************************************
 * \fn Log& Listener::log ()
 *
 * Returns a reference to the log to which the Listener messages are printed.
 ******************************************************************************/

/*******************************************************************************
 * Removes a descriptor (a socket) from the listener.
 *
 * The connection on the descriptor must be closed before calling
 * this.
 *
 * @return 0 if successful, otherwise an error code.
 ******************************************************************************/
MSrvResult Listener::removeDescriptor (int fd)
{
	/* Note: This method is a bit too crowded with array manipulation. */
	
	mThreadLock.lock ();

	/* Find the descriptor. */
	bool found = false;
	for (int i=0; i<mDescriptors.length(); ++i)
		if (mDescriptors[i].mFd == fd) {
			/* Found it. Remove it. */
			mDescriptors.remove (i);

			found = true; /* Note that we found a descriptor. */
			break;
		}

	mThreadLock.unlock ();

	/* If no descriptor was found, the argument was invalid. */
	if (!found)
		return MSRVERR_DESCRIPTOR_NOT_FOUND;
	
	return 0;
}

/*******************************************************************************
 * Descriptor status changed.
 *
 * Inheritor should reimplement this to handle events on descriptors.
 *
 * @return 0 if successful, otherwise error. If the return value is
 *         MSRVERR_SHUTDOWN_EVENT, the @ref listen() will stop as soon as
 *         possible.
 ******************************************************************************/
MSrvResult Listener::descriptorEvent (
	int   fd,  /**< Descriptor which status changed.                       */
	void* data /**< Pointer to data object associated with the descriptor. */)
{
	return 0;
}

/*******************************************************************************
 * Timeout event occurred during @ref listen().
 *
 * Inheritor should reimplement this to handle timeout events.
 *
 * @return 0 if successful, otherwise error. If the return value is
 *         MSRVERR_SHUTDOWN_EVENT, the @ref listen() will stop as soon as
 *         possible.
 ******************************************************************************/
MSrvResult Listener::timeoutEvent	()
{
	return 0;
}

/*******************************************************************************
 * Handles shutdown tasks
 *
 * The inheritor must reimplement this method to handle all relevant
 * tasks needed when it is shutting down.
 ******************************************************************************/
MSrvResult Listener::shutdown ()
{
	return 0;
}

end_namespace (MSrv);
