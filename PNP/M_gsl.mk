#
# GNU BLAS/LAPACK library 
#
# Set GSLVERSION to version part of GSL directory (if present)
GSLVERSION?=
# Set GSLROOTDIR to possible locations for GSL
GSLROOTDIR+=/opt /opt/local /usr /usr/local

#
#  Should not be necessary to alter below here
#
ifndef GSLROOT
GSLROOT=$(strip $(foreach dir_,$(GSLROOTDIR),$(shell test -d $(dir_)/include/gsl$(GSLVERSION) && echo $(dir_))))
endif

ifeq ($(GSLROOT),)
GSL_INC=$(error "GNU Scientific library was requested but not found")
GSL_LIB=$(error "GNU Scientific library was requested but not found") 
LAPACKINC=$(error "GNU Scientific library was requested but not found")
LAPACKLIB=$(error "GNU Scientific library was requested but not found")
MATHVER=$(error "GNU Scientific library was requested but not found")
else
GSL_INC:=$(foreach dir,$(GSLROOT),-I$(dir)/include/gsl$(GSLVERSION))
GSL_LIB:=$(foreach dir,$(GSLROOT),-L$(dir)/lib) 
LAPACKLIB:=$(GSL_LIB) -lgsl -llapack
LAPACKINC:=-DUSE_GSL $(GSL_INC)
MATHVER:="call math_lib_version(libver)"
endif

# Local Variables:
# mode: Makefile
# end:
