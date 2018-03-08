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

#include "trial/choice_manager.hpp"
#include "utility/archive.hpp"

#include "trial/base_chooser.hpp"
#include "trial/choice.hpp"
#include "particle/change_set.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "utility/random.hpp"
#include "particle/specie.hpp"
#include "core/input_document.hpp"
#include "core/input_delegater.hpp"

// Manuals
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "trial/choice_meta.hpp"
#include "trial/std_choices.hpp"
#include "utility/fuzzy_equals.hpp"

#include <boost/format.hpp>
// -
namespace trial {

// Transfer ownership of chooser 'choice' into our chooser list.
//

void choice_manager::add_chooser(std::unique_ptr< base_chooser > choice) 
{
   this->choosers_.push_back( choice.get() );
   choice.release();
}

// Details about the current choosers to be written to the
// log at the start of the simulation.
void choice_manager::description(std::ostream & os) const 
{
  if ( not this->choosers_.empty() )
  {
    os << core::strngs::horizontal_bar () << "\n";
    os << " Trial types and rates\n";
    os << "----------------------\n";
    static const boost::format choice_header(" %6s %7s");
    os << boost::format(choice_header) % "type" % "rate(%)" << "\n";
    for (auto const& chsr : this->choosers_ )
    {
       chsr.description( os );
    }
    if ( not this->choices_.empty() )
    {
       static const boost::format choice_header(" %6s %4s %7s");
       os << boost::format(choice_header) % "type" % "spc." % "rate(%)" << "\n";
       static const boost::format choice_row(" %6d %4d %7.2f");
       for (auto const& choice : this->choices_)
       {
          os << boost::format(choice_row) % choice.key().subtype() % choice.key().key() % (choice.probability()*100.0) << "\n";
       }
    }
  }

}

// Select a choice based on 'selector' and generate a new change set.

std::unique_ptr< particle::change_set > choice_manager::generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr, double selector)
{
  // select a move
  std::unique_ptr< particle::change_set > trial;
  for( auto & choice: this->choices_ )
  {
    selector -= choice.probability();
    if (selector <= 0.0)
    {
      trial = choice.generate( particles, regions, rgnr );
      break;
    }
  }
  return trial;
  

}

// Does this manager have a chooser with the given name?
bool choice_manager::has_chooser(std::string label) const
{
  for( std::size_t idx = 0; idx != this->choosers_.size(); ++idx )
  {
    if( this->choosers_[ idx ].type() == label )
    {
      return true;
    }
  }
  return false;

}

// Are any of the choices grand canonical moves?
bool choice_manager::is_grand_canonical() const
{
  const std::string gc_type0{ "individual" };
  for( std::size_t idx = 0; idx != this->choosers_.size(); ++idx )
  {
    if( this->choosers_[ idx ].type() == gc_type0 ) return true;
  }
  return false;

}

// Create choice list from choosers.  Resets the choice list before
// generating a new list.
//
// Will throw input error if no choices where created.
// 
// \pre not empty_chooser
// \post size /= 0
void choice_manager::prepare(const std::vector< particle::specie >& species, const geometry::geometry_manager & gman, utility::random_distribution & rgnr)
{
  UTILITY_REQUIRE( not this->choosers_.empty (), "Can not run simulation with no possible trials" );
  // Build choices
  this->choices_.clear();
  for (auto &chsr : this->choosers_)
  {
    chsr.prepare_choices( species, gman, this->choices_ );
  }
  if( this->choices_.empty() )
  {
    throw core::input_error::input_logic_error( "Trial definition", core::strngs::fstry(), "No actual trial generators could be created from the trials defined in the input." );
  }
  // Ensure sum of probabilities is 1.0
  double sum_choice_rates { 0.0 };
  // reset choice objects
  for( auto &choice : this->choices_)
  {
    UTILITY_CHECK( choice.probability() > 0.0, "No choice should have zero or negative probability." );
    UTILITY_CHECK( not utility::feq(  choice.probability(), 0.0 ), "No choice should have zero probability." );
    sum_choice_rates += choice.probability();
  }
  UTILITY_CHECK( sum_choice_rates > 0.0, "No set of choices should have zero or negative probability." );
  UTILITY_CHECK( not utility::feq( sum_choice_rates, 0.0 ), "No set of choices should have zero probability." );
  if( not utility::feq( sum_choice_rates, 1.0 ))
  {
    for (auto &choice : this->choices_)
    {
      choice.set_probability( choice.probability() / sum_choice_rates );
    }
  }
  // Randomize choice order
  rgnr.shuffle( this->choices_ );
  

}

// Write choosers to an input document
void choice_manager::write_document(core::input_document & wr) 
{
  // Write choosers
  for (auto const& choice : this->choosers_)
  {
     choice.write_document( wr );
  }

}

// Adds factory methods and input parsers for all the types that can be instantiated from 
// the input file.
void choice_manager::build_input_delegater(boost::shared_ptr< choice_manager > cman, core::input_delegater & delegate)
{
  ///////////////
  // Choice types
  boost::shared_ptr< trial::choice_meta > cmeta { new trial::choice_meta( cman ) };
  
  // Small displacement in a sphere
  trial::move_choice::add_definition( *cmeta );
  
  // Jump anywhere in simulation
  trial::jump_choice::add_definition( *cmeta );
  
  // Jump into a region
  trial::jump_in::add_definition( *cmeta );
  
  // Jump out of a region
  trial::jump_out::add_definition( *cmeta );
  
  // Jump anywhere in a region
  trial::jump_around::add_definition( *cmeta );
  
  // Individual grand canonical
  trial::add_specie::add_definition( *cmeta );
  
  delegate.add_input_delegate( cmeta );

}


} // namespace trial
