#ifndef IONCH_OBSERVABLE_REPORT_MANAGER_HPP
#define IONCH_OBSERVABLE_REPORT_MANAGER_HPP

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

#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <iostream>
#include <string>
#include <cstddef>

namespace observable { class tracked_observable; } 
namespace observable { class base_observable; } 
namespace observable { class base_sink; } 
namespace particle { class change_set; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace core { class input_document; } 
namespace core { class input_delegater; } 

namespace observable {

//Manage statistical accumlators
//
//Design concept.
//
//The report manager provides an interface between the statistics samplers
//and the rest of the simulation.  Samplers register to collect data in
//either the inner loop (tracked_observable) or outer loop 
//(base_observable). During the simulation the simulator
//calls on_trial_end and on_sample of the manager, which then calls the
//registered samplers. At some point the simulator calls on_report which
//signals the samplers to save aggregated data to permanent storage. The
//report_manager manages the connection between the samplers and storage.
//
//The system uses shared pointers to the samplers. On checkpointing the
//report_manager will pass on write_document and serialize calls to the
//samplers. This ensures that any samplers not shared with another object
//will be correctly saved.

class report_manager
 {
  friend class boost::serialization::access;

   private:
    std::vector< boost::shared_ptr< tracked_observable > > variables_;

    std::vector< boost::shared_ptr< base_observable > > samples_;
    //Manages serialization of output data to long-term storage.
    boost::shared_ptr< base_sink > sink_;

// LIFETIME METHODS


   public:
    report_manager();

    ~report_manager(){}


   private:
    report_manager(report_manager && source) = delete;

    report_manager(const report_manager & source) = delete;

    report_manager & operator=(const report_manager & source) = delete;

    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      // data_source_ not serialized.
      ar & this->variables_;
      ar & this->samples_;
      ar & this->sink_;
    }

// MODIFY METHODS


   public:
    //add sampled variable to list.
    //
    ///pre not has_sample(var.label())
    void add_sample(boost::shared_ptr< base_observable > var);

    //add variable to list.
    //
    ///pre not has_tracked(var.label())
    void add_tracked(boost::shared_ptr< tracked_observable > var);

    //Accumulate data after every trial
    void on_trial_end(const particle::change_set & trial);

    //Accumulate data after a sequence of trials
    void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman);

    // Get all samplers to save statistical data to permanent storage
    //
    // \pre has_sink
    void on_report(std::ostream & out);

    // Get all samplers to prepare themselves for a simulation. Check for
    // connection to signals of interest and connect if necessary.
    
    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) const;


   private:
    // remove sampled variable from list.
    //
    // \pre has_sample(var->get_label()) and get_sample(var->get_label()).get() == var.get()
    // \post not has_sample(var->get_label())
    void remove_sample(boost::shared_ptr< base_observable > & var);

    // remove tracked variable from list.
    //
    // \pre has_tracked(var->get_label()) and get_tracked(var->get_label()).get() == var.get()
    //
    // \post not has_sample(var->get_label())
    void remove_tracked(boost::shared_ptr< tracked_observable > & var);


   public:
    // Set the object that manages where the output data goes. Silently
    // overwrites any previous value.
    void set_sink(boost::shared_ptr< base_sink > sink)
    {
      this->sink_ = sink;
    }

// ACCESSOR METHODS

    // Get all samplers to write their descriptions.
    //
    // \pre has_sink
    void description(std::ostream & out) const;

    // Are there any observables?
    bool empty() const
    {
      return this->variables_.empty() and this->samples_.empty();
    }

    // Get reference to the data output sink
    //
    // \pre has_sink
    base_sink & get_sink() const;

    //Attempt to retrieve a variable by label.
    //
    ///pre has_sample
    boost::shared_ptr< base_observable > get_sample(std::string name);

    //Attempt to retrieve a variable by label.
    //
    ///pre has_sample
    boost::shared_ptr< base_observable > get_sample(std::string name) const;

    //Attempt to retrieve a variable by label.
    //
    ///pre has_tracked
    boost::shared_ptr< tracked_observable > get_tracked(std::string name);

    //Attempt to retrieve a variable by label.
    //
    ///pre has_tracked
    boost::shared_ptr< tracked_observable > get_tracked(std::string name) const;

    // Has the output sink been set?
    bool has_sink() const
    {
      return ( this->sink_.get() != nullptr );
    }

    //Attempt to retrieve a variable by label
    //
    ///nothrow
    bool has_sample(std::string name) const;

    //Attempt to retrieve a variable by label
    //
    ///nothrow
    bool has_tracked(std::string name) const;

    // How many observables?
    std::size_t size() const
    {
      return this->variables_.size() + this->samples_.size();
    }

    // Get all samplers to write their input information.
    void write_document(core::input_document & wr) const;


   private:
    //Retrieve iterator pointing to sampled variable
    //with the given name or "end".
    //
    ///nothrow
    
    std::vector< boost::shared_ptr< base_observable > >::iterator get_sample_(std::string name);

    //Retrieve iterator pointing to sampled variable
    //with the given name or "end".
    //
    ///nothrow
    
    std::vector< boost::shared_ptr< base_observable > >::const_iterator get_sample_(std::string name) const;

    //Retrieve iterator pointing to tracked variable
    //with the given name or "end".
    //
    ///nothrow
    
    std::vector< boost::shared_ptr< tracked_observable > >::iterator get_tracked_(std::string name);

    //Retrieve iterator pointing to tracked variable
    //with the given name or "end".
    //
    ///nothrow
    
    std::vector< boost::shared_ptr< tracked_observable > >::const_iterator get_tracked_(std::string name) const;


   public:
    // Adds factory methods and input parsers for all the types that can be instantiated from 
    // the input file.
    static void build_input_delegater(boost::shared_ptr< report_manager > rman, core::input_delegater & delegate);


};

} // namespace observable
#endif
