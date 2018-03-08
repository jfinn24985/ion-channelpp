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

#include "observable/output_text.hpp"
#include "observable/output_datum.hpp"
#include "observable/base_sink.hpp"

// manual includes
#include "core/strngs.hpp"
// -
// -
namespace observable {

// Simple dtor
output_text::~output_text()
{}

// Attempt to accept a datum.
void output_text::accept(std::unique_ptr< output_datum > datum)
{
  datum->visit( *this );
}

// Write the header of the data file
//
// \pre not empty
void output_text::do_header(std::ostream & os) const
{
  if ( not this->title().empty() )
  {
    os << core::strngs::comment_begin() << " TITLE: \"" << this->title() << "\" " << core::strngs::comment_end() << "\n";
  }
  if( not this->field_labels_.empty() )
  {
    os << core::strngs::comment_begin() << " FIELDS: " << this->field_labels_ << " " << core::strngs::comment_end() << "\n";
  }
  if( not this->units_.empty() )
  {
    os << core::strngs::comment_begin() << " UNITS: " << this->units_ << " " << core::strngs::comment_end() << "\n";
  }

}

// Capture data.
//
// \pre not buf.empty
void output_text::receive_data(std::string buf)
{
  UTILITY_REQUIRE( not buf.empty(), "Can not receive a nul buffer." );
  if( this->is_serial_ )
  {
    this->buffer_.append( buf );
  }
  else
  {
    std::swap( this->buffer_, buf );
  }

}

// Create file if it doesn't exist and write the header and body. If
// the file exists append new data.
//
// \pre not empty
void output_text::write(base_sink & sink) const
{
  if( not this->buffer_.empty() )
  {
    if( this->is_serial_ and sink.exists( this->label() ) )
    {
      sink.append( this->label(), this->buffer_ );
    }
    else
    {
      std::stringstream os;
      sink.header( os );
      this->do_header( os );
      os << this->buffer_;
      sink.write( this->label(), os.str() );
    }
    this->buffer_.clear();
  }

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::output_text );