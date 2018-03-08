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

#include "trial/base_chooser.hpp"
#include "utility/archive.hpp"

#include "geometry/geometry_manager.hpp"
#include "trial/choice.hpp"
#include "particle/specie.hpp"
#include "core/input_document.hpp"

// Manual include
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "utility/fuzzy_equals.hpp"
//-
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
// -
// -
namespace trial {

// Provide a description of the chooser state.
void base_chooser::description(std::ostream & os) const
{
  static const boost::format header(" %6s %7.2f");
  os << boost::format(header) % this->type_ % (100.0*this->rate_) << "\n";

}

// Generate and add choices to choice list.
//
// If choices already exist they are removed and new choices
// generated.
void base_chooser::prepare_choices(const std::vector< particle::specie >& species, const geometry::geometry_manager & gman, boost::ptr_vector< base_choice >& choices) const
{
  std::set< std::string > include, exclude;
  if( this->has_specie_list() )
  {
    std::string bad_labels;
    // Have inclusion/exclusion list
    boost::char_separator< char > sep( " " );
    std::string value( this->specie_list_.value() );
    boost::tokenizer< boost::char_separator< char > > tok( value, sep );
    for( auto const& entry : tok )
    {
      bool is_include{ true };
      std::string label{};
      if( entry[0] == '+' )
      {
        label = entry.substr( 1, 2 );
        include.insert( label );
      }
      else if( entry[0] == '-' )
      {
        is_include = false;
        label = entry.substr( 1, 2 );
      }
      else
      {
        label = entry.substr( 0, 2 );
      }
      bool label_found{ false };
      for( auto const& spec : species )
      {
        if( spec.label() == label )
        {
          label_found = true;
          break;
        }
      }
      if( not label_found )
      {
        std::stringstream sps;
        sps << "(";
        for( auto const& spec : species )
        {
          sps << spec.label() << ",";
        }
        std::string list( sps.str() );
        list.back() = ')';
        core::input_parameter_memo tmp( this->specie_list_ );
        tmp.set_value( label );
        throw core::input_error::parameter_value_error( "Include/exclude specie list", core::strngs::fstry(), tmp, ( core::input_error::bad_key_message_format() % list ).str() );
      }
      if( is_include )
      {
        exclude.insert( entry.substr( 1, 2 ) );
      }
      else
      {
        include.insert( entry.substr( 0, 2 ) );
      }
    }
  }
  // Require that species are not in both include and exclude lists
  if( not include.empty() and not exclude.empty() )
  {
    for( auto const& entry : include )
    {
      if( 0 != exclude.count( entry ) )
      {
        throw core::input_error::parameter_value_error( "Specie label list", core::strngs::fstry(), this->specie_list_, "Specie label "+entry+" set as both included and excluded" );
      }
    }
  }
  boost::ptr_vector< base_choice > new_choices;
  double rate_sum = 0.0;
  for( std::size_t ispec = 0; ispec != species.size(); ++ispec )
  {
    auto const& spec = species.at( ispec );
    // Only species in include OR not in exclude list.
    if( ( include.empty() or 0 < include.count( spec.label() ) )
        and ( exclude.empty() or 0 == exclude.count( spec.label() ) ) )
    {
      // Only species with non-zero specie rate can have trials
      if( not utility::feq( spec.rate(), 0.0 ) )
      {
        if( this->is_permitted( spec ) )
        {
          // chooser may add more than one choice
          std::size_t next = new_choices.size();
          this->add_choice( ispec, gman, this->parameter_set_, new_choices );
          // Update all added
          for( std::size_t idx = next; idx != new_choices.size(); ++idx )
          {
            rate_sum += spec.rate();
            new_choices[ idx ].set_probability( spec.rate() );
          }
        }
      }
    }
  }
  // new_choices may be empty.
  if( not new_choices.empty() )
  {
    const double rate_scale = this->rate_ / rate_sum;
    for( std::size_t ii = 0; ii != new_choices.size(); ++ii )
    {
      new_choices[ii].set_probability( rate_scale * new_choices[ii].probability() );
    }
    // Move choices to result list.
    choices.transfer( choices.end(), new_choices );
  }

}

// Check is specie list has been defined.
bool base_chooser::has_specie_list() const
{
  return not this->specie_list_.name().empty();
}

// Add an input file section.
//
// only throw possible should be from os.write() operation
//
// The output of this factory method is made up like
//
// chooser
// <call do_write_document>
// end

void base_chooser::write_document(core::input_document & wr) const 
{
std::size_t ix = wr.add_section( core::strngs::fstry() );
wr[ ix ].add_entry( core::strngs::fstype(), this->type_ );
if( not this->specie_list_.value().empty() )
{
  wr[ ix ].add_entry( this->specie_list_.name(), this->specie_list_.value() );
}
wr[ ix ].add_entry( core::strngs::rate_label(), this->rate_ );
for( auto const& param : this->parameter_set_ )
{
  if( param.name() != core::strngs::fsend() )
  {
    wr[ ix ].add_entry( param.name(), param.value() );
  }
}

}


} // namespace trial

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(trial::base_chooser, "trial::base_chooser");