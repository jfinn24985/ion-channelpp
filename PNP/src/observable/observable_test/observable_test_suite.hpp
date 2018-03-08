#ifndef IONCH__OBSERVABLE_TEST_SUITE_HPP
#define IONCH__OBSERVABLE_TEST_SUITE_HPP

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
#include <string>

namespace observable { class acceptance_observable; } 
namespace core { class constants; } 
namespace observable { class d3_distribution; } 
namespace observable { class density_zaxis; } 
namespace geometry { class digitizer_3d; } 
namespace observable { class metropolis_sampler; } 
namespace observable { class output_field; } 
namespace observable { class output_series; } 
namespace observable { class rdf_sampler; } 
namespace observable { class report_manager; } 
namespace observable { class sampler_definition; } 
namespace observable { class sampler_meta; } 
namespace observable { class specie_count; } 
namespace observable { class specie_region_count; } 
namespace core { class strngs; } 
namespace observable { class tracked_definition; } 
namespace observable { class tracked_observable; } 
namespace observable { class trial_observer; } 
namespace observable { class widom; } 
namespace core { class input_parameter_memo; } 
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace evaluator { class evaluator_manager; } 
namespace observable { class base_observable; } 

class observable_test_suite
 {
   public:
    static std::vector< core::input_parameter_memo > mockup_params();

    // Mockup set of realistic species.
    //
    //  specie_count = 5
    //  solute = "Na" (x2 particles) and "Cl" (x1 particle)
    //  mobile = "CA" (x1 particle)
    //  flexible = "CO" (x1 particle)
    //  channel only = "OX" (x1 particle)
    //  
    static boost::shared_ptr< particle::particle_manager > mockup_particle_manager();

    // Create geometry manager with PBC cube "cell" and 
    // open cube "bulk" regions.
    static boost::shared_ptr< geometry::geometry_manager > mockup_geometry_manager();

    // Mockup evaluator
    //
    //  size = 1
    //  get_evaluator()[ 0 ].type_label = "coulomb"
    //  
    static boost::shared_ptr< evaluator::evaluator_manager > mockup_evaluator_manager();

    // Series of tests to exercise base observable class operations.
    //
    // - serialize
    // - get_label
    // - description
    // - write_document
    static void base_observable_method_test(boost::shared_ptr< observable::base_observable > obs, std::string label);










};
#endif
