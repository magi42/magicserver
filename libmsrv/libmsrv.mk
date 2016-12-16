################################################################################
#    This file is part of the MagiCServer++ library.                           #
#                                                                              #
#    Copyright (C) 2003 Marko Grönroos <magi@iki.fi>                           #
#                                                                              #
################################################################################
#                                                                              #
#   This library is free software; you can redistribute it and/or              #
#   modify it under the terms of the GNU Library General Public                #
#   License as published by the Free Software Foundation; either               #
#   version 2 of the License, or (at your option) any later version.           #
#                                                                              #
#   This library is distributed in the hope that it will be useful,            #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of             #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          #
#   Library General Public License for more details.                           #
#                                                                              #
#   You should have received a copy of the GNU Library General Public          #
#   License along with this library; see the file COPYING.LIB.  If             #
#   not, write to the Free Software Foundation, Inc., 59 Temple Place          #
#   - Suite 330, Boston, MA 02111-1307, USA.                                   #
#                                                                              #
################################################################################

DEBUG = 1

################################################################################
# Define root directory of the source tree
################################################################################
export SRCDIR ?= ..

################################################################################
# Define module name and compilation type
################################################################################
modname          = libmsrv
#modtarget        = libmsrv
compile_library  = 1
makedox          = 1
#modversionfile   = lmversion.mk
#modversionheader = mversion.h

################################################################################
# Extra files and directories
################################################################################

################################################################################
# Include build framework
################################################################################
include $(SRCDIR)/build/magicdef.mk

################################################################################
# Source files for libmagic.a
################################################################################

sources = msrvserver.cc msrvlistener.cc msrvlog.cc msrvthread.cc \
          msrvworker.cc msrvrequest.cc

headers = msrvserver.h msrvlistener.h msrvlog.h msrvthread.h msrvdef.h \
          msrvworker.h msrvcontainer.h msrverror.h msrvrequest.h

headersubdir = magicserver

################################################################################
# Recursively compile some subprojects
################################################################################
makemodules = 

################################################################################
# Compile
################################################################################
include $(SRCDIR)/build/magiccmp.mk

