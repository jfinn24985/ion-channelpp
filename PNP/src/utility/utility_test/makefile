##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target utility_exec
UTILITY_TEST_UTILITY_EXECSRC :=  \
	estimater_tests.cpp \
	mathutil_test.cpp
UTILITY_TEST_UTILITY_EXEC_DINC := $(filter %$(sufdep), $(UTILITY_TEST_UTILITY_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(UTILITY_TEST_UTILITY_EXECSRC:.c=$(sufdep)))
utility_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_program_options -lboost_system -lboost_filesystem -lboost_serialization -lboost_unit_test_framework -luuid
utility_exec$(sufexe) $(UTILITY_TEST_UTILITY_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
UTILITY_TEST_UTILITY_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

UTILITY_TEST_UTILITY_EXEC_OBJ+=$(filter %$(sufobj), $(UTILITY_TEST_UTILITY_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(UTILITY_TEST_UTILITY_EXEC_OBJ)
utility_exec$(sufexe) : $(UTILITY_TEST_UTILITY_EXEC_SRC) $(UTILITY_TEST_UTILITY_EXEC_DINC)
-include $(UTILITY_TEST_UTILITY_EXEC_DINC)
$(eval $(call do_link_exe,utility_exec$(sufexe),$(UTILITY_TEST_UTILITY_EXEC_OBJ)))

all :: utility_exec$(sufexe)
TARGETS+=utility_exec$(sufexe)

##END:target utility_exec
##START:target lapack_exec
UTILITY_TEST_LAPACK_EXECSRC :=  \
	lapack_test.cpp
UTILITY_TEST_LAPACK_EXEC_DINC := $(filter %$(sufdep), $(UTILITY_TEST_LAPACK_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(UTILITY_TEST_LAPACK_EXECSRC:.c=$(sufdep)))
lapack_exec$(sufexe): LDFLAGS+= $(LAPACKLIB) $(BOOST_LIB) -lboost_random -lboost_system -lboost_timer -lboost_unit_test_framework
lapack_exec$(sufexe) $(UTILITY_TEST_LAPACK_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
UTILITY_TEST_LAPACK_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_lapack$(suflib)
UTILITY_TEST_LAPACK_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

UTILITY_TEST_LAPACK_EXEC_OBJ+=$(filter %$(sufobj), $(UTILITY_TEST_LAPACK_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(UTILITY_TEST_LAPACK_EXEC_OBJ)
lapack_exec$(sufexe) : $(UTILITY_TEST_LAPACK_EXEC_SRC) $(UTILITY_TEST_LAPACK_EXEC_DINC)
-include $(UTILITY_TEST_LAPACK_EXEC_DINC)
$(eval $(call do_link_exe,lapack_exec$(sufexe),$(UTILITY_TEST_LAPACK_EXEC_OBJ)))

all :: lapack_exec$(sufexe)
TARGETS+=lapack_exec$(sufexe)

##END:target lapack_exec
