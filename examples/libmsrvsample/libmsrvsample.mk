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
export SRCDIR ?= ../../..

################################################################################
# Define module name and compilation type
################################################################################
modname          = libmsrvsample
modpath          = examples/$(modname)
compile_library  = 1
#makedox          = 1
#modversionfile   = lmversion.mk
#modversionheader = mversion.h

################################################################################
# Include build framework
################################################################################
include $(SRCDIR)/build/magicdef.mk

################################################################################
# Source files for libmagic.a
################################################################################
sources = samplehandler.cc samplemain.cc

headers = msrvsamplehandler.h msrvsamplemain.h

#headersubdir = 

################################################################################
# Recursively compile some subprojects
################################################################################
makemodules =

################################################################################
# Compile
################################################################################
include $(SRCDIR)/build/magiccmp.mk

