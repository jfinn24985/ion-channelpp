#ifndef IONCH_OBSERVABLE_TRIAL_OBSERVER_HPP
#define IONCH_OBSERVABLE_TRIAL_OBSERVER_HPP

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
#include "particle/change_hash.hpp"
#include <iostream>
#include <boost/serialization/vector.hpp>
#include <cstddef>
#include <string>
#include <boost/serialization/shared_ptr.hpp>

namespace particle { class change_set; } 
namespace observable { class sampler_meta; } 
namespace core { class input_parameter_memo; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace observable { class base_sink; } 
namespace observable { class report_manager; } 
namespace core { class input_document; } 

namespace observable {

//  Record each trial.
class trial_observer : public tracked_observable
 {
   public:
    // Data for a single trial.
    struct record 
    {
        // The trial's ID
        particle::change_hash hash_;

        // The computed energy
        double energy_;

        // The trial's probabiliy factor
        double probability_factor_;

        // The trial's probabiliy factor
        double exponential_factor_;

        // Did the trial critically fail (ie hard-sphere overlap)
        bool fail_;

        // Did the simulation accept the trial?
        bool accept_;

        record()
        : hash_(), energy_(), probability_factor_(), exponential_factor_(), fail_(), accept_()
        {}
        

        ~record() = default;

        record(record && source)
        : hash_( std::move( source.hash_ ) )
        , energy_( source.energy_ )
        , probability_factor_( source.probability_factor_ )
        , exponential_factor_( source.exponential_factor_ )
        , fail_( source.fail_ )
        , accept_( source.accept_ )
        {}

        record(const record & source)
        : hash_( source.hash_ )
        , energy_( source.energy_ )
        , probability_factor_( source.probability_factor_ )
        , exponential_factor_( source.exponential_factor_ )
        , fail_( source.fail_ )
        , accept_( source.accept_ )
        {}

        void swap(record & source)
        {
          std::swap( hash_, source.hash_ );
          std::swap( energy_, source.energy_ );
          std::swap( probability_factor_, source.probability_factor_ );
          std::swap( exponential_factor_, source.exponential_factor_ );
          std::swap( fail_, source.fail_ );
          std::swap( accept_, source.accept_ );
        }

        record & operator=(record source)
        {
          this->swap( source );
          return *this;
        }
    
  friend class boost::serialization::access;
       private:
        template<class Archive>
          inline void serialize(Archive & ar, const unsigned int version) {
          ar & hash_;
          ar & energy_;
          ar & probability_factor_;
          ar & exponential_factor_;
          ar & fail_;
          ar & accept_;
        }


       public:
        // Set record from a change_set object.
        void set(const particle::change_set & source);

        // Write to a stream.
        void write(std::ostream & os) const
        {
          os << this->hash_ << " " << this->energy_ << " " << this->probability_factor_ << " " << this->exponential_factor_ << " " << this->fail_ << " " << this->accept_ << "\n";
        }

        // Write record to stream
        friend std::ostream& operator<<(std::ostream & os, const record & rhs)
        {
          rhs.write( os );
          return os;
        }


    };
    

   private:
    // The log of the trials.
    std::vector<record> buffer_;

    // The current index into the buffer.
    std::size_t index_;

    // For serialization and factory
    trial_observer();


   public:
    virtual ~trial_observer();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version) {
      ar & boost::serialization::base_object< tracked_observable >(*this);
      ar & buffer_;
      ar & index_;
    }


   public:
    // Add definition for generating objects of this 
    // class to the meta object.
    static void add_definition(sampler_meta & meta);

    //Log message descibing the observable and its parameters
    void description(std::ostream & out) const override;

    std::string get_label() const override;

    //Retrieve the current estimate of the boltzmann factor and
    //energy change.
    //
    ///return Type pair< const vector<record>*, std::size_t >
    boost::any get_value() const override;

    static boost::shared_ptr< tracked_observable > make_sampler(const std::vector< core::input_parameter_memo >& param_set);

    //Collect information about the a_trial move
    virtual void on_trial_end(const particle::change_set & trial) override;

    // Do nothing.
    void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) override;

    // Reporting operations.
    void on_report(std::ostream & out, base_sink & sink) override;

    // Prepare for a main simulation loop
    virtual void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman) override;

    static std::string type_label_();


   private:
    // Add type of sampler to wr[ix] document section
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const;


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::trial_observer );
#endif
