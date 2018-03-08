# Set BOOSTROOTDIRS to possible locations for Boost
BOOSTROOTDIRS+=/opt/local /usr/local /usr
LIBSUBDIRS=stage/lib intel64 universal em64t lib64 lib
#
#  Should not be necessary to alter below here
#

ifndef BOOSTROOT
BOOSTROOT:=$(strip $(firstword $(foreach dir_,$(BOOSTROOTDIRS),$(shell test -e $(dir_)/boost/version.hpp && echo $(dir_)))))
endif

ifeq ($(BOOSTROOT),)
BOOSTROOT:=$(strip $(firstword $(foreach dir_,$(BOOSTROOTDIRS),$(shell test -e $(dir_)/include/boost/version.hpp && echo $(dir_)))))

ifeq ($(BOOSTROOT),)
BOOST_INC=$(error "BOOST library was requested but not found 1")
BOOST_LIB=$(error "BOOST library was requested but not found 2")
else
BOOST_INC:=-I$(BOOSTROOT)/include
BOOST_LIB:=$(strip $(foreach dir_,$(LIBSUBDIRS),$(shell test -d $(BOOSTROOT)/$(dir_) && echo -L$(BOOSTROOT)/$(dir_))))
endif

else

BOOST_INC:=$(BOOSTROOT)
BOOST_LIB:=$(strip $(foreach dir_,$(LIBSUBDIRS),$(shell test -d $(BOOSTROOT)/$(dir_) && echo -L$(BOOSTROOT)/$(dir_))))
endif

ifeq ($(BOOST_LIB),-L)
BOOST_LIB:=$(SPACE)
endif

CCCFLAGS+=-I$(BOOST_INC)
LDFLAGS+=$(BOOST_LIB)

# Local Variables:
# mode: Makefile
# end:
