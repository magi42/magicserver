################################################################################
#                                                                              #
#    magicdist.mk: Build framework targets for making source distribution      #
#                  package.                                                    #
#                                                                              #
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
# Definitions for top-level make
################################################################################
ifndef modpath
fullpackagename = $(packagename)-$(vermajor).$(verminor).$(verbuild)$(versuffix)
packagedir      = $(distdir)/$(fullpackagename)
export packagedir

ifdef USE_GZ
packagefilename = $(fullpackagename).src.tar.gz
tarparams       = zcf
else
packagefilename = $(fullpackagename).src.tar.bz2
tarparams       = jcf
endif
packagefullname = $(distdir)/$(packagefilename)

fullbuildfiles  = $(foreach file,$(buildfiles),build/$(file))
fulldistdirs    = $(foreach dir,$(defdistdirs) $(distdirs),$(packagedir)/$(dir))
fulldistfiles   = $(foreach file,$(defdistfiles) $(distfiles) $(fullbuildfiles),$(packagedir)/$(file))

################################################################################
# Top-level directory
################################################################################
dist:	$(packagedir) \
	dist_dirs \
	$(fulldistfiles) \
	make_dist_modules \
	$(packagefullname) \
	$(extra_dist_targets)

$(packagedir): FORCE
	rm -rf $(packagedir)
	mkdir -p $(packagedir)

$(packagefullname): FORCE
	tar $(tarparams) $(packagefullname) -C $(distdir) $(fullpackagename)
	rm -rf $(packagedir)

FORCE:

else
################################################################################
# Module-level directory
################################################################################
packagemoddir = $(packagedir)/$(modpath)

# Determine directories to create
distdirs   =	
ifdef docfiles
distdirs  +=	docs
endif
ifdef buildfiles
distdirs  +=	build
endif
ifdef sources
distdirs  +=	src
endif
ifdef headers
distdirs  +=	include include/$(headersubdir)
endif
ifdef configfiles
distdirs  +=	config
endif
ifdef datafiles
distdirs  +=	data
endif
fulldistdirs  = $(foreach dir,$(distdirs),$(packagedir)/$(modpath)/$(dir))

# Determine files to copy
distfiles  =	$(modname).mk $(EXTRA_DIST_FILES) \
		$(foreach file,$(docfiles),docs/$(file)) \
		$(foreach file,$(configfiles),config/$(file)) \
		$(foreach file,$(datafiles),data/$(file)) \
		$(foreach file,$(sources),src/$(file))
fulldistfiles = $(foreach file,$(distfiles),$(packagedir)/$(modpath)/$(file)) \
		$(foreach file,$(headers),$(packagedir)/$(modpath)/include/$(headersubdir)/$(file)) \
		$(foreach file,$(shared_headers),$(packagedir)/include/$(headersubdir)/$(file))

dist:	mod_dir \
	dist_dirs \
	$(fulldistfiles) \
	make_dist_modules

$(packagemoddir)/%: $(SRCDIR)/$(modpath)/%
	cp $< $@

mod_dir:
	mkdir -p $(packagemoddir)

endif

################################################################################
# Common rules
################################################################################

# Copy files
$(packagedir)/%: $(SRCDIR)/%
	cp $< $@

# Make extra directories
dist_dirs:
ifdef distdirs
	mkdir -p $(fulldistdirs)
endif

# Make sub-modules recursively
make_dist_modules:
	$(makedist_cmds)

