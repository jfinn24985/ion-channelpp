#
## CULA BLAS/LAPACK library 
#
## Set CULAVERSION to version part of CULA directory (if present)
CULAVERSION?=
# Set CULAROOTDIR to possible locations for CULA
CULAROOTDIR+=/opt /opt/local /usr /usr/local
#
#
#  Should not be necessary to alter below here
#
ifndef CULAROOT
CULAROOT=$(strip $(foreach dir_,$(CULAROOTDIR),$(shell test -f $(dir_)/cula/include/cula.h && echo $(dir_)/cula)))
endif
#
ifeq ($(CULAROOT),)
CULA_INC=$(error "CULA was requested but not found")
CULA_LIB=$(error "CULA was requested but not found") 
LAPACKINC=$(error "CULA was requested but not found")
LAPACKLIB=$(error "CULA was requested but not found")
MATHVER=$(error "CULA was requested but not found")
else
CULA_INC:=$(foreach dir,$(CULAROOT),-I$(dir)/include)
CULA_LIB:=$(foreach dir,$(CULAROOT),-L$(dir)/lib -L$(dir)/lib64) 
LAPACKLIB:=$(CULA_LIB) -lcula_core -lcula_lapack -lcublas -lcula_lapack_link -lcudart
LAPACKINC:=-DUSE_CULA $(CULA_INC)
MATHVER:="call math_lib_version(libver)"
endif

# Local Variables:
# mode: Makefile
# end: