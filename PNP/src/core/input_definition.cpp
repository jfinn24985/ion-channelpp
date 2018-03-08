

#ifndef DEBUG
#define DEBUG 0
#endif

#include "core/input_definition.hpp"
#include "core/input_help.hpp"

// manual includes
#include "utility/utility.hpp"
//-
namespace core {

input_definition::~input_definition()
{}

// Add a help entry for this subtype
//
// \pre not had_definition( entry.title )
void input_definition::add_definition(help_entry entry)
{
  UTILITY_REQUIRE( not this->has_definition( entry.title() ), "Can not overwrite existing definition." );
  this->definitions_[ entry.title() ] = entry;
}

bool input_definition::has_definition(std::string name) const
{
  return this->definitions_.count( name ) == 1;
}

// Put help information from definition into helper
// object.
void input_definition::publish_help(input_help & helper, std::string seclabel) const
{
// Subtype parameter documentation.
core::help_subtype subtyp( this->label_, this->description_ );
for( auto const& jter : this->definitions_ )
{
  subtyp.add_entry( jter.second );
}
if( not helper.has_section( seclabel ) )
{
  helper.add_section( { seclabel, "" } );
}
helper.get_section( seclabel ).add_subtype( subtyp );

}


} // namespace core
