##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_utility
UTILITY_LIBIONCH_UTILITYSRC :=  \
	basic_histogram.cpp \
	basic_mean.cpp \
	binary_estimate.cpp \
	digitizer.cpp \
	estimate_array.cpp \
	estimater.cpp \
	estimater_matrix.cpp \
	histogram.cpp \
	machine.cpp \
	mathutil.cpp \
	mean_algorithm.cpp \
	random.cpp \
	utility.cpp
UTILITY_LIBIONCH_UTILITY_DINC := $(filter %$(sufdep), $(UTILITY_LIBIONCH_UTILITYSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(UTILITY_LIBIONCH_UTILITYSRC:.c=$(sufdep)))
UTILITY_LIBIONCH_UTILITY_OBJ+=$(filter %$(sufobj), $(UTILITY_LIBIONCH_UTILITYSRC:.cpp=$(sufobj)))
USEROBJ+=$(UTILITY_LIBIONCH_UTILITY_OBJ)
libionch_utility$(suflib) : $(UTILITY_LIBIONCH_UTILITY_SRC) $(UTILITY_LIBIONCH_UTILITY_DINC)
-include $(UTILITY_LIBIONCH_UTILITY_DINC)
$(eval $(call do_archive,libionch_utility$(suflib),$(UTILITY_LIBIONCH_UTILITY_OBJ)))

all :: libionch_utility$(suflib)
TARGETS+=libionch_utility$(suflib)

$(eval $(call do_install,libionch_utility$(suflib),LIB))

##END:target libionch_utility
##START:target libionch_lapack
UTILITY_LIBIONCH_LAPACKSRC :=  \
	lapack.cpp
UTILITY_LIBIONCH_LAPACK_DINC := $(filter %$(sufdep), $(UTILITY_LIBIONCH_LAPACKSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(UTILITY_LIBIONCH_LAPACKSRC:.c=$(sufdep)))
libionch_lapack$(suflib): LDFLAGS+= $(LAPACKLIB)

lapack$(sufobj) lapack$(sufdep) : CPPFLAGS+=$(LAPACKINC) 


UTILITY_LIBIONCH_LAPACK_OBJ+=$(filter %$(sufobj), $(UTILITY_LIBIONCH_LAPACKSRC:.cpp=$(sufobj)))
USEROBJ+=$(UTILITY_LIBIONCH_LAPACK_OBJ)
libionch_lapack$(suflib) : $(UTILITY_LIBIONCH_LAPACK_SRC) $(UTILITY_LIBIONCH_LAPACK_DINC)
-include $(UTILITY_LIBIONCH_LAPACK_DINC)
$(eval $(call do_archive,libionch_lapack$(suflib),$(UTILITY_LIBIONCH_LAPACK_OBJ)))

all :: libionch_lapack$(suflib)
TARGETS+=libionch_lapack$(suflib)

$(eval $(call do_install,libionch_lapack$(suflib),LIB))

##END:target libionch_lapack
