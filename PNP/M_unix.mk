 # Definition of for loop in local shell.
define doit
$(if $(2),STATUS=0 ; for subdir in $(2); do $(MAKE) -C $${subdir}  $(1) || STATUS=$$? ; done ; exit $${STATUS})
endef

# MAKE MOST VARIABLES ONCE-ONLY
ifndef M_UNIX_MAK
M_UNIX_MAK:=1

# Flags for the install targets.
BINIFLAGS?=-m 755
FILEIFLAGS?=-m 644
INSTALL?=install

# Standard suffixes
sufobj:=.o
sufexe:=
suflib:=.a
sufshr:=.so
sufshrlink:=.so
sufdep:=.d

# Path separator
SLASH?=/

# END OF ONCE-ONLY.
endif

define do_install
install:: $(1) ; $$(INSTALL) -d $(if $(2),$$($(2)),$(3)) ; $$(INSTALL) $$($(if BIN==$(2),BIN,FILE)IFLAGS) $(1) $(if $(2),$$($(2)),$(3))/$(1)
endef

define do_archive
$(1): $(2) ; $$(AR) $$(ARFLAGS) $(1) $(2)
endef

# Local Variables:
# mode: Makefile
# end:
