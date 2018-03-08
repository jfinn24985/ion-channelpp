##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_core
CORE_LIBIONCH_CORESRC :=  \
	base_input_parameter.cpp \
	constants.cpp \
	help_entry.cpp \
	help_section.cpp \
	help_subtype.cpp \
	input_base_meta.cpp \
	input_base_reader.cpp \
	input_definition.cpp \
	input_delegater.cpp \
	input_document.cpp \
	input_error.cpp \
	input_help.cpp \
	input_node.cpp \
	input_preprocess.cpp \
	input_reader.cpp \
	strngs.cpp
CORE_LIBIONCH_CORE_DINC := $(filter %$(sufdep), $(CORE_LIBIONCH_CORESRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(CORE_LIBIONCH_CORESRC:.c=$(sufdep)))
CORE_LIBIONCH_CORE_OBJ+=$(filter %$(sufobj), $(CORE_LIBIONCH_CORESRC:.cpp=$(sufobj)))
USEROBJ+=$(CORE_LIBIONCH_CORE_OBJ)
libionch_core$(suflib) : $(CORE_LIBIONCH_CORE_SRC) $(CORE_LIBIONCH_CORE_DINC)
-include $(CORE_LIBIONCH_CORE_DINC)
$(eval $(call do_archive,libionch_core$(suflib),$(CORE_LIBIONCH_CORE_OBJ)))

all :: libionch_core$(suflib)
TARGETS+=libionch_core$(suflib)

$(eval $(call do_install,libionch_core$(suflib),LIB))

##END:target libionch_core
