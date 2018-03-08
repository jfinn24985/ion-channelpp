##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target trial_exec
TRIAL_TEST_TRIAL_EXECSRC :=  \
	trial_test.cpp
TRIAL_TEST_TRIAL_EXEC_DINC := $(filter %$(sufdep), $(TRIAL_TEST_TRIAL_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(TRIAL_TEST_TRIAL_EXECSRC:.c=$(sufdep)))
trial_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lboost_program_options -lboost_system -lboost_random -lboost_serialization -lboost_unit_test_framework -luuid
trial_exec$(sufexe) $(TRIAL_TEST_TRIAL_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
TRIAL_TEST_TRIAL_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
TRIAL_TEST_TRIAL_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
TRIAL_TEST_TRIAL_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
TRIAL_TEST_TRIAL_EXEC_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
TRIAL_TEST_TRIAL_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

TRIAL_TEST_TRIAL_EXEC_OBJ+=$(filter %$(sufobj), $(TRIAL_TEST_TRIAL_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(TRIAL_TEST_TRIAL_EXEC_OBJ)
trial_exec$(sufexe) : $(TRIAL_TEST_TRIAL_EXEC_SRC) $(TRIAL_TEST_TRIAL_EXEC_DINC)
-include $(TRIAL_TEST_TRIAL_EXEC_DINC)
$(eval $(call do_link_exe,trial_exec$(sufexe),$(TRIAL_TEST_TRIAL_EXEC_OBJ)))

all :: trial_exec$(sufexe)
TARGETS+=trial_exec$(sufexe)

##END:target trial_exec
