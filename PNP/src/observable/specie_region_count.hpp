#ifndef IONCH_OBSERVABLE_SPECIE_REGION_COUNT_HPP
#define IONCH_OBSERVABLE_SPECIE_REGION_COUNT_HPP

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
#include "utility/estimate_array.hpp"
#include <boost/serialization/vector.hpp>
#include <cstddef>
#include <iostream>
#include <string>
#include <boost/serialization/shared_ptr.hpp>

namespace observable { class output_series; } 
namespace core { class strngs; } 
namespace observable { class sampler_meta; } 
namespace core { class input_parameter_memo; } 
namespace observable { class base_sink; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace observable { class report_manager; } 
namespace core { class input_document; } 

namespace observable {

// Observe and report the average number of particles per region with
// periodic sampling.
class specie_region_count : public base_observable
 {
   private:
    // The particle count estimates.
    utility::estimate_array count_;

    // The per-region volumes for each specie. vol(reg,spc) = volume_[ ireg * nspec = ispec ].
    std::vector< double > volume_;

    // The number of active species.
    std::size_t nspec_;

    // The number of times we have reported data.
    std::size_t rank_;

    // For serialization and factory
    specie_region_count() = default;


   public:
    virtual ~specie_region_count();

  friend class boost::serialization::access;
    template< class Archive > void serialize(Archive & ar, std::size_t version)
    {
      ar & boost::serialization::base_object< base_observable >(*this);
      ar & this->count_; ar & this->volume_; ar & this->nspec_; ar & this->rank_;
    }

    // Add definition for generating objects of this 
    // class to the meta object.
    static void add_definition(sampler_meta & meta);

    //Log message descibing the observable and its parameters
    void description(std::ostream & out) const override;

    std::string get_label() const override
    {
      return specie_region_count::type_label_();
    }

    //Retrieve the current specie count estimates
    //
    // \return type estimater_array const*const
    boost::any get_value() const override;

    // Generate specie_count object using data in paramset. As this
    // sampler takes no special parameters the paramset should be
    // empty.
    static boost::shared_ptr< base_observable > make_sampler(std::vector< core::input_parameter_memo > const& paramset);

    // Write the average specie counts to out and the data sink.
    void on_report(std::ostream & out, base_sink & sink) override;

    // Sample average particle numbers.
    void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) override;

    // Prepare the sampler for a simulation run. Reset any existing data.
    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman) override;

    static std::string type_label_();


   private:
    // Write an input file section.
    //
    // only throw possible should be from os.write() operation
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Do we have any estimates of specie count
    bool empty() const
    {
      return this->count_.empty();
    }

    // Mean count of specie at given index
    //
    // \pre idx < size
    double mean(std::size_t idx) const
    {
      return this->count_.mean( idx );
    }
    

    // How many estimates do we have
    std::size_t size() const
    {
      return this->count_.size();
    }

    // Variance of the count of specie at given index
    //
    // \pre idx < size
    double variance(std::size_t idx) const
    {
      return this->count_.variance( idx );
    }
    

    // Mean count of specie at given index
    //
    // \pre idx < size
    double volume(std::size_t idx) const
    {
      return this->volume_[ idx ];
    }
    


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::specie_region_count );
#endif
