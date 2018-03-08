#ifndef IONCH_OBSERVABLE_WIDOM_HPP
#define IONCH_OBSERVABLE_WIDOM_HPP

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
#include "utility/estimater.hpp"
#include <cstddef>
#include <string>
#include <iostream>
#include <boost/serialization/vector.hpp>
#include "utility/random.hpp"
#include "utility/archive.hpp"

#include <boost/serialization/shared_ptr.hpp>

namespace trial { class add_specie; } 
namespace trial { class base_choice; } 
namespace particle { class change_atom; } 
namespace particle { class ensemble; } 
namespace utility { class fp_env; } 
namespace core { class input_reader; } 
namespace core { class strngs; } 
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

// Widom observable, calculate the excess chemical potential from insertion
// trials. This class calculates an average global value.
class widom : public tracked_observable
 {
   public:
    //Widom based free energy estimate for one specie.
    struct widom_datum
     {
        // The average number of particles of this specie
        utility::estimater specie_count_;

        //The probability {exp(-energy)} based chemical potential estimate
        utility::estimater exp_potential_;

        //The count of the number of add attempts completed so far in
        //one sampling cycle.
        std::size_t count_;

        // Species target concentration.
        double conc_;

        // The excess chemical potential of the specie at the start of the
        // simulation.
        double excess_potential_;

        //The specie index of this datum.
        std::size_t key_;

        // The label of the specie.
        std::string label_;

        // Volume of simulation for this specie
        double volume_;

    
  friend class boost::serialization::access;
       private:
        template<class Archive>
        inline void serialize(Archive & ar, const unsigned int version)
        {
          ar & specie_count_;
          ar & exp_potential_;
          ar & count_;
          ar & conc_;
          ar & excess_potential_;
          ar & key_;
          ar & label_;
          ar & volume_;
        }


       public:
        widom_datum()
        : specie_count_()
        , exp_potential_()
        , count_()
        , conc_()
        , excess_potential_()
        , key_()
        , label_()
        , volume_()
        {}

        //Constructor with specie key
        widom_datum(std::size_t key)
        : specie_count_()
        , exp_potential_()
        , count_()
        , conc_()
        , excess_potential_()
        , key_( key )
        , label_()
        , volume_()
        {}

        ~widom_datum()
        {}

        widom_datum(widom_datum && source)
        : specie_count_( std::move( source.specie_count_ ) )
        , exp_potential_( std::move( source.exp_potential_ ) )
        , count_( std::move( source.count_ ) )
        , conc_( std::move( source.conc_ ) )
        , excess_potential_( std::move( source.excess_potential_ ) )
        , key_( std::move( source.key_ ) )
        , label_( std::move( source.label_ ) )
        , volume_( std::move( source.volume_ ) )
        {}

        widom_datum(const widom_datum & source)
        : specie_count_( source.specie_count_ )
        , exp_potential_( source.exp_potential_ )
        , count_( source.count_ )
        , conc_( source.conc_ )
        , excess_potential_( source.excess_potential_ )
        , key_( source.key_ )
        , label_( source.label_ )
        , volume_( source.volume_ )
        {}

        widom_datum & operator=(widom_datum source)
        {
          this->swap(source);
          return *this;
        }

        void swap(widom_datum & source)
        {
           std::swap( this->specie_count_, source.specie_count_ );
           std::swap( this->exp_potential_, source.exp_potential_ );
           std::swap( this->count_, source.count_ );
           std::swap( this->conc_, source.conc_ );
           std::swap( this->excess_potential_, source.excess_potential_ );
           std::swap( this->key_, source.key_ );
           std::swap( this->label_, source.label_ );
           std::swap( this->volume_, source.volume_ );
        }

        // Output a space separated list of the field labels.
        virtual void labels(std::ostream & os) const;

        // Merge the given row into this row.
        virtual void merge(widom_datum & source);

        // Output a space separated list of the field entries.
        virtual void row(std::ostream & os) const;

        // Output a space separated list of the field units.
        virtual void units(std::ostream & os) const;

        // Check that source datum specie matches ours.
        bool same_specie(const widom_datum & source) const
        {
          return this->key_ == source.key_;
        }

        void reset();


    };
    

   private:
    std::vector< widom_datum > data_;

    // Optional restricted list of specie labels to use, from the input file. If empty use
    // all SOLUTE species.
    std::string key_labels_;

    //(Justin) Use a separate random number
    //generator for the widom sampler so that runs of 
    //the main simulation should be identical regardless
    //of whether the Widom sampler is used or not. 
    utility::random_distribution ranf_;

    // Number of trials to make per sample cycle, default is 0
    std::size_t trials_;

    // How many times have samples been taken?
    std::size_t loop_count_;

    // Counter for how many times report has been called.
    std::size_t rank_;


  friend class boost::serialization::access;    template<class Archive>
    inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< tracked_observable >(*this);
      ar & data_;
      ar & key_labels_;
      ar & ranf_;
      ar & trials_;
      ar & loop_count_;
      ar & rank_;
    }

    // default ctor (for serialize and factory only)
    widom();


   public:
    virtual ~widom();

    // Add definition for generating objects of this 
    // class to the meta object.
    static void add_definition(sampler_meta & meta);

    //Log message descibing the observable and its parameters
    //
    //TODO: Develop description further
    void description(std::ostream & out) const override;

    std::string get_label() const override
    {
      return type_label_();
    }

    //value object is of type vector< widom_datum >
    boost::any get_value() const override;

    // Create a new widom sampler using the given parameter set.
    static boost::shared_ptr< tracked_observable > make_sampler(std::vector< core::input_parameter_memo > const& params);

    // Save energy estimates to permanent storage
    void on_report(std::ostream & out, base_sink & sink) override;

    // Perform insertion trials to reach 'trials' attempts per sample cycle.
    void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) override;

    // Update variable after every trial. Sample every insertion attempt.
    virtual void on_trial_end(const particle::change_set & trial) override;

    // Prepare Widom observable for a new simulation
    //
    // Reset all accumulated data. Build list of specie
    // keys from input list of labels (key_labels) and
    // adjust key_labels to contain only used keys. 
    // Allocate storage for accumulating data once the
    // number of species to observe is known. 
    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman) override;

    static std::string type_label_()
    {
      return std::string ("widom");
    }

    // Add details of widom sampler to wr[ix].
    void do_write_document(core::input_document & wr, std::size_t ix) const override;

    //The number of trials to make per sampling cycle.
    std::size_t loop_count() const
    {
      return this->loop_count_;
    }

    // The number of species to be sampled.
    std::size_t size() const
    {
      return this->data_.size();
    }

    //(optional) list of specie labels from input that
    //limit the specie to use.  An empty string means
    //all solute type species will be used.
    std::string specie_list() const
    {
      return this->key_labels_;
    }

    //Is specie of given index target of Widom observation?
    //
    ///nothrow
    bool specie_of_interest(std::size_t idx) const;
    

    //The number of trials to make per sampling cycle.
    std::size_t trials() const
    {
      return this->trials_;
    }


   private:
    // Collect a sample from a trial
    void update_data(const particle::change_atom & atom, bool is_fail);


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::widom );
#endif
