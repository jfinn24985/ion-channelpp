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

#include "core/help_subtype.hpp"
#include "core/fixed_width_output_filter.hpp"

// manual includes
#include "utility/utility.hpp"
// --
#include <boost/iostreams/filtering_stream.hpp>
// --
namespace core {

// Write help description to destination stream using the given filter.
// Include descriptions for child entries.
void help_subtype::write(fixed_width_output_filter & filt, std::ostream & dest) const
{
  namespace io = boost::iostreams;
  {
    io::filtering_ostream os;
    os.push( filt );
    os.push( dest );
    os << this->title_ << "\n";
  }
  {
    core::indent_guard grd( filt );
    {
      io::filtering_ostream out;
      out.push( filt );
      out.push( dest );
      if( not this->description_.empty() )
      {
        out << this->description_ << "\n";
      }
      if( not this->children_.empty() )
      {
        out << "Options:\n";
      }
    }
    if( not this->children_.empty() )
    {
      for( auto const& entry : this->children_ )
      {
        entry.second.write( filt, dest );
      }
    }
  }
  

}

// Write help description for a specific parameter to destination stream using the 
// given filter, if the parameter exists. Otherwise do nothing.
//
// \return true if param was entry in this subtype (== has_entry)
//
// \pre not param.empty
bool help_subtype::write(fixed_width_output_filter & filt, std::string param, std::ostream & dest) const
{
  UTILITY_REQUIRE( not param.empty(), "Input parameter name should not be empty when using this 'write' method." );
  if( this->children_.count( param ) != 0 )
  {
    namespace io = boost::iostreams;
    {
      io::filtering_ostream os;
      os.push( filt );
      os.push( dest );
      os << this->title_ << "\n";
    }
    {
      core::indent_guard grd( filt );
      {
        io::filtering_ostream out;
        out.push( filt );
        out.push( dest );
        if( not this->description_.empty() )
        {
          out << this->description_ << "\n";
        }
        out << "Option:\n";
      }
      this->children_.at( param ).write( filt, dest );
    }
    return true;
  }
  return false;

}

// Add a new help entry to this master entry
//
// \pre not has_entry( entry.title )
// \post has_entry( entry.title )
void help_subtype::add_entry(help_entry entry)
{
  UTILITY_REQUIRE( not this->has_entry( entry.title() ), "Can not add two entries with the same title." );
  this->children_.insert( std::make_pair( entry.title(), entry ) );
  UTILITY_ENSURE( this->has_entry( entry.title() ), "Failed to add entry with title ["+entry.title()+"]." );
}

// Get entry with the given title.
//
// \pre has_entry( title )
help_entry& help_subtype::get_entry(std::string title)
{
  auto result = this->children_.find( title );
  UTILITY_REQUIRE( result != this->children_.end(), "No entry with given title found." );
  return result->second;
}

// Get entry with the given title.
//
// \pre has_entry( title )
const help_entry& help_subtype::get_entry(std::string title) const
{
  auto result = this->children_.find( title );
  UTILITY_REQUIRE( result != this->children_.end(), "No entry with given title found." );
  return result->second;
}


} // namespace core
