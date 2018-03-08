##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target observable_exec
OBSERVABLE_TEST_OBSERVABLE_EXECSRC :=  \
	observable_test_suite.cpp
OBSERVABLE_TEST_OBSERVABLE_EXEC_DINC := $(filter %$(sufdep), $(OBSERVABLE_TEST_OBSERVABLE_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(OBSERVABLE_TEST_OBSERVABLE_EXECSRC:.c=$(sufdep)))
observable_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_program_options -lboost_system -lboost_filesystem -lboost_serialization -lboost_unit_test_framework -luuid
observable_exec$(sufexe) $(OBSERVABLE_TEST_OBSERVABLE_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ+=$(filter %$(sufobj), $(OBSERVABLE_TEST_OBSERVABLE_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ)
observable_exec$(sufexe) : $(OBSERVABLE_TEST_OBSERVABLE_EXEC_SRC) $(OBSERVABLE_TEST_OBSERVABLE_EXEC_DINC)
-include $(OBSERVABLE_TEST_OBSERVABLE_EXEC_DINC)
$(eval $(call do_link_exe,observable_exec$(sufexe),$(OBSERVABLE_TEST_OBSERVABLE_EXEC_OBJ)))

all :: observable_exec$(sufexe)
TARGETS+=observable_exec$(sufexe)

##END:target observable_exec
##START:target output_exec
OBSERVABLE_TEST_OUTPUT_EXECSRC :=  \
	output_test_suite.cpp
OBSERVABLE_TEST_OUTPUT_EXEC_DINC := $(filter %$(sufdep), $(OBSERVABLE_TEST_OUTPUT_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(OBSERVABLE_TEST_OUTPUT_EXECSRC:.c=$(sufdep)))
output_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lzip -lboost_program_options -lboost_system -lboost_serialization -lboost_unit_test_framework -luuid
output_exec$(sufexe) $(OBSERVABLE_TEST_OUTPUT_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)
OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_zip_sink$(suflib)

OBSERVABLE_TEST_OUTPUT_EXEC_OBJ+=$(filter %$(sufobj), $(OBSERVABLE_TEST_OUTPUT_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(OBSERVABLE_TEST_OUTPUT_EXEC_OBJ)
output_exec$(sufexe) : $(OBSERVABLE_TEST_OUTPUT_EXEC_SRC) $(OBSERVABLE_TEST_OUTPUT_EXEC_DINC)
-include $(OBSERVABLE_TEST_OUTPUT_EXEC_DINC)
$(eval $(call do_link_exe,output_exec$(sufexe),$(OBSERVABLE_TEST_OUTPUT_EXEC_OBJ)))

all :: output_exec$(sufexe)
TARGETS+=output_exec$(sufexe)

##END:target output_exec
