##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target geometry_exec
GEOMETRY_TEST_GEOMETRY_EXECSRC :=  \
	geometry_test.cpp
GEOMETRY_TEST_GEOMETRY_EXEC_DINC := $(filter %$(sufdep), $(GEOMETRY_TEST_GEOMETRY_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(GEOMETRY_TEST_GEOMETRY_EXECSRC:.c=$(sufdep)))
geometry_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_program_options -lboost_system -lboost_filesystem -lboost_serialization -lboost_unit_test_framework
geometry_exec$(sufexe) $(GEOMETRY_TEST_GEOMETRY_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
GEOMETRY_TEST_GEOMETRY_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
GEOMETRY_TEST_GEOMETRY_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
GEOMETRY_TEST_GEOMETRY_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

GEOMETRY_TEST_GEOMETRY_EXEC_OBJ+=$(filter %$(sufobj), $(GEOMETRY_TEST_GEOMETRY_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(GEOMETRY_TEST_GEOMETRY_EXEC_OBJ)
geometry_exec$(sufexe) : $(GEOMETRY_TEST_GEOMETRY_EXEC_SRC) $(GEOMETRY_TEST_GEOMETRY_EXEC_DINC)
-include $(GEOMETRY_TEST_GEOMETRY_EXEC_DINC)
$(eval $(call do_link_exe,geometry_exec$(sufexe),$(GEOMETRY_TEST_GEOMETRY_EXEC_OBJ)))

all :: geometry_exec$(sufexe)
TARGETS+=geometry_exec$(sufexe)

##END:target geometry_exec
##START:target geom_distribution_exec
GEOMETRY_TEST_GEOM_DISTRIBUTION_EXECSRC :=  \
	geom_distribution_test.cpp
GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_DINC := $(filter %$(sufdep), $(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXECSRC:.c=$(sufdep)))
geom_distribution_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_program_options -lboost_system -lboost_filesystem -lboost_serialization -lboost_unit_test_framework
geom_distribution_exec$(sufexe) $(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_OBJ+=$(filter %$(sufobj), $(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_OBJ)
geom_distribution_exec$(sufexe) : $(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_SRC) $(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_DINC)
-include $(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_DINC)
$(eval $(call do_link_exe,geom_distribution_exec$(sufexe),$(GEOMETRY_TEST_GEOM_DISTRIBUTION_EXEC_OBJ)))

all :: geom_distribution_exec$(sufexe)
TARGETS+=geom_distribution_exec$(sufexe)

##END:target geom_distribution_exec
