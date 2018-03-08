#ifndef IONCH_PERIODIC_CUBE_JUMP_CHOICE_PBC_HPP
#define IONCH_PERIODIC_CUBE_JUMP_CHOICE_PBC_HPP

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
#include <cstddef>
#include <boost/serialization/map.hpp>
#include <string>
#include "utility/unique_ptr.hpp"

namespace particle { class ensemble; } 
namespace particle { class change_set; } 
namespace platform { class simulator; } 

namespace periodic_cube {

//Objects of this class define an MC move that involves a
//jump within the simulation a cube. The displacement is defined by the
//simulation cube length by a random number in [0,1)
//for each coordinate.
//
// Choice hash subtype is "1", so pattern is { X, 1, 1, 1 }
class jump_choice_pbc : public trial::base_choice
 {
   private:
    // default ctor for deserialization only
    jump_choice_pbc()
    : base_choice()
    {}


   public:
    jump_choice_pbc(std::size_t ispec, const std::map< std::string, std::string >& params);

    virtual ~jump_choice_pbc();

    std::unique_ptr< particle::change_set > generate(platform::simulator & sys) override;

    //Name for this trial
    std::string label() const override;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
      }


};

} // namespace periodic_cube

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY(periodic_cube::jump_choice_pbc);

#endif
