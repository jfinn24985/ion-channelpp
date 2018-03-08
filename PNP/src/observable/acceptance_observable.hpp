#ifndef IONCH_OBSERVABLE_ACCEPTANCE_OBSERVABLE_HPP
#define IONCH_OBSERVABLE_ACCEPTANCE_OBSERVABLE_HPP

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

#include "observable/base_observable.hpp"
#include <boost/serialization/map.hpp>
#include <cstddef>
#include <iostream>
#include <string>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace utility { class estimate_array; } 
namespace particle { class change_hash; } 
namespace observable { class sampler_meta; } 
namespace core { class input_parameter_memo; } 
namespace observable { class base_sink; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace particle { class change_set; } 
namespace observable { class report_manager; } 
namespace core { class input_document; } 

namespace observable {

class acceptance_observable : public tracked_observable
 {
   private:
    // Estimates of the success trials.
    std::map< particle::change_hash, utility::estimate_array > data_;

    // Sampling data: Map of change_hash ids to attempt and success counts.
    std::map< particle::change_hash, std::pair< std::size_t, std::size_t > > dictionary_;

    //The total number of attempted trials.
    std::size_t total_;

    // Once a report has been generated the object can no longer
    // extend the set of reported trials.
    bool locked_;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< tracked_observable >(*this);
      ar & this->data_;
      ar & this->dictionary_;
      ar & this->total_;
      ar & this->locked_;
    }


   protected:
    //Only construct through make_sampler and serialize
    acceptance_observable()
    : data_(), dictionary_(), total_(), locked_( false ) {}


   public:
    virtual ~acceptance_observable() {}


   private:
    //no move
    acceptance_observable(acceptance_observable && source) = delete;

    //no copy
    acceptance_observable(const acceptance_observable & source) = delete;

    //no assignment
    acceptance_observable & operator=(const acceptance_observable & source) = delete;


   public:
    // Add definition for generating objects of this 
    // class to the meta object.
    static void add_definition(sampler_meta & meta);

    // Log message descibing the observable and its parameters
    virtual void description(std::ostream & out) const override;

    //The human readable label of this variable.
    //
    //General format will be "{type-label}[:obj-label]"
    //
    //For derived classes that will only have a single 
    //sampler instantiated, this label will be the same
    //as the sampler type-label in the input document.
    virtual std::string get_label() const override
    {
      return acceptance_observable::type_label_();
    }

    //Retrieve the current value of the variable. In this case the
    //value is of type map< change_hash, estimater_array >.
    virtual boost::any get_value() const override;

    //Make a trial_acceptance_observable from input file
    //
    //no parameters
    static boost::shared_ptr< tracked_observable > make_sampler(std::vector< core::input_parameter_memo > const& param_set);

    // Save statistical data to permanent storage. 
    virtual void on_report(std::ostream & out, base_sink & sink) override;

    //Accumulate data after a sequence of trials.
    virtual void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) override;

    //Update variable after every trial.
    virtual void on_trial_end(const particle::change_set & trial) override;

    // Prepare the sampler for a simulation phase. Reset
    // all data.
    virtual void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman) override;

    //Type label is "trial-success"
    static std::string type_label_();


   private:
    // Write an input file section.
    //
    // only throw possible should be from os.write() operation
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Does this have an estimate of the acceptance data?
    //
    // This requires at least one sampling cycle to be complete
    // before it will be true.
    const utility::estimate_array& get_datum(const particle::change_hash & id) const;

    // Get counts of acceptance and trials in the current
    // sample cycle.
    //
    // \pre has_sample( id )
    std::pair< std::size_t, std::size_t > get_sample(const particle::change_hash & id) const;

    // Does this have an estimate of the acceptance data?
    //
    // This requires at least one sampling cycle to be complete
    // before it will be true.
    bool has_datum(const particle::change_hash & id) const;

    // Get counts of acceptance and trials in the current
    // sample cycle.
    //
    // \pre has_sample( id )
    bool has_sample(const particle::change_hash & id) const;

    // The number of trials found during the current sampling phase.
    std::size_t total_trials() const
    {
      return this->total_;
    }

    // Can we extend the reported trials?
    bool locked() const
    {
      return this->locked_;
    }


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::acceptance_observable );
#endif
