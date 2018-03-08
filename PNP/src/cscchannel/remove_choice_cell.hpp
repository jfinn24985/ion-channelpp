#ifndef IONCH_ION_CHANNEL_REMOVE_CHOICE_CELL_HPP
#define IONCH_ION_CHANNEL_REMOVE_CHOICE_CELL_HPP

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

#include "trial/choice.hpp"
#include "geometry/cylinder_region.hpp"
#include "utility/unique_ptr.hpp"
#include <string>
#include <cstddef>

namespace particle { class change_set; } 
namespace platform { class simulator; } 

namespace ion_channel {

// Remove particle move generator for a single specie/region in channel simulation.
//
// The difficulty of this choice is that it must find a particle in its region!
class remove_choice_cell : public trial::base_choice
 {
   private:
    geometry::cylinder_region region_;


   public:
    //Create a particle addition/remove trial
    virtual std::unique_ptr< particle::change_set > generate(platform::simulator & sys) override;

    std::string label() const override;

    // Add/remove given specie
    //
    
    remove_choice_cell(std::size_t ispec, trial::base_choice & parent, const geometry::cylinder_region & geom)
    : base_choice( { ispec, 1, 0, 0 }, &parent )
    , region_( geom )
    {}

    // Default add/remove given specie (only for serialization)
    //
    
    remove_choice_cell()
    : base_choice ()
    {}

    virtual ~remove_choice_cell();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
        ar & region_;
      }


};

} // namespace ion_channel
#endif
