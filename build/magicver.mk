################################################################################
#    This file is part of the MagiC++ library.                                 #
#                                                                              #
#    Copyright (C) 1998-2002 Marko Grönroos <magi@iki.fi>                      #
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
# Build rule to update version info
################################################################################
version: version-makefile version-header

################################################################################
# Update version info makefile
################################################################################
version-makefile: $(tmpdir)
ifdef modversionfile
	perl -pe 's/(VERSION_BUILD *= *)([0-9]+)/$$1.($$2+1)/e;' \
	< $(modbuilddir)/$(modversionfile) > $(tmpdir)/tmp-version.mk
	mv $(tmpdir)/tmp-version.mk $(modbuilddir)/$(modversionfile)
endif

################################################################################
# Update version info header
################################################################################
version-header:
ifdef modversionheader
endif

