#
# Self Optimising BLAS/LAPACK library 
#
# Set ATLASVERSION to version part of ATLAS directory (if present)
ATLASVERSION?=
# Set ATLASROOTDIR to possible locations for ATLAS
ATLASROOTDIR+=/opt /opt/local /usr /usr/local

#
#  Should not be necessary to alter below here
#

ifndef ATLASROOT
ATLASROOT:=$(strip $(foreach dir_,$(ATLASROOTDIR),$(shell test -d $(dir_)/include/atlas && echo $(dir_))))
endif
ATLASROOTDIR:=

ifeq ($(ATLASROOT),)
ATLAS_INC=$(error "Atlas library was requested but not found")
ATLAS_LIB=$(error "Atlas library was requested but not found") 
LAPACKLIB=$(error "Atlas library was requested but not found")
LAPACKINC=$(error "Atlas library was requested but not found")
MATHVER:=$(error "Atlas library was requested but not found")
else
ATLAS_INC:=-I$(ATLASROOT)/include/atlas
# ATLASLIB:=$(firstword $(foreach dir_,lib/atlas$(ATLASVERSION) lib,$(shell test -f $(ATLASROOT)/$(dir_)/libatlas.a && echo $(ATLASROOT)/$(dir_))))
ATLASLIB:=-L$(ATLASROOT)/lib
ATLASLIB_STATIC:=$(ATLASROOT)/lib/atlas-base
LAPACKINC:=-DUSE_ATLAS $(ATLAS_INC)
MATHVER:="call math_lib_version(libver)"
ifdef static_build
# Use LAPACK variable for static libs and object files
LAPACKLIB_STATIC:=$(ATLASLIB_STATIC)/liblapack_atlas.a $(ATLASLIB_STATIC)/libf77blas.a $(ATLASLIB_STATIC)/libcblas.a $(ATLASLIB_STATIC)/libatlas.a
else
# Use LAPACKLIB for dynamic libs
LAPACKLIB:=-L$(ATLASLIB) -lblas3gf -llapack3gf
endif
endif


# Local Variables:
# mode: Makefile
# end: