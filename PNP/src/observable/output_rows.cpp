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

#include "observable/output_rows.hpp"
#include "observable/base_sink.hpp"
#include "observable/output_datum.hpp"

// manual includes
#include "core/strngs.hpp"
// -
// -
namespace observable {

// Simple dtor
output_rows::~output_rows()
{}

// Create file if it doesn't exist and write the header and body. If
// the file exists append new data.
void output_rows::write(base_sink & sink) const
{
  if( not this->rows_.empty() )
  {
    std::stringstream os;
    // Do header
    sink.header( os );
    if ( not this->title().empty() )
    {
      os << core::strngs::comment_begin() << " TITLE: \"" << this->title() << "\" " << core::strngs::comment_end() << "\n";
    }
    os << core::strngs::comment_begin() << " FIELDS: ";
    this->rows_.front().labels( os );
    os << core::strngs::comment_end() << "\n";
    os << core::strngs::comment_begin() << " UNITS: ";
    this->rows_.front().units( os );
    os << core::strngs::comment_end() << "\n";
  
    // Do body
    if( not this->is_serial_ )
    {
      this->rows_.front().row( os );
      os << "\n";
    }
    else
    {
      for( auto const& row : this->rows_ )
      {
        row.row( os );
        os << "\n";
      }
    }
    // Save to sink
    sink.write( this->label(), os.str() );
  }

}

// Attempt to accept a datum.
void output_rows::accept(std::unique_ptr< output_datum > datum)
{
  datum->visit( *this );
}

// Capture data.
//
// \pre arr /= nul
void output_rows::receive_data(std::size_t count, std::unique_ptr< output_row > arr)
{
  UTILITY_REQUIRE( arr, "Can not receive a nul pointer." );
  if( count < this->rows_.size() )
  {
    this->rows_[ count ].merge( std::move( arr ) );
  }
  else if( count == this->rows_.size() )
  {
    this->rows_.push_back( arr.release() );
  }
  else 
  {
    UTILITY_REQUIRE( count <= this->rows_.size(), "Can only append array, not resize." ); 
  }

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::output_rows );