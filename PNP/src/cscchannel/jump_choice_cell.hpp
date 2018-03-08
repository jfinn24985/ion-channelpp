#ifndef IONCH_ION_CHANNEL_JUMP_CHOICE_CELL_HPP
#define IONCH_ION_CHANNEL_JUMP_CHOICE_CELL_HPP

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
#include <boost/serialization/map.hpp>
#include <string>
#include "utility/unique_ptr.hpp"
#include "utility/archive.hpp"


namespace particle { class ensemble; } 
namespace ion_channel { class channel_system; } 
namespace particle { class change_set; } 
namespace platform { class simulator; } 

namespace ion_channel {

//Objects of this class define an MC move that involves a
//jump anywhere within the simulation cell.
//
//change_hash subtype is "1", so hash pattern is ( X, 1, 1, 1 )
class jump_choice_cell : public trial::base_choice
 {
   private:
    //  Region to operate on
    geometry::cylinder_region region_;


   public:
    //
    //Choice hash subtype for jump move is "1", so 
    //pattern for jump move is { X, 1, 1, 1 }.
    jump_choice_cell(std::size_t ispec, std::map< std::string, std::string> const& params, const channel_system & geom);


   private:
    //Default ctor for serialization.
    jump_choice_cell() = default;


   public:
    virtual ~jump_choice_cell();

    std::unique_ptr< particle::change_set > generate(platform::simulator & sys) override;

    //Name for this trial
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
