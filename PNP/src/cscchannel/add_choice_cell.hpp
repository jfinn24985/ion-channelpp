#ifndef IONCH_ION_CHANNEL_ADD_CHOICE_CELL_HPP
#define IONCH_ION_CHANNEL_ADD_CHOICE_CELL_HPP

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
#include <cstddef>
#include "utility/unique_ptr.hpp"
#include <string>

namespace particle { class change_set; } 
namespace platform { class simulator; } 
namespace utility { class random_distribution; } 

namespace ion_channel {

// Add particle move generator for a single specie
//
// This version is for use in a cell/region.
class add_choice_cell : public trial::base_choice
 {
   private:
    // Region for addition
    geometry::cylinder_region region_;


   public:
    // Add/remove given specie in a subregion of a cell
    //
    
    add_choice_cell(const std::size_t & key, trial::base_choice & parent, const geometry::cylinder_region & reg)
    : base_choice( { key, 0, 1, 0 }, &parent )
    , region_( reg )
    {}

    // Default add/remove given specie (only for serialization)
    //
    
    inline add_choice_cell() : base_choice ()
        {};

    virtual ~add_choice_cell();

    //Create a particle addition/remove trial
    //
    //(C++ forwards to two argument method)
    virtual std::unique_ptr< particle::change_set > generate(platform::simulator & sys) override;

    //Create a particle addition/remove trial using
    //an external random number source. This
    //version accepts a constant simulator object.
    std::unique_ptr< particle::change_set > generate(const platform::simulator & sys, utility::random_distribution & ranf);

    std::string label() const override;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::base_object< base_choice >(*this);
            ar & region_;
          };


};

} // namespace ion_channel
#endif
