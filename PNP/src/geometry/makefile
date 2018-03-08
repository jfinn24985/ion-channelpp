##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Mar 8 17:25:49 2018
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target libionch_geometry
GEOMETRY_LIBIONCH_GEOMETRYSRC :=  \
	base_region.cpp \
	coordinate.cpp \
	cube_region.cpp \
	cubic_grid.cpp \
	cylinder_region.cpp \
	digitizer_3d.cpp \
	distance_calculator.cpp \
	geometry_manager.cpp \
	grid.cpp \
	membrane_cell_inserter.cpp \
	membrane_cylinder_region.cpp \
	non_periodic_region.cpp \
	open_cylinder_region.cpp \
	open_split_cylinder_region.cpp \
	periodic_cube_region.cpp \
	region_meta.cpp \
	tubular_grid.cpp
GEOMETRY_LIBIONCH_GEOMETRY_DINC := $(filter %$(sufdep), $(GEOMETRY_LIBIONCH_GEOMETRYSRC:.cpp=$(sufdep))) $(filter %$(sufdep), $(GEOMETRY_LIBIONCH_GEOMETRYSRC:.c=$(sufdep)))
GEOMETRY_LIBIONCH_GEOMETRY_OBJ+=$(filter %$(sufobj), $(GEOMETRY_LIBIONCH_GEOMETRYSRC:.cpp=$(sufobj)))
USEROBJ+=$(GEOMETRY_LIBIONCH_GEOMETRY_OBJ)
libionch_geometry$(suflib) : $(GEOMETRY_LIBIONCH_GEOMETRY_SRC) $(GEOMETRY_LIBIONCH_GEOMETRY_DINC)
-include $(GEOMETRY_LIBIONCH_GEOMETRY_DINC)
$(eval $(call do_archive,libionch_geometry$(suflib),$(GEOMETRY_LIBIONCH_GEOMETRY_OBJ)))

all :: libionch_geometry$(suflib)
TARGETS+=libionch_geometry$(suflib)

$(eval $(call do_install,libionch_geometry$(suflib),LIB))

##END:target libionch_geometry
