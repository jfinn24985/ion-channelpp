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

#include "core/input_delegater.hpp"
#include "core/input_base_reader.hpp"
#include <boost/lexical_cast.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>


#include "core/strngs.hpp"
#include "core/input_base_meta.hpp"
#include "core/input_help.hpp"

// MANUAL INC
#include "core/input_error.hpp"
// - 
#include "utility/utility.hpp"
#include <fstream>
#include <iostream>
#include <set>
// -
namespace core {

input_delegater::input_delegater(std::size_t maxver)
: input_delegate_map_()
, section_list_()
, file_version_(0ul)
, max_version_(maxver)
{}

void input_delegater::add_input_delegate(boost::shared_ptr< input_base_meta > delegate) 
{
  UTILITY_REQUIRE(nullptr != delegate.get (), "Can not add null object.");
  // Get copy of label string as 'delegate' will become invalid at some point during argument passing
  std::string label (delegate->section_label ());
  UTILITY_REQUIRE(0 == this->input_delegate_map_.count (label), "Can not add two meta objects for the same input section.\""+label+"\"");
  this->input_delegate_map_.insert (std::make_pair (label, delegate));
}

void input_delegater::get_documentation(input_help & helper)
{
  for(auto const& meta : this->input_delegate_map_ ) meta.second->publish_help( helper );
}

void input_delegater::read_input(input_base_reader & reader)
{
  typedef std::map< std::string, boost::shared_ptr< input_base_meta > >::iterator iterator;
  UTILITY_REQUIRE( not this->input_delegate_map_.empty(), "Attempt to read input file before creating delegate map" );
  
  // Assume current is max
  this->file_version_ = max_version_;
  
  // The labels of included sections recorded as they appear
  // so we can check for missing sections at the end of reading
  // the input file.
  std::set< std::string > included_sections;
  
  // work through input document.
  try
  {
    while( reader.next() )
    {
      if (strngs::fsfver() == reader.name())
      {
        // Looking for top-level option names:
        //  'filever' input file version
        this->file_version_ = reader.get_ordinal( "Input file version", std::string{} );
        if( this->file_version_ > max_version_ )
        {
          reader.parameter_value_error( "Input file version", std::string{}, "File version is too recent for this program" );
        }
      }
      else
      {
        // Looking for section names:
        iterator known_section = this->input_delegate_map_.find( reader.name () );
        // Section label not known
        if( known_section == this->input_delegate_map_.end() )
        {
          std::stringstream kss;
          for( auto const& entry : this->input_delegate_map_ )
          {
            kss << entry.first << " ";
          }
          std::string keys( kss.str() );
          keys.erase( keys.size() - 1, 1 );
          throw input_error::invalid_section_error( reader.name(), &reader, keys );
        }
        // Only sections with multiple tag can appear twice.
        if( not known_section->second->multiple() and 0 != included_sections.count( reader.name () ) )
        {
          throw input_error::duplicate_section_error( reader.name(), &reader );
        }
        // Add to the section_list.
  
        included_sections.insert( reader.name () );
  
        // Get delegate to read its input section
        known_section->second->read_section( reader );
  
        // check reader is at 'end', the delegate should have thrown an error
        // otherwise.
        UTILITY_CHECK( reader.name() == strngs::fsend()
                       , "Reader delegate should have read to an 'end' label.");
      }
    }
  }
  catch( core::input_error const& err )
  {
    throw err;
  }
  catch (std::runtime_error const& err)
  {
    std::stringstream ss;
    ss << "Error in input file " << reader.current_filename()
       << " at line " << reader.current_line_number() << "\n" << err.what();
    throw std::runtime_error( ss.str() );
  }
  // Check for required input sections
  std::stringstream ss;
  for (auto & itr : input_delegate_map_)
  {
    if( itr.second->required() and 0 == included_sections.count( itr.first ) )
    {
      ss << itr.first << " ";
    }
  }
  std::string missing( ss.str() );
  if( not missing.empty() )
  {
    missing.erase( missing.size() - 1, 1 );
    throw input_error::missing_section_error( missing );
  }

}


} // namespace core
