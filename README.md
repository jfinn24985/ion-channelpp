# ion-channelpp

Reworking of the Fortran ion-channel program as a C++ project.

## Enhanced features

* runtime selectable energy evaluators

* runtime selectable observers

* codable geometry 

## Future options

The internal design for energy evaluators and observers should make it
possible to add a runtime plug-in system for these in future version.

A python binding that would allow:

 * geometry to be coded in a python script
 
 * develop new evaluators and observers in python

# Submitting bug fixes

For minor changes, patches to the C++ source files in the ´PNP´
directory can be submitted to the maintainer.

# Project development

This project is developed using the bouml tool. The bouml project
files are in the ´pnp´ (lowercase) directory. The files generated 
by the bouml tool (compilable source etc) are in the ´PNP´ directory.
This means that any direct changes to the ´PNP´ directory will not be 
retained. Instead changes should be made in the ´pnp´ directory.

**NOTE** Editing a bouml project by multiple people requires some care 
to avoid unmergable edits. The suggested process should go as follows.

* clone the repository

* copy the ´pnp´ directory to a local working directory

* edit the project in your local working directory

* pull the repository 

* use bouml's projectSynchro to synchronise your local working
 directory to the ´pnp´ directory (bidirectional)

* resolve conflicts

* push the repository

