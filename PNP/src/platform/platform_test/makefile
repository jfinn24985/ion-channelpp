##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libtest_platform
PLATFORM_TEST_LIBTEST_PLATFORMSRC :=  \
	simulator_fixture.cpp \
	simulator_test_dummy.cpp
PLATFORM_TEST_LIBTEST_PLATFORM_DINC := $(filter %$(sufdep), $(PLATFORM_TEST_LIBTEST_PLATFORMSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_TEST_LIBTEST_PLATFORMSRC:.c=$(sufdep)))
PLATFORM_TEST_LIBTEST_PLATFORM_OBJ+=$(filter %$(sufobj), $(PLATFORM_TEST_LIBTEST_PLATFORMSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_TEST_LIBTEST_PLATFORM_OBJ)
libtest_platform$(suflib) : $(PLATFORM_TEST_LIBTEST_PLATFORM_SRC) $(PLATFORM_TEST_LIBTEST_PLATFORM_DINC)
-include $(PLATFORM_TEST_LIBTEST_PLATFORM_DINC)
$(eval $(call do_archive,libtest_platform$(suflib),$(PLATFORM_TEST_LIBTEST_PLATFORM_OBJ)))

all :: libtest_platform$(suflib)
TARGETS+=libtest_platform$(suflib)

$(eval $(call do_install,libtest_platform$(suflib),LIB))

##END:target libtest_platform
##START:target platform_exec
PLATFORM_TEST_PLATFORM_EXECSRC :=  \
	platform_test.cpp
PLATFORM_TEST_PLATFORM_EXEC_DINC := $(filter %$(sufdep), $(PLATFORM_TEST_PLATFORM_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_TEST_PLATFORM_EXECSRC:.c=$(sufdep)))
platform_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lzip -lboost_random -lboost_serialization -lboost_system -lboost_unit_test_framework -luuid
platform_exec$(sufexe) $(PLATFORM_TEST_PLATFORM_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/platform/libionch_newplatform$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)
PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_zip_sink$(suflib)

PLATFORM_TEST_PLATFORM_EXEC_OBJ+=$(filter %$(sufobj), $(PLATFORM_TEST_PLATFORM_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_TEST_PLATFORM_EXEC_OBJ)
platform_exec$(sufexe) : $(PLATFORM_TEST_PLATFORM_EXEC_SRC) $(PLATFORM_TEST_PLATFORM_EXEC_DINC)
-include $(PLATFORM_TEST_PLATFORM_EXEC_DINC)
$(eval $(call do_link_exe,platform_exec$(sufexe),$(PLATFORM_TEST_PLATFORM_EXEC_OBJ)))

all :: platform_exec$(sufexe)
TARGETS+=platform_exec$(sufexe)

##END:target platform_exec
##START:target sim_manager_exec
PLATFORM_TEST_SIM_MANAGER_EXECSRC :=  \
	sim_manager_test.cpp
PLATFORM_TEST_SIM_MANAGER_EXEC_DINC := $(filter %$(sufdep), $(PLATFORM_TEST_SIM_MANAGER_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_TEST_SIM_MANAGER_EXECSRC:.c=$(sufdep)))
sim_manager_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lzip -lboost_random -lboost_serialization -lboost_system -lboost_unit_test_framework -luuid
sim_manager_exec$(sufexe) $(PLATFORM_TEST_SIM_MANAGER_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/platform/libionch_newplatform$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)
PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_zip_sink$(suflib)

PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ+=$(filter %$(sufobj), $(PLATFORM_TEST_SIM_MANAGER_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ)
sim_manager_exec$(sufexe) : $(PLATFORM_TEST_SIM_MANAGER_EXEC_SRC) $(PLATFORM_TEST_SIM_MANAGER_EXEC_DINC)
-include $(PLATFORM_TEST_SIM_MANAGER_EXEC_DINC)
$(eval $(call do_link_exe,sim_manager_exec$(sufexe),$(PLATFORM_TEST_SIM_MANAGER_EXEC_OBJ)))

all :: sim_manager_exec$(sufexe)
TARGETS+=sim_manager_exec$(sufexe)

##END:target sim_manager_exec
##START:target storage_manager_exec
PLATFORM_TEST_STORAGE_MANAGER_EXECSRC :=  \
	storage_manager_test.cpp
PLATFORM_TEST_STORAGE_MANAGER_EXEC_DINC := $(filter %$(sufdep), $(PLATFORM_TEST_STORAGE_MANAGER_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_TEST_STORAGE_MANAGER_EXECSRC:.c=$(sufdep)))
storage_manager_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lzip -lboost_random -lboost_serialization -lboost_system -lboost_unit_test_framework -luuid
storage_manager_exec$(sufexe) $(PLATFORM_TEST_STORAGE_MANAGER_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/platform/libionch_newplatform$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)
PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_zip_sink$(suflib)

PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ+=$(filter %$(sufobj), $(PLATFORM_TEST_STORAGE_MANAGER_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ)
storage_manager_exec$(sufexe) : $(PLATFORM_TEST_STORAGE_MANAGER_EXEC_SRC) $(PLATFORM_TEST_STORAGE_MANAGER_EXEC_DINC)
-include $(PLATFORM_TEST_STORAGE_MANAGER_EXEC_DINC)
$(eval $(call do_link_exe,storage_manager_exec$(sufexe),$(PLATFORM_TEST_STORAGE_MANAGER_EXEC_OBJ)))

all :: storage_manager_exec$(sufexe)
TARGETS+=storage_manager_exec$(sufexe)

##END:target storage_manager_exec
##START:target imc_convergence_exec
PLATFORM_TEST_IMC_CONVERGENCE_EXECSRC :=  \
	imc_convergence_test.cpp
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_DINC := $(filter %$(sufdep), $(PLATFORM_TEST_IMC_CONVERGENCE_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_TEST_IMC_CONVERGENCE_EXECSRC:.c=$(sufdep)))
imc_convergence_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lzip -lboost_random -lboost_serialization -lboost_system -lboost_unit_test_framework -luuid
imc_convergence_exec$(sufexe) $(PLATFORM_TEST_IMC_CONVERGENCE_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -DBOOST_TEST_DYN_LINK 
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/platform/libionch_newplatform$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)
PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_zip_sink$(suflib)

PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ+=$(filter %$(sufobj), $(PLATFORM_TEST_IMC_CONVERGENCE_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ)
imc_convergence_exec$(sufexe) : $(PLATFORM_TEST_IMC_CONVERGENCE_EXEC_SRC) $(PLATFORM_TEST_IMC_CONVERGENCE_EXEC_DINC)
-include $(PLATFORM_TEST_IMC_CONVERGENCE_EXEC_DINC)
$(eval $(call do_link_exe,imc_convergence_exec$(sufexe),$(PLATFORM_TEST_IMC_CONVERGENCE_EXEC_OBJ)))

all :: imc_convergence_exec$(sufexe)
TARGETS+=imc_convergence_exec$(sufexe)

##END:target imc_convergence_exec
##START:target mpi_storage_exec
PLATFORM_TEST_MPI_STORAGE_EXECSRC :=  \
	mpi_storage_test.cpp
PLATFORM_TEST_MPI_STORAGE_EXEC_DINC := $(filter %$(sufdep), $(PLATFORM_TEST_MPI_STORAGE_EXECSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_TEST_MPI_STORAGE_EXECSRC:.c=$(sufdep)))
mpi_storage_exec$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lzip -lboost_mpi $(MPI_LIB) -lmpi -lmpi_cxx -lboost_random -lboost_serialization -lboost_system -lboost_unit_test_framework -luuid
mpi_storage_exec$(sufexe) $(PLATFORM_TEST_MPI_STORAGE_EXEC_DINC) : CPPFLAGS+=-I$(BOOST_INC) -I$(MPI_INC) -DBOOST_TEST_DYN_LINK 
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/platform/libionch_newplatform$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/platform/libionch_platform_mpi$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)
PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(ROOTDIR)/src/observable/libionch_zip_sink$(suflib)

PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ+=$(filter %$(sufobj), $(PLATFORM_TEST_MPI_STORAGE_EXECSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ)
mpi_storage_exec$(sufexe) : $(PLATFORM_TEST_MPI_STORAGE_EXEC_SRC) $(PLATFORM_TEST_MPI_STORAGE_EXEC_DINC)
-include $(PLATFORM_TEST_MPI_STORAGE_EXEC_DINC)
$(eval $(call do_link_exe,mpi_storage_exec$(sufexe),$(PLATFORM_TEST_MPI_STORAGE_EXEC_OBJ)))

all :: mpi_storage_exec$(sufexe)
TARGETS+=mpi_storage_exec$(sufexe)

##END:target mpi_storage_exec
