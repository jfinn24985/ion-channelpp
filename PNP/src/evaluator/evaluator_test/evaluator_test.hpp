#ifndef IONCH_EVALUATOR_EVALUATOR_TEST_HPP
#define IONCH_EVALUATOR_EVALUATOR_TEST_HPP

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

namespace particle { class change_atom; } 
namespace evaluator { class coulomb; } 
namespace evaluator { class evaluator_definition; } 
namespace evaluator { class evaluator_manager; } 
namespace evaluator { class evaluator_meta; } 
namespace evaluator { class hard_sphere_overlap; } 
namespace evaluator { class icc_fortran; } 
namespace evaluator { class icc_matrix; } 
namespace evaluator { class icc_surface_grid; } 
namespace evaluator { class induced_charge; } 
namespace evaluator { class integrator; } 
namespace evaluator { class lennard_jones; } 
namespace evaluator { class localizer; } 
namespace evaluator { class object_overlap; } 
namespace evaluator { class patch_matrix_fixture; } 
namespace core { class input_parameter_memo; } 

namespace evaluator {

class evaluator_test
 {
   public:
    static std::vector< core::input_parameter_memo > mockup_params();




















































































































































































};

} // namespace evaluator
#endif
