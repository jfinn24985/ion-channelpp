#
# Compiler definitions
#
# set INTELVER to version part of icc name, usually blank
INTELVER?=
# Set INTELROOTDIR to possible locations for INTEL
INTELROOTDIR+=/opt/bin /opt/local/bin /usr/bin /usr/local/bin
# you can also set INTELROOT to empty if in path
# INTELROOT:=

#
#  Should not be necessary to alter below here
#
ifndef INTELROOT
INTELROOT:=$(strip $(foreach dir_,$(INTELROOTDIR),$(shell test -f $(dir_)/icc$(INTELVER) && echo $(dir_))))
endif
INTELROOTDIR:=
# -m64
#
# Compiler characteristics
# use -DHAVE_LLRINT=1 if you get "error: function llrint(double) already declared"
# use -DHAVE_UNIQPTR=1 if std::unique_ptr is available
CXXFLAGS+=-DHAVE_LLRINT=1

#
# Should not need to change below here
#
CC=$(INTELROOT:=/)icc$(INTELVER)
CXX=$(INTELROOT:=/)icpc$(INTELVER)
CCVERSION=$(CXX)$(shell $(CXX) --version)
CCTARGET=$(shell $(CXX) -dumpmachine)
FC=$(INTELROOT:=/)ifort$(INTELVER)
FCVERSION=$(FC)$(shell $(FC) --version)
FCTARGET=$(shell $(FC) -dumpmachine)

BASE_FLAGS:=-fp-model strict -fp-model except 

ifeq ($(VARIANT),DEBUG)

BASE_FLAGS:=-g -gdwarf-2 -O0
FORTRANFLAGS+=-fpp -check bounds -check uninit -ftrapuv

else
# note that Intel's -fast option cannot be used with scientific code.

# set to the optimisation flag for your architecture,
BASE_FLAGS+=-xHost

# x86 SSE arch flags
TEST:=$(shell gcc -dM -E -xc /dev/null)
SSEFLAGS:=$(if $(findstring __SSE2__,$(TEST)),-axSSE2)
SSEFLAGS:=$(if $(findstring __SSE3__,$(TEST)),-axSSE3)
SSEFLAGS:=$(if $(findstring __SSSE3__,$(TEST)),-axSSSE3)
SSEFLAGS:=$(if $(findstring __SSE4.1__,$(TEST)),-axSSE4.1)
TEST:=

BASE_FLAGS+=$(SSEFLAGS) -g -gdwarf-2 -O2 -fpp -no-ansi-alias -unroll -mkl=sequential -mp1 -ipo
endif

OBJ+=*__genmod.f90 *__genmod.mod

CCFLAGS=$(BASE_FLAGS) -Wall
CCCFLAGS=$(BASE_FLAGS) -Wall
FORTRANFLAGS=$(BASE_FLAGS) -warn all
# -pad

ifndef serial_build
OPENMP:=-openmp
else
OPENMP:=
endif

ifdef static_build
LDFLAGS+=-cxxlib
else
LDFLAGS+=-shared-intel -shared-libgcc -cxxlib
endif


STATIC_BEGIN=
STATIC_END=

# Local Variables:
# mode: Makefile
# end:
