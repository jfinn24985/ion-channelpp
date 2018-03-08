#ifndef IONCH__CORE_TEST_SUITE_HPP
#define IONCH__CORE_TEST_SUITE_HPP

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

namespace core { class constants; } 
namespace core { class fixed_width_output_filter; } 
namespace core { class help_entry; } 
namespace core { class help_subtype; } 
namespace core { class help_section; } 
namespace core { class input_definition; } 
namespace core { class input_delegater; } 
namespace core { class input_error; } 
namespace core { class input_help; } 
namespace core { class input_node; } 
namespace core { class input_parameter_memo; } 
namespace core { class input_parameter_reference; } 
namespace core { class input_preprocess; } 
namespace core { class input_reader; } 
namespace core { class input_section; } 
namespace core { class strngs; } 
class test_meta;

class core_test_suite
 {
   public:
    //Test common functionality of input_base_reader derived classes
    template< class Reader > static void test_read_input_buffer();

    //  Test  'input_base_meta' derived class base class functionality
    //  - check no default ctor
    //  - check no copy ability
    //  - check virtual dtor
    //  - check three argument ctor : label, is_multiple, is_required
    template< class Meta > static void test_input_base_meta();
























































































};
#endif
