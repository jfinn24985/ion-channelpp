##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_platform
PLATFORM_LIBIONCH_PLATFORMSRC :=  \
	ionch_main.cpp \
	simulator.cpp \
	simulator_meta.cpp
PLATFORM_LIBIONCH_PLATFORM_DINC := $(filter %$(sufdep), $(PLATFORM_LIBIONCH_PLATFORMSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_LIBIONCH_PLATFORMSRC:.c=$(sufdep)))
PLATFORM_LIBIONCH_PLATFORM_OBJ+=$(filter %$(sufobj), $(PLATFORM_LIBIONCH_PLATFORMSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_LIBIONCH_PLATFORM_OBJ)
libionch_platform$(suflib) : $(PLATFORM_LIBIONCH_PLATFORM_SRC) $(PLATFORM_LIBIONCH_PLATFORM_DINC)
-include $(PLATFORM_LIBIONCH_PLATFORM_DINC)
$(eval $(call do_archive,libionch_platform$(suflib),$(PLATFORM_LIBIONCH_PLATFORM_OBJ)))

all :: libionch_platform$(suflib)
TARGETS+=libionch_platform$(suflib)

$(eval $(call do_install,libionch_platform$(suflib),LIB))

##END:target libionch_platform
##START:target libionch_platform_mpi
PLATFORM_LIBIONCH_PLATFORM_MPISRC :=  \
	mpi_storage.cpp
PLATFORM_LIBIONCH_PLATFORM_MPI_DINC := $(filter %$(sufdep), $(PLATFORM_LIBIONCH_PLATFORM_MPISRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_LIBIONCH_PLATFORM_MPISRC:.c=$(sufdep)))

mpi_storage$(sufobj) mpi_storage$(sufdep) : CPPFLAGS+=$(MPI_INC) 


PLATFORM_LIBIONCH_PLATFORM_MPI_OBJ+=$(filter %$(sufobj), $(PLATFORM_LIBIONCH_PLATFORM_MPISRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_LIBIONCH_PLATFORM_MPI_OBJ)
libionch_platform_mpi$(suflib) : $(PLATFORM_LIBIONCH_PLATFORM_MPI_SRC) $(PLATFORM_LIBIONCH_PLATFORM_MPI_DINC)
-include $(PLATFORM_LIBIONCH_PLATFORM_MPI_DINC)
$(eval $(call do_archive,libionch_platform_mpi$(suflib),$(PLATFORM_LIBIONCH_PLATFORM_MPI_OBJ)))

all :: libionch_platform_mpi$(suflib)
TARGETS+=libionch_platform_mpi$(suflib)

$(eval $(call do_install,libionch_platform_mpi$(suflib),LIB))

##END:target libionch_platform_mpi
##START:target libionch_newplatform
PLATFORM_LIBIONCH_NEWPLATFORMSRC :=  \
	chemical_potential_update.cpp \
	imc_simulation.cpp \
	imc_update.cpp \
	lamperski_igcmc.cpp \
	malasics_igcmc.cpp \
	serial_storage.cpp \
	simulation.cpp \
	simulation_manager.cpp \
	simulator_meta.cpp \
	specie_monitor.cpp \
	standard_simulation.cpp \
	storage_manager.cpp \
	storage_meta.cpp
PLATFORM_LIBIONCH_NEWPLATFORM_DINC := $(filter %$(sufdep), $(PLATFORM_LIBIONCH_NEWPLATFORMSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_LIBIONCH_NEWPLATFORMSRC:.c=$(sufdep)))
libionch_newplatform$(suflib): LDFLAGS+= $(BOOST_LIB) -lboost_program_options -lboost_system -lboost_filesystem
libionch_newplatform$(suflib) $(PLATFORM_LIBIONCH_NEWPLATFORM_DINC) : CPPFLAGS+=-I$(BOOST_INC) 
PLATFORM_LIBIONCH_NEWPLATFORM_OBJ+=$(filter %$(sufobj), $(PLATFORM_LIBIONCH_NEWPLATFORMSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_LIBIONCH_NEWPLATFORM_OBJ)
libionch_newplatform$(suflib) : $(PLATFORM_LIBIONCH_NEWPLATFORM_SRC) $(PLATFORM_LIBIONCH_NEWPLATFORM_DINC)
-include $(PLATFORM_LIBIONCH_NEWPLATFORM_DINC)
$(eval $(call do_archive,libionch_newplatform$(suflib),$(PLATFORM_LIBIONCH_NEWPLATFORM_OBJ)))

all :: libionch_newplatform$(suflib)
TARGETS+=libionch_newplatform$(suflib)

$(eval $(call do_install,libionch_newplatform$(suflib),LIB))

##END:target libionch_newplatform
##START:target ionch
PLATFORM_IONCHSRC :=  \
	ionch_main.cpp
PLATFORM_IONCH_DINC := $(filter %$(sufdep), $(PLATFORM_IONCHSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_IONCHSRC:.c=$(sufdep)))
ionch$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lzip -lboost_program_options -lboost_system -lboost_random -lboost_regex -lboost_serialization -luuid
ionch$(sufexe) $(PLATFORM_IONCH_DINC) : CPPFLAGS+=-I$(BOOST_INC) 
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/platform/libionch_newplatform$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)
PLATFORM_IONCH_OBJ+=$(ROOTDIR)/src/observable/libionch_zip_sink$(suflib)

PLATFORM_IONCH_OBJ+=$(filter %$(sufobj), $(PLATFORM_IONCHSRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_IONCH_OBJ)
ionch$(sufexe) : $(PLATFORM_IONCH_SRC) $(PLATFORM_IONCH_DINC)
-include $(PLATFORM_IONCH_DINC)
$(eval $(call do_link_exe,ionch$(sufexe),$(PLATFORM_IONCH_OBJ)))

all :: ionch$(sufexe)
TARGETS+=ionch$(sufexe)

$(eval $(call do_install,ionch$(sufexe),BIN))

##END:target ionch
##START:target ionch_mpi
PLATFORM_IONCH_MPISRC :=  \
	mpi_main.cpp
PLATFORM_IONCH_MPI_DINC := $(filter %$(sufdep), $(PLATFORM_IONCH_MPISRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PLATFORM_IONCH_MPISRC:.c=$(sufdep)))
ionch_mpi$(sufexe): LDFLAGS+= $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lzip -lboost_mpi $(MPI_LIB) -lmpi -lmpi_cxx -lboost_program_options -lboost_system -lboost_random -lboost_serialization -luuid
ionch_mpi$(sufexe) $(PLATFORM_IONCH_MPI_DINC) : CPPFLAGS+=-I$(BOOST_INC) -I$(MPI_INC) 
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/geometry/libionch_geometry$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/platform/libionch_newplatform$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/platform/libionch_platform_mpi$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)
PLATFORM_IONCH_MPI_OBJ+=$(ROOTDIR)/src/observable/libionch_zip_sink$(suflib)

mpi_main$(sufobj) mpi_main$(sufdep) : CPPFLAGS+=-I$(MPI_INC) $(MPI_INC) 


PLATFORM_IONCH_MPI_OBJ+=$(filter %$(sufobj), $(PLATFORM_IONCH_MPISRC:.cpp=$(sufobj)))
USEROBJ+=$(PLATFORM_IONCH_MPI_OBJ)
ionch_mpi$(sufexe) : $(PLATFORM_IONCH_MPI_SRC) $(PLATFORM_IONCH_MPI_DINC)
-include $(PLATFORM_IONCH_MPI_DINC)
$(eval $(call do_link_exe,ionch_mpi$(sufexe),$(PLATFORM_IONCH_MPI_OBJ)))

all :: ionch_mpi$(sufexe)
TARGETS+=ionch_mpi$(sufexe)

$(eval $(call do_install,ionch_mpi$(sufexe),BIN))

##END:target ionch_mpi
