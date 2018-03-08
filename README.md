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
