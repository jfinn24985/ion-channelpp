##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_particle
PARTICLE_LIBIONCH_PARTICLESRC :=  \
	change_set.cpp \
	ensemble.cpp \
	particle_manager.cpp \
	specie.cpp \
	specie_meta.cpp
PARTICLE_LIBIONCH_PARTICLE_DINC := $(filter %$(sufdep), $(PARTICLE_LIBIONCH_PARTICLESRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(PARTICLE_LIBIONCH_PARTICLESRC:.c=$(sufdep)))
PARTICLE_LIBIONCH_PARTICLE_OBJ+=$(filter %$(sufobj), $(PARTICLE_LIBIONCH_PARTICLESRC:.cpp=$(sufobj)))
USEROBJ+=$(PARTICLE_LIBIONCH_PARTICLE_OBJ)
libionch_particle$(suflib) : $(PARTICLE_LIBIONCH_PARTICLE_SRC) $(PARTICLE_LIBIONCH_PARTICLE_DINC)
-include $(PARTICLE_LIBIONCH_PARTICLE_DINC)
$(eval $(call do_archive,libionch_particle$(suflib),$(PARTICLE_LIBIONCH_PARTICLE_OBJ)))

all :: libionch_particle$(suflib)
TARGETS+=libionch_particle$(suflib)

$(eval $(call do_install,libionch_particle$(suflib),LIB))

##END:target libionch_particle
