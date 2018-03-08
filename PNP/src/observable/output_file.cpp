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

#include "observable/output_file.hpp"
#include "utility/estimate_array.hpp"
#include "observable/base_sink.hpp"
#include "observable/output_datum.hpp"

// - manual includes
#include "utility/utility.hpp"
// -
#include <boost/filesystem.hpp>
#include <fstream>
// -
namespace observable {

// Simple default ctor
output_field::output_field()
: label_()
, unit_()
{}

// Main ctor
output_field::output_field(std::string name, std::string unit)
: label_( name )
, unit_( unit )
{}

output_field::~output_field()
{}

// Write out formatted field data to the stream. For data
// sets, this could be the field in the idxth row of the data.
// For data series the idx can be ignored.
void output_field::write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const
{
   UTILITY_REQUIRE( this->valid(), "Can not write incomplete field." );
   this->do_write( os, arr, idx, rank );
}

// Transfer arbitrary data as a character buffer. This version 
void output_dataset::receive_data(std::string buf)
{
  UTILITY_REQUIRE( false, "This output dataset can not accept a buffer as data." );
}


} // namespace observable
