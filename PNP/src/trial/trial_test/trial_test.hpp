#ifndef IONCH__TRIAL_TEST_HPP
#define IONCH__TRIAL_TEST_HPP

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

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <string>
#include <cstddef>

namespace particle { class change_atom; } 
namespace particle { class change_set; } 
namespace particle { class change_hash; } 
namespace trial { class choice_manager; } 
namespace trial { class choice_meta; } 
namespace trial { template<class Choice> class chooser; } 
namespace trial { class move_choice; } 
class test_choice;
namespace particle { class particle_manager; } 
namespace geometry { class geometry_manager; } 
namespace core { class input_parameter_memo; } 
namespace trial { class base_choice; } 

class trial_test
 {
   public:
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

    static std::vector< core::input_parameter_memo > mockup_params();

    // Tests key, probabilty (get/set) and serialization. 
    //
    // * Does not test trial generation.
    // 
    static void test_base_choice_methods(boost::shared_ptr< trial::base_choice > choice);

    // Run choice.generate on a mockup ensemble. Uses
    // Choice::permitted( spc ) and Choice::make_choice( idx, *params* ) 
    // to create choice object. Params arg to make_choice is the
    // params arg passed to this function.
    //
    //  ens.size = 6
    //  specie_count = 5
    //  solute = "Na" (x2 particles) and "Cl" (x1 particle)
    //  mobile = "CA" (x1 particle)
    //  flexible = "CO" (x1 particle)
    //  channel only = "OX" (x1 particle)
    //  system region = "cell" (PBC cube)
    //  subregion = "bulk" (open cube)
    //  all particles inside "bulk" (and "cell")
    //  
    template< class Choice > static void test_choice_generate(const std::vector< core::input_parameter_memo > & params);

    // Run "Chooser" on a mockup ensemble. Uses
    // base_chooser.generate_choices( *params*, *type*, *specielist*, *rate* ) 
    // to create choice object. "*args*" to are directly the args passed to 
    // this function.
    //
    // \param count : expected number of choices
    // \param exp_rate : expected rate of each choice
    //
    //  ens.size = 6
    //  specie_count = 5
    //  solute = "Na" (x2 particles) and "Cl" (x1 particle)
    //  mobile = "CA" (x1 particle)
    //  flexible = "CO" (x1 particle)
    //  channel only = "OX" (x1 particle)
    //  system region = "cell" (PBC cube)
    //  subregion = "bulk" (open cube)
    //  all particles inside "bulk" (and "cell")
    //  
    template< class Chooser > static void test_make_chooser_methods(const std::vector< core::input_parameter_memo > & params, std::string type, const core::input_parameter_memo & specielist, double rate, std::size_t count, double exp_rate);


};
#endif
