#ifndef IONCH_PERIODIC_CUBE_REMOVE_SPECIE_PBC_HPP
#define IONCH_PERIODIC_CUBE_REMOVE_SPECIE_PBC_HPP

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
#include "utility/unique_ptr.hpp"
#include <string>
#include <cstddef>

namespace particle { class change_set; } 
namespace platform { class simulator; } 

namespace periodic_cube {

// Add particle move generator for a single specie
//
// (instantiated from input file via gc_choice objects)
//
// Choice hash pattern is { X, 1, 0, 0 }
class remove_specie_pbc : public trial::base_choice
 {
   public:
    //Create a particle addition/remove trial
    virtual std::unique_ptr< particle::change_set > generate(platform::simulator & sys) override;

    std::string label() const override;

    // Add/remove given specie
    //
    
    remove_specie_pbc(std::size_t ispec, trial::base_choice * parent)
    : base_choice ( { ispec, 1, 0, 0 }, parent)
    {}

    // Default add/remove given specie (only for serialization)
    //
    
    remove_specie_pbc()
    : base_choice ()
    {}

    virtual ~remove_specie_pbc();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
      }


};

} // namespace periodic_cube

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY(periodic_cube::remove_specie_pbc);

#endif
