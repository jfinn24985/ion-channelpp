#ifndef IONCH_PERIODIC_CUBE_BULK_SIMULATOR_META_HPP
#define IONCH_PERIODIC_CUBE_BULK_SIMULATOR_META_HPP


#include "platform/simulator_meta.hpp"
#include "utility/bitset.hpp"

namespace periodic_cube { class periodic_system; } 
namespace core { class input_base_reader; } 
namespace platform { class simulator; } 

namespace periodic_cube {

//Linker class between the input file and derived classes
//of type 'base_simulator'
//
//multiple = true
//
//Implementation Notes: The simulator objects are held by
//the application as owned pointers. Therefore we
//can not retain a pointer to the simulator that can be
//recovered after read/write serialization.  Not only
//that, but the simulator objects are deleted ater their
//part of the simulation has been run. In order to be
//able to write a facsimile of the input file at any
//time, this meta object retains the initial input data.

class bulk_simulator_meta : public platform::simulator_meta
 {
   private:
    // Pointer to periodic cube simulator 
    //
    // (note this has identical address to base class sim_, but different type)
    periodic_system * bulk_sim_;

    //  Which input entrys have been seen
    std::bitset<1> missing_required_tags_;

    // indices in missing option bitset
    enum
     {
      SIMULATOR_EPSW = 0// entry 'epsw'

    };


   protected:
    // Read an entry in the input file
    //
    // throw an error if input file is incorrect (using UTILITY_INPUT macro)
    bool do_read_entry(core::input_base_reader & reader) override;

    // Check for completeness of input at end of section
    void do_read_end(platform::simulator & sim) override;


   public:
    bulk_simulator_meta(periodic_system & sim);

    virtual ~bulk_simulator_meta();


   private:
    //for serialization only
    bulk_simulator_meta() = default;


};

} // namespace periodic_cube
#endif
