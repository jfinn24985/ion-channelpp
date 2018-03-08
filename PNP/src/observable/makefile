##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_observable
OBSERVABLE_LIBIONCH_OBSERVABLESRC :=  \
	acceptance_observable.cpp \
	base_observable.cpp \
	base_sink.cpp \
	d3_distribution.cpp \
	density_zaxis.cpp \
	field_format.cpp \
	memory_sink.cpp \
	metropolis_sampler.cpp \
	output_buffer_datum.cpp \
	output_file.cpp \
	output_row_datum.cpp \
	output_rows.cpp \
	output_series.cpp \
	output_series_datum.cpp \
	output_text.cpp \
	rdf_sampler.cpp \
	report_manager.cpp \
	sampler_meta.cpp \
	specie_count.cpp \
	specie_region_count.cpp \
	trial_observer.cpp \
	widom.cpp \
	widom_row.cpp
OBSERVABLE_LIBIONCH_OBSERVABLE_DINC := $(filter %$(sufdep), $(OBSERVABLE_LIBIONCH_OBSERVABLESRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(OBSERVABLE_LIBIONCH_OBSERVABLESRC:.c=$(sufdep)))
OBSERVABLE_LIBIONCH_OBSERVABLE_OBJ+=$(filter %$(sufobj), $(OBSERVABLE_LIBIONCH_OBSERVABLESRC:.cpp=$(sufobj)))
USEROBJ+=$(OBSERVABLE_LIBIONCH_OBSERVABLE_OBJ)
libionch_observable$(suflib) : $(OBSERVABLE_LIBIONCH_OBSERVABLE_SRC) $(OBSERVABLE_LIBIONCH_OBSERVABLE_DINC)
-include $(OBSERVABLE_LIBIONCH_OBSERVABLE_DINC)
$(eval $(call do_archive,libionch_observable$(suflib),$(OBSERVABLE_LIBIONCH_OBSERVABLE_OBJ)))

all :: libionch_observable$(suflib)
TARGETS+=libionch_observable$(suflib)

$(eval $(call do_install,libionch_observable$(suflib),LIB))

##END:target libionch_observable
##START:target libionch_zip_sink
OBSERVABLE_LIBIONCH_ZIP_SINKSRC :=  \
	archive_file.cpp
OBSERVABLE_LIBIONCH_ZIP_SINK_DINC := $(filter %$(sufdep), $(OBSERVABLE_LIBIONCH_ZIP_SINKSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(OBSERVABLE_LIBIONCH_ZIP_SINKSRC:.c=$(sufdep)))
libionch_zip_sink$(suflib): LDFLAGS+= -lzip
OBSERVABLE_LIBIONCH_ZIP_SINK_OBJ+=$(filter %$(sufobj), $(OBSERVABLE_LIBIONCH_ZIP_SINKSRC:.cpp=$(sufobj)))
USEROBJ+=$(OBSERVABLE_LIBIONCH_ZIP_SINK_OBJ)
libionch_zip_sink$(suflib) : $(OBSERVABLE_LIBIONCH_ZIP_SINK_SRC) $(OBSERVABLE_LIBIONCH_ZIP_SINK_DINC)
-include $(OBSERVABLE_LIBIONCH_ZIP_SINK_DINC)
$(eval $(call do_archive,libionch_zip_sink$(suflib),$(OBSERVABLE_LIBIONCH_ZIP_SINK_OBJ)))

all :: libionch_zip_sink$(suflib)
TARGETS+=libionch_zip_sink$(suflib)

$(eval $(call do_install,libionch_zip_sink$(suflib),LIB))

##END:target libionch_zip_sink
##START:target libionch_gdbm_sink
OBSERVABLE_LIBIONCH_GDBM_SINKSRC :=  \
	gdbm_sink.cpp
OBSERVABLE_LIBIONCH_GDBM_SINK_DINC := $(filter %$(sufdep), $(OBSERVABLE_LIBIONCH_GDBM_SINKSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(OBSERVABLE_LIBIONCH_GDBM_SINKSRC:.c=$(sufdep)))
libionch_gdbm_sink$(suflib): LDFLAGS+= -lgdbm
OBSERVABLE_LIBIONCH_GDBM_SINK_OBJ+=$(filter %$(sufobj), $(OBSERVABLE_LIBIONCH_GDBM_SINKSRC:.cpp=$(sufobj)))
USEROBJ+=$(OBSERVABLE_LIBIONCH_GDBM_SINK_OBJ)
libionch_gdbm_sink$(suflib) : $(OBSERVABLE_LIBIONCH_GDBM_SINK_SRC) $(OBSERVABLE_LIBIONCH_GDBM_SINK_DINC)
-include $(OBSERVABLE_LIBIONCH_GDBM_SINK_DINC)
$(eval $(call do_archive,libionch_gdbm_sink$(suflib),$(OBSERVABLE_LIBIONCH_GDBM_SINK_OBJ)))

all :: libionch_gdbm_sink$(suflib)
TARGETS+=libionch_gdbm_sink$(suflib)

$(eval $(call do_install,libionch_gdbm_sink$(suflib),LIB))

##END:target libionch_gdbm_sink
