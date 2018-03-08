#ifndef IONCH_ION_CHANNEL_CELL_SIMULATOR_META_HPP
#define IONCH_ION_CHANNEL_CELL_SIMULATOR_META_HPP

//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------

#include "platform/simulator_meta.hpp"
#include "utility/bitset.hpp"

namespace ion_channel { class channel_system; } 
namespace core { class input_base_reader; } 
namespace platform { class simulator; } 

namespace ion_channel {

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

class cell_simulator_meta : public platform::simulator_meta
 {
   private:
    // indices in missing option bitset
    enum
     {
      SIMULATOR_EPSW = 0,// entry 'epsw'
      SIMULATOR_ZL1 = 1,
      SIMULATOR_RL1,
      SIMULATOR_RL4 = 3,
      SIMULATOR_RLVEST = 4,
      SIMULATOR_RLCURV = 5,
      CELL_MAX_OPTION = 6

    };

    // This is the same address as this->sim_ but different type.
    channel_system * cell_sim_;

    //  Which input entrys have been seen
    std::bitset<CELL_MAX_OPTION> missing_required_tags_;


   protected:
    // Read an entry in the input file
    //
    // throw an error if input file is incorrect (using UTILITY_INPUT macro)
    bool do_read_entry(core::input_base_reader & reader) override;

    // Check for completeness of input at end of section
    void do_read_end(platform::simulator & sim) override;


   public:
    cell_simulator_meta(channel_system & sim);

    virtual ~cell_simulator_meta();


   private:
    //for serialization only
    cell_simulator_meta() = default;


};

} // namespace ion_channel
#endif
