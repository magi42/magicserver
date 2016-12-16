################################################################################
#    This file is part of the MagiCServer++.                                   #
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
# Define root directory of the source tree
################################################################################
export SRCDIR ?= .

################################################################################
# Package and version info
################################################################################
export packagename = magicserver++
export vermajor    = 0
export verminor    = 1
#export verbuild    = 1
export versuffix   = beta1

################################################################################
# Extra files and directories
################################################################################
distdirs   =	docs docs/images
distfiles  =	docs/magicserver-devguide.sxw docs/magicserver-devguide.pdf \
		docs/images/flowcharts-listener1.png \
		docs/images/flowcharts-listener2.png \
		docs/images/flowcharts.sxd \
		docs/images/msrvsample-worker-1-crop.png \
		docs/images/msrvsample-worker-1.png \
		docs/images/msrvsample-worker-2-crop.png \
		docs/images/msrvsample-worker-2.png
buildfiles =	conf-reqs.sh \
		magicdef.mk magiccmp.mk magicdist.mk magictop.mk magicver.mk

################################################################################
# Include build framework
################################################################################
include $(SRCDIR)/build/magicdef.mk

extra_targets      = copy_devguide
extra_dist_targets = dist_devguide

################################################################################
# Recursively call sub-makes for modules
################################################################################
makemodules = libmsrv examples

################################################################################
# Include build rules
################################################################################
include $(SRCDIR)/build/magictop.mk

################################################################################
# Copy documentation
################################################################################
devguidetarget = $(packagename)-devguide-$(version_info).pdf

dist_devguide: $(distdir)/$(devguidetarget)

$(distdir)/$(devguidetarget): $(docdir)/$(devguidetarget)
	cp -f $< $@

copy_devguide: $(docdir)/$(devguidetarget)

$(docdir)/$(devguidetarget): $(grpdocdir)/magicserver-devguide.pdf
	cp -f $< $@

