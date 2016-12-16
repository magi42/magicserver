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

#ifndef __MAGICSERVER_MSRVDEF_H__
#define __MAGICSERVER_MSRVDEF_H__

/*******************************************************************************
 * Macros
 ******************************************************************************/

/** Macro for entering a namespace without a block in source file. */
#ifndef begin_namespace
#define begin_namespace(ns) namespace ns {
#endif

/** Macro for leaving a namespace without a block in source file. */
#ifndef end_namespace
#define end_namespace(ns) }
#endif

/** Debug macro for printing the execution of a line in source code. */
#define TRACELINE printf ("LINETRACE %d [%s]\n", __LINE__, __FILE__);

/*******************************************************************************
 * Data types
 ******************************************************************************/

/** Result type for return values.
 *
 *  Negative values indicate an error, zero or more indicates success.
 *  A positive value typically represents a length or size.
 **/
typedef int MSrvResult;

/** Unsigned integer type. */
typedef unsigned int uint;

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MSRV_MAX_SELECT_ERROR_COUNT    10   /**< Maximum number of successive errors before exiting. */
#define MSRV_READ_BUFFER_LEN           1024 /**< Read buffer length for reading data from socket.    */

#endif
