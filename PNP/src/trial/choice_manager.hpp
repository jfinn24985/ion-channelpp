#ifndef IONCH_TRIAL_CHOICE_MANAGER_HPP
#define IONCH_TRIAL_CHOICE_MANAGER_HPP

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

#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <cstddef>
#include "utility/unique_ptr.hpp"
#include <iostream>
#include <string>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace trial { class base_chooser; } 
namespace trial { class base_choice; } 
namespace particle { class change_set; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace utility { class random_distribution; } 
namespace particle { class specie; } 
namespace core { class input_document; } 
namespace core { class input_delegater; } 

namespace trial {

// Manage objects that generate new trials.
//
// The objects that generate trials are created in two steps.
// Reading the input file creates 'chooser' objects. These are
// meta-choice objects that act as factories for the 'choice'
// objects. At the beginning of the simulation run, the real
// choice objects are created based on the meta-choice objects
// and the simulation. For example a "move" chooser object
// instantiates a "move_choice" object for every specie in the
// simulation ensemble.
//
// The 'chooser' objects contain information about a type of
// trial, for example a "move" chooser might represent trials that
// displace a single particle within a small cube. The 'choice'
// objects make this concrete, for example a "move_choice" might
// generate trials that displace in a cube a certain specie in
// a certain sub-region of the simulation.
//
// The 'chooser' objects maintain a probability rate indicating
// the proportion of the number of trials that should be this
// type of trial compared to all trials. The sum of these rates
// need not be unity, the manager will normalize the rates to one
// when needed. A 'choice' object's probability rate indicates
// this specific object's chance of generating a trial. In this
// case the sum of the choice objects' rates will be unity.
// This is enforced by this manager object when it generates
// the choice list.
//
// During a simulation the list of choice objects is used to
// select the current trial move. Before the simulation the list
// is randomised. On a trial a number is generated between 0 and
// 1 and the choice list is walked to find a choice object. At
// each step the current choice's probability is subtracted from
// the generated number and if the number is now less than 0
// the choice object is used.

class choice_manager : public boost::noncopyable
 {
   private:
    // Reading the input file creates this 'chooser' list. These are
    // meta-choice objects that act as factories for the 'choice'
    // objects. The 'chooser' objects contain information about a
    // type of trial, for example a "move" chooser might represent
    // trials that displace a single particle within a small cube,
    // and a probability rate indicating the proportion of the
    // number of trials that should be this type of trial compared
    // to all trials. The sum of these rates need not be unity, the
    // manager will normalize the rates to one when needed. At the
    // start of a simulation the list of choice objects is generated
    // from this list of choosers.
    //
    
    boost::ptr_vector< base_chooser > choosers_;

    // The list of 'choice' objects that generate trials. A 'choice'
    // object's probability rate indicates this specific object's
    // chance of generating a trial. The sum of the choice objects'
    // rates in this list will be unity. During a simulation this
    // list of choice objects is used to select the current trial
    // move. Before the simulation the list is randomised. On a trial
    // a number is generated between 0 and 1 and the choice list
    // is walked to find a choice object. At each step the current
    // choice's probability is subtracted from the generated number
    // and if the number is now less than 0 the choice object is used.
    //
    
    boost::ptr_vector< base_choice > choices_;


   public:
    // Constant iterator over choice list
    typedef boost::ptr_vector< base_choice >::const_iterator const_iterator;

    choice_manager() = default;

    ~choice_manager() = default;

   private:
    //No move
    choice_manager(choice_manager && source) = delete;

    //No copy
    choice_manager(const choice_manager & source) = delete;

    //No assignment
    choice_manager & operator=(const choice_manager & source) = delete;


  friend class boost::serialization::access;
   public:
    //Read/write object to an archive.
    template< class Archive > void serialize(Archive & ar, std::size_t a_ver)
    {
      ar & choosers_;
      ar & choices_;
    }

    // Transfer ownership of chooser 'choice' into our chooser list.
    //
    
    void add_chooser(std::unique_ptr< base_chooser > choice);

    // Iterator over the choice list.
    const_iterator begin() const
    {
      return this->choices_.begin();
    }

    boost::ptr_vector< base_chooser >::const_iterator begin_chooser() const
    {
      return this->choosers_.begin();
    }

    // Details about the current choosers to be written to the
    // log at the start of the simulation.
    void description(std::ostream & os) const;

    // Do we have any choices.
    bool empty() const
    {
      return this->choices_.empty();
    }

    // Do we have and _choosers_?
    bool empty_chooser() const
    {
      return this->choosers_.empty();
    }

    const_iterator end() const
    {
      return this->choices_.end();
    }

    boost::ptr_vector< base_chooser >::const_iterator end_chooser() const
    {
      return this->choosers_.end();
    }

    // Select a choice based on 'selector' and generate a new change set.
    
    std::unique_ptr< particle::change_set > generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr, double selector);

    // Does this manager have a chooser with the given name?
    bool has_chooser(std::string label) const;

    // Are any of the choices grand canonical moves?
    bool is_grand_canonical() const;

    // Create choice list from choosers.  Resets the choice list before
    // generating a new list.
    //
    // Will throw input error if no choices where created.
    // 
    // \pre not empty_chooser
    // \post size /= 0
    void prepare(const std::vector< particle::specie >& species, const geometry::geometry_manager & gman, utility::random_distribution & rgnr);

    // Number of choices.
    std::size_t size() const
    {
      return this->choices_.size();
    }

    // Number of choices.
    std::size_t size_chooser() const
    {
      return this->choosers_.size();
    }

    // Write choosers to an input document
    void write_document(core::input_document & wr);

    // Adds factory methods and input parsers for all the types that can be instantiated from 
    // the input file.
    static void build_input_delegater(boost::shared_ptr< choice_manager > cman, core::input_delegater & delegate);


};

} // namespace trial
#endif
