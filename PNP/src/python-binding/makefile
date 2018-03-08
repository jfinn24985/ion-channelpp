##START:version
#Butter version 1.2beta.175
##END:version
##START:date
#Thu Jan 8 11:31:39 2015
##END:date
##START:preamble
include $(ROOTDIR)/M_sys.mk

##END:preamble
##START:target csc_python
PYTHON-BINDING_CSC_PYTHONSRC :=  \
	python_wrappers.cpp \
	python_binding.cpp
csc_python: PYTHON_EXTENSIONFLAGS+=-lboost_program_options -lboost_system -lboost_filesystem -lboost_python -lboost_random -lboost_serialization
csc_python : $(PYTHON-BINDING_CSC_PYTHONSRC) $(ROOTDIR)/src/core/libionch_core$(suflib) $(ROOTDIR)/src/particle/libionch_particle$(suflib) $(ROOTDIR)/src/utility/libionch_utility$(suflib) 
	$(PYTHON_EXTENSION) $(PYTHON_EXTENSIONFLAGS) csc_python $(PYTHON-BINDING_CSC_PYTHONSRC)


all :: csc_python
TARGETS+=csc_python

$(eval $(call do_install,csc_python,LIB))

##END:target csc_python
