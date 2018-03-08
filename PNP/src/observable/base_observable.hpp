#ifndef IONCH_OBSERVABLE_BASE_OBSERVABLE_HPP
#define IONCH_OBSERVABLE_BASE_OBSERVABLE_HPP

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

#include <boost/any.hpp>

#include "utility/archive.hpp"

#include <iostream>
#include <string>
#include <cstddef>

namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace observable { class report_manager; } 
namespace observable { class base_sink; } 
namespace core { class input_document; } 
namespace particle { class change_set; } 

namespace observable {

//Perform sampling of a simulation that leads to some estimated
//result. 
//
//Standard operation:
//- create sampler and add to simulator : sim.add_observable(...);
//- after input file has been reador the system is deserialized 
//the owning simulator calls enrol : obs.enrol(sim); This 
//should register any signals produced by the sampler (and 
//optionally connect to simulator signals).
//- before every simulation sequence the simulator object
//calls 'prepare' on its children. The children should connect
//to signals of interest to them and setup internal state in
//preparation for the simulation. At this point the sampler
//should have connected to all signals of interest.
//- simulator runs the simulation, issuing 'inner-loop',
//'outer-loop', 'report' and 'checkpoint' signals.
//- at any point the simulator may call serialize to 
//archive the simulation.
//- destructor is called.
//
//
//The observable_meta class and make_XXX methods for a particular
//derived class are responsible for determining if an observable 
//is useable with the given simulation type.  This mechanism
//allows make_XXX methods to choose between particular derived 
//types that sample the same 'observation' in different
//simulation subtypes (bulk, channel etc).

class base_observable
 {

  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
    }


   public:
    virtual ~base_observable();

    base_observable() = default;


   private:
    base_observable(base_observable && source) = delete;

    base_observable(const base_observable & source) = delete;

    base_observable & operator=(const base_observable & source) = delete;


   public:
    //Log message descibing the observable and its parameters
    virtual void description(std::ostream & out) const = 0;

    //The human readable label of this variable.
    //
    //General format will be "{type-label}[:obj-label]"
    //
    //For derived classes that will only have a single 
    //sampler instantiated, this label will be the same
    //as the sampler type-label in the input document.
    virtual std::string get_label() const = 0;

    //Retrieve the current value of the variable.
    virtual boost::any get_value() const = 0;

    // Prepare the sampler for use with the given simulator. Reset
    // all data.
    virtual void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman) = 0;

    // Save statistical data to output log and permanent storage
    virtual void on_report(std::ostream & out, base_sink & sink) = 0;

    //Accumulate data after a sequence of trials.
    virtual void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) = 0;

    // Add an input file section to wr.
    //
    // Output of this method is something like
    //
    // sampler
    // <call do_write_input_section>
    // end
    
    void write_document(core::input_document & wr) const;


   private:
    // Update wr[ix] section with information from the derived class.
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const = 0;


};
//A dependent variable that may obtain a new value after every 
//trial.
class tracked_observable : public base_observable
 {
   public:
    tracked_observable() = default;

    virtual ~tracked_observable();


   private:
    tracked_observable(tracked_observable & source) = delete;

    tracked_observable(tracked_observable && source) = delete;

    tracked_observable & operator=(const tracked_observable & source) = delete;


  friend class boost::serialization::access;
   public:
    //Save variable to checkpoint
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< base_observable >(*this);
    }

    // Update variable after every trial.
    virtual void on_trial_end(const particle::change_set & trial) = 0;


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::tracked_observable );
#endif
