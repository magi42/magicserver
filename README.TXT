================================================================================
$Author: magi $ $Date: 2003/06/14 14:13:14 $ $Revision: 1.2 $

MagiCServer++ framework

Copyright (C) 2003 Marko Grönroos (magi@iki.fi)

Libraries distributed under GNU Lesser General Public License.
Applications distributed under GNU General Public License.
Documentation distributed under GNU Free Documentation License.
================================================================================

========
Contents
========

  * Introduction
  * Installation
  * Compatibility
  * Other Notes

============
Introduction
============

  MagiCServer++ is a framework for implementing efficient and flexible
  Internet server applications. It supports both connection-based TCP
  and connectionless UDP datagram protocols in a transparent fashion.

  The framework is implemented as a C++ library, which has been kept
  as independent from other libraries as possible, to make reuse
  easier. Error handling is done with error codes; exceptions are not
  used except for constructors. For data structures, low-level C data
  structures are used for most tasks. Simple support tools are
  provided for logging, threading, and queues.

  See MagiCServer++ Developer's Guide and Reference Manual for more
  details.

---------------------------------------------------------------------
LISENCE NOTICE: While the libraries are under LGPL, any applications
                included in the package, such as any non-library
                code is under GNU GPL (General Public License).
---------------------------------------------------------------------

============
Installation
============

Configuration, building, and installation is done using the MagiCBuild
build system.

Installation is done as follows:

   $ tar jxf magicserver++-0.1beta1.tar.bz2
   $ cd magicserver++-0.1beta1
   $ ./configure --prefix=/usr/local
   $ make deps
   $ make
   $ make install

-------------
COMPATIBILITY
-------------

MagiCServer++ currently compiles under GNU/Linux using various
versions of the GCC compiler.

It has been tested under Red Hat 9 and Debian 2.2 distributions.

See Developer's Guide for more details.

-----------
OTHER NOTES
-----------

===========
End-Of-File
===========
