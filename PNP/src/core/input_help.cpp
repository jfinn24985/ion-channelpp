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

#include "core/input_help.hpp"

// manuals
#include "utility/utility.hpp"
//-
#include <boost/program_options/options_description.hpp>
//=
namespace core {

input_help & input_help::exemplar()
{
  static std::unique_ptr< input_help > ex_;
  if( NULL == ex_.get() ) ex_.reset( new input_help );
  return *ex_;

}

input_help::input_help()
: sections_()
{}

input_help::~input_help()
{}

// Add a section description.
//
// \pre not has_section( sect.title() )
// \post has_section( sect.title() )
void input_help::add_section(help_section sect)
{
UTILITY_REQUIRE( not this->has_section( sect.title() ), "Can not add two sections with the same name: " + sect.title() );
this->sections_.insert( std::make_pair( sect.title(), sect ) );
UTILITY_ENSURE( this->has_section( sect.title() ), "Failed to add section named " + sect.title() );

}

// Is there a section description set for this section label
bool input_help::has_section(std::string label) const
{
  return this->sections_.find( label ) != this->sections_.end();
}

help_section & input_help::get_section(std::string label)
{
  auto result = this->sections_.find( label );
  UTILITY_REQUIRE( result != this->sections_.end(), "Can not find section named " + label);
  return result->second;
}

help_section const& input_help::get_section(std::string label) const
{
  auto result = this->sections_.find( label );
  UTILITY_REQUIRE( result != this->sections_.end(), "Can not find section named " + label);
  return result->second;
}

// Create an error message and throw an exception.
void input_help::do_assert(char const* test, std::string mesg, std::string section, input_base_reader const* const reader, char const* func, char const* filename, int linenum, void* const* backtrace, int backsz) const
{
  std::string result;
  {
    std::stringstream errlog;
    errlog << "\nError in input file: \"";
    errlog << mesg << "\"";
    if( reader )
    {
      errlog << " Error in input file \"" << reader->current_filename()
             << "\", line " << reader->current_line_number() << ".";
    }
    errlog << "\n\nUSAGE:\n\n";
  
    // Print out any help text
    this->write( section, errlog );
  
    // --
    errlog << "\n================== DETAILS ==================\n"
           << "REASON: \"" << mesg << "\"\n"
           << "FAILED TEST: (" << test << ")\n"
           << "FILE: " << filename << ", LINE: " << linenum << ".\n";
  
    if( NULL != func )
    {
      errlog << "FUNCTION: " << func << ".\n";
    }
  
    if( NULL != backtrace )
    {
      errlog << "BACKTRACE: \n";
      boost::shared_ptr< char * > trace( core::backtrace_symbols( backtrace, backsz ), &std::free );
  
      for( int ll = 0; ll < backsz; ++ll )
      {
        errlog << * ( trace.get() + ll ) << "\n";
      }
    }
    errlog << "================== DETAILS ==================\n";
    result = errlog.str();
  }
  throw std::runtime_error( result );

}

// Write help information for the name section to errlog.
//
// \pre not section.empty
void input_help::write(std::string section, std::ostream & errlog) const
{
  UTILITY_REQUIRE( not section.empty(), "Can not use this 'write' method with an empty section name" );
  // Print out any help text
  {
    fixed_width_output_filter filt( 2, 1, 70 );
    if( this->has_section( section ) )
    {
      this->get_section( section ).write( filt, errlog );
    }
    else
    {
      errlog << "Input section '" << section << "' is undocumented.\n";
    }
    errlog << "\n";
  }

}

// Write the whole help information to errlog.
void input_help::write(std::ostream & errlog) const
{
  // Print out any help text
  {
    fixed_width_output_filter filt( 2, 1, 70 );
    for( auto const& sect : this->sections_ )
    {
      sect.second.write( filt, errlog );
    }
    errlog << "\n";
  }

}

// Write help information for the named parameter (optionally
// in the given section) to errlog.
//
// \pre not param.empty (but section.empty is okay.)

void input_help::write(std::string section, std::string param, std::ostream & errlog) const
{
  UTILITY_REQUIRE( not param.empty(), "Can not call this method with an empty parameter name." );
  // Print out any help text
  {
    fixed_width_output_filter filt( 2, 1, 70 );
    if( not section.empty() )
    {
      if( this->has_section( section ) )
      {
        if( this->get_section( section ).write( filt, param, errlog ) )
        {
           errlog << "\n";
           return;
        }
      }
      errlog << "Input parameter '" << param << "' in section '" << section << "' is undocumented.\n\n";
    }
    else
    {
      for( auto const& sect : this->sections_ )
      {
        if( sect.second.write( filt, param, errlog ) )
        {
          errlog << "\n";
          return;
        }
      }
      errlog << "Input parameter '" << param << "' is undocumented.\n\n";
    }
  }

}


} // namespace core
