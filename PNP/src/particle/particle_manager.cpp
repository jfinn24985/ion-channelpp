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

#include "particle/particle_manager.hpp"
#include "particle/ensemble.hpp"
#include "core/input_document.hpp"
#include "particle/change_set.hpp"
#include "geometry/geometry_manager.hpp"
#include "utility/random.hpp"
#include "core/input_delegater.hpp"

// manual includes
#include "core/fixed_width_output_filter.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "geometry/grid.hpp"
#include "particle/specie_meta.hpp"
#include "utility/fuzzy_equals.hpp"
//-
#include <boost/iostreams/filtering_stream.hpp>
//-
namespace particle {

particle_manager::particle_manager()
: ensemble_( new particle::ensemble )
, species_()
, target_particles_()
{}

// Current charge of system
double particle_manager::charge() const
{
  double result = 0.0;
  for( std::size_t ii = 0; ii != this->ensemble_->known_keys(); ++ii )
  {
    result += ( this->species_[ii].valency() * this->ensemble_->specie_count( ii ) );
  }
  return result;
  

}

// Details about the current ensemble of particles.
void particle_manager::description(std::ostream & os) const 
{
  os << core::strngs::horizontal_bar() << "\n";
  os << "Particle system details.\n";
  os << "------------------------\n";
  os << "     number of species : " << this->specie_count() << "\n";
  os << " target particle count : " << this->target_count() << "\n"; 
  os << " target ionic strength : " << this->target_ionic_strength() << "\n"; 
  if( 0 != this->specie_count() )
  {
    os << core::strngs::horizontal_bar() << "\n";
    for( auto const& spc : this->species_ )
    {
      spc.description( os );
    }
  }
  if( this->ensemble_ )
  {
    os << core::strngs::horizontal_bar() << "\n";
    this->ensemble_->description( os );
  }

}

// Get the key of a specie from a label. 
//
// \pre has_specie(label)
// (C++ throws out_of_range if not found)
// (Python raises KeyError if not found)

std::size_t particle_manager::get_specie_key(std::string label) const 
{
  for (std::size_t ix = 0; ix != this->species_.size(); ++ix)
  {
    if (label == species_[ix].label ()) return ix;
  }
  throw std::out_of_range(label+" is not a specie label");
  return this->species_.size();

}

// Do we have a specie with this label?
bool particle_manager::has_specie(std::string name) const 
{
  for (auto const& spc: this->species_)
  {
    if (name == spc.label ()) return true;
  }
  return false;

}

// [Molar] Calculate the current ionic strength
// (N/volume). This calculation only includes the solute
// particles in the particle count.

double particle_manager::ionic_strength(double volume) const
{
  std::size_t ionic_particles = 0ul;
  for( std::size_t ispec = 0ul; ispec != this->specie_count(); ++ispec )
  {
    if( this->get_specie( ispec ).is_solute() )
    {
      ionic_particles += this->ensemble_->specie_count( ispec );
    }
  }
  return ( ionic_particles == 0ul ? 0.0 : ( ionic_particles * core::constants::to_SI() )/volume );

}

// [Molar] Sum of the specie target concentrations,
double particle_manager::target_ionic_strength() const
{
  double result = 0.0; // ionic strength
  for( auto const& spc: this->species_ )
  {
    if( spc.is_solute() )
    {
      result += spc.concentration();
    }
  }
  return result;

}

// [Molar] Calculate the target ionic strength (N/volume) based
// on the target particle count set in the input.

double particle_manager::target_ionic_strength(double volume) const
{
  std::size_t ionic_particles = this->target_count();
  return ( ionic_particles == 0ul ? 0.0 : ( ionic_particles * core::constants::to_SI() )/volume );

}

// [N] Calculate the count based on the target ionic strength
// (C/volume). 
//
// return : count as a real number.

double particle_manager::target_count(double volume) const
{
  const double ionic_conc = this->target_ionic_strength();
  return ( ionic_conc == 0.0 ? 0.0 : ( ionic_conc * volume ) / core::constants::to_SI() );

}

// Calculated from the target particle count and target
// ionic strength.
//
// \pre target_ionic_strength /= 0
double particle_manager::target_volume() const
{
  const double conc = this->target_ionic_strength();
  UTILITY_REQUIRE( conc != 0.0, "Cannot calculate target volume without a target concentration" );
  return ( ( this->target_count() == 0ul ) ? 0.0 : ( this->target_count() * core::constants::to_SI() )/conc );

}

// Write ensemble and specie information as an input document
void particle_manager::write_document(core::input_document & wr) 
{
  // Add specie definitions.
  if( this->specie_count() != 0ul )
  {
    // Update specie position information.
    //////////////////////////////////////
    if( this->ensemble_->count() != 0ul )
    {
      // local index of each specie
      std::vector< std::size_t > cursors( this->specie_count(), 0ul );
  
      for( std::size_t idx = 0ul; idx != this->ensemble_->size(); ++ idx )
      {
        const std::size_t ispec { this->ensemble_->key( idx ) };
        if( ispec != particle::specie_key::nkey )
        {
          this->species_[ ispec ].update_position( cursors[ ispec ], this->ensemble_->position( idx ) );
          ++cursors[ ispec ];
        }
      }
    }
    // Write updated specie definitions.
    ////////////////////////////////////
    for( auto const& spc : this->species_ )
    {
      spc.write_document( wr );
    }
  }
  
  

}

// Add (copy) specie to set and return the index of the specie
std::size_t particle_manager::add_specie(const specie & spc) 
{
  this->species_.push_back(spc);
  return this->species_.size() - 1;
}

// Copy particles defined in the input file into the ensemble 
// (Should only be called when generating the initial ensemble)
void particle_manager::add_predefined_particles()
{
  // Local reference to the ensemble.
  particle::ensemble &ens( *this->ensemble_ );
  
  // Adjust ensemble to recognise all specie keys
  ens.set_known_keys( this->species_.size() );
  
  // Add any predifined particles to the ensemble
  for (std::size_t ispec { 0 }; ispec != this->species_.size(); ++ispec)
  {
     auto const& spc = this->species_[ ispec ];
     if ( spc.get_position_size() != 0 )
     {
        for (std::size_t idx { 0 }; idx != spc.get_position_size(); ++idx)
        {
           ens.append_position( ispec, spc.get_position( idx ) );
        }
     }
  }

}

// Change, add or remove a particles defined by change set if
// change has not failed.
//
// \pre not atomset.fail
void particle_manager::commit(change_set & atomset)
{
  UTILITY_REQUIRE( not atomset.fail(), "Can not commit failed change." );
  this->ensemble_->commit( atomset );
}

// Calculate the system size parameters ( concentration, volume,
// particle count ) based on which of these have been provided
// in the input file.  If all are provided then priority is
// concentration > particle count > volume.
//
// Returns concentration, volume and count as well as boolean
// values noting if the values where altered.  Most valid inputs
// will have none of the parameters set to zero and one or more
// of the booleans should be true.
//
// It is usually an error if no concentration and no solute
// particles are (pre)defined.  This leads to both the
// concentration and count parameters being at zero.  It is not
// an error if the system has non-solute species and particles
// defined (and no Grand-canonical trials), as this would be a
// simulation without solute.  This method tests for this special
// case before throwing an error.
//

std::tuple< double, bool, double, bool, std::size_t, bool > particle_manager::compute_initial_size(double input_volume)
{
  // Initial value (may be 0.0)
  double conc{ this->target_ionic_strength() };
  bool conc_set{ false };
  // Initial values (may not be 0.0)
  double volume{ input_volume };
  bool volume_set{ false };
  // Initial values (may be 0)
  std::size_t count{ this->target_count() };
  bool count_set{ false };
  
  UTILITY_CHECK( volume > 0.0, "System volume should never be zero or less." );
  UTILITY_CHECK( conc >= 0.0, "System concentration should never be less than zero." );
  
  // if target conc == 0, skip volume/count adjustment
  if( conc != 0.0 )
  {
    // Number of solute particles in simulation
    if( count == 0 )
    {
      // CASE 1 : *N* = V . C
      // set npart from volume
      double dcount{ ( conc * volume ) / core::constants::to_SI() };
      UTILITY_CHECK( dcount > 0.0, "Target count should never be zero or less here." );
      count = static_cast< std::size_t >( std::nearbyint( dcount ) );
      count_set = true;
    }
    else
    {
      // CASE 2 : *V* = N / C
      volume = ( count * core::constants::to_SI() )/ conc;
      volume_set = true;
    }
  }
  else
  {
    // CASE 3 : C = N / V
    // Calculate target concentration from particle numbers
    // and volume
    if( count == 0 )
    {
      // Count particles from ensemble
      if( this->ensemble_->size() > 0 )
      {
        for( std::size_t idx = 0; idx != this->ensemble_->size(); ++idx )
        {
          std::size_t key = this->ensemble_->key( idx );
          if( key < this->species_.size() and this->species_[ key ].is_solute() )
          {
            ++count;
          }
        }
      }
      else
      {
        for( std::size_t idx = 0; idx != this->species_.size(); ++idx )
        {
          if( this->species_[ idx ].is_solute() )
          {
            count += this->species_[ idx ].get_position_size();
          }
        }
      }
      if( count != 0 )
      {
        count_set = true;
        conc = ( count * core::constants::to_SI() ) / volume;
        conc_set = true;
      }
      else
      {
        // Error if no particles are predefined.
        if( this->ensemble_->size() == 0 )
        {
          std::size_t nonsolute{ 0ul };
          bool has_solute{ false };
          for( std::size_t idx = 0; idx != this->species_.size(); ++idx )
          {
            if( this->species_[ idx ].is_solute() )
            {
              has_solute = true;
              break;
            }
            else
            {
              nonsolute += this->species_[ idx ].get_position_size();
            }
          }
          if( has_solute )
          {
            std::stringstream ss;
            ss << "Add predefined particles (parameter \""
               << core::strngs::fsn() << "\") or target concentration (parameter \""
               << core::strngs::fsctrg() << "\") to solute species.";
            throw core::input_error::input_logic_error( "Unable to automatically determine initial per-specie particle counts from input", core::strngs::fsspec(), ss.str() );
          }
          if( nonsolute == 0 )
          {
            std::stringstream ss;
            ss << "Add predefined particles (parameter \""
               << core::strngs::fsn() << "\") non-solute species.";
             throw core::input_error::input_logic_error( "Unable to automatically determine initial per-specie particle counts from input", core::strngs::fsspec(), ss.str() );
          }
        }
      }
    }
    else
    {
      conc = ( count * core::constants::to_SI() ) / volume;
      conc_set = true;
    }
  }
  return std::make_tuple( conc, conc_set, volume, volume_set, count, count_set );

}

// Generate an initial ensemble to run a simulation.
//
// Throws an INPUT error if there is an error in the predefined
// particles or if it can not add enough particles to satisfy
// the target concentration.  The last case probably indicates
// an input error as the grid based position generator should
// give structures close to that of a crystal before it fails.

void particle_manager::generate_simulation(geometry::geometry_manager & gman, utility::random_distribution & ranf, std::ostream & oslog)
{
  {
    // Stage 1 : Calculate target particle count or system volume.
    //////////////////////////////////////////////////////////////
    double conc, vol;
    std::size_t count;
    bool cset, vset, nset;
    std::tie( conc, cset, vol, vset, count, nset ) = this->compute_initial_size( gman.system_region().volume( 0.0 ) );
  
    // if target conc == 0, skip volume/count adjustment
    if( vset )
    {
      gman.change_volume( vol, 0.0 );
    }
    else
    {
      if( nset )
      {
        this->set_target_count( count );
      }
      if( cset )
      {
        // SET CONCENTRATIONS
        /////////////////////
        // Ionic strength set. This means that target
        // concentrations have not been set on the solute
        // species but solute particles have been defined.
        UTILITY_CHECK( this->ensemble_->size() > 0, "Predefined particles not yet added." );
        const double factor{ conc / count };
        for( auto &spec : this->species_ )
        {
          if( spec.is_solute() )
          {
            spec.set_concentration( spec.get_position_size() * factor );
          }
        }
      }
    }
  }
  // Set system size
  auto const& ens = *( this->ensemble_ );
  {
  // Stage 2 : Add predefined particles.
  ////////////////////////////////////////////////////////
    this->add_predefined_particles();
    // verify added particles
    if( ens.size() > 0 )
    {
      std::size_t nspec = this->specie_count();
      std::vector< double > rads( nspec );
      for( std::size_t ispec = 0; ispec != nspec; ++ispec )
      {
        rads[ispec] = this->get_specie( ispec ).radius();
      }
      oslog << core::strngs::horizontal_bar() << "\n";
      oslog << " Predefined particles\n";
      oslog << core::strngs::horizontal_bar() << "\n";
      boost::format line( " %|3|  %|3|   %|8|  %|8|  %|8| " );
      oslog << boost::format( line ) % "IDX" % "KEY" % "X" % "Y" % "Z" << "\n";
  
      auto const& cell = gman.system_region();
      bool is_overlap_with_region = false;
      bool is_overlap_in_predefined_particles = false;
      std::stringstream errmsg;
      for( std::size_t idx = 0; idx != ens.size(); ++idx )
      {
        const std::size_t ispec = ens.key( idx );
        if( ispec < nspec )
        {
          oslog << boost::format( line ) % ( idx + 1 ) % ens.key( idx )
                % ens.x( idx ) % ens.y( idx ) % ens.z( idx ) << "\n";
          // check particle is in a valid position.
          /////////////////////////////////////////
          if( not cell.is_inside( ens.position( idx ), rads[ ispec ] ) )
          {
            is_overlap_with_region = true;
            errmsg << "OUT-OF-BOUNDS: Particle " << ( idx + 1 ) << " is not in valid position in the cell.\n";
          }
          // check for overlap in initial system.
          ///////////////////////////////////////
          if( idx + 1 < ens.size() )
          {
            std::vector< double > rij;
            gman.calculate_distances( ens.position( idx ), ens.get_coordinates(), rij, idx + 1, ens.size() );
            const double iradius = rads[ ispec ];
            for( std::size_t jdx = idx + 1; jdx != rij.size(); ++jdx )
            {
              const std::size_t jspec = ens.key( idx );
              if( jspec < nspec )
              {
                const double min_distance = iradius + rads[ jspec ];
                if( min_distance > rij[ jdx ] )
                {
                  is_overlap_in_predefined_particles = true;
                  errmsg << "OVERLAP: Distance |" << ( idx + 1 ) << ", " << ( jdx + 1 ) << "| (= "
                         << rij[ jdx ] << " ) needs to be greater than " << min_distance << ".\n";
                }
              }
            }
          }
        }
      }
      oslog << core::strngs::horizontal_bar() << "\n";
      if( is_overlap_in_predefined_particles or is_overlap_with_region )
      {
        oslog << errmsg.rdbuf();
        if( is_overlap_in_predefined_particles )
        {
          throw core::input_error::parameter_value_error( "Particle position", core::strngs::fsspec(), core::strngs::fsn(), "see log output", nullptr, "Need to remove particle-particle overlap in predifined particle set." );
        }
        if( is_overlap_with_region )
        {
          throw core::input_error::parameter_value_error( "Particle position", core::strngs::fsspec(), core::strngs::fsn(), "see log output", nullptr, "Need to remove particle-object overlap in predifined particle set." );
        }
      }
    }
  }
  
  // if target conc == 0, skip adding solute particles
  if( not utility::feq( this->target_ionic_strength(), 0.0 ) )
  {
  // Stage 3 : Add solute particles to reach target count.
  ////////////////////////////////////////////////////////
    const double target_conc = this->target_ionic_strength();
  
    // Randomised list of keys
    std::vector< std::size_t > keys;
  
    // Total number of solute particles
    const std::size_t add_target = this->target_count();
  
    UTILITY_CHECK( add_target > 0, "generate_simulation stage 1 should ensure target count is greater than 0" );
  
    // Number of species
    const std::size_t nspec = this->specie_count();
  
    // Per specie particle count
    std::vector< std::size_t > to_add( nspec );
  
    // Per specie particle radii
    std::vector< double > rads( nspec );
  
    for( std::size_t ispec = 0; ispec != nspec; ++ispec )
    {
      rads[ispec] = this->get_specie( ispec ).radius();
      if( this->get_specie( ispec ).is_solute() )
      {
        const std::size_t specie_target = std::nearbyint( add_target * this->get_specie( ispec ).concentration() / target_conc );
        const std::size_t specie_current = ens.specie_count( ispec );
        // Ignore excess if too many particles present.
        if( specie_target < specie_current )
        {
          std::stringstream message;
          message << "WARNING: Specie " << this->get_specie( ispec ).label()
                  << " has " << specie_current << " particle defined in the "
                  << "input file but only "<< specie_target 
                  << " are required for the simulation. You may"
                  << "ignore this message if the simulation involves "
                  << "grand-canonical moves. You may want to change "
                  << "the input file and restart the simulation for "
                  << "fixed-size simulations.";
          {
            const std::string msg( message.str() );
            core::fixed_width_output_filter aset( 2, 1, 65 );
            boost::iostreams::filtering_ostream out;
            out.push( aset );
            out.push( oslog );
            boost::iostreams::write( out, msg.data(), msg.size() );
          }
        }
        to_add[ispec] = ( specie_target <= specie_current ? 0ul : specie_target - specie_current );
        if( to_add[ispec] > 0 )
        {
          keys.resize( keys.size() + to_add[ispec], ispec );
        }
      }
    }
    // Only if we have particles
    if( keys.size() != 0 )
    {
      // Randomly order the specie keys
      if( keys.size() > 1 )
      {
        ranf.shuffle( keys.begin(), keys.end() );
      }
      // Calculate minimum grid spacing
      double min_spacing( 0.0 );
      for( std::size_t ispec = 0; ispec != nspec; ++ispec )
      {
        min_spacing = std::max( min_spacing, rads[ispec] );
      }
      min_spacing *= 2.01;
  
      {
        // get gridder based on minimum spacing
        auto gridder = gman.system_region().make_gridder( min_spacing, ranf );
  
        if( gridder->size() < keys.size() )
        {
          throw core::input_error::parameter_value_error( "Concentration", core::strngs::fsspec(), core::strngs::fsctrg(), "see log", nullptr, "Unable to create initial system as concentration is too high, need to reduce concentration of some or all species." );
        }
        // index into keys vector
        std::size_t idx = 0;
        // Only need to check overlap with predefined particles
        // as grid ensures no overlap between the one we insert here.
        const std::size_t predef = ens.size();
        geometry::coordinate pos;
        while( idx != keys.size() and gridder->next( pos ) )
        {
          const std::size_t ispec = keys[ idx ];
          const double iradius = rads[ ispec ];
  
          // Check for overlap.
          bool overlap = false;
          if( predef > 0 )
          {
            std::vector< double > rij;
            gman.calculate_distances( pos, ens.get_coordinates(), rij, 0ul, predef );
            for( std::size_t jdx = 0; jdx != predef; ++jdx )
            {
              const std::size_t jspec = ens.key( jdx );
              if( jdx < nspec )
              {
                if( iradius + rads[ jspec ] > rij[ jdx ] )
                {
                  overlap = true;
                  break;
                }
              }
            }
          }
          // if overlap skip this grid point
          if( overlap ) continue;
  
          this->ensemble_->append_position( ispec, pos );
          ++idx;
        }
        if( idx < keys.size() )
        {
          throw core::input_error::parameter_value_error( "Concentration", core::strngs::fsspec(), core::strngs::fsctrg(), "see log", nullptr, "Unable to create initial system as concentration is too high, need to reduce concentration of some or all species." );
        }
      }
    }
  }

}

// Calculated the target particle count from target
// ionic strength and the given volume.
//
// \pre target_ionic_strength /= 0
// \pre vol /= 0
//
// count = vol * target_ionic_strength / to_SI
void particle_manager::set_target_count_by_volume(double vol)
{
  UTILITY_REQUIRE( vol > 0.0, "Cannot calculate target count without a positive volume" );
  const double conc = this->target_ionic_strength();
  UTILITY_REQUIRE( conc > 0.0, "Cannot calculate target count without a positive target concentration" );
  this->set_target_count( std::nearbyint( conc * vol / core::constants::to_SI() ) );

}

// Adds factory methods and input parsers for all the types that can be instantiated from 
// the input file.
void particle_manager::build_input_delegater(boost::shared_ptr< particle_manager > pman, core::input_delegater & delegate)
{
  //////////
  // Species
  boost::shared_ptr< core::input_base_meta > smeta{ new particle::specie_meta( pman ) };
  delegate.add_input_delegate( smeta );

}


} // namespace particle
