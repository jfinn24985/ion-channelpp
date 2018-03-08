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

#include "observable/output_series.hpp"
#include "core/strngs.hpp"
#include "observable/base_sink.hpp"
#include "observable/output_datum.hpp"

namespace observable {

// Simple dtor
output_series::~output_series()
{}

// Write the data section of the output file
//
// \pre not empty
void output_series::do_array_body(std::ostream & os) const
{
  UTILITY_REQUIRE( not this->empty(), "Can not write output with no fields defined." );
  UTILITY_REQUIRE( this->data_.size() == 1, "Can only write array data with one data array defined." );
  // series is an array
  //
  // standard output format is:
  //
  // # COUNT sample_count
  // <key0> a0.mean a0.var
  // <key1> a1.mean a1.var
  // <key2> a1.mean a1.var
  auto const& arr = this->data_.front();
  os << core::strngs::comment_begin() << " SAMPLE COUNT: ";
  os << arr.count();
  os << core::strngs::comment_end() << "\n";
  for( std::size_t idx = 0; idx != arr.size(); ++idx )
  {
    for( auto const& field : *this )
    {
      field.write( os, arr, idx, 0 );
      os << " ";
    }
    os << "\n";
  }

}

// Write the data section of the output file
//
// \pre not empty
void output_series::do_series_body(std::ostream & os) const
{
  UTILITY_REQUIRE( not this->empty(), "Can not write output with no fields defined." );
  UTILITY_REQUIRE( not this->data_.empty(), "Can not write output with no data defined." );
  // Each element in "data_" is a data set in a series.
  //
  // standard output
  // index0 sample_count0 a0.mean a0.var a1.mean a1.var ...
  // index1 sample_count1 a0.mean a0.var a1.mean a1.var ...
  for( std::size_t rank = 0; rank != this->data_.size(); ++rank )
  {
    auto const& arr = this->data_[ rank ];
    for( std::size_t idx{ 0 }, fld{ 0 }; idx != arr.size();  )
    {
      this->entries_[ fld ].write( os, arr, idx, rank );
      if( this->entries_[ fld ].consume_value() )
      {
        ++idx;
      }
      else
      {
        UTILITY_REQUIRE( fld != this->entries_.size(), "Last field must consume values" );
      }
      if( fld != this->entries_.size() - 1 )
      {
        ++fld;
      }
      os << " ";
    }
    os << "\n";
  }

}

// Write the header of the data file
//
// \pre not empty
void output_series::do_header(std::ostream & os) const
{
  UTILITY_REQUIRE( not this->empty(), "Can not write output with no fields defined." );
  if ( not this->title().empty() )
  {
     os << core::strngs::comment_begin() << " TITLE: \"" << this->title() << "\" " << core::strngs::comment_end() << "\n";
  }
  os << core::strngs::comment_begin() << " FIELDS: ";
  for ( auto const& field : *this )
  {
     os << field.label() << " ";
  }
  os << core::strngs::comment_end() << "\n";
  os << core::strngs::comment_begin() << " UNITS: ";
  for ( auto const& field : *this )
  {
     os << field.unit() << " ";
  }
  os << core::strngs::comment_end() << "\n";

}

// Create file if it doesn't exist and write the header and body. If
// the file exists append new data.
//
// \pre not empty
void output_series::write(base_sink & sink) const
{
  UTILITY_REQUIRE( not this->empty(), "Can not write output with no fields defined." );
  std::stringstream os;
  sink.header( os );
  this->do_header( os );
  if( this->is_serial_ )
  {
    // If series.size > 1, then each element is a different measure
    //
    // standard output
    // <key0> sample_count0 a0.mean a0.var a1.mean a1.var ...
    // <key1> sample_count1 a0.mean a0.var a1.mean a1.var ...
    this->do_series_body( os );
  }
  else
  {
    // series is an array
    //
    // standard output
    // # COUNT sample_count
    // <key0> a0.mean a0.var
    // <key1> a1.mean a1.var
    // <key2> a1.mean a1.var
    this->do_array_body( os );
  }
  sink.write( this->label(), os.str() );

}

// Attempt to accept a datum.
void output_series::accept(std::unique_ptr< output_datum > datum)
{
  datum->visit( *this );
}

// Capture data.
//
// \pre arr /= nul
void output_series::receive_data(std::size_t count, std::unique_ptr< utility::estimate_array > arr)
{
  UTILITY_REQUIRE( arr, "Can not receive a nul pointer." );
  if( count < this->data_.size() )
  {
    auto & vec = this->data_[ count ];
    if( vec.size() == 0 )
    {
      this->data_.replace( count, arr.release() );
    }
    else
    {
      UTILITY_REQUIRE( vec.size() == arr->size(), "Attempt to merge different sized vectors" );
      vec.merge( *arr );
    }
  }
  else if( count == this->data_.size() )
  {
    this->data_.push_back( arr.release() );
  }
  else 
  {
    // assert count = this->data_.size() ??
    this->data_.resize( count );
    this->data_.replace( count, arr.release() );
  }

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::output_series );