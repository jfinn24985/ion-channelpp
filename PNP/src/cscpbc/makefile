##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_pbc
CSCPBC_LIBIONCH_PBCSRC :=  \
	add_specie_pbc.cpp \
	alternater_choice.cpp \
	alternater_chooser.cpp \
	alternater_sampler.cpp \
	bulk_simulator_meta.cpp \
	gc_choice_pbc.cpp \
	jump_choice_pbc.cpp \
	periodic_system.cpp \
	remove_specie_pbc.cpp
CSCPBC_LIBIONCH_PBC_DINC := $(filter %$(sufdep), $(CSCPBC_LIBIONCH_PBCSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(CSCPBC_LIBIONCH_PBCSRC:.c=$(sufdep)))
CSCPBC_LIBIONCH_PBC_OBJ+=$(filter %$(sufobj), $(CSCPBC_LIBIONCH_PBCSRC:.cpp=$(sufobj)))
USEROBJ+=$(CSCPBC_LIBIONCH_PBC_OBJ)
libionch_pbc$(suflib) : $(CSCPBC_LIBIONCH_PBC_SRC) $(CSCPBC_LIBIONCH_PBC_DINC)
-include $(CSCPBC_LIBIONCH_PBC_DINC)
$(eval $(call do_archive,libionch_pbc$(suflib),$(CSCPBC_LIBIONCH_PBC_OBJ)))

all :: libionch_pbc$(suflib)
TARGETS+=libionch_pbc$(suflib)

$(eval $(call do_install,libionch_pbc$(suflib),LIB))

##END:target libionch_pbc
##START:target cscbulk
CSCPBC_CSCBULKSRC :=  \
	cscmc_bulk.cpp
CSCPBC_CSCBULK_DINC := $(filter %$(sufdep), $(CSCPBC_CSCBULKSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(CSCPBC_CSCBULKSRC:.c=$(sufdep)))
cscbulk$(sufexe): LDFLAGS+= $(LAPACKLIB) $(BOOST_LIB) -lboost_date_time -lgdbm -lboost_program_options -lboost_system -lboost_filesystem -lboost_random -lboost_serialization -luuid
cscbulk$(sufexe) $(CSCPBC_CSCBULK_DINC) : CPPFLAGS+=-I$(BOOST_INC) $(LAPACKINC) 
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/utility/libionch_lapack$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/cscpbc/libionch_pbc$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/platform/libionch_platform$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
CSCPBC_CSCBULK_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

CSCPBC_CSCBULK_OBJ+=$(filter %$(sufobj), $(CSCPBC_CSCBULKSRC:.cpp=$(sufobj)))
USEROBJ+=$(CSCPBC_CSCBULK_OBJ)
cscbulk$(sufexe) : $(CSCPBC_CSCBULK_SRC) $(CSCPBC_CSCBULK_DINC)
-include $(CSCPBC_CSCBULK_DINC)
$(eval $(call do_link_exe,cscbulk$(sufexe),$(CSCPBC_CSCBULK_OBJ)))

all :: cscbulk$(sufexe)
TARGETS+=cscbulk$(sufexe)

$(eval $(call do_install,cscbulk$(sufexe),BIN))

##END:target cscbulk
##START:target cscbulk_mpi
CSCPBC_CSCBULK_MPISRC :=  \
	cscmc_bulk_mpi.cpp
CSCPBC_CSCBULK_MPI_DINC := $(filter %$(sufdep), $(CSCPBC_CSCBULK_MPISRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(CSCPBC_CSCBULK_MPISRC:.c=$(sufdep)))
cscbulk_mpi$(sufexe): LDFLAGS+= $(LAPACKLIB) $(BOOST_LIB) -lboost_date_time -lgdbm -lboost_mpi $(MPI_LIB) -lmpi -lmpi_cxx -lboost_program_options -lboost_system -lboost_filesystem -lboost_random -lboost_serialization -luuid
cscbulk_mpi$(sufexe) $(CSCPBC_CSCBULK_MPI_DINC) : CPPFLAGS+=-I$(BOOST_INC) -I$(MPI_INC) $(LAPACKINC) 
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/utility/libionch_lapack$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/cscpbc/libionch_pbc$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/platform/libionch_platform$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/platform/libionch_platform_mpi$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
CSCPBC_CSCBULK_MPI_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

CSCPBC_CSCBULK_MPI_OBJ+=$(filter %$(sufobj), $(CSCPBC_CSCBULK_MPISRC:.cpp=$(sufobj)))
USEROBJ+=$(CSCPBC_CSCBULK_MPI_OBJ)
cscbulk_mpi$(sufexe) : $(CSCPBC_CSCBULK_MPI_SRC) $(CSCPBC_CSCBULK_MPI_DINC)
-include $(CSCPBC_CSCBULK_MPI_DINC)
$(eval $(call do_link_exe,cscbulk_mpi$(sufexe),$(CSCPBC_CSCBULK_MPI_OBJ)))

all :: cscbulk_mpi$(sufexe)
TARGETS+=cscbulk_mpi$(sufexe)

$(eval $(call do_install,cscbulk_mpi$(sufexe),BIN))

##END:target cscbulk_mpi
