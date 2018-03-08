#ifndef IONCH_TRIAL_BASE_CHOOSER_HPP
#define IONCH_TRIAL_BASE_CHOOSER_HPP

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

#include <boost/noncopyable.hpp>

#include <boost/serialization/vector.hpp>
#include "core/input_parameter_memo.hpp"
#include <string>
#include <cstddef>
#include <iostream>
#include <boost/ptr_container/serialize_ptr_vector.hpp>

namespace geometry { class geometry_manager; } 
namespace trial { class base_choice; } 
namespace particle { class specie; } 
namespace core { class input_document; } 

namespace trial {

// Generator class for choices.
//
// The chooser interface has a single virtual function,
// generate_choices. This method creates and adds 
// choice objects to a list of possible trial types.
//
// Choosers can be set in the input file. This implies that
// they are self describing and can be used to recreate 
// an input file during the simulation. Such object 
// persistence implies they must also be serializable.
//
// The input key word is given by core::strngs::fstry()
// { default = "trial" }
class base_chooser : public boost::noncopyable
 {
   private:
    // Non-standard parameters used to generate the choice 
    // from the input file.
    //
    // This contains all parameters other than "type" and "rate" that
    // where specified in the input file. The chooser generator functions 
    // are responsible for checking this parameter set has valid
    // entries (it should not be necessary to check the parameters in
    // a constructor.)
    std::vector<core::input_parameter_memo> parameter_set_;

    //  The relative rate of the generated choices.
    //
    //  This matches the (normalized) "rate" value used in the input file.
    double rate_;

    // Optional list of allowed and/or disallowed species.
    core::input_parameter_memo specie_list_;

    //  A label for the trial generated by the choice.
    //
    //  This matches the (normalized) "type" label used in the input file.
    std::string type_;


   protected:
    // Create empty chooser (mainly for serialization).
    base_chooser() = default;


   public:
    // Create chooser from data in input file.
    base_chooser(std::vector< core::input_parameter_memo > const& params, std::string type, const core::input_parameter_memo & specie_list, double rate)
    : parameter_set_( params )
    , rate_( rate )
    , specie_list_( specie_list )
    , type_( type )
    {}


   private:
    // No Copy contents. BEWARE would cause slicing.
    base_chooser(const base_chooser & source) = delete;
    // No Move. BEWARE this would cause slicing.
    base_chooser(base_chooser && source) = delete;
    // No Assignment. BEWARE this would cause slicing.
    base_chooser& operator=(const base_chooser & source) = delete;


   public:
    virtual ~base_chooser() = default;


  friend class boost::serialization::access;    //Read/write object to an archive.
    template< class Archive > void serialize(Archive & ar, std::size_t a_ver)
    {
      ar & parameter_set_;
      ar & rate_;
      ar & specie_list_;
      ar & type_;
    }

    // Provide a description of the chooser state.
    virtual void description(std::ostream & os) const;


   private:
    // Generate a choice object and add to list.
    virtual void add_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params, boost::ptr_vector< base_choice >& choice_list) const = 0;


   public:
    // Generate and add choices to choice list.
    //
    // If choices already exist they are removed and new choices
    // generated.
    void prepare_choices(const std::vector< particle::specie >& species, const geometry::geometry_manager & gman, boost::ptr_vector< base_choice >& choices) const;


   private:
    // Can this specie be used to create a trial of the current type?
    virtual bool is_permitted(const particle::specie & spec) const = 0;


   public:
    // Get chooser's parameter set.
    const std::vector< core::input_parameter_memo >& parameters() const
    {
      return this->parameter_set_;
    }

    // Get chooser's base probability rate.
    double rate() const
    {
      return this->rate_;
    }

    // Check is specie list has been defined.
    bool has_specie_list() const;

    // Get chooser's list of specifically allowed/disallowed 
    // species (may be empty). No list give in input if 
    // return.name().empty()
    const core::input_parameter_memo& specie_list() const
    {
      return this->specie_list_;
    }

    // Get chooser's input file trial "type" name.
    std::string type() const
    {
      return this->type_;
    }

    // Add an input file section.
    //
    // only throw possible should be from os.write() operation
    //
    // The output of this factory method is made up like
    //
    // chooser
    // <call do_write_document>
    // end
    
    void write_document(core::input_document & wr) const;


};

} // namespace trial
#endif
