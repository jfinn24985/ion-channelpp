#ifndef IONCH_ION_CHANNEL_GC_CHOICE_CELL_HPP
#define IONCH_ION_CHANNEL_GC_CHOICE_CELL_HPP

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
#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include "cscchannel/add_choice_cell.hpp"
#include <cstddef>
#include <boost/serialization/map.hpp>
#include <string>
#include "utility/unique_ptr.hpp"

namespace particle { class ensemble; } 
namespace ion_channel { class remove_choice_cell; } 
namespace ion_channel { class channel_system; } 
namespace particle { class change_set; } 
namespace platform { class simulator; } 
namespace particle { class specie; } 

namespace ion_channel {

// Single particle Grand canonical move generator, delegates
// move generation to add_choice/remove_choice children
//
// This version is for simulation in a periodic box.
class gc_choice_cell : public trial::base_choice
 {
   private:
    //Set of adders by region.
    boost::ptr_vector< add_choice_cell > adder_;

    //Per-region removers.
    boost::ptr_vector< remove_choice_cell > remover_;


   public:
    //Create a grand canonical individual move.
    //
    //choice hash subtype is "0" (one particle changes)
    //hash pattern is ( X, 0, 1, 0 ). NOTE: GC moves use
    //the hash pattern of their "add" sub-choice.
    gc_choice_cell(const std::size_t & ispec, std::map< std::string, std::string> const& params, const channel_system & geom);


   private:
    //For serialization
    gc_choice_cell() = default;


   public:
    virtual ~gc_choice_cell();

    //Create a particle addition/remove trial
    virtual std::unique_ptr< particle::change_set > generate(platform::simulator & sys);

    add_choice_cell const& get_adder(std::size_t ireg) const;

    const remove_choice_cell& get_remover(std::size_t ireg) const;

    std::string label() const override;

    //Return true only if {spec} is a solute species.
    virtual bool permitted(const particle::specie & spec) const;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< base_choice >(*this);
        ar & this->adder_;
        ar & this->remover_;
      };


};

} // namespace ion_channel
#endif
