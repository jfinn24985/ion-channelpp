#ifndef IONCH_IONCH_IMPLEMENTATION_ICCGRID_H
#define IONCH_IONCH_IMPLEMENTATION_ICCGRID_H


namespace ionch {

namespace implementation {

//Base class for ICC surface integrators.
//
//* Requirement *
//
//+ Provide piecewise ICC matrix elements for surface patches.
//

class Integrator
 {
   public:
    //Function that integrates 'patch' into 'ptchset'
    void __call__();


};

} // namespace ionch::implementation

} // namespace ionch
#endif
