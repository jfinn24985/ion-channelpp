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

#include "core/help_section.hpp"
#include "core/fixed_width_output_filter.hpp"

// manual includes
#include "utility/utility.hpp"
// --
#include <boost/iostreams/filtering_stream.hpp>
// --
namespace core {

// Write help description to destination stream using the given filter.
// Include descriptions for child entries.
void help_section::write(fixed_width_output_filter & filt, std::ostream & dest) const
{
  this->help_subtype::write( filt, dest );
  {
    namespace io = boost::iostreams;
    core::indent_guard grd( filt );
    {
      io::filtering_ostream out;
      out.push( filt );
      out.push( dest );
      if( not this->subtypes_.empty() )
      {
        out << "type\n";
      }
    }
    if( not this->subtypes_.empty() )
    {
      core::indent_guard grd1( filt );
      {
        io::filtering_ostream out1;
        out1.push( filt );
        out1.push( dest );
        out1 << "[one of: ";
        bool first = true;
        for( auto const& entry : this->subtypes_ )
        {
          if( not first )  out1 << ",";
          else             first = false;
          out1 << entry.first;
        }
        out1 << "]\n";
        out1 << "This section represents a class of types in the ";
        out1 << "the simulation. Each type can have ";
        out1 << "individual options. These options may only ";
        out1 << "be valid in combination with the specific type.\n\n";
        out1 << "Description of available types:\n";
      }
      for( auto const& entry : this->subtypes_ )
      {
        entry.second.write( filt, dest );
      }
    }
  }

}

// Write help description for the named parameter to destination stream 
// using the given filter. If not an immediate entry, try subtype entries.
bool help_section::write(fixed_width_output_filter & filt, std::string param, std::ostream & dest) const
{
  UTILITY_REQUIRE( not param.empty(), "Should not be using this 'write' with empty parameter name." );
  if( this->help_subtype::write( filt, param, dest ) )
  {
    // param was immediate child
    return true;
  }
  else
  {
    // search in subtypes
    for( auto const& entry : this->subtypes_ )
    {
      if( entry.second.has_entry( param ) )
      {
        // found subtype
        namespace io = boost::iostreams;
        {
          io::filtering_ostream os;
          os.push( filt );
          os.push( dest );
          os << this->title() << "\n";
        }
        {
          core::indent_guard grd( filt );
          {
            io::filtering_ostream out;
            out.push( filt );
            out.push( dest );
            if( not this->description().empty() )
            {
              out << this->description() << "\n";
            }
            out << "type:\n";
          }
          {
            entry.second.write( filt, param, dest );
          }
        }
        return true;
      }
    }
  }
  return false;

}

// Add a new help entry to this master entry
//
// \pre not has_subtype( entry.title )
// \post has_subtype( entry.title )
void help_section::add_subtype(help_subtype entry)
{
  UTILITY_REQUIRE( not this->has_subtype( entry.title() ), "Can not add two subtypes with the same title." );
  this->subtypes_.insert( std::make_pair( entry.title(), entry ) );
  UTILITY_ENSURE( this->has_subtype( entry.title() ), "Failed to add subtype with title ["+entry.title()+"]." );
}

// Get subtype entry with the given title.
//
// \pre has_subtype( title )
help_subtype& help_section::get_subtype(std::string title)
{
  auto result = this->subtypes_.find( title );
  UTILITY_REQUIRE( result != this->subtypes_.end(), "No subtype with given title found." );
  return result->second;
}

// Get subtype entry with the given title.
//
// \pre has_subtype( title )
const help_subtype& help_section::get_subtype(std::string title) const
{
  auto result = this->subtypes_.find( title );
  UTILITY_REQUIRE( result != this->subtypes_.end(), "No subtype with given title found." );
  return result->second;
}


} // namespace core
