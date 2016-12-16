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

#ifndef __MAGICSERVER_MSRVERROR_H__
#define __MAGICSERVER_MSRVERROR_H__

/*******************************************************************************
 * Generic error codes
 ******************************************************************************/
#define MSRVERR_GENERIC_BASE              0
#define MSRVERR_NULL_ARGUMENT             (MSRVERR_GENERIC_BASE - 1)
#define MSRVERR_SHORT_WRITE               (MSRVERR_GENERIC_BASE - 2)
#define MSRVERR_TOO_MANY_ERRORS           (MSRVERR_GENERIC_BASE - 3)
#define MSRVERR_TIMEOUT                   (MSRVERR_GENERIC_BASE - 4)
#define MSRVERR_INVALID_ARGUMENT          (MSRVERR_GENERIC_BASE - 5)
#define MSRVERR_UNSPECIFIED_LOWERLEVEL    (MSRVERR_GENERIC_BASE - 6)

/*******************************************************************************
 * Listener and socket related error codes
 ******************************************************************************/
#define MSRVERR_SERVER_BASE               -1000
#define MSRVERR_SOCKET_FAILED             (MSRVERR_SERVER_BASE - 1)
#define MSRVERR_BIND_FAILED               (MSRVERR_SERVER_BASE - 2)
#define MSRVERR_LISTEN_FAILED             (MSRVERR_SERVER_BASE - 3)
#define MSRVERR_ACCEPT_FAILED             (MSRVERR_SERVER_BASE - 4)
#define MSRVERR_SELECT_FAILED             (MSRVERR_SERVER_BASE - 5)
#define MSRVERR_SHUTDOWN_EVENT            (MSRVERR_SERVER_BASE - 6) /* Special code. */
#define MSRVERR_READ_FAILED               (MSRVERR_SERVER_BASE - 7)
#define MSRVERR_DESCRIPTOR_NOT_FOUND      (MSRVERR_SERVER_BASE - 8)
#define MSRVERR_CONNECTION_NO_SOCKET      (MSRVERR_SERVER_BASE - 9)
#define MSRVERR_NO_LISTENER               (MSRVERR_SERVER_BASE - 10)
#define MSRVERR_SET_SOCKET_OPTIONS_FAILED (MSRVERR_SERVER_BASE - 10)

/*******************************************************************************
 * Log module error codes
 ******************************************************************************/
#define MSRVERR_LOG_BASE                  -2000
#define MSRVERR_LOG_OPEN_FAILED           (MSRVERR_LOG_BASE - 1)
#define MSRVERR_LOG_WRITE_FAILED          (MSRVERR_LOG_BASE - 2)
#define MSRVERR_LOG_NOT_OPEN              (MSRVERR_LOG_BASE - 3)
#define MSRVERR_LOG_NO_FILENAME           (MSRVERR_LOG_BASE - 4)
#define MSRVERR_LOG_ALREADY_OPEN          (MSRVERR_LOG_BASE - 5)
#define MSRVERR_LOG_INVALID_FATALITY      (MSRVERR_LOG_BASE - 6)

/*******************************************************************************
 * Thread module error codes
 ******************************************************************************/
#define MSRVERR_THREAD_BASE               -3000
#define MSRVERR_THREAD_CREATE_FAILED      (MSRVERR_THREAD_BASE - 1)
#define MSRVERR_THREAD_JOIN_FAILED        (MSRVERR_THREAD_BASE - 2)

/*******************************************************************************
 * Worker pool error codes
 ******************************************************************************/
#define MSRVERR_WORKER_BASE               -4000
#define MSRVERR_REPEAT_SHUTDOWN_REQUEST   (MSRVERR_WORKER_BASE - 1)

#endif
