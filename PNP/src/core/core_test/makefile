##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target core_exec
CORE_TEST_CORE_EXECSRC :=  \
	core_test_suite.cpp \
	test_meta.cpp
CORE_TEST_CORE_EXEC_DINC := $(filter %$(sufdep), $(CORE_TEST_CORE_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(CORE_TEST_CORE_EXECSRC:.c=$(sufdep)))
core_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lboost_iostreams -lboost_program_options -lboost_system -lboost_random -lboost_serialization -lboost_unit_test_framework -luuid
core_exec$(sufexe) $(CORE_TEST_CORE_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
CORE_TEST_CORE_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
CORE_TEST_CORE_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

CORE_TEST_CORE_EXEC_OBJ+=$(filter %$(sufobj), $(CORE_TEST_CORE_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(CORE_TEST_CORE_EXEC_OBJ)
core_exec$(sufexe) : $(CORE_TEST_CORE_EXEC_SRC) $(CORE_TEST_CORE_EXEC_DINC)
-include $(CORE_TEST_CORE_EXEC_DINC)
$(eval $(call do_link_exe,core_exec$(sufexe),$(CORE_TEST_CORE_EXEC_OBJ)))

all :: core_exec$(sufexe)
TARGETS+=core_exec$(sufexe)

##END:target core_exec
