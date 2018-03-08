#ifndef IONCH_OBSERVABLE_RDF_SAMPLER_HPP
#define IONCH_OBSERVABLE_RDF_SAMPLER_HPP

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
#include "utility/archive.hpp"

#include <boost/serialization/vector.hpp>
#include <string>
#include <iostream>
#include <boost/serialization/shared_ptr.hpp>
#include <cstddef>

namespace utility { class histogram; } 
namespace observable { class sampler_meta; } 
namespace core { class input_parameter_memo; } 
namespace observable { class base_sink; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace observable { class report_manager; } 
namespace core { class input_document; } 

namespace observable {

//sample a radial distribution function around each ion collated by
//ion specie

class rdf_sampler : public base_observable
 {
   private:
    // The radial  distribution functions.
    //
    // As these are symmetric with respect to the specie, only the
    // "upper triangle" of the matrix is saved : M(i,j) where i<=j.
    // The data is linearly so care must be taken when calculating
    // the index. 
    //
    // Where N is the number of specie:
    //   M(i,j){i<=j} = data_sets_[(2N+1-i)*i/2 + j]
    //   data_sets_.size = N*(N+1)/2
    std::vector< utility::histogram > data_sets_;

    // Vector of specie labels.
    std::vector<std::string> specie_labels_;

    //The width of the histogram bins (default 0.2).  Narrower bins
    //gives better detail but requires longer runs.
    double stepsize_;

    // The width of the total RDF histogram. Acts as a cut-off.
    double width_;

    // For serialization and factory only
    rdf_sampler();


   public:
    virtual ~rdf_sampler();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< base_observable >(*this);
      ar & data_sets_;
      ar & specie_labels_;
      ar & stepsize_;
      ar & width_;
    }


   public:
    // Add definition for generating objects of this 
    // class to the meta object.
    static void add_definition(sampler_meta & meta);

    // Get the default bin/step size
    static double default_bin_width()
    {
      return 0.2;
    }

    // Get the default sampled width (this is starting value
    static double default_width()
    {
      return 20.0;
    }

    //Log message descibing the observable and its parameters
    //
    //TODO: describe specific RDF functions (if not default)
    void description(std::ostream & out) const override;

    //Retrieve the radial distribution profiles
    //
    ///return vector< estimate_array > const*const where
    //  rdf(i, j) is at index [i x specie_count + j]
    //
    //
    //
    
    boost::any get_value() const override;

    //Type label to use in input file for this observable
    std::string get_label() const
    {
      return rdf_sampler::type_label_();
    }

    // Make an rdf_observable (independent of type) from input file
    //
    // parameters : "region" "stepsize"
    static boost::shared_ptr< base_observable > make_sampler(std::vector< core::input_parameter_memo > const& param_set);

    // Report radial distribution/density profiles
    void on_report(std::ostream & out, base_sink & sink) override;

    void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) override;

    // Prepare the sampler for a simulation.
    //
    // Initialize histogram matrix and capture specie labels.
    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman) override;

    //Type label is "rdf-specie"
    static std::string type_label_();


   private:
    // Add sampler type, width and stepsize to wr[ix]
    void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Get histogram for specie pair (i,j)
    //
    // \pre ispec < specie_count and jspec < specie_count
    const utility::histogram& get_histogram(std::size_t ispec, std::size_t jspec) const;


   private:
    // Get histogram index for specie pair (i,j)
    //
    // ASSUMES \pre ispec <= jspec < specie_count
    std::size_t get_index(std::size_t ispec, std::size_t jspec) const
    {
      return (((2 * this->specie_count() + 1 - ispec) * ispec ) / 2) + (jspec - ispec);
    }


   public:
    // The number of pairwise data sets
    std::size_t size() const
    {
      return this->data_sets_.size();
    }

    // The number of species.
    //
    // Only meaningful after "prepare" is called.
    std::size_t specie_count() const
    {
      return this->specie_labels_.size();
    }

    // Get specie label.
    //
    // Only valid after "prepare".
    //
    // \pre ispec < specie_count
    std::string specie_label(std::size_t ispec) const
    {
      return this->specie_labels_.at( ispec );
    }

    // The actual step size used
    double stepsize() const
    {
      return this->stepsize_;
    }

    // The actual step size used
    double width() const
    {
      return this->width_;
    }


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::rdf_sampler );
#endif
