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

#include "trial/std_choices.hpp"
#include <boost/bind.hpp>

#include "utility/archive.hpp"

#include "particle/change_set.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "utility/random.hpp"
#include "trial/choice_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "particle/specie.hpp"

// Manual includes
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "particle/ensemble.hpp"
#include "trial/chooser.hpp"
#include "trial/chooser_pair.hpp"
// -
#include <boost/algorithm/string.hpp>
// -
namespace trial {

//  Default delta value if none is given in the input.
const double move_choice::default_delta = 0.1;

move_choice::move_choice(std::size_t ispec)
: base_choice( particle::change_hash( ispec, 1, 1, 0 ) )
, delta_( default_delta )
{}

move_choice::~move_choice() = default;

std::unique_ptr< particle::change_set > move_choice::generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr)
{
  std::unique_ptr< particle::change_set > result( new particle::change_set( this->key() ) );
  auto const& ens = particles.get_ensemble();
  // Select a particle at random
  particle::change_atom atom;
  atom.key = this->key().key();
  const std::size_t spc_count = ens.specie_count( this->key().key() );
  switch( spc_count )
  {
  case 0:
  {
    result->set_fail(); // No particles of this specie
    result->add_atom( atom );
    return result;
  }
  break;
  case 1:
  {
    atom.index = ens.nth_specie_index( atom.key, 0 );
  }
  break;
  default:
  {
    atom.index = ens.nth_specie_index( atom.key, rgnr.randint( 0, spc_count - 1 ) );
  }
  break;
  }
  // Get old position
  atom.old_position = ens.position( atom.index );
  
  // give particle a random displacement from old position
  atom.new_position = atom.old_position;
  const bool isok = regions.system_region().new_position_offset( rgnr, atom.new_position, this->delta_, particles.get_specie( atom.key ).radius() );
  
  // new position not inside system region.
  if( not isok )
  {
    result->set_fail();
  }
  result->add_atom( atom );
  return result;
  

}

// Add a definition object for this choice class to the
// meta object.
void move_choice::add_definition(choice_meta & meta)
{
  std::string desc( "MC move that involves a displacement within a small sphere. The displacement is defined by a displacement up to \"max_displacement\" from the original position. The move does not have equivolume probability because it uses a uniform linear displacement. Move is applied to all non-fixed species." );
  std::unique_ptr< trial::choice_definition > defn( new trial::choice_definition( "move", desc, &trial::chooser< move_choice >::make_chooser ) );
  {
    std::string help = "The maximum magnitude of the displacement from the original position made by this trial.";
    defn->add_definition( { "delta", "distance in Angstrom", ">0", std::to_string( default_displacement() ), help } );
  }
  UTILITY_CHECK( not meta.has_trial_type( defn->label() ), "This type has already been added to this meta object." );
  meta.add_trial_type( defn );

}

// Get delta from params and then create a new object.
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature:
//   static unique_ptr< base_choice > make_choice( size_t, geometry_manager const&, map< string, string > const& )
// 

std::unique_ptr< move_choice > move_choice::make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params)
{
  // needed information
  // ispec : from ispec arg
  // delta : from param arg
  double delta {};
  const std::string delta_label { "delta" }; 
  bool have_delta { false };
  // check for required parameters.
  for( auto & param : params )
  {
    if( param.name() == delta_label )
    {
      // will use non-default delta. Get and validate value.
      delta = param.get_float( "Move trial", core::strngs::fstry(), true, false );
      have_delta = true;
    }
  }
  // build region object.
  std::unique_ptr< move_choice > current;
  current.reset( new move_choice( ispec ) );
  if( have_delta )
  {
    current->set_max_displacement( delta );
  }
  return current;

}

jump_choice::~jump_choice() 
{}

std::unique_ptr< particle::change_set > jump_choice::generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr)
{
  std::unique_ptr< particle::change_set > resultset( new particle::change_set( this->key() ) );
  auto const& ens = particles.get_ensemble();
  particle::change_atom atom;
  atom.key = this->key().key();
  const std::size_t spc_count( ens.specie_count( atom.key ) );
  // Select a particle at random
  switch (spc_count)
  {
  case 0:
  {
    resultset->set_fail(); // No particles of this specie
    resultset->add_atom (atom);
    return resultset;
  }
  break;
  case 1:
  {
    atom.index = ens.nth_specie_index( atom.key, 0 );
  }
  break;
  default:
  {
    atom.index = ens.nth_specie_index( atom.key, rgnr.randint( 0, spc_count - 1) );
  }
  break;
  }
  // Get old position
  atom.old_position = ens.position(atom.index);
  // New random position within cell
  atom.new_position = regions.system_region().new_position( rgnr, particles.get_specie( atom.key ).radius() );
  // No need to check for valid position because new
  // position should always give a valid position.
  resultset->add_atom (atom);
  return resultset;

}

// Add a definition object for this choice class to the
// meta object.
void jump_choice::add_definition(choice_meta & meta)
{
  std::string desc( "MC move that involves a jump from anywhere to anywhere within the simulation. Move has equivolume probability. Move is applied to all solute species." );
  std::unique_ptr< trial::choice_definition > defn( new trial::choice_definition( "jump", desc, &trial::chooser< jump_choice >::make_chooser ) );
  
  // NO extra parameters
  
  UTILITY_CHECK( not meta.has_trial_type( defn->label() ), "This type has already been added to this meta object." );
  meta.add_trial_type( defn );

}

// Get delta from params and then create a new object.
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature:
//   static unique_ptr< base_choice > make_choice( size_t, geometry_manager, map< string, string > const& )
// 

std::unique_ptr< jump_choice > jump_choice::make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params)
{
  UTILITY_CHECK( params.size() == 1, "Jump trial requires no extra parameters" );
  // needed information
  // ispec : from ispec arg
  std::unique_ptr< jump_choice > current;
  current.reset( new jump_choice( ispec ) );
  return current;

}

// Only solute species can have this trial type?
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature
//   static bool make_choice( specie const& )
bool jump_choice::permitted(const particle::specie & spec)
{
  return spec.is_solute();
}

jump_in::~jump_in() 
{}

std::unique_ptr< particle::change_set > jump_in::generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr)
{
  std::unique_ptr< particle::change_set > resultset( new particle::change_set( this->key() ) );
  auto const& ens = particles.get_ensemble();
  particle::change_atom atom;
  atom.key = this->key().key();
  const std::size_t spc_count( ens.specie_count( atom.key ) );
  // Select a particle at random: We can do this because we are selecting from
  // anywhere.
  switch( spc_count )
  {
  case 0:
  {
    resultset->set_fail(); // No particles of this specie
    resultset->add_atom( atom );
    return resultset;
  }
  break;
  case 1:
  {
    atom.index = ens.nth_specie_index( atom.key, 0 );
  }
  break;
  default:
  {
    atom.index = ens.nth_specie_index( atom.key, rgnr.randint( 0, spc_count - 1 ) );
  }
  break;
  }
  // Get old position
  atom.old_position = ens.position( atom.index );
  // New random position within specific region
  atom.new_position = regions.get_region( this->region_index_ ).new_position( rgnr, particles.get_specie( atom.key ).radius() );
  // No need to check for valid position because new
  // position should always give a valid position.
  resultset->add_atom( atom );
  return resultset;
  

}

// Add a definition object for this choice class to the
// meta object.
void jump_in::add_definition(choice_meta & meta)
{
  std::string desc( "MC move that involves a jump from anywhere to a specific region within the simulation. Move has equivolume probability within the target region. Move is applied to all solute species." );
  std::unique_ptr< trial::choice_definition > defn( new trial::choice_definition( "jump-in", desc, &trial::chooser< jump_in >::make_chooser ) );
  
  {
    std::string help = "The name of one of the sub-regions defined in the simulation as target of move. It is an error to use the name of the whole simulation region.";
    defn->add_definition( { core::strngs::fsregn(), "label", "region label", "required", help } );
  }
  UTILITY_CHECK( not meta.has_trial_type( defn->label() ), "This type has already been added to this meta object." );
  meta.add_trial_type( defn );

}

// Get delta from params and then create a new object.
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature:
//   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
// 

std::unique_ptr< jump_in > jump_in::make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params)
{
  // needed information
  // ispec : from ispec arg
  // region_name : from param arg
  std::size_t region_index {0ul};
  const std::string region_name_label { core::strngs::fsregn() };
  bool have_region{ false };
  // --------------------
  // region label (required)
  // check for required parameters.
  for( auto & param : params )
  {
    if( param.name() == region_name_label )
    {
      std::string region_name = param.get_text( "Jump-in target region", core::strngs::fstry() );
      have_region = true;
      boost::algorithm::trim( region_name );
      const std::size_t region_index = gman.region_key( region_name );
      if( region_index == gman.region_count() )
      {
        // make list of region labels.
        std::stringstream rss{};
        rss << "(";
        for( std::size_t ireg = 0; ireg != gman.region_count(); ++ireg )
        {
          rss << gman.get_region( ireg ).label() << ",";
        }
        std::string list( rss.str() );
        list.back() = ')';
        throw core::input_error::parameter_value_error( "Jump-in target region", core::strngs::fstry(), param, ( core::input_error::bad_key_message_format() % list ).str() );
      }
      if( region_index == 0 )
      {
        throw core::input_error::parameter_value_error( "Jump-in target region", core::strngs::fstry(), param, "Jump-in target region cannot be the system region" );
      }
    }
  }
  if( not have_region )
  {
    throw core::input_error::missing_parameters_error( "Jump-in subtype trial", core::strngs::fstry(), region_name_label, params.back().filename(), params.back().line_number() );
  }
  
  // build choice object.
  std::unique_ptr< jump_in > current( new jump_in( ispec, region_index ) );
  return current;
  
  
  
  

}

// Only solute species can have this trial type?
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature
//   static bool make_choice( specie const& )
bool jump_in::permitted(const particle::specie & spec)
{
  return spec.is_solute();
}

jump_out::~jump_out() 
{}

std::unique_ptr< particle::change_set > jump_out::generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr)
{
  std::unique_ptr< particle::change_set > resultset( new particle::change_set( this->key() ) );
  auto const& ens = particles.get_ensemble();
  particle::change_atom atom;
  atom.key = this->key().key();
  const std::size_t spc_count( ens.specie_count( atom.key ) );
  auto const& oldregion = regions.get_region( this->region_index_ );
  // Select a particle at random: This is time-consuming as we don't have a list
  // of particles in the target region.
  std::vector< std::size_t > inregion_index;
  inregion_index.reserve( spc_count );
  const double spc_radius = particles.get_specie( atom.key ).radius();
  for (std::size_t gidx = 0; gidx != ens.size(); ++gidx)
  {
    if (ens.key( gidx ) == atom.key)
    {
      if (oldregion.is_inside( ens.position( gidx ), spc_radius ) )
      {
        inregion_index.push_back( gidx );
      }
    }
  }
  switch( inregion_index.size() )
  {
  case 0:
  {
    resultset->set_fail(); // No particles of this specie
    resultset->add_atom( atom );
    return resultset;
  }
  break;
  case 1:
  {
    atom.index = inregion_index[ 0 ];
  }
  break;
  default:
  {
    atom.index = inregion_index[ rgnr.randint( 0, inregion_index.size() - 1 ) ];
  }
  break;
  }
  // Get old position
  atom.old_position = ens.position( atom.index );
  // New random position within specific region
  atom.new_position = regions.system_region().new_position( rgnr, spc_radius );
  // No need to check for valid position because new
  // position should always give a valid position.
  resultset->add_atom( atom );
  return resultset;
  

}

// Add a definition object for this choice class to the
// meta object.
void jump_out::add_definition(choice_meta & meta)
{
  std::string desc( "MC move that involves a jump from a specific region to anywhere within the simulation. The move has equivolume probability in the simulation cell. Move is applied to all solute species." );
  std::unique_ptr< trial::choice_definition > defn( new trial::choice_definition( "jump-out", desc, &trial::chooser< jump_out >::make_chooser ) );
  
  {
    std::string help = "The name of one of the sub-regions defined in the simulation as origin of move. It is an error to use the name of the whole simulation region.";
    defn->add_definition( { core::strngs::fsregn(), "label", "region label", "required", help } );
  }
  UTILITY_CHECK( not meta.has_trial_type( defn->label() ), "This type has already been added to this meta object." );
  meta.add_trial_type( defn );

}

// Get delta from params and then create a new object.
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature:
//   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
// 

std::unique_ptr< jump_out > jump_out::make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params)
{
  // needed information
  // ispec : from ispec arg
  // region_name : from param arg
  std::size_t region_index {0ul};
  const std::string region_name_label { core::strngs::fsregn() };
  bool have_region{ false };
  // --------------------
  // region label (required)
  // check for required parameters.
  for( auto & param : params )
  {
    if( param.name() == region_name_label )
    {
      std::string region_name = param.get_text( "Jump-out origin region", core::strngs::fstry() );
      have_region = true;
      boost::algorithm::trim( region_name );
      const std::size_t region_index = gman.region_key( region_name );
      if( region_index == gman.region_count() )
      {
        // make list of region labels.
        std::stringstream rss{};
        rss << "(";
        for( std::size_t ireg = 0; ireg != gman.region_count(); ++ireg )
        {
          rss << gman.get_region( ireg ).label() << ",";
        }
        std::string list( rss.str() );
        list.back() = ')';
        throw core::input_error::parameter_value_error( "Jump-out origin region", core::strngs::fstry(), param, ( core::input_error::bad_key_message_format() % list ).str() );
      }
      if( region_index == 0 )
      {
        throw core::input_error::parameter_value_error( "Jump-out origin region", core::strngs::fstry(), param, "Jump-out target region cannot be the system region" );
      }
    }
  }
  if( not have_region )
  {
    throw core::input_error::missing_parameters_error( "Jump-out subtype trial", core::strngs::fstry(), region_name_label, params.back().filename(), params.back().line_number() );
  }
  // build choice object.
  std::unique_ptr< jump_out > current( new jump_out( ispec, region_index ) );
  return current;
  
  
  
  

}

// Only solute species can have this trial type?
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature
//   static bool make_choice( specie const& )
bool jump_out::permitted(const particle::specie & spec)
{
  return spec.is_solute();
}

jump_around::~jump_around() 
{}

std::unique_ptr< particle::change_set > jump_around::generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr)
{
  std::unique_ptr< particle::change_set > resultset( new particle::change_set( this->key() ) );
  auto const& ens = particles.get_ensemble();
  particle::change_atom atom;
  atom.key = this->key().key();
  const std::size_t spc_count( ens.specie_count( atom.key ) );
  // Select a particle at random: We can do this because we are selecting from
  // anywhere.
  switch( spc_count )
  {
  case 0:
  {
    resultset->set_fail(); // No particles of this specie
    resultset->add_atom( atom );
    return resultset;
  }
  break;
  case 1:
  {
    atom.index = ens.nth_specie_index( atom.key, 0 );
  }
  break;
  default:
  {
    atom.index = ens.nth_specie_index( atom.key, rgnr.randint( 0, spc_count - 1 ) );
  }
  break;
  }
  // Get old position
  atom.old_position = ens.position( atom.index );
  // New random position within specific region
  const double radius = particles.get_specie( atom.key ).radius();
  const auto& regn = regions.get_region( this->region_index_ );
  UTILITY_ALWAYS( regn.is_inside( atom.old_position, radius ), "Particle starts outside target region: Trial type \"jump-around\" requires that particles always begin (and end) in the same region." );
  atom.new_position = regn.new_position( rgnr, radius );
  // No need to check for valid position because new
  // position should always give a valid position.
  resultset->add_atom( atom );
  return resultset;
  

}

// Add a definition object for this choice class to the
// meta object.
void jump_around::add_definition(choice_meta & meta)
{
  std::string desc( "MC move that involves a jump from anywhere to anywhere within a specific region. It is an exception if any particles of the given species are not all in the stated region. This trial is primarily for channel-only species." );
  std::unique_ptr< trial::choice_definition > defn( new trial::choice_definition( "jump-around", desc, &trial::chooser< jump_around >::make_chooser ) );
  
  {
    std::string help = "The name of one of the sub-regions defined in the simulation. It is an error to use the name of the whole simulation region.";
    defn->add_definition( { core::strngs::fsregn(), "label", "region label", "required", help } );
  }
  UTILITY_CHECK( not meta.has_trial_type( defn->label() ), "This type has already been added to this meta object." );
  meta.add_trial_type( defn );

}

// Get delta from params and then create a new object.
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature:
//   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
// 

std::unique_ptr< jump_around > jump_around::make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params)
{
  // needed information
  // ispec : from ispec arg
  // region_name : from param arg
  std::size_t region_index {0ul};
  const std::string region_name_label { core::strngs::fsregn() };
  bool have_region{ false };
  // --------------------
  // region label (required)
  // check for required parameters.
  for( auto & param : params )
  {
    if( param.name() == region_name_label )
    {
      std::string region_name = param.get_text( "Jump-around target region", core::strngs::fstry() );
      have_region = true;
      boost::algorithm::trim( region_name );
      const std::size_t region_index = gman.region_key( region_name );
      if( region_index == gman.region_count() )
      {
        // make list of region labels.
        std::stringstream rss{};
        rss << "(";
        for( std::size_t ireg = 0; ireg != gman.region_count(); ++ireg )
        {
          rss << gman.get_region( ireg ).label() << ",";
        }
        std::string list( rss.str() );
        list.back() = ')';
        throw core::input_error::parameter_value_error( "Jump-around target region", core::strngs::fstry(), param, ( core::input_error::bad_key_message_format() % list ).str() );
      }
      if( region_index == 0 )
      {
        throw core::input_error::parameter_value_error( "Jump-around target region", core::strngs::fstry(), param, "Jump-around target region cannot be the system region" );
      }
    }
  }
  if( not have_region )
  {
    throw core::input_error::missing_parameters_error( "Jump-around subtype trial", core::strngs::fstry(), region_name_label, params.back().filename(), params.back().line_number() );
  }
  
  // build region object.
  std::unique_ptr< jump_around > current( new jump_around( ispec, region_index ) );
  return current;

}

// Only solute species can have this trial type?
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature
//   static bool make_choice( specie const& )
bool jump_around::permitted(const particle::specie & spec)
{
  return spec.is_channel_only();
}

remove_specie::~remove_specie() 
{
}

// Create a particle remove trial
std::unique_ptr< particle::change_set > remove_specie::generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr)
{
  std::unique_ptr< particle::change_set > result( new particle::change_set( this->key() ) );
  // REMOVE
  // ------
  particle::change_atom atom{}; // change being built
  atom.key = this->key().key();
  const auto& ens = particles.get_ensemble();
  auto specie_count = ens.specie_count( this->key().key() );
  if (specie_count == 0)
  {
    result->set_fail(); // No particles of this specie
  }
  else
  {
    atom.do_new = false;
  // Choose any particle of this specie
    atom.index = ens.nth_specie_index( atom.key, rgnr.randint( 0, specie_count - 1));
  // Set old position
    atom.old_position = ens.position( atom.index );
  
    auto const& spc = particles.get_specie( atom.key );
    result->update_exponential_factor( -spc.chemical_potential() );
    result->update_probability_factor( specie_count / regions.system_region().volume( spc.radius() ) );
  }
  result->add_atom( atom );
  return result;

}

// Get delta from params and then create a new object.
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature:
//   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
// 

std::unique_ptr< remove_specie > remove_specie::make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params)
{
  UTILITY_CHECK( params.size() == 1, "Trial type \"individual\" should be given no extra parameters" );
  // needed information
  // ispec : from ispec arg
  std::unique_ptr< remove_specie > current( new remove_specie( ispec ) );
  return current;

}

// Only solute species can have this trial type?
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature
//   static bool make_choice( specie const& )
bool remove_specie::permitted(const particle::specie & spec)
{
  return spec.is_solute();
}

add_specie::~add_specie()
{

}

// Create a particle addition trial
std::unique_ptr< particle::change_set > add_specie::generate(const particle::particle_manager & particles, const geometry::geometry_manager & regions, utility::random_distribution & rgnr)
{
std::unique_ptr< particle::change_set > resultset( new particle::change_set( this->key() ) );
// ADD
// ---
particle::change_atom result;
result.key = this->key().key();
const auto& ens = particles.get_ensemble();
if( ens.count() + 1 >= std::max(64ul,2ul * particles.target_count()) )
{
  resultset->set_fail(); // Too many particles
}
else
{
  result.do_old = false;
  const auto& spc = particles.get_specie( result.key );

  // give particle a random position within boundary
  result.new_position = regions.system_region().new_position( rgnr, spc.radius() );

  // Use ensemble size as 'invalid' particle index
  result.index = ens.size();

  resultset->update_exponential_factor( spc.chemical_potential() );
  resultset->update_probability_factor( regions.system_region().volume( spc.radius() ) / ( ens.specie_count( result.key ) + 1 ) );
}
resultset->add_atom( result );
return resultset;



}

// Add a definition object for this choice class to the
// meta object.
//
// NOTE: For add/remove paired type add_definition is
// only defined in add half of pair.
void add_specie::add_definition(choice_meta & meta)
{
  std::string desc( "Grand Canonical MC move involving a single particle. There is a 50:50 chance the move is an insertion or a removal trial. The move is applied to all solute species. Particles are inserted anywhere in the simulation with equivolume probability." );
  std::unique_ptr< trial::choice_definition > defn( new trial::choice_definition( "individual", desc, &trial::chooser_pair< add_specie, remove_specie >::make_chooser ) );
  // No special parameters.
  UTILITY_CHECK( not meta.has_trial_type( defn->label() ), "This type has already been added to this meta object." );
  meta.add_trial_type( defn );

}

// Get delta from params and then create a new object.
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature:
//   static unique_ptr< base_choice > make_choice( size_t, map< string, string > const& )
// 

std::unique_ptr< add_specie > add_specie::make_choice(std::size_t ispec, const geometry::geometry_manager & gman, const std::vector< core::input_parameter_memo >& params)
{
  UTILITY_CHECK( params.size() == 1, "Trial type \"individual\" should be given no extra parameters" );
  // needed information
  // ispec : from ispec arg
  std::unique_ptr< add_specie > current( new add_specie( ispec ) );
  return current;

}

// Only solute species can have this trial type?
//
// Needs to be defined as static method in derived classes that
// use chooser<>.
//
// signature
//   static bool make_choice( specie const& )
bool add_specie::permitted(const particle::specie & spec)
{
  return spec.is_solute();
}


} // namespace trial

BOOST_CLASS_EXPORT_IMPLEMENT(trial::move_choice);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::jump_choice);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::jump_in);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::jump_out);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::jump_around);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::add_specie);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::remove_specie);


BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::move_choice >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::jump_choice >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::jump_in >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::jump_out >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::jump_around >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser_pair< trial::add_specie BOOST_PP_COMMA() trial::remove_specie >);
