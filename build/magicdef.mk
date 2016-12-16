################################################################################
#    This file is part of the MagiCBuild configuration and build system.       #
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

################################################################################
# Names
################################################################################
ifdef modname
ifndef modpath
modpath = $(modname)
endif
ifndef modtarget
modtarget = $(modname)
endif
endif

################################################################################
# Early definitions
################################################################################
export BUILDTYPE  ?= release

################################################################################
# Include local configuration
################################################################################

# Try to read the main parameters from the configuration file
configexists := $(shell ls $(SRCDIR)/build/config.mk)
ifdef configexists
include $(SRCDIR)/build/config.mk
else

# Look at default build directory
configexists := $(shell ls /tmp/$(USER)/build/config.mk)
ifdef configexists
include $(SRCDIR)/build/config.mk
else
notall:
	@echo ERROR: No local configuration file.
	@echo Stopped.
endif
endif

# See if there is a subconfiguration file available
ifdef modpath
subconfigfile = $(BUILDDIR)/$(PLATFORM)-$(ARCH)/$(BUILDTYPE)/configs/$(modpath)/config.mk
subconfigexists := $(shell if [ -f $(subconfigfile) ]; then echo "yes"; fi)

ifdef subconfigexists
include $(subconfigfile)
endif
endif

################################################################################
# Main target
################################################################################

ifndef SRCDIR
ifndef MAGICDIR
notall:
	@echo 'Error: $$MAGICDIR environment variable must be defined.'
	@echo
	@echo 'Environment variables:'
	@echo '$$MAGICDIR     MagiC++ installation directory (or sources).'
	@echo '$$SRCDIR       Root of source directory tree.'
	@echo '$$BUILDDIR     Root of output directory tree (/tmp/magicbuild).'
	@echo '$$ARCH         Processor architecture (e.g. "i386")'
	@echo '$$PLATFORM     Operating system (e.g. "linux")'
	@echo '$$INSTALLDIR   Installation directory (e.g. "/usr/local")'
	@echo
	@echo 'For example:'
	@echo
	@echo 'export MAGICDIR=/opt/MagiClib'
	@echo 'export BUILDDIR=/tmp/build'
	@echo 'make SRCDIR=. PLATFORM=linux ARCH=i386'
endif
export MAGICDIR ?= $(SRCDIR)
endif

################################################################################
# Top definitions
################################################################################

export MAGICDIR   ?= ..
export SRCDIR     ?= ..
export BUILDDIR   ?= /tmp/$(USER)/build
export PLATFORM   ?= linux
export ARCH       ?= i386
export INSTALLDIR ?= /usr/local
export BUILDTYPE  ?= release

################################################################################
# Applications
################################################################################
export CXX        ?= g++

################################################################################
# Source directory definitions
################################################################################
magicincludedir ?= $(MAGICDIR)/include
magiclibdir     ?= $(MAGICDIR)/lib

# Group (top level) directories
grpincdir        = $(SRCDIR)/include
grpbuilddir      = $(SRCDIR)/build
grpdocdir        = $(SRCDIR)/docs

# Module directories
moddir           = $(SRCDIR)/$(modpath)
modsrcdir        = $(moddir)/src
modincdir        = $(moddir)/include
modcfgdir        = $(moddir)/config
moddatadir       = $(moddir)/data
modbuilddir      = $(moddir)/build
modlexdir        = $(moddir)/lex
moduidir         = $(moddir)/gui

################################################################################
# Output directory definitions
################################################################################
archdir          = $(BUILDDIR)/$(PLATFORM)-$(ARCH)
outputdir        = $(archdir)/$(BUILDTYPE)
libdir           = $(outputdir)/lib
bindir           = $(outputdir)/bin
objdir           = $(outputdir)/obj
mocdir           = $(outputdir)/moc
uidir            = $(outputdir)/ui
incdir           = $(outputdir)/include
sharedir         = $(outputdir)/share
docdir           = $(outputdir)/docs
tmpdir           = $(outputdir)/tmp
distdir          = $(outputdir)/dist
xsrcdir          = $(outputdir)/src

# Module-specific output directories
objmoddir        = $(objdir)/$(modpath)
incmoddir        = $(incdir)/$(modpath)
docmoddir        = $(docdir)/$(modpath)
mocmoddir        = $(mocdir)/$(modpath)
uimoddir         = $(uidir)/$(modpath)
sharemoddir      = $(sharedir)/apps/$(modname)
cfgmoddir        = $(sharemoddir)/config
datamoddir       = $(sharemoddir)/data
xsrcmoddir       = $(xsrcdir)/$(modpath)

################################################################################
# Qt related definitions
################################################################################
ifdef QTDIR
ifdef compile_qt
qtincdir    = $(QTDIR)/include
qt_includes = -I$(qtincdir)
UIC         = $(QTDIR)/bin/uic
MOC         = $(QTDIR)/bin/moc
qt_linking  = -lqt-mt -L$(QTDIR)/lib
endif
endif

################################################################################
# Version info strings
################################################################################
version_info =$(vermajor)
ifdef verminor
version_info := $(version_info).$(verminor)
ifdef verbuild
version_info := $(version_info).$(verbuild)
endif
endif
ifdef versuffix
version_info := $(version_info)$(versuffix)
endif

################################################################################
# Compiler options
################################################################################
OPTIMIZATION = -O2
CXXFLAGS     = -Wall

ifdef DEBUG
DEBUGFLAGS   = -g
OPTIMIZATION = 
export DEBUG
endif

################################################################################
# Distribution package definitions
################################################################################
defdistdirs  = build
defdistfiles = README.TXT configure Makefile

################################################################################
# End-of-file
################################################################################
