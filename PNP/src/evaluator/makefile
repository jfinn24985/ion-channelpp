##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_evaluator
EVALUATOR_LIBIONCH_EVALUATORSRC :=  \
	base_evaluator.cpp \
	coulomb.cpp \
	evaluator_manager.cpp \
	evaluator_meta.cpp \
	hard_sphere_overlap.cpp \
	icc_matrix.cpp \
	icc_surface_grid.cpp \
	induced_charge.cpp \
	integrator.cpp \
	lennard_jones.cpp \
	localizer.cpp \
	object_overlap.cpp
EVALUATOR_LIBIONCH_EVALUATOR_DINC := $(filter %$(sufdep), $(EVALUATOR_LIBIONCH_EVALUATORSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(EVALUATOR_LIBIONCH_EVALUATORSRC:.c=$(sufdep)))
EVALUATOR_LIBIONCH_EVALUATOR_OBJ+=$(filter %$(sufobj), $(EVALUATOR_LIBIONCH_EVALUATORSRC:.cpp=$(sufobj)))
USEROBJ+=$(EVALUATOR_LIBIONCH_EVALUATOR_OBJ)
libionch_evaluator$(suflib) : $(EVALUATOR_LIBIONCH_EVALUATOR_SRC) $(EVALUATOR_LIBIONCH_EVALUATOR_DINC)
-include $(EVALUATOR_LIBIONCH_EVALUATOR_DINC)
$(eval $(call do_archive,libionch_evaluator$(suflib),$(EVALUATOR_LIBIONCH_EVALUATOR_OBJ)))

all :: libionch_evaluator$(suflib)
TARGETS+=libionch_evaluator$(suflib)

$(eval $(call do_install,libionch_evaluator$(suflib),LIB))

##END:target libionch_evaluator
##START:target libionch_fevaluator
EVALUATOR_LIBIONCH_FEVALUATORSRC :=  \
	cfortran.cpp \
	icc_fortran.cpp \
	patch.f90
EVALUATOR_LIBIONCH_FEVALUATOR_DINC := $(filter %$(sufdep), $(EVALUATOR_LIBIONCH_FEVALUATORSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(EVALUATOR_LIBIONCH_FEVALUATORSRC:.c=$(sufdep)))
libionch_fevaluator$(suflib): LDFLAGS+= $(FORTRANLIBS)

EVALUATOR_LIBIONCH_FEVALUATOR_OBJ+=patch$(sufobj)

EVALUATOR_LIBIONCH_FEVALUATOR_OBJ+=$(filter %$(sufobj), $(EVALUATOR_LIBIONCH_FEVALUATORSRC:.cpp=$(sufobj)))
USEROBJ+=$(EVALUATOR_LIBIONCH_FEVALUATOR_OBJ)
libionch_fevaluator$(suflib) : $(EVALUATOR_LIBIONCH_FEVALUATOR_SRC) $(EVALUATOR_LIBIONCH_FEVALUATOR_DINC)
-include $(EVALUATOR_LIBIONCH_FEVALUATOR_DINC)
$(eval $(call do_archive,libionch_fevaluator$(suflib),$(EVALUATOR_LIBIONCH_FEVALUATOR_OBJ)))

all :: libionch_fevaluator$(suflib)
TARGETS+=libionch_fevaluator$(suflib)

$(eval $(call do_install,libionch_fevaluator$(suflib),LIB))

##END:target libionch_fevaluator
