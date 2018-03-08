##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_trial
TRIAL_LIBIONCH_TRIALSRC :=  \
	base_chooser.cpp \
	choice.cpp \
	choice_manager.cpp \
	choice_meta.cpp \
	std_choices.cpp
TRIAL_LIBIONCH_TRIAL_DINC := $(filter %$(sufdep), $(TRIAL_LIBIONCH_TRIALSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(TRIAL_LIBIONCH_TRIALSRC:.c=$(sufdep)))
TRIAL_LIBIONCH_TRIAL_OBJ+=$(filter %$(sufobj), $(TRIAL_LIBIONCH_TRIALSRC:.cpp=$(sufobj)))
USEROBJ+=$(TRIAL_LIBIONCH_TRIAL_OBJ)
libionch_trial$(suflib) : $(TRIAL_LIBIONCH_TRIAL_SRC) $(TRIAL_LIBIONCH_TRIAL_DINC)
-include $(TRIAL_LIBIONCH_TRIAL_DINC)
$(eval $(call do_archive,libionch_trial$(suflib),$(TRIAL_LIBIONCH_TRIAL_OBJ)))

all :: libionch_trial$(suflib)
TARGETS+=libionch_trial$(suflib)

$(eval $(call do_install,libionch_trial$(suflib),LIB))

##END:target libionch_trial
