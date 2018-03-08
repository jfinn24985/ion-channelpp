#ifndef IONCH_OBSERVABLE_D3_DISTRIBUTION_HPP
#define IONCH_OBSERVABLE_D3_DISTRIBUTION_HPP

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
#include "utility/fixed_size_histogram.hpp"
#include "geometry/digitizer_3d.hpp"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <string>
#include <cstddef>
#include <iostream>
#include <boost/serialization/shared_ptr.hpp>

namespace core { class strngs; } 
namespace observable { class sampler_meta; } 
namespace core { class input_parameter_memo; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace observable { class report_manager; } 
namespace observable { class base_sink; } 
namespace core { class input_document; } 

namespace observable {

//Sample the 3D distribution of particles. 
class d3_distribution : public base_observable
 {
   public:
    typedef utility::fixed_size_histogram<geometry::digitizer_3d> histogram_type;


   private:
    // Particle distributions indexed by specie key
    std::vector< histogram_type > distributions_;

    // Map of specie labels to regions.
    std::map<std::string, std::string> regions_;

    // Vector of specie labels.
    std::vector<std::string> specie_labels_;

    // The width of the histogram bins (default 0.2).  Narrower bins
    // gives better detail but requires more memory and converge 
    // slower.
    double stepsize_;

    // For serialization
    d3_distribution();


   public:
    virtual ~d3_distribution() {}

  friend class boost::serialization::access;
    template< class Archive > void serialize(Archive & ar, std::size_t version)
    {
      ar & boost::serialization::base_object< base_observable >(*this);
      ar & this->distributions_;
      ar & this->regions_;
      ar & this->specie_labels_;
      ar & this->stepsize_;
    }

    // Add definition for generating objects of this 
    // class to the meta object.
    static void add_definition(sampler_meta & meta);

    // Get the default bin/step size
    static double default_bin_width()
    {
      return 0.2;
    }

    //Log message descibing the observable and its parameters
    void description(std::ostream & os) const;

    //Retrieve the radial distribution profiles
    //
    ///return vector< estimate_array > const*const where
    //  rdf(i, j) is at index [i x specie_count + j]
    //
    //
    //
    
    boost::any get_value() const override;

    std::string get_label() const override
    {
      return d3_distribution::type_label_();
    }

    static boost::shared_ptr< base_observable > make_sampler(std::vector< core::input_parameter_memo > const& paramset);

    // Prepare the evaluator for a simulation.
    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman) override;

    // Save 3d average particle counts to a data sink.
    void on_report(std::ostream & out, base_sink & sink) override;

    // Sample particle density
    virtual void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) override;

    static std::string type_label_();


   private:
    // Write an input file section.
    //
    // only throw possible should be from os.write() operation
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Access a particular histogram
    //
    // \pre ispec < size
    const histogram_type& get_histogram(std::size_t ispec) const
    {
      return this->distributions_.at( ispec );
    }

    // Parse a list of "label:region" pairs into the map of specie
    // label to regions.
    
    static std::map< std::string, std::string > process_region_list(std::string list);

    // Access the specie label : region name map.
    const std::map< std::string, std::string >& region_map() const
    {
      return this->regions_;
    }

    // The number of distributions
    //
    // \invariant size <= {specie_count}
    std::size_t size() const
    {
      return this->distributions_.size();
    }

    // Get the objects defined step size
    double stepsize() const
    {
      return this->stepsize_;
    }


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::d3_distribution );
#endif
