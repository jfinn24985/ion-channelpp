#
# M_sys.mk
#
##################################
# Settings for current environment
##################################

OS ?= unix
ifeq ($(origin COMP),default)
COMP = gcc
endif
VARIANT ?= DEBUG

# Preprocessor flags (defines and include directories)
#  CPPFLAGS
# C language flags
#  CCFLAGS
# C++ language flags
#  CCCFLAGS
# Fortran language flags
#  FORTRANFLAGS
# Linker flags
#  LDFLAGS

# Optimisation/debugging flags
# are added depending on VARIANT

##########################
## Include compiler and OS
##########################
-include $(ROOTDIR)$(or $(SLASH),/)M_local.mk
include $(ROOTDIR)$(or $(SLASH),/)M_$(OS).mk
include $(ROOTDIR)$(or $(SLASH),/)M_$(COMP).mk

##############
## Unify Vars
##############
CFLAGS = $(CCFLAGS)
CXXFLAGS = $(CCCFLAGS)
FFLAGS = $(FORTRANFLAGS)
FC = $(FORTRAN)

#############################################
##  Define standard directory descent targets
#############################################
export
unexport dirs

all:: ; $(call doit,all,$(dirs))

install:: ; $(call doit,install,$(dirs))

run_tests:: ; $(call doit,run_tests,$(dirs))

distclean: doclean
	$(call doit,distclean,$(dirs))
	$(RM) core core.* $(TARGETS) *~ $(if $(sufdep),*$(sufdep))

clean: doclean
	$(call doit,clean,$(dirs))

doclean: 
	$(if $(USEROBJ),$(RM) $(USEROBJ))

#############################
##  Default staging locations
#############################
PREFIX=$(ROOTDIR)$(SLASH)stage
BINDIR?=$(PREFIX)$(SLASH)bin
LIBDIR?=$(PREFIX)$(SLASH)lib
INCDIR?=$(PREFIX)$(SLASH)include
DATADIR?=$(PREFIX)$(SLASH)share
DOCDIR?=$(DATADIR)$(SLASH)doc
MANDIR?=$(DATADIR)$(SLASH)man
HTMLDIR?=$(DATADIR)$(SLASH)html
################################################
##  Example link library setup with alternatives
################################################
##  
##  # Make this a one time only definition
##  
##  ifndef USE_XML_MAK
##  USE_XML_MAK:=1
##  
##  ifdef ($(XMLLIB))
##  ifeq ($(XMLLIB),EXPAT)
##  CPPFLAGS+=-DEXPAT
##  LDFLAGS+=-lexpat
##  endif
##  
##  ifeq ($(XMLLIB),XML2)
##  CPPFLAGS+=-DXML2 `xml2-config --cflags`
##  LDFLAGS+=`xml2-config --libs`
##  endif
##  
##  endif # XMLLIB
##  
##  endif # USE_XML_MAK

# Local Variables:
# mode: Makefile
# end:
