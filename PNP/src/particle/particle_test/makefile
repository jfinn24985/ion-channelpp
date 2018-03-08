##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target particle_exec
PARTICLE_TEST_PARTICLE_EXECSRC :=  \
	particle_test.cpp
PARTICLE_TEST_PARTICLE_EXEC_DINC := $(filter %$(sufdep), $(PARTICLE_TEST_PARTICLE_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PARTICLE_TEST_PARTICLE_EXECSRC:.c=$(sufdep)))
particle_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lboost_program_options -lboost_system -lboost_random -lboost_serialization -lboost_unit_test_framework -luuid
particle_exec$(sufexe) $(PARTICLE_TEST_PARTICLE_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
PARTICLE_TEST_PARTICLE_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
PARTICLE_TEST_PARTICLE_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
PARTICLE_TEST_PARTICLE_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
PARTICLE_TEST_PARTICLE_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

PARTICLE_TEST_PARTICLE_EXEC_OBJ+=$(filter %$(sufobj), $(PARTICLE_TEST_PARTICLE_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(PARTICLE_TEST_PARTICLE_EXEC_OBJ)
particle_exec$(sufexe) : $(PARTICLE_TEST_PARTICLE_EXEC_SRC) $(PARTICLE_TEST_PARTICLE_EXEC_DINC)
-include $(PARTICLE_TEST_PARTICLE_EXEC_DINC)
$(eval $(call do_link_exe,particle_exec$(sufexe),$(PARTICLE_TEST_PARTICLE_EXEC_OBJ)))

all :: particle_exec$(sufexe)
TARGETS+=particle_exec$(sufexe)

##END:target particle_exec
