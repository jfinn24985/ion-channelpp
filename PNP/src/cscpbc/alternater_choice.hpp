#ifndef IONCH_PERIODIC_CUBE_ALTERNATER_CHOICE_HPP
#define IONCH_PERIODIC_CUBE_ALTERNATER_CHOICE_HPP

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

#include <cstddef>
#include "utility/unique_ptr.hpp"
#include "utility/archive.hpp"


namespace particle { class change_set; } 
namespace platform { class simulator; } 

namespace periodic_cube {

//Objects of this class can be used to generate a possible MC trial moves. 
class alternater_choice
 {
   public:
    virtual ~alternater_choice();

    alternater_choice(std::size_t ispec);

    //Generate a new change set based on this choice type.
    std::unique_ptr< particle::change_set > generate(platform::simulator & sys) override;


   private:
    alternater_choice()
    : gc_choice_pbc()
    {}

  friend class boost::serialization::access;
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< gc_choice_pbc >(*this);
      }


};

} // namespace periodic_cube
#endif
