#
# Intel Maths Kernel: BLAS/LAPACK library 
#
# Set MKLVERSION to version part of MKL directory (if present)
MKLVERSION?=

# test for absence of MKL_LIB
ifeq ($(MKL_LIB),)
# Set MKLROOTDIR to possible locations for MKL
MKLROOTDIRS+=/opt/intel/mkl/$(MKLVERSION) /usr/local/intel/Frameworks/mkl

#
#  Should not be necessary to alter below here
#

ifndef MKLROOT
MKLROOT:=$(strip $(firstword $(foreach dir_,$(MKLROOTDIRS),$(shell test -e $(dir_)/include/mkl.h && echo $(dir_)))))
endif
else
# Assume if MKL_LIB set, then root is the parent directory.
MKLROOT:=$(dir $(MKL_LIB))
endif

ifeq ($(MKLROOT),)
MKL_INC=$(error "Intel Maths library was requested but not found")
MKL_LIB=$(error "Intel Maths library was requested but not found")
LAPACKLIB=$(error "Intel Maths library was requested but not found")
LAPACKINC=$(error "Intel Maths library was requested but not found")
MATHVER=$(error "Intel Maths library was requested but not found")
else
ifeq ($(MKL_INC),)
MKL_INC:=$(MKLROOT)/include
endif
ifeq ($(MKL_LIB),)
LIBSUBDIRS=intel64 universal em64t
MKL_LIB:=$(strip $(foreach dir_,$(LIBSUBDIRS),$(shell test -d $(MKLROOT)/lib/$(dir_) && echo $(MKLROOT)/lib/$(dir_))))
endif

LAPACKINC:=-DUSE_MKL -I$(MKL_INC)
LAPACKLIB:=-L$(MKL_LIB) -lmkl_intel_lp64 -lmkl_core -lmkl_sequential
MKLROOT_STATIC:=$(strip $(firstword $(foreach dir_,universal em64t x86_64,$(shell test -e $(MKLROOT)/lib/$(dir_)/libmkl_core.a && echo $(dir_)))))
LAPACKLIB_STATIC:= $(MKLROOT)/lib/$(MKLROOT_STATIC)/libmkl_intel_lp64.a $(MKLROOT)/lib/$(MKLROOT_STATIC)/libmkl_core.a  $(MKLROOT)/lib/$(MKLROOT_STATIC)/libmkl_sequential.a
#LAPACKLIB_STATIC:= $(MKLROOT)/lib/$(MKLROOT_STATIC)/libmkl_intel_lp64.a $(MKLROOT)/lib/$(MKLROOT_STATIC)/libmkl_core.a  $(MKLROOT)/lib/$(MKLROOT_STATIC)/libmkl_intel_thread.a -lpthread
MATHVER:="call mkl_get_version_string(libver)"
endif

# Local Variables:
# mode: Makefile
# end:

