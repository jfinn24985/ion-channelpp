##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target evaluator_exec
EVALUATOR_TEST_EVALUATOR_EXECSRC :=  \
	evaluator_fixture.cpp \
	evaluator_test.cpp
EVALUATOR_TEST_EVALUATOR_EXEC_DINC := $(filter %$(sufdep), $(EVALUATOR_TEST_EVALUATOR_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(EVALUATOR_TEST_EVALUATOR_EXECSRC:.c=$(sufdep)))
evaluator_exec$(sufexe): LDFLAGS+= $(LAPACKLIB) $(BOOST_LIB) -lboost_date_time -lboost_filesystem $(FORTRANLIBS) -lboost_program_options -lboost_system -lboost_random -lboost_serialization -lboost_unit_test_framework -luuid
evaluator_exec$(sufexe) $(EVALUATOR_TEST_EVALUATOR_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) $(LAPACKINC) -DBOOST_TEST_DYN_LINK 
EVALUATOR_TEST_EVALUATOR_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
EVALUATOR_TEST_EVALUATOR_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
EVALUATOR_TEST_EVALUATOR_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_fevaluator$(suflib)
EVALUATOR_TEST_EVALUATOR_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
EVALUATOR_TEST_EVALUATOR_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_lapack$(suflib)
EVALUATOR_TEST_EVALUATOR_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
EVALUATOR_TEST_EVALUATOR_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

EVALUATOR_TEST_EVALUATOR_EXEC_OBJ+=$(filter %$(sufobj), $(EVALUATOR_TEST_EVALUATOR_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(EVALUATOR_TEST_EVALUATOR_EXEC_OBJ)
evaluator_exec$(sufexe) : $(EVALUATOR_TEST_EVALUATOR_EXEC_SRC) $(EVALUATOR_TEST_EVALUATOR_EXEC_DINC)
-include $(EVALUATOR_TEST_EVALUATOR_EXEC_DINC)
$(eval $(call do_link_exe,evaluator_exec$(sufexe),$(EVALUATOR_TEST_EVALUATOR_EXEC_OBJ)))

all :: evaluator_exec$(sufexe)
TARGETS+=evaluator_exec$(sufexe)

##END:target evaluator_exec
