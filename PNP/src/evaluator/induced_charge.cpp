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

#include "evaluator/induced_charge.hpp"
#include "utility/archive.hpp"

#include "evaluator/evaluator_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "particle/change_set.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "core/input_document.hpp"
#include "geometry/membrane_cylinder_region.hpp"
#include "geometry/coordinate.hpp"

// manual includes
#include "core/constants.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "particle/ensemble.hpp"
// -
#include <array>
// For "dump" method.
#include <fstream>
// - 
namespace evaluator {

induced_charge::induced_charge()
: amx_()
, c_()
, cnew_()
, grid_( new icc_surface_grid )
, h_()
, hnew_()
, rip_()
, ripnew_()
, a_matrix_filename_()
, patch_filename_()
, alfa_()
, epsw_()
, old_potential_()
, new_potential_()
, epspr_( 10.0 )
, protein_radius_()
, membrane_arc_radius_()
{}


// Create and add a evaluator_definition to the meta object.
void induced_charge::add_definition(evaluator_meta & meta)
{
  std::string desc( "Calculate the energy contribution from the interactions of charged particles with the induced surface charges." );
  std::unique_ptr< evaluator_definition > defn( new evaluator_definition( type_label_(), desc, &make_evaluator ) );
  // Add parameter definitions.
  defn->add_definition( { core::strngs::fsdxf(), "number", ">0", "1.6", "The factor determining integration patches around the axis of rotation." } );
  defn->add_definition( { core::strngs::fsdxw(), "number", ">0", "1.6", "The factor determining integration patches along the axis of rotation." } );
  defn->add_definition( { core::strngs::fsnsub(), "ordinal", ">0", "10", "The factor determining number of integration subpatches when integrating within a patch." } );
  defn->add_definition( { core::strngs::fsepsp(), "number", ">0", "10", "The relative permittivity of the protein media." } );
  defn->add_definition( { core::strngs::fsgrl4(), "number", ">0", "(rl1+rlvest+rlcurv)", "The outer radius of the protein toroid." } );
  defn->add_definition( { core::strngs::fsgrlc(), "number", ">0", "10", "The arc radius of outer curve of the protein toroid." } );
  defn->add_definition( { patch_file_label(), "file path", "", "(optional)", "If this option is present, the surface patch information is written into a file of the given name." } );
  defn->add_definition( { amx_file_label(), "file path", "", "(optional)", "If this option is present, the A matrix is written into a file of the given name." } );
  
  meta.add_definition( defn );

}

//Set up the evaluator using the given map of name/value pairs
//
//Valid only for channel_system simtypes. Accepts no input parameters.
std::unique_ptr< base_evaluator > induced_charge::make_evaluator(const std::vector< core::input_parameter_memo >& param_set)
{
std::unique_ptr< induced_charge > cc( new induced_charge );
const std::string patch_label{ patch_file_label() };
const std::string amx_label{ amx_file_label() };
for( auto & param : param_set )
{
  if ( core::strngs::fsdxf() == param.name() )
  {
    const double dxf{ param.get_float( "Angular integration factor", core::strngs::evaluator_label(), true, false ) };
    cc->set_dxf( dxf );
  }
  else if ( core::strngs::fsdxw() == param.name() )
  {
    const double dxw{ param.get_float( "Axial integration factor", core::strngs::evaluator_label(), true, false ) };
    cc->set_dxw( dxw );
  }
  else if ( core::strngs::fsnsub() == param.name() )
  {
    const std::size_t nsub0{ param.get_ordinal( "Subpatch integration factor", core::strngs::evaluator_label() ) };
    if( nsub0 == 0ul )
    {
      throw core::input_error::parameter_value_error( "Subpatch integration factor", core::strngs::evaluator_label(), param, core::input_error::number_zero_message() );
    }
    cc->set_nsub0( nsub0 );
  }
  else if ( core::strngs::fsepsp() == param.name() )
  {
    const double epspr{ param.get_float( "Protein relative permittivity", core::strngs::evaluator_label(), true, false ) };
    cc->set_protein_permittivity( epspr );
  }
  else if ( core::strngs::fsgrl4() == param.name() )
  {
    const double radius{ param.get_float( "Protein outer radius", core::strngs::evaluator_label(), true, false ) };
    cc->set_protein_radius( radius );
  }
  else if ( core::strngs::fsgrlc() == param.name() )
  {
    const double radius{ param.get_float( "Protein outer arc radius", core::strngs::evaluator_label(), true, false ) };
    cc->set_membrane_arc_radius( radius );
  }
  else if ( patch_label == param.name() )
  {
    std::string filename{ param.get_text( "Patch file path", core::strngs::evaluator_label() ) };
    cc->set_patch_filename( filename );
  }
  else if ( amx_label == param.name() )
  {
    std::string filename{ param.get_text( "Patch file path", core::strngs::evaluator_label() ) };
    cc->set_a_matrix_filename( filename );
  }
  else
  {
    UTILITY_REQUIRE( param.name() == core::strngs::fsend(), "Parameter check should have been performed in reader.do_read_end" );
  }
}
std::unique_ptr< base_evaluator > result( cc.release() );
return result;

}

// UNIFIED ICC ENERGY
// 
// This method calculates the change in coulombic potential energy between
// the particles and the surface patches.

void induced_charge::compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const
{
if( not changes.fail() )
{
  //
  // Calculate the electric field at each tile. This
  //   should be the same as the previous field adjusted
  //   for the changes. We also calculate ripnew
  //   distance arrays for any changes with new positions
  //
  // O(changes.size() * icc.size())
  std::copy( this->c_.begin(), this->c_.end(), this->cnew_.begin() );

  std::valarray< double > valency_array( pman.specie_count() );
  for( std::size_t idx = 0; idx != valency_array.size(); ++idx )
  {
    valency_array[ idx ] = pman.get_specie( idx ).valency();
  }

  // Check size of ripnew
  if ( this->ripnew_.size() < changes.size() )
  {
    this->ripnew_.resize( changes.size(), std::vector< double >( this->size() ) );
  }

  const double epsw{ this->epsw_ };
  const particle::ensemble &ens{ pman.get_ensemble() };
  //
  //   C = (1/4.pi.eps) SUM{ (q_i dot(r_ip,u_i) / ||r_ip||^3 ) }
  //
  for (std::size_t cursor = 0; cursor != changes.size(); ++cursor)
  {
    auto const& atom = changes[ cursor ];
    const double valency = valency_array[ atom.key ];
    if ( atom.do_old )
    {
      // Subtract field from old position from cnew
      // (that actually means we add to cnew)
      std::array< std::size_t, 2 > idx;
      idx[ 0 ] = atom.index;
      for ( std::size_t ipch = 0; ipch != this->size(); ++ipch )
      {
        idx[ 1 ] = ipch;
        // vector is from tile to charge
        const double rxki = atom.old_position.x - this->grid_->x( ipch );
        const double ryki = atom.old_position.y - this->grid_->y( ipch );
        const double rzki = atom.old_position.z - this->grid_->z( ipch );
        const double rip  = this->rip_( idx );
        const double rki  = rxki * this->grid_->ux( ipch )
                            + ryki * this->grid_->uy( ipch )
                            + rzki * this->grid_->uz( ipch );
        const double delta_eps { this->grid_->eps_in( ipch ) - this->grid_->eps_out( ipch ) };
        this->cnew_[ ipch ] += valency * rki * delta_eps /
                               (4 * core::constants::pi() * epsw * std::pow( rip, 3 ) );
      }
    }
    if ( atom.do_new )
    {
      // Add field from new position to cnew
      // (that actually means we subtract from cnew)
      for (std::size_t ipch = 0; ipch != this->size(); ++ipch )
      {
        const double rxki = atom.new_position.x - this->grid_->x( ipch );
        const double ryki = atom.new_position.y - this->grid_->y( ipch );
        const double rzki = atom.new_position.z - this->grid_->z( ipch );
        const double rip  = std::sqrt( std::pow( rxki, 2 ) + std::pow( ryki, 2 ) + std::pow( rzki, 2 ) );
        const double rki  = rxki * this->grid_->ux( ipch )
                            + ryki * this->grid_->uy( ipch )
                            + rzki * this->grid_->uz( ipch );
        const double delta_eps { this->grid_->eps_in( ipch ) - this->grid_->eps_out( ipch ) };
        this->cnew_[ ipch ] -= valency * rki * delta_eps /
                               (4 * core::constants::pi() * epsw * std::pow( rip, 3 ) );
        // Calculate and store distances between patches and particle
        this->ripnew_[ cursor ][ ipch ] = rip;
      }
    }
  }
#ifdef CHECK_CH
  std::vector< double > cchk( this->size(), 0.0 );
  for (std::size_t ii = 0; ii != ens.size(); ++ii)
  {
    const std::size_t ispec = ens.key( ii );
    if (ispec == particle::specie_key::nkey)
    {
      continue;
    }
    bool skip_index( false );
    switch( changes.size() )
    {
    case 2ul:
      if (ii == changes[ 1 ].index) continue;
    // fall through
    case 1ul:
      if (ii == changes[ 0 ].index) continue;
      break;
    default:
      for (size_t cursor = 0; cursor != changes.size(); ++cursor)
      {
        if (ii == changes[ cursor ].index)
        {
          skip_index = true;
          break;
        }
      }
      break;
    }
    if (skip_index) continue;
    const double valency = valency_array[ ispec ];
    const geometry::coordinate pos( ens.position( ii ) );
    //
    //  calculate ( r_ij . n_i ) / | r_ij |^3
    //
    //  NOTE: direction of vector r_ij is important for the
    //  dot product. The vector goes from tile i to particle j.
    for (std::size_t ipch = 0; ipch != this->size(); ++ipch )
    {
      const double rxki = pos.x - this->grid_->x( ipch );
      const double ryki = pos.y - this->grid_->y( ipch );
      const double rzki = pos.z - this->grid_->z( ipch );
      const double rip  = std::sqrt( std::pow( rxki, 2 ) + std::pow( ryki, 2 ) + std::pow( rzki, 2 ) );
      const double rki  = rxki * this->grid_->ux( ipch )
                          + ryki * this->grid_->uy( ipch )
                          + rzki * this->grid_->uz( ipch );
      // Scale by change in eps over boundary.
      const double delta_eps { this->grid_->eps_in( ipch ) - this->grid_->eps_out( ipch ) };
      cchk[ ipch ] -= valency * delta_eps * rki / (epsw * 4 * core::constants::pi() * std::pow( rip, 3 ) );
    }
  }
  for (auto const& atom : changes )
  {
    if (atom.do_new)
    {
      const double valency = valency_array[ atom.key ];
      const geometry::coordinate pos( atom.new_position );
      const double epsw = epsw;
      //
      //  calculate ( r_ij . n_i ) / | r_ij |^3
      //
      //  NOTE: direction of vector r_ij is important for the
      //  dot product. The vector goes from tile i to particle j.
      for (std::size_t ipch = 0; ipch != this->size(); ++ipch )
      {
        const double rxki = pos.x - this->grid_->x( ipch );
        const double ryki = pos.y - this->grid_->y( ipch );
        const double rzki = pos.z - this->grid_->z( ipch );
        const double rip  = std::sqrt( std::pow( rxki, 2 ) + std::pow( ryki, 2 ) + std::pow( rzki, 2 ) );
        const double rki  = rxki * this->grid_->ux( ipch )
                            + ryki * this->grid_->uy( ipch )
                            + rzki * this->grid_->uz( ipch );
        // Scale by change in eps over boundary.
        const double delta_eps { this->grid_->eps_in( ipch ) - this->grid_->eps_out( ipch ) };
        cchk[ ipch ] -= valency * delta_eps * rki / (epsw * 4 * core::constants::pi() * std::pow( rip, 3 ) );
      }
    }
  }
  bool pass_cchk = true;
  for (size_t index = 0; index != cchk.size(); ++index)
  {
    if (not utility::feq(cchk[index], this->cnew_[index]))
    {
      std::cerr << "INDEX: " << index;
      switch( changes.size() )
      {
      case 2ul:
        std::cerr << " CHANGE[1].index: " << changes[1].index;
      // fall through
      case 1ul:
        std::cerr << " CHANGE[0].index: " << changes[0].index;
        break;
      default:
        for (size_t cursor = 0; cursor != changes.size(); ++cursor)
        {
          std::cerr << " CHANGE[" << cursor << "].index: " << changes[0].index;
        }
        break;
      }
      std::cerr << " C_OLD: " << this->c_[index];
      std::cerr << " C_NEW: " << this->cnew_[index];
      std::cerr << " C_CHK: " << cchk[index];
      std::cerr << " DELTA: " << (cchk[index]- this->cnew_[index]) << "\n";
      pass_cchk = false;
    }
  }
#endif // CHECK_CH
//
// Calculate the new H
//
  std::copy( this->cnew_.begin(), this->cnew_.end(), this->hnew_.begin() );

  this->amx_.back_substitute( this->hnew_ );

#ifdef CHECK_CH
  bool pass_hchk = true;
  this->amx_.back_substitute( cchk );

  for (size_t index = 0; index != cchk.size(); ++index)
  {
    if (std::abs(cchk[index] - this->hnew_[index]) > 1E-20)
    {
      std::cerr << "INDEX: " << index;
      switch( changes.size() )
      {
      case 2ul:
        std::cerr << " CHANGE[1].index: " << changes[1].index;
      // fall through
      case 1ul:
        std::cerr << " CHANGE[0].index: " << changes[0].index;
        break;
      default:
        for (size_t cursor = 0; cursor != changes.size(); ++cursor)
        {
          std::cerr << " CHANGE[" << cursor << "].index: " << changes[0].index;
        }
        break;
      }
      std::cerr << " H_OLD: " << this->h_[index];
      std::cerr << " H_NEW: " << this->hnew_[index];
      std::cerr << " H_CHK: " << cchk[index];
      std::cerr << " DELTA: " << (cchk[index]- this->hnew_[index]) << "\n";
      pass_cchk = false;
    }
  }
  UTILITY_ALWAYS(pass_cchk and pass_hchk, "Error computing C_new at index ");
#endif // CHECK_CH

  //
  // Calculate new potential
  //
  //   U = (1/4.pi.eps) SUM{ z_i . h_p . area_p / 2 ||r_ip|| }
  //
  // O(ens.size() * icc.size())
  //
  double old_potential = 0.0;
#ifdef CHECK_OLD_POTENTIAL
  // cache charge density * area calculation.
  std::valarray< double > surface_field( this->size() );
#endif // CHECK_OLD_POTENTIAL
  double new_potential = 0.0;
  // cache charge density * area calculation.
  std::valarray< double > surface_field_new( this->size() );
  for ( std::size_t ipch = 0; ipch != this->size(); ++ipch )
  {
#ifdef CHECK_OLD_POTENTIAL
    surface_field[ ipch ] = this->h_[ ipch ] * this->grid_->area( ipch );
#endif // CHECK_OLD_POTENTIAL
    surface_field_new[ ipch ] = this->hnew_[ ipch ] * this->grid_->area( ipch );
  }

  // POTENTIAL OPENMP LOOP
  for (std::size_t ii = 0; ii != ens.size(); ++ii)
  {
    const std::size_t ispec( ens.key( ii ) );
    if (ispec != particle::specie_key::nkey )
    {
      std::array< std::size_t, 2 > idx;
      idx[ 0 ] = ii;
      const double valency = valency_array[ ispec ];
      for ( std::size_t ipch = 0; ipch != this->size(); ++ipch )
      {
        idx[ 1 ] = ipch;
#ifdef CHECK_OLD_POTENTIAL
        old_potential += valency * surface_field[ ipch ] / ( 2 * this->rip_( idx ) );
#endif // CHECK_OLD_POTENTIAL
        new_potential += valency * surface_field_new[ ipch ] / ( 2 * this->rip_( idx ) );
#ifdef CHECK_RIP
        const geometry::coordinate pos( ens.position( ii ) );
        const double rxki = pos.x - this->grid_->x( ipch );
        const double ryki = pos.y - this->grid_->y( ipch );
        const double rzki = pos.z - this->grid_->z( ipch );
        const double rip  = std::sqrt( std::pow( rxki, 2 ) + std::pow( ryki, 2 ) + std::pow( rzki, 2 ) );
        if (this->rip_( idx ) != rip)
        {
          std::cerr << "RIP ERROR: PATCH: " << ipch << " INDEX: " << ii;
          std::cerr << "RIP(P,I): " << this->rip_( idx );
          std::cerr << "RIP: " << rip;
          std::cerr << "DELTA: " << (this->rip_( idx ) - rip) << "\n";
        }
#endif // CHECK_RIP
      }
    }
  }
  // Remove effect of changes from new_potential
  for (std::size_t cursor = 0; cursor != changes.size(); ++cursor)
  {
    auto const& atom = changes[ cursor ];
    const double valency = valency_array[ atom.key ];
    std::array< std::size_t, 2 > idx;
    if ( atom.do_old )
    {
      // Subtract potential from old position
      idx[ 0 ] = atom.index;
      for ( std::size_t ipch = 0; ipch != this->size(); ++ipch )
      {
        idx[ 1 ] = ipch;
        new_potential -= valency * surface_field_new[ ipch ] / (2 * this->rip_( idx ) );
      }
    }
    if ( atom.do_new )
    {
      // Add potential from new position
      for (std::size_t ipch = 0; ipch != this->size(); ++ipch )
      {
        new_potential += valency * surface_field_new[ ipch ] / (2 * this->ripnew_[ cursor ][ ipch ] );
      }
    }
  }
  // Scale energy to correct units (k_BT) + SI
  new_potential *= this->alfa_;
#ifdef CHECK_OLD_POTENTIAL
  old_potential *= this->alfa_;
  if (std::abs(this->old_potential_ - old_potential) > 5E-16)
  {
    std::cerr << "OLD POTENTIAL MISMATCH: CACHE: " << this->old_potential_;
    std::cerr << " CALC: " << old_potential;
    std::cerr << " DELTA: " << (this->old_potential_ - old_potential) << "\n";
  }
#endif // CHECK_OLD_POTENTIAL
  old_potential = this->old_potential_;
  this->new_potential_ = new_potential;
  // Set potential energy on first atom in change set
  changes.begin()->energy_old += old_potential;
  changes.begin()->energy_new += new_potential;
}

}

//Calculate the total potential energy.
double induced_charge::compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const
{
  return this->old_potential_;

}

// Prepare the evaluator for running a simulation loop set.
void induced_charge::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman)
{
  if (this->size() == 0)
  {
    this->set_alfa( eman.temperature() );
    this->epsw_ = eman.permittivity();
  
    // create A matrix.  Need to down cast region to be
    // able to get protein geometry.
    const geometry::membrane_cylinder_region * cellrgn = nullptr;
    cellrgn = dynamic_cast< const geometry::membrane_cylinder_region* >( &(gman.system_region()) );
    if( nullptr == cellrgn )
    {
      throw core::input_error::input_logic_error( "Induced charge computation \"" + type_label_() +"\"", core::strngs::evaluator_label(), "This evaluator requires the system region to be of type \""+geometry::membrane_cylinder_region::type_label()+"\" or similar." );
    }
  
    this->create_amx( *cellrgn );
  
    // write out data if dump filenames defined
    this->write_data();
  }
  // (Re)Generate the initial rip, c and h vectors
  this->compute_initial_c_h( pman );
  

}

// Log message descibing the evaluator and its parameters
void induced_charge::do_description(std::ostream & os) const
{
  os << " Compute the induced charge on the dielectric boundary surface\n";
  os << " and the resulting electrostatic potential on the particles.\n\n";
  os << " Computing the induced charge is made reasonalbly efficient\n";
  os << " by fixing the position of the boundary surface during the\n";
  os << " simulation and dividing the surface. Using these surface\n";
  os << " elements allows the generation of a set of simultaneous equations\n";
  os << " that relate the external field on the surface elements to the \n";
  os << " induced charge on the elements. These equations can be \n";
  os << " presolved and the solution matrix stored. Then at each\n";
  os << " simulation step the electric field at each surface element \n";
  os << " is calculated. Back-substituion into the solved simultaneous\n";
  os << " equations then gives the induced surface charges. These are\n";
  os << " then used to calculate the screened Coulomb potential between\n";
  os << " each surface element and each particle.\n\n";
  os << " Parameters:\n";
  os << "   dxf : The approximate tile width (in Angstrom) in the direction\n";
  os << "         around the rotation axis.\n";
  os << "   dxw : The approximate tile width (in Angstrom) in the direction\n";
  os << "         parallel and radial to the rotation axis.\n";
  os << "  nsub : Integration parameter indicating how many sub-tiles a\n";
  os << "         surface element is divided into during integration.\n\n";
  os << " References for induced charge:\n";
  os << "  R. Allen and J.-P. Hansen and S. Melchionna \"Electrostatic potential\n";
  os << "    inside ionic solutions confined by dielectrics: a variational approach\"\n";
  os << "    Phys Chem Chem Physics, 2001, 3, 4177-4186\n";
  os << " This implementation based on:\n";
  os << "  Dezso Boda, Dirk Gillespie, Wolfgang Nonner, Douglas Henderson, \n";
  os << "   and Bob Eisenberg \"Computing induced charges in inhomogeneous \n";
  os << "   dielectric media: Application in a Monte Carlo simulation of \n";
  os << "   complex ionic systems\" Physical Review E, 2004, 69, 046702\n";
  
  

}

// Write evaluator specific content of input file section.
//
// only throw possible should be from os.write() operation
void induced_charge::do_write_document(core::input_document & wr, std::size_t ix) const
{
wr[ ix ].add_entry( core::strngs::fsdxf(), this->grid_->get_dxf() );
wr[ ix ].add_entry( core::strngs::fsdxw(), this->grid_->get_dxw() );
wr[ ix ].add_entry( core::strngs::fsnsub(), this->grid_->get_nsub0() );

}

// Filename to save A matrix data. If empty no data is saved separately.
const std::string induced_charge::get_a_matrix_filename() const
{
  return a_matrix_filename_;
}

// Get the patch/tile delta to divide up the circumference
// of circles normal to and centred on the Z-axis
double induced_charge::get_dxf() const
{
   return this->grid_->get_dxf();
}

// Get the patch/tile delta that divides lines and arcs that
// are parallel or radial to the z-axis.
double induced_charge::get_dxw() const
{
   return this->grid_->get_dxw();
}

// Get the subtiling factor that determines how many subtiles
// a patch/tile is divided into during integration 
std::size_t induced_charge::get_nsub0() const
{
   return this->grid_->get_nsub0();
}

// Filename to save patch data. If empty no data is saved separately.
const std::string induced_charge::get_patch_filename() const
{
  return patch_filename_;
}

//Induced charge on a given surface element
double induced_charge::surface_charge(std::size_t idx) const
{
  return this->grid_->area( idx ) * this->h_[ idx ] / this->grid_->deps( idx ) / std::sqrt( core::constants::electron_charge() * this->alfa_ );
  

}

//Area of a given surface element
double induced_charge::surface_area(std::size_t idx) const
{
  return this->grid_->area( idx );
}

// Total area of all surface elements
double induced_charge::surface_area() const
{
  return this->grid_->surface_area();
}

std::string induced_charge::type_label_()
{
  return core::strngs::fsptch();

}

// Write surface tile and A matrix data to files
void induced_charge::write_data() const
{
  // save patch data and amx
  if( not this->patch_filename_.empty() )
  {
    std::ofstream os_pch( this->patch_filename_ );
    this->grid_->write_grid( os_pch );
  }
  if( not this->a_matrix_filename_.empty() )
  {
    std::ofstream os_amx( this->a_matrix_filename_ );
    this->amx_.write_a_matrix( os_amx );
  }

}

//Compute the initial C and H matrices
void induced_charge::compute_initial_c_h(const particle::particle_manager & pman)
{
  //
  // Calculate the electric field at each tile.
  //
  // O(ens.size() * icc.size())
  //
  // Convert energy units
  // Check size of rip
  const particle::ensemble &ens{ pman.get_ensemble() };
  if (this->rip_.shape()[0] < ens.size() or
        this->rip_.shape()[1] < this->size())
  {
     this->rip_.resize( boost::extents[ens.size()][this->size()] );
  }
  if ( this->c_.size() < this->amx_.size() )
  {
     this->c_.resize( this->amx_.size() );
  }
  if ( this->h_.size() < this->amx_.size() )
  {
     this->h_.resize( this->amx_.size() );
  }
  if ( this->cnew_.size() < this->amx_.size() )
  {
     this->cnew_.resize( this->amx_.size() );
  }
  if ( this->hnew_.size() < this->amx_.size() )
  {
     this->hnew_.resize( this->amx_.size() );
  }
  
  std::valarray< double > valency_array( pman.specie_count() );
  for( std::size_t idx = 0; idx != valency_array.size(); ++idx )
  {
    valency_array[ idx ] = pman.get_specie( idx ).valency();
  }
  
  std::fill( this->c_.begin(), this->c_.end(), 0.0 );
  
  const double epsw{ this->epsw_ };
  //
  //   C = (1/4.pi.eps) SUM{ (q_i dot(r_ip,u_i) / ||r_ip||^3 ) }
  //
  for (std::size_t ii = 0; ii != ens.size(); ++ii)
  {
     const std::size_t ispec = ens.key( ii );
     if (ispec == particle::specie_key::nkey)
     {
        continue;
     }
     const double valency{ valency_array[ ispec ]};
     const geometry::coordinate pos{ ens.position( ii ) };
     std::array< std::size_t, 2 > idx;
     idx[ 0 ] = ii;
     //
     //  calculate ( r_ij . n_i ) / | r_ij |^3
     //
     //  NOTE: direction of vector r_ij is important for the
     //  dot product. The vector goes from tile i to particle j.
     for (std::size_t ipch = 0; ipch != this->size(); ++ipch )
     {
        idx[ 1 ] = ipch;
        const double rxki = pos.x - this->grid_->x( ipch );
        const double ryki = pos.y - this->grid_->y( ipch );
        const double rzki = pos.z - this->grid_->z( ipch );
        const double rip  = std::sqrt( std::pow( rxki, 2 ) + std::pow( ryki, 2 ) + std::pow( rzki, 2 ) );
        const double rki  = rxki * this->grid_->ux( ipch )
                            + ryki * this->grid_->uy( ipch )
                            + rzki * this->grid_->uz( ipch );
        // Scale by change in eps over boundary.
        const double delta_eps{ this->grid_->eps_in( ipch ) - this->grid_->eps_out( ipch ) };
        this->c_[ ipch ] -= valency * delta_eps * rki / (epsw * 4 * core::constants::pi() * std::pow( rip, 3 ) );
        // Calculate and store distances between patches and particle
        this->rip_( idx ) = rip;
     }
  }
  
  //
  // Calculate the new H
  //
  std::copy( this->c_.begin(), this->c_.end(), this->h_.begin() );
  this->amx_.back_substitute( this->h_ );
  
  // cache charge density * area calculation.
  std::valarray< double > surface_field( this->size() );
  for ( std::size_t ipch = 0; ipch != this->size(); ++ipch )
  {
     surface_field[ ipch ] = this->h_[ ipch ] * this->grid_->area( ipch );
  }
  this->old_potential_ = 0.0;
  // POTENTIAL OPENMP LOOP
  for (std::size_t ii = 0; ii != ens.size(); ++ii)
  {
     const std::size_t ispec( ens.key( ii ) );
     if (ispec != particle::specie_key::nkey )
     {
        std::array< std::size_t, 2 > idx;
        idx[ 0 ] = ii;
        const double valency = valency_array[ ispec ];
        for ( std::size_t ipch = 0; ipch != this->size(); ++ipch )
        {
           idx[ 1 ] = ipch;
           this->old_potential_ += valency * surface_field[ ipch ] / ( 2 * this->rip_( idx ) );
        }
     }
  }
  this->old_potential_ *= this->alfa_;

}

//Compute the total surface area and charge
void induced_charge::compute_total_surface_charge(double & charget, double & areat) const 
{
  areat = 0.0;
  charget = 0.0;
  for (std::size_t ipch = 0; ipch != this->size(); ++ipch)
  {
    const double area =  this->grid_->area( ipch );
    areat += area;
    charget += area * this->h_[ipch];
  }

}

// Create the  matrix, this involves defining the patch grid,
// integrating the grid to generate the A' matrix then inverting
// this matrix to get the A matrix.
//
// Use the input geometry to define the information that the
// 'patch' module will use to generate a set of 'patches' that
// cover the protein surface.
//
// fortran equiv patch::defgrd + patch::matrix
//  see ionch::iccgrid::add_XXXX for patch::goXXXX
//  see ionch::XXXX_integrator types for patch::intXXXX
void induced_charge::create_amx(const geometry::membrane_cylinder_region & rgn)
{
   this->create_amx( rgn.channel_half_length(), rgn.channel_radius(), rgn.arc_radius() );
}

// Create the  matrix, this involves defining the patch grid,
// integrating the grid to generate the A' matrix then inverting
// this matrix to get the A matrix.
//
// Use the input geometry to define the information that the
// 'patch' module will use to generate a set of 'patches' that
// cover the protein surface.
//
// fortran equiv patch::defgrd + patch::matrix
//  see ionch::iccgrid::add_XXXX for patch::goXXXX
//  see ionch::XXXX_integrator types for patch::intXXXX
void induced_charge::create_amx(double zl1, double rl1, double rlvest)
{
UTILITY_CHECK( this->grid_->empty(), "Cannot define grid twice");

//       |theta1 = pi/2
// theta2|     theta0 = 0.0
//  ---- z ----- theta4 = 2pi
// = pi  |
//       | theta3 = 3/2 pi
// d_theta = pi/2
const double theta0( 0.0);
const double theta1( core::constants::pi() / 2);
const double theta2( core::constants::pi() );
const double theta3{ core::constants::pi() * 3.0/2.0 };
const double theta4{ 2 * core::constants::pi() };

// Get geometry data
const double geom_zl1{ zl1 };
const double geom_rl1{ rl1 };
const double geom_rl2{ rl1 + rlvest };
const double geom_rlvest{ rlvest };
const double geom_zl2{ zl1 + rlvest };
const double geom_rl3{ this->protein_radius_ - this->membrane_arc_radius_ };
const double geom_zl3{ zl1 + rlvest - this->membrane_arc_radius_ };
const double geom_rl4{ this->protein_radius_ };
const double geom_rlcurv{ this->membrane_arc_radius_ };

// Delta permittivity in channel is used here in case
// future work adds code to put second permittivity
// zone inside the channel.
const double epsw{ this->epsw_ };
const double epsch{ this->epsw_ };
const double epspr{ this->epspr_ };

// Minimum number of tiles along a dimension
const int channel_min_z_tiles{ 10 };
const int outer_min_z_tiles{ 4 };
const int min_r_tiles{ 16 };

// Channel pore cylinder
this->grid_->add_line( -geom_zl1, geom_zl1, geom_rl1, epspr, epsch, channel_min_z_tiles, min_r_tiles, true );

// Hi-z inner arc
this->grid_->add_arc( geom_zl1, geom_rl2, geom_rlvest, theta3, theta4, epspr, epsch, outer_min_z_tiles, min_r_tiles, true );

// hi-z wall
this->grid_->add_wall( geom_zl2, geom_rl2, geom_rl3, epspr, epsw, 0, 0, true );

// Hi-z outer arc
this->grid_->add_arc( geom_zl3, geom_rl3, geom_rlcurv, theta0, theta1, epspr, epsw, outer_min_z_tiles, min_r_tiles, true );

// Channel protein outer cylinder
this->grid_->add_line( -geom_zl3, geom_zl3, geom_rl4, epspr, epsw, outer_min_z_tiles, min_r_tiles, false );

// lo-z outer arc
this->grid_->add_arc( -geom_zl3, geom_rl3, geom_rlcurv, theta1, theta2, epspr, epsw, outer_min_z_tiles, min_r_tiles, true );

// lo-z wall
this->grid_->add_wall( -geom_zl2, geom_rl2, geom_rl3, epspr, epsw, 0, 0, false );

// lo-z inner arc
this->grid_->add_arc( -geom_zl1, geom_rl2, geom_rlvest, theta2, theta3, epspr, epsch, outer_min_z_tiles, min_r_tiles, true );

this->amx_.compute_amx( *this->grid_ );
this->amx_.lu_decompose_amx();

}

// Swap the internatl grid with the supplied
// grid and create the A matrix, this involves
// integrating the grid to generate the A' matrix
// then inverting this matrix to get the A matrix.
//
// Use the input geometry to define the information
// that the 'patch' module will use to generate a
// set of 'patches' that cover the protein surface.
//
// fortran equiv patch::defgrd + patch::matrix
//  see ionch::iccgrid::add_XXXX for patch::goXXXX
//  see ionch::XXXX_integrator types for
//  patch::intXXXX

void induced_charge::create_amx(std::unique_ptr< icc_surface_grid >& grid)
{
UTILITY_CHECK( this->grid_->empty(), "Cannot define grid twice");

std::swap( this->grid_, grid );

this->amx_.compute_amx( *this->grid_ );
this->amx_.lu_decompose_amx();

}

// Create the matrix like fortran does, this involves defining the patch grid,
// integrating the grid to generate the A' matrix then inverting
// this matrix to get the A matrix.
//
// Use the input geometry to define the information that the
// 'patch' module will use to generate a set of 'patches' that
// cover the protein surface.
//
// fortran equiv patch::defgrd + patch::matrix
//  see ionch::iccgrid::add_XXXX for patch::goXXXX
//  see ionch::XXXX_integrator types for patch::intXXXX
void induced_charge::create_amx_f(double zl1, double rl1, double rlvest)
{
UTILITY_CHECK( this->grid_->empty(), "Cannot define grid twice");

//       |theta1 = pi/2
// theta2|     theta0 = 0.0
//  ---- z ----- theta4 = 2pi
// = pi  |
//       | theta3 = 3/2 pi
// d_theta = pi/2
const double theta0( 0.0);
const double theta1( core::constants::pi() / 2);
const double theta2( core::constants::pi() );
const double theta3{ core::constants::pi() * 3.0/2.0 };
const double theta4{ 2 * core::constants::pi() };

// Get geometry data
const double geom_zl1{ zl1 };
const double geom_rl1{ rl1 };
const double geom_rl2{ rl1 + rlvest };
const double geom_rlvest{ rlvest };
const double geom_zl2{ zl1 + rlvest };
const double geom_rl3{ this->protein_radius_ - this->membrane_arc_radius_ };
const double geom_zl3{ zl1 + rlvest - this->membrane_arc_radius_ };
const double geom_rl4{ this->protein_radius_ };
const double geom_rlcurv{ this->membrane_arc_radius_ };

// Delta permittivity in channel is used here in case
// future work adds code to put second permittivity
// zone inside the channel.
const double epsw{ this->epsw_ };
const double epsch{ this->epsw_ };
const double epspr{ this->epspr_ };

// Minimum number of tiles along a dimension
const int channel_min_z_tiles{ 10 };
const int outer_min_z_tiles{ 4 };
const int min_r_tiles{ 16 };

// Channel pore cylinder
this->grid_->add_line( -geom_zl1, geom_zl1, geom_rl1, epspr, epsch, channel_min_z_tiles, min_r_tiles, true );

// Channel protein outer cylinder
this->grid_->add_line( -geom_zl3, geom_zl3, geom_rl4, epspr, epsw, outer_min_z_tiles, min_r_tiles, false );

// Hi-z inner arc
this->grid_->add_arc( geom_zl1, geom_rl2, geom_rlvest, theta3, theta4, epspr, epsch, outer_min_z_tiles, min_r_tiles, true );

// Hi-z outer arc
this->grid_->add_arc( geom_zl3, geom_rl3, geom_rlcurv, theta0, theta1, epspr, epsw, outer_min_z_tiles, min_r_tiles, true );

// lo-z outer arc
this->grid_->add_arc( -geom_zl3, geom_rl3, geom_rlcurv, theta1, theta2, epspr, epsw, outer_min_z_tiles, min_r_tiles, true );

// lo-z inner arc
this->grid_->add_arc( -geom_zl1, geom_rl2, geom_rlvest, theta2, theta3, epspr, epsch, outer_min_z_tiles, min_r_tiles, true );

// hi-z wall
this->grid_->add_wall( geom_zl2, geom_rl2, geom_rl3, epspr, epsw, 0, 0, true );

// lo-z wall
this->grid_->add_wall( -geom_zl2, geom_rl2, geom_rl3, epspr, epsw, 0, 0, false );

this->amx_.compute_amx( *this->grid_ );
this->amx_.lu_decompose_amx();

}

//  Called after the result of the trial is known.
void induced_charge::on_conclude_trial(const particle::change_set & changes)
{
  if (changes.accept())
  {
     std::swap(this->h_, this->hnew_);
     std::swap(this->c_, this->cnew_);
     this->old_potential_ = this->new_potential_;
     for (std::size_t cursor = 0; cursor != changes.size(); ++cursor)
     {
        auto const& atom = changes[ cursor ];
        std::array< std::size_t, 2 > idx;
        idx[ 0 ] = atom.index;
        // increase rip_ if necessary
        if (this->rip_.shape()[0] <= idx[ 0 ])
        {
          this->rip_.resize( boost::extents[idx[ 0 ] + 1][this->size()] );
        }
        for (std::size_t ipch = 0; ipch != this->size(); ++ipch)
        {
           idx[ 1 ] = ipch;
           this->rip_( idx ) = this->ripnew_[ cursor ][ ipch ];
        }
     }
  }

}

// Set filename to save A matrix information. If not set then no data is saved.
void induced_charge::set_a_matrix_filename(std::string value)
{
  a_matrix_filename_ = value;

}

//  Set the alfa scaling factor:
//  alfa = k{temperature} . echg / (4 * pi * eps0 * angstrom * {permittivity} )
//
// \param temperature : in kelvin
// \param permittivity : relative permittivity.
void induced_charge::set_alfa(double temperature)
{
  this->alfa_ = core::constants::electron_charge() / std::sqrt(4 * core::constants::pi() * core::constants::epsilon_0() * core::constants::boltzmann_constant() * temperature * core::constants::angstrom() );
  

}

// Set the patch/tile delta to divide up the circumference
// of circles normal to and centred on the Z-axis (only 
// useful before generating A matrix)
void induced_charge::set_dxf(double val)
{
  this->grid_->set_dxf( val );
}

// Set the patch/tile delta that divides lines and arcs that
// are parallel or radial to the z-axis.  (only useful before 
// generating A matrix)
void induced_charge::set_dxw(double val)
{
  this->grid_->set_dxw( val );
}

// Set the subtiling factor that determines how many subtiles
// a patch/tile is divided into during integration  (only useful 
// before generating A matrix)
void induced_charge::set_nsub0(std::size_t val)
{
  this->grid_->set_nsub0( val );
}

// Set filename to save patch information. If not set then no patch
// data is saved.
void induced_charge::set_patch_filename(std::string value)
{
  patch_filename_ = value;

}

//  Dump data from current object
void induced_charge::dump(std::string fname) const
{
  std::ofstream dos( fname );
  
  dos << "# alfa : " << this->alfa_ << "\n";
  dos << "# patch name : " << this->patch_filename_ << "\n";
  dos << "# amx name : " << this->a_matrix_filename_ << "\n";
  dos << "#  C  CNEW H HNEW RIPNEW\n";
  for ( std::size_t ii = 0; ii != this->size(); ++ii )
  {
     dos << c_[ ii ] << " " << cnew_[ ii ] << " " 
        << h_[ ii ] << " " << hnew_[ ii ] << " "
        << ripnew_[ 0 ][ ii ] << "\n";
  }
  
  dos << "# RIP\n";
  std::array< std::size_t, 2 > idx;
  for ( std::size_t ii = 0; ii != this->rip_.shape()[0]; ++ii)
  {
     idx[ 0 ] = ii;
     for (std::size_t ipch = 0; ipch != this->size(); ++ipch)
     {
        idx[ 1 ] = ipch;
        dos << ii << " " << ipch << " " << this->rip_( idx ) << "\n";
     }
  }
  dos << "# A MATRIX\n";
  this->amx_.write_a_matrix( dos );
  dos << "# GRID\n";
  this->grid_->write_grid( dos );
  

}

// Get the patch centre point and normal
void induced_charge::get_patch_coordinate(std::size_t idx, geometry::coordinate & pos, geometry::coordinate & norm)
{
  pos.x = this->grid_->x( idx );
  pos.y = this->grid_->y( idx );
  pos.z = this->grid_->z( idx );
  norm.x = this->grid_->ux( idx );
  norm.y = this->grid_->uy( idx );
  norm.z = this->grid_->uz( idx );

}

// calculate a gaussbox over ALL surfaces, so parea_ must be composed
// of closed surfaces
double induced_charge::gaussbox() const
{
  double gaussbox_{ 0.0 };
  for( std::size_t idx = 0; idx != this->size(); ++idx )
  {
      gaussbox_ += this->grid_->area( idx ) * this->h_[ idx ] / this->grid_->deps(idx);
  }
  return gaussbox_;

}


} // namespace evaluator

BOOST_CLASS_EXPORT_IMPLEMENT( evaluator::induced_charge );