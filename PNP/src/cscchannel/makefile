##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target csccell
CSCCHANNEL_CSCCELLSRC :=  \
	cscmc_cell.cpp
CSCCHANNEL_CSCCELL_DINC := $(filter %$(sufdep), $(CSCCHANNEL_CSCCELLSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(CSCCHANNEL_CSCCELLSRC:.c=$(sufdep)))
csccell$(sufexe): LDFLAGS+= $(LAPACKLIB) $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lgdbm -lboost_iostreams -lboost_program_options -lboost_system -lboost_random -lboost_regex -lboost_serialization -luuid
csccell$(sufexe) $(CSCCHANNEL_CSCCELL_DINC) : CPPFLAGS+=-I$(BOOST_INC) $(LAPACKINC) 
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/cscchannel/libionch_cell$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/utility/libionch_lapack$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/platform/libionch_platform$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
CSCCHANNEL_CSCCELL_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

CSCCHANNEL_CSCCELL_OBJ+=$(filter %$(sufobj), $(CSCCHANNEL_CSCCELLSRC:.cpp=$(sufobj)))
USEROBJ+=$(CSCCHANNEL_CSCCELL_OBJ)
csccell$(sufexe) : $(CSCCHANNEL_CSCCELL_SRC) $(CSCCHANNEL_CSCCELL_DINC)
-include $(CSCCHANNEL_CSCCELL_DINC)
$(eval $(call do_link_exe,csccell$(sufexe),$(CSCCHANNEL_CSCCELL_OBJ)))

all :: csccell$(sufexe)
TARGETS+=csccell$(sufexe)

$(eval $(call do_install,csccell$(sufexe),BIN))

##END:target csccell
##START:target csccell_mpi
CSCCHANNEL_CSCCELL_MPISRC :=  \
	cscmc_cell_mpi.cpp
CSCCHANNEL_CSCCELL_MPI_DINC := $(filter %$(sufdep), $(CSCCHANNEL_CSCCELL_MPISRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(CSCCHANNEL_CSCCELL_MPISRC:.c=$(sufdep)))
csccell_mpi$(sufexe): LDFLAGS+= $(LAPACKLIB) $(BOOST_LIB) -lboost_date_time -lboost_filesystem -lboost_iostreams -lgdbm -lboost_mpi $(MPI_LIB) -lmpi -lmpi_cxx -lboost_program_options -lboost_system -lboost_random -lboost_regex -lboost_serialization -luuid
csccell_mpi$(sufexe) $(CSCCHANNEL_CSCCELL_MPI_DINC) : CPPFLAGS+=-I$(BOOST_INC) -I$(MPI_INC) $(LAPACKINC) 
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/cscchannel/libionch_cell$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/core/libionch_core$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/evaluator/libionch_evaluator$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/observable/libionch_gdbm_sink$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/utility/libionch_lapack$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/observable/libionch_observable$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/particle/libionch_particle$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/platform/libionch_platform$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/platform/libionch_platform_mpi$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/trial/libionch_trial$(suflib)
CSCCHANNEL_CSCCELL_MPI_OBJ+=$(ROOTDIR)/src/utility/libionch_utility$(suflib)

CSCCHANNEL_CSCCELL_MPI_OBJ+=$(filter %$(sufobj), $(CSCCHANNEL_CSCCELL_MPISRC:.cpp=$(sufobj)))
USEROBJ+=$(CSCCHANNEL_CSCCELL_MPI_OBJ)
csccell_mpi$(sufexe) : $(CSCCHANNEL_CSCCELL_MPI_SRC) $(CSCCHANNEL_CSCCELL_MPI_DINC)
-include $(CSCCHANNEL_CSCCELL_MPI_DINC)
$(eval $(call do_link_exe,csccell_mpi$(sufexe),$(CSCCHANNEL_CSCCELL_MPI_OBJ)))

all :: csccell_mpi$(sufexe)
TARGETS+=csccell_mpi$(sufexe)

$(eval $(call do_install,csccell_mpi$(sufexe),BIN))

##END:target csccell_mpi
##START:target libionch_cell
CSCCHANNEL_LIBIONCH_CELLSRC :=  \
	add_choice_cell.cpp \
	cell_simulator_meta.cpp \
	channel_system.cpp \
	cylinder_region.cpp \
	gc_choice_cell.cpp \
	jump_choice_cell.cpp \
	remove_choice_cell.cpp
CSCCHANNEL_LIBIONCH_CELL_DINC := $(filter %$(sufdep), $(CSCCHANNEL_LIBIONCH_CELLSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(CSCCHANNEL_LIBIONCH_CELLSRC:.c=$(sufdep)))
CSCCHANNEL_LIBIONCH_CELL_OBJ+=$(filter %$(sufobj), $(CSCCHANNEL_LIBIONCH_CELLSRC:.cpp=$(sufobj)))
USEROBJ+=$(CSCCHANNEL_LIBIONCH_CELL_OBJ)
libionch_cell$(suflib) : $(CSCCHANNEL_LIBIONCH_CELL_SRC) $(CSCCHANNEL_LIBIONCH_CELL_DINC)
-include $(CSCCHANNEL_LIBIONCH_CELL_DINC)
$(eval $(call do_archive,libionch_cell$(suflib),$(CSCCHANNEL_LIBIONCH_CELL_OBJ)))

all :: libionch_cell$(suflib)
TARGETS+=libionch_cell$(suflib)

$(eval $(call do_install,libionch_cell$(suflib),LIB))

##END:target libionch_cell
