//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or (at
//your option) any later version.
//
//This program is distributed in the hope that it will be useful, but
//WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------


#ifndef DEBUG
#define DEBUG 0
#endif

#include "observable/base_sink.hpp"
#include <uuid/uuid.h>

#include "observable/output_datum.hpp"

#include <core/strngs.hpp>
namespace observable {

//Generate a UUID string
std::string base_sink::make_uuid_string()
{
  // Generate the UUID string
  uuid_t val;
  uuid_generate(val);
  char p[3];
  std::string uuid_string(32, ' ');
  for (int i = 0; i != 16; ++i)
  {
    std::sprintf(&p[0], "%02X", val[i]);
    uuid_string[2*i] = p[0];
    uuid_string[2*i + 1] = p[1];
  }
  return uuid_string;

}

// Write UUID header line
void base_sink::header(std::ostream & os)
{
  os << core::strngs::comment_begin() << " UUID: " << this->uuid_ << " " << core::strngs::comment_end() << "\n";

}

// Write UUID to description stream
void base_sink::description(std::ostream & os)
{
  os << " run identification: " << this->uuid_ << "\n";

}

// Is this dataset defined
bool base_sink::has_dataset(std::string label) const
{
  for( auto const& dat : this->data_sets_ )
  {
    if( dat.label() == label )
    {
      return true;
    }
  }
  return false;

}

// Add a data set definition
//
// \pre not has_dataset( dataset.label )
// \post has_dataset( dataset.label )
void base_sink::add_dataset(std::unique_ptr< output_dataset > dataset)
{
  UTILITY_REQUIRE( not this->has_dataset( dataset->label() ), "Attempt to add two data sets with the same label" );
  this->data_sets_.push_back( dataset.release() );
}

// Move data into the set object.
//
// \pre has_dataset( label )
void base_sink::receive_data(std::string label, std::unique_ptr< output_datum > datum)
{
  for( auto & dat : this->data_sets_ )
  {
    if( dat.label() == label )
    {
      dat.accept( std::move( datum ) );
      return;
    }
  }
  UTILITY_REQUIRE( this->has_dataset( label ), "Attempt to add data to non-existent data set" ); 

}

// Write all of the data sets.
void base_sink::write_dataset()
{
  for( auto const& dat : this->data_sets_ )
  {
    dat.write( *this );
  }

}


} // namespace observable
