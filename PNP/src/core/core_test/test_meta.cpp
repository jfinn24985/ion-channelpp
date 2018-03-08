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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "core/core_test/test_meta.hpp"
#include "core/input_base_reader.hpp"
#include "core/input_help.hpp"

test_meta::~test_meta() 
{}

//Read an entry in the input file. Return true if the entry was processed.
//
//throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool test_meta::do_read_entry(core::input_base_reader & reader)
{
   ++this->read_entry_count_;
   this->entries_.insert( std::make_pair( reader.name(), reader.value() ) );
   return true;
}

// Perform checks at the end of reading a section.
void test_meta::do_read_end(const core::input_base_reader & reader)
{
   ++this->read_end_count_;
}
// Perform checks at the end of reading a section.
void test_meta::do_reset()
{
   ++this->reset_count_;
}

