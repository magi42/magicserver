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
# Definitions and rules ror recursive make
#
# This can be called both from a top-level or a module-level recursive make
################################################################################

# Following module path is necessary for lower-level recursive sub-makes
ifdef modpath
makemodules_prefix = $(modpath)/
endif

################################################################################
# Define make commands for recursively making each primary target
################################################################################

# Put each command on separate line
define defline
$(1)

endef

recursive_make = $(foreach module,$(makemodules), \
	$(call defline,$(MAKE) -f $(SRCDIR)/$(makemodules_prefix)$(module)/$(module).mk $(1)))

makemodules_cmds   = $(call recursive_make,)
makeclean_cmds     = $(call recursive_make, clean)
makedeps_cmds      = $(call recursive_make, deps)
makedox_cmds       = $(call recursive_make, dox)
makeinstall_cmds   = $(call recursive_make, install)
makeuninstall_cmds = $(call recursive_make, uninstall)
makedist_cmds      = $(call recursive_make, dist)

################################################################################
# Top-level make rules
################################################################################

ifndef sources
# Sourceless modules
all: makemodules $(extra_targets)

else
# Modules that need compiling
all: $(extra_targets) compile-all makemodules 
endif

makemodules:
	$(makemodules_cmds)

# Clean, subdirs first
clean: clean-subdirs make-clean
clean-subdirs:
	$(makeclean_cmds)

################################################################################
# Dependencies
# - Make them only if source or header files are available
################################################################################
ifdef sources
depsdeps = make-deps
endif
ifdef headers
depsdeps = make-deps
endif
deps: $(depsdeps)
	$(makedeps_cmds)

################################################################################
# Install
################################################################################
install: make-install
	$(makeinstall_cmds)

uninstall: make-uninstall
	$(makeuninstall_cmds)

################################################################################
# Make source documentation
################################################################################
ifdef makedox
dox: make-dox
	$(makedox_cmds)
else
dox:
	$(makedox_cmds)
endif

################################################################################
# Dummy definitions for top-level makefiles
################################################################################
ifndef __magiccmp__
make-clean:
	@-rmdir $(objmoddir) $(docmoddir)

make-deps:

make-install:

make-uninstall:
endif # ifndef __magiccmp__

################################################################################
# Top-level make rules for 
################################################################################

# Building source distribution packages
include $(SRCDIR)/build/magicdist.mk
