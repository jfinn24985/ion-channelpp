#ifndef IONCH_PERIODIC_CUBE_GC_CHOICE_PBC_HPP
#define IONCH_PERIODIC_CUBE_GC_CHOICE_PBC_HPP

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

#include "trial/base_chooser.hpp"
#include <string>
#include <boost/serialization/map.hpp>
#include <iostream>
#include <boost/ptr_container/serialize_ptr_vector.hpp>

namespace particle { class ensemble; } 
namespace platform { class simulator; } 
namespace trial { class base_choice; } 

namespace periodic_cube {

// Single particle Grand canonical move generator, delegates
// move generation to add_choice/remove_choice objects
//
// This version is for simulation in a periodic cube.
//
// Use same choice hash pattern as add_choice: { X, 0, 1, 0 }
class gc_chooser_pbc : public trial::base_chooser
 {
   public:
    //For serialization and make_chooser method
    gc_chooser_pbc() = default;

    virtual ~gc_chooser_pbc();

    std::string label() const override;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< trial::base_chooser >(*this);
      };


   public:
    //  Function to add move choice objects to the simulation.
    static void make_chooser(const std::map< std::string, std::string >& params, double rate, platform::simulator & sim);

    // Provide a description of the chooser state.
    virtual void description(std::ostream & os) const;

    //  Generate and add choices to simulator.
    void generate_choices(const platform::simulator & sim, boost::ptr_vector< trial::base_choice >& choices) const override;


};

} // namespace periodic_cube

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY(periodic_cube::gc_choice_pbc);

#endif
