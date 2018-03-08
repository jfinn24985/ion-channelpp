#ifndef IONCH_OBSERVABLE_DENSITY_ZAXIS_HPP
#define IONCH_OBSERVABLE_DENSITY_ZAXIS_HPP

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
#include "utility/histogram.hpp"
#include <string>
#include <iostream>
#include <boost/serialization/shared_ptr.hpp>
#include <cstddef>

namespace observable { class sampler_meta; } 
namespace core { class input_parameter_memo; } 
namespace observable { class base_sink; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace observable { class report_manager; } 
namespace core { class input_document; } 

namespace observable {

// sample a density distribution along the z axis (axis of rotation)

class density_zaxis : public base_observable
 {
   private:
    // Per-specie 1D population histograms.
    std::vector< utility::histogram > data_sets_;

    // The names of the output data. Indexing is the same as for data_sets_
    std::vector<std::string> filenames_;

    //The width of the histogram bins (default 0.2).  Narrower bins
    //gives better detail but requires longer runs.
    double stepsize_;

    // For serialization and factory only
    density_zaxis();


   public:
    virtual ~density_zaxis();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< base_observable >(*this);
      ar & this->data_sets_;
      ar & this->filenames_;
      ar & this->stepsize_;
    }


   public:
    // Add definition for generating objects of this 
    // class to the meta object.
    static void add_definition(sampler_meta & meta);

    // Log message descibing the observable and its parameters
    void description(std::ostream & out) const override;

    // The default step size in the digitizers
    static double default_stepsize()
    {
      return 0.2;
    }

    //Type label to access an object of this type in a report_manager and use in 
    //input file for this observable
    std::string get_label() const override
    {
      return density_zaxis::type_label_();
    }

    //Retrieve the current z-axis density profiles.
    //
    //return value is of type const*const vector< estimate_array >
    boost::any get_value() const override;

    // Make a z-axis population sampler
    //
    // Allowed parameters : "stepsize"
    static boost::shared_ptr< base_observable > make_sampler(std::vector< core::input_parameter_memo > const& param_set);

    // Report 1D population histograms
    void on_report(std::ostream & out, base_sink & sink) override;

    void on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman) override;

    // Prepare the observer for a simulation. Reset any existing data.
    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman) override;

    //Type label is "population-zaxis"
    static std::string type_label_();


   private:
    // Add sampler type, width and stepsize to wr[ix]
    virtual void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Data set for specie ispec
    //
    // \pre ispec < size
    utility::histogram data_set(std::size_t ispec) const;

    // The output filename for specie ispec
    //
    // \pre ispec < size
    std::string filename(std::size_t ispec) const
    {
      return this->filenames_.at( ispec );
    }

    // The number of data sets.
    std::size_t size() const;

    // Get the objects defined step size
    double stepsize() const
    {
      return this->stepsize_;
    }


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::density_zaxis );
#endif
