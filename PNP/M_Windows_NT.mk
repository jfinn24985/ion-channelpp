# Definition of for loop in local shell.

define doit
IF "$(2)" NEQ "" FOR %%W IN ( $(2) ) DO "$(subst /,\,$(MAKE))" -C %%W $(1)
endef

ifndef M_WINNT_MAK
M_WINNT_MAK:=1

# Standard Suffixes
sufobj:=.obj
sufexe:=.exe
sufdep:=.dep
suflib:=.lib
sufshr:=.dll
sufshrlink:=.lib
# Programs
MKDIR?=MD
COPY?=COPY /Y /B /V
RM:=-$(shell echo %COMSPEC%) /C DEL /F

# Path separator
SLASH?=\\

# END OF ONCE-ONLY.
endif

define do_install
install:: $(1) ; IF "" NEQ "$$($(2))" ( ( IF NOT EXIST $(if $(2),$$($(2)),$(3)) ( $$(MKDIR) $(if $(2),$$($(2)),$(3)) ) ) & $$(COPY) $(1) $(if $(2),$$($(2)),$(3))\$(1) )
endef

# Local Variables:
# mode: Makefile
# end:
