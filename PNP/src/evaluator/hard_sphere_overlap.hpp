#ifndef IONCH_EVALUATOR_HARD_SPHERE_OVERLAP_HPP
#define IONCH_EVALUATOR_HARD_SPHERE_OVERLAP_HPP

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

#include "evaluator/base_evaluator.hpp"
#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"
#include <boost/serialization/vector.hpp>
#include "utility/archive.hpp"

#include <iostream>
#include <string>
#include <cstddef>
#include "utility/unique_ptr.hpp"

namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace particle { class change_set; } 
namespace evaluator { class evaluator_manager; } 
namespace core { class input_document; } 
namespace evaluator { class evaluator_meta; } 
namespace core { class input_parameter_memo; } 

namespace evaluator {

// Calculate if particles overlap as hard-spheres.
class hard_sphere_overlap : public base_evaluator
 {
   private:
    // Cached specie radii
    std::vector<double> radii_;


   public:
    virtual ~hard_sphere_overlap() {}


  friend class boost::serialization::access;
   private:
    //Serialize all but the observable set.
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
      ar & boost::serialization::base_object< base_evaluator >(*this);
      ar & this->radii_;
      };


   public:
    // Energy is always zero, but set fail if particles overlap as
    // hard spheres.
    void compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const override;

    // Assume no-overlap in verified ensemble, so energy is always zero.
    double compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const override;


   private:
    // Log message descibing the derived evaluator and its parameters
    virtual void do_description(std::ostream & os) const override;


   public:
    // Capture specie radii.
    void prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman) override;

    std::string type_label() const override
    {
      return hard_sphere_overlap::type_label_();
    }

    static std::string type_label_();


   private:
    // No derived content
    void do_write_document(core::input_document & wr, std::size_t ix) const override;


   public:
    // Create and add a evaluator_definition to the meta object.
    static void add_definition(evaluator_meta & meta);

    // Set up the evaluator using the given map of name/value pairs.
    static std::unique_ptr< base_evaluator > make_evaluator(const std::vector< core::input_parameter_memo > & param_set);


};

} // namespace evaluator

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( evaluator::hard_sphere_overlap );

#endif
