####################################
##  Definitions for GNU g++ compiler
####################################
ifndef M_GCC_MAK
M_GCC_MAK:=1

#
# Configure where your GCC compilers are
#
# set GCCVER to version part of gcc name (if present)
# (Here is minimum required version)
GCCVER?=-4.4
# Set GCCROOTDIR to possible locations for GCC
GCCROOTDIR+=/opt/bin /opt/local/bin /usr/bin /usr/local/bin
# you can also set GCCROOT to empty if in path
# GCCROOT:=

#
#  Should not be necessary to alter below here
#
ifndef GCCROOT
GCCROOT:=$(strip $(firstword $(foreach dir_,$(GCCROOTDIR),$(shell test -f $(dir_)/gcc$(GCCVER) && echo $(dir_)))))
GFCROOT:=$(strip $(firstword $(foreach dir_,$(GCCROOTDIR),$(shell test -f $(dir_)/gfortran$(GCCVER) && echo $(dir_)))))
endif
GCCROOTDIR:=

##
## Define compiler variables once only.
## 
CC?=$(GCCROOT:=/)gcc$(GCCVER) -m64
CCC?=$(GCCROOT:=/)g++$(GCCVER) -m64
CXX=$(CCC)
FORTRAN?=$(GFCROOT:=/)gfortran$(GCCVER) -m64
CPPFLAGS?=

# compiler version information
CCVERSION:=$(shell $(CCC) -dumpversion)
CCTARGET:=$(shell $(CCC) -dumpmachine)
FCVERSION:=$(shell $(FORTRAN) -dumpversion)
FCTARGET:=$(shell $(FORTRAN) -dumpmachine)

BASE_FLAGS:=-Wall -ggdb -gdwarf-2 -pipe -fmessage-length=0 -ftrapping-math

# --------------------------------------------------
#
# User selected variant
#
# --------------------------------------------------
ifeq ($(VARIANT),DEBUG)
BASE_FLAGS+=-O0 -DDEBUG=1
CCFLAGS+=-pedantic
CCCFLAGS+=
#-Weffc++
FORTRANFLAGS+= -fno-automatic -ffpe-trap=invalid,zero -fbounds-check -Wconversion -mieee-fp -fbounds-check
else
BASE_FLAGS+=-O2 -march=native -DDEBUG=0 -fno-common -funroll-loops -finline -finline-limit=600 -fno-math-errno -fprefetch-loop-arrays -finline-functions -mtune=generic $(GNUOPTFLAGS)
FORTRANFLAGS+= -ffpe-trap=invalid,zero

# --------------------------------------------------
#
# x86 arch SSE flags in non-debug
#
# --------------------------------------------------
ifndef bad_mmx_register
MFPMATH=-mfpmath=sse,387 $(SSEFLAGS)
else
MFPMATH=-mfpmath=sse $(SSEFLAGS)
endif

TEST:=$(shell gcc -dM -E -xc /dev/null)

BASE_FLAGS+=$(if $(findstring __SSE_MATH__,$(TEST)),$(MFPMATH))
SSEFLAGS:=$(if $(findstring __MMX__,$(TEST)),-mmmx)
SSEFLAGS:=$(if $(findstring __SSE__,$(TEST)),-msse)
SSEFLAGS:=$(if $(findstring __SSE2__,$(TEST)),-msse2)
TEST:=$(shell cat /proc/cpuinfo)
SSEFLAGS:=$(if $(findstring sse3,$(TEST)),-msse3)
SSEFLAGS:=$(if $(findstring sse3,$(TEST)),-mssse3)
SSEFLAGS:=$(if $(findstring sse4_1,$(TEST)),-msse4.1)
SSEFLAGS:=$(if $(findstring sse4_2,$(TEST)),-msse4.2)
BASE_FLAGS+=$(SSEFLAGS)
TEST:=

#
# Unscientific floating-math optimisations
#
DEATH= -ffast-math -floop-interchange -floop-block -floop-strip-mine -ftree-loop-distribution -fgcse-sm -fgcse-las -funsafe-loop-optimizations -Wunsafe-loop-optimizations

endif

CCFLAGS+= $(BASE_FLAGS) -Wno-format -Wno-trigraphs 
CCCFLAGS+= $(BASE_FLAGS) -Wno-format -fvisibility-inlines-hidden -Wno-trigraphs 
FORTRANFLAGS+= $(BASE_FLAGS) -fmodulo-sched

# --------------------------------------------------
#
# Compiler version settings
#
# --------------------------------------------------
CCCFLAGS+= -std=c++11 -DHAVE_LLRINT=1 -DHAVE_UNIQPTR=1 -DHAVE_LRINT=1
FORTRANFLAGS+= -cpp -Warray-temporaries -std=f2003 -fall-intrinsics -Wunderflow -fexternal-blas
# -ftree-vectorize
# -flto

LDFLAGS?=
SHRFLAGS?=-fpic

FORTRANLIBS:=-lgfortran


ifndef serial_build
OPENMP:=-fopenmp
else
OPENMP:=
endif

ifdef static_build
LDFLAGS+=-static-libgcc -static-libgfortran ./libstdc++.a
STATIC_DEFSH?=[ -f libstdc++.a ] || ln -s `$(CXX) -print-file-name=libstdc++.a` .
else
ifdef mac_os
SUFLIB=dylib
else
SUFLIB=so
endif
stdc++_lib_dir=$(dir $(shell $(CC) -print-file-name=libstdc++.$(SUFLIB)))
LDFLAGS+=$(foreach dir_,x86_64 .,$(shell test -d $(stdc++_lib_dir)/$(dir_) && echo -L$(stdc++_lib_dir)/$(dir_))) -lstdc++
endif

STATIC_BEGIN=-Wl,"-("
STATIC_END=-Wl,"-)"


endif
# END ONCE-ONLY

define _makedep_
%$(sufdep): %$(1)
	set -e; $$(CCC) -MM $$(CPPFLAGS) $(CCCFLAGS) $$< \
	| sed 's/\($$*\)$$(sufobj)[ :]*/\1$$(sufobj) $$@ : /g' > $$@;\
	[ -s $$@ ] || rm -f $$@
endef

$(foreach suff,.c .C .cc .cpp .cxx,$(eval $(call _makedep_,$(suff))))
_makedep_:=

ifeq ($(findstring 4.2,$(CCVERSION)),4.2)
define do_link_exe
$(1) : $(2); $$(LINK.cpp) $$(OUTPUT_OPTIONS) -o $(1) $(2)
endef
define do_link_shr
$(1) : CCFLAGS+=$(SHRFLAGS)
$(1) : CCCFLAGS+=$(SHRFLAGS)
$(1) : $(2); $$(LINK.cpp) $$(OUTPUT_OPTIONS) -shared -Wl,-soname,$(1) -o $(1) $(2)
endef
else

define do_link_exe
$(1) : $(2); $$(LINK.cpp) $$(OUTPUT_OPTIONS) -o $(1) -Wl,--start-group $(2) -Wl,--end-group
endef
define do_link_shr
$(1) : CCFLAGS+=$(SHRFLAGS)
$(1) : CCCFLAGS+=$(SHRFLAGS)
$(1) : $(2); $$(LINK.cpp) $$(OUTPUT_OPTIONS) -shared -Wl,-soname,$(1) -o $(1) -Wl,--start-group $(2) -Wl,--end-group
endef
endif

# Local Variables:
# mode: Makefile
# end:
