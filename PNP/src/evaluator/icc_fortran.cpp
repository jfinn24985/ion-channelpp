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

#include "evaluator/icc_fortran.hpp"
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
//
#define sunFortran
#include "evaluator/cfortran.hpp"
// -

extern "C" {
// prototype Fortran functions
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GETX,__patch_MOD_getx,INT);
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GETY,__patch_MOD_gety,INT);
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GETZ,__patch_MOD_getz,INT);
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GETUX,__patch_MOD_getux,INT);
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GETUY,__patch_MOD_getuy,INT);
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GETUZ,__patch_MOD_getuz,INT);
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GETAREA,__patch_MOD_getarea,INT);
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GETDEP,__patch_MOD_getdep,INT);
PROTOCCALLSFFUN0(INT,__patch_MOD_GETNPCH,__patch_MOD_getnpch);
PROTOCCALLSFFUN1(DOUBLE,__patch_MOD_GAUSSBOX,__patch_MOD_gaussbox,DOUBLEV);
PROTOCCALLSFFUN0(DOUBLE,__patch_MOD_TOTALAREA,__patch_MOD_totalarea);
PROTOCCALLSFSUB1(__patch_MOD_BAKSUB,__patch_MOD_baksub,DOUBLEV);
PROTOCCALLSFSUB1(__patch_MOD_WRITAM,__patch_MOD_writam,STRING);
PROTOCCALLSFSUB1(__patch_MOD_WRITPC,__patch_MOD_writpc,STRING);
PROTOCCALLSFSUB11(__patch_MOD_MATRIX,__patch_MOD_matrix,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,INT,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE);
#define IONCH_PRX(A) CCALLSFFUN1(__patch_MOD_GETX,__patch_MOD_getx,INT,(A))
#define IONCH_PRY(A) CCALLSFFUN1(__patch_MOD_GETY,__patch_MOD_gety,INT,(A))
#define IONCH_PRZ(A) CCALLSFFUN1(__patch_MOD_GETZ,__patch_MOD_getz,INT,(A))
#define IONCH_PUX(A) CCALLSFFUN1(__patch_MOD_GETUX,__patch_MOD_getux,INT,(A))
#define IONCH_PUY(A) CCALLSFFUN1(__patch_MOD_GETUY,__patch_MOD_getuy,INT,(A))
#define IONCH_PUZ(A) CCALLSFFUN1(__patch_MOD_GETUZ,__patch_MOD_getuz,INT,(A))
#define IONCH_AREA(A) CCALLSFFUN1(__patch_MOD_GETAREA,__patch_MOD_getarea,INT,(A))
#define IONCH_DEP(A) CCALLSFFUN1(__patch_MOD_GETDEP,__patch_MOD_getdep,INT,(A))
#define IONCH_NPATCH() CCALLSFFUN0(__patch_MOD_GETNPCH,__patch_MOD_getnpch)
#define IONCH_GAUSSBOX(A) CCALLSFFUN1(__patch_MOD_GAUSSBOX,__patch_MOD_gaussbox,DOUBLEV,(A))
#define IONCH_TOTALAREA() CCALLSFFUN0(__patch_MOD_TOTALAREA,__patch_MOD_totalarea)
#define IONCH_WPATCH(A) CCALLSFSUB1(__patch_MOD_WRITPC,__patch_MOD_writpc,STRING,(A))
#define IONCH_WAMX(A) CCALLSFSUB1(__patch_MOD_WRITAM,__patch_MOD_writam,STRING,(A))
#define IONCH_BAKSUB(A) CCALLSFSUB1(__patch_MOD_BAKSUB,__patch_MOD_baksub,DOUBLEV,(A))


}


namespace evaluator {

icc_fortran::icc_fortran()
: c_()
, cnew_()
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
, dx_zaxis_( 1.6 )
, dx_radial_( 1.6 )
, nsub0_( 10 )
{}


// Create and add a evaluator_definition to the meta object.
void icc_fortran::add_definition(evaluator_meta & meta)
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
std::unique_ptr< base_evaluator > icc_fortran::make_evaluator(const std::vector< core::input_parameter_memo >& param_set)
{
std::unique_ptr< icc_fortran > cc( new icc_fortran );
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

void icc_fortran::compute_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman, particle::change_set & changes) const
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
      const int iipch{ ipch };
      const double rxki = atom.old_position.x - IONCH_PRX( iipch );
      const double ryki = atom.old_position.y - IONCH_PRY( iipch );
      const double rzki = atom.old_position.z - IONCH_PRZ( iipch );
      const double rip  = this->rip_( idx );
      const double rki  = rxki * IONCH_PUX( iipch )
                          + ryki * IONCH_PUY( iipch )
                          + rzki * IONCH_PUZ( iipch );
      // Scale by change in eps over boundary.
      const double delta_eps{ IONCH_DEP( iipch ) };
      this->cnew_[ ipch ] += valency * delta_eps * rki / (epsw * 4 * core::constants::pi() * std::pow( rip, 3 ) );
    }
  }
  if ( atom.do_new )
  {
    // Add field from new position to cnew
    // (that actually means we subtract from cnew)
    for (std::size_t ipch = 0; ipch != this->size(); ++ipch )
    {
      const int iipch{ ipch };
      const double rxki = atom.new_position.x - IONCH_PRX( iipch );
      const double ryki = atom.new_position.y - IONCH_PRY( iipch );
      const double rzki = atom.new_position.z - IONCH_PRZ( iipch );
      const double rip  = std::sqrt( std::pow( rxki, 2 ) + std::pow( ryki, 2 ) + std::pow( rzki, 2 ) );
      const double rki  = rxki * IONCH_PUX( iipch )
                          + ryki * IONCH_PUY( iipch )
                          + rzki * IONCH_PUZ( iipch );
      const double delta_eps{ IONCH_DEP( iipch ) };
      this->cnew_[ ipch ] -= valency * rki * delta_eps /
                             (4 * core::constants::pi() * epsw * std::pow( rip, 3 ) );
      // Calculate and store distances between patches and particle
      this->ripnew_[ cursor ][ ipch ] = rip;
    }
  }
}
//
// Calculate the new H
//
std::copy( this->cnew_.begin(), this->cnew_.end(), this->hnew_.begin() );
IONCH_BAKSUB(this->hnew_.data());

//
// Calculate new potential
//
//   U = (1/4.pi.eps) SUM{ z_i . h_p . area_p / 2 ||r_ip|| }
//
// O(ens.size() * icc.size())
//
double old_potential = 0.0;
double new_potential = 0.0;
// cache charge density * area calculation.
std::valarray< double > surface_field_new( this->size() );
for ( std::size_t ipch = 0; ipch != this->size(); ++ipch )
{
  surface_field_new[ ipch ] = this->hnew_[ ipch ] * IONCH_AREA( ipch );
}

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
         new_potential += valency * surface_field_new[ ipch ] / ( 2 * this->rip_( idx ) );
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
old_potential = this->old_potential_;
this->new_potential_ = new_potential;
// Set potential energy on first atom in change set
changes.begin()->energy_old += old_potential;
changes.begin()->energy_new += new_potential;

}

//Calculate the total potential energy.
double icc_fortran::compute_total_potential(const particle::particle_manager & pman, const geometry::geometry_manager & gman) const
{
  return this->old_potential_;

}

// Prepare the evaluator for running a simulation loop set.
void icc_fortran::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator_manager & eman)
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
void icc_fortran::do_description(std::ostream & os) const
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
void icc_fortran::do_write_document(core::input_document & wr, std::size_t ix) const
{
wr[ ix ].add_entry( core::strngs::fsdxf(), this->get_dxf() );
wr[ ix ].add_entry( core::strngs::fsdxw(), this->get_dxw() );
wr[ ix ].add_entry( core::strngs::fsnsub(), this->get_nsub0() );
wr[ ix ].add_entry( core::strngs::fsepsp(), this->get_protein_permittivity() );
wr[ ix ].add_entry( core::strngs::fsgrl4(), this->get_protein_radius() );
wr[ ix ].add_entry( core::strngs::fsgrlc(), this->get_membrane_arc_radius() );
if( not this->get_patch_filename().empty() )
{
  wr[ ix ].add_entry( patch_file_label(), this->get_patch_filename() );
}
if( not this->get_a_matrix_filename().empty() )
{
  wr[ ix ].add_entry( amx_file_label(), this->get_a_matrix_filename() );
}

}

// Filename to save A matrix data. If empty no data is saved separately.
const std::string icc_fortran::get_a_matrix_filename() const
{
  return a_matrix_filename_;
}

// Filename to save patch data. If empty no data is saved separately.
const std::string icc_fortran::get_patch_filename() const
{
  return patch_filename_;
}

//Induced charge on a given surface element
double icc_fortran::surface_charge(std::size_t idx) const
{
  const int iidx{ idx + 1 };
  return IONCH_AREA( iidx ) * this->h_[ idx ] / IONCH_DEP( iidx ) / std::sqrt( core::constants::electron_charge() * this->alfa_ );
  

}

//Area of a given surface element
double icc_fortran::surface_area(std::size_t idx) const
{
  const int iidx{ idx + 1 };
  return IONCH_AREA( iidx );
}

// Total area of all surface elements
double icc_fortran::surface_area() const
{
  return IONCH_TOTALAREA();
}

std::string icc_fortran::type_label_()
{
  return "fpatch";

}

// Write surface tile and A matrix data to files
void icc_fortran::write_data() const
{
  // save patch data and amx
  //if( not this->patch_filename_.empty() )
  //{
  //  std::ofstream os_pch( this->patch_filename_ );
  //  this->grid_->write_grid( os_pch );
  //}
  //if( not this->a_matrix_filename_.empty() )
  //{
  //  std::ofstream os_amx( this->a_matrix_filename_ );
  //  this->amx_.write_a_matrix( os_amx );
  //}

}

//Compute the initial C and H matrices
void icc_fortran::compute_initial_c_h(const particle::particle_manager & pman)
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
        const int iipch{ ipch };
        const double rxki = pos.x - IONCH_PRX( iipch );
        const double ryki = pos.y - IONCH_PRY( iipch );
        const double rzki = pos.z - IONCH_PRZ( iipch );
        const double rip  = std::sqrt( std::pow( rxki, 2 ) + std::pow( ryki, 2 ) + std::pow( rzki, 2 ) );
        const double rki  = rxki * IONCH_PUX( iipch )
                            + ryki * IONCH_PUY( iipch )
                            + rzki * IONCH_PUZ( iipch );
        // Scale by change in eps over boundary.
        const double delta_eps{ IONCH_DEP( iipch ) };
        this->c_[ ipch ] -= valency * delta_eps * rki / (epsw * 4 * core::constants::pi() * std::pow( rip, 3 ) );
        // Calculate and store distances between patches and particle
        this->rip_( idx ) = rip;
     }
  }
  
  //
  // Calculate the new H
  //
  std::copy( this->c_.begin(), this->c_.end(), this->h_.begin() );
  IONCH_BAKSUB(this->h_.data());
  
  // cache charge density * area calculation.
  std::valarray< double > surface_field( this->size() );
  for ( std::size_t ipch = 0; ipch != this->size(); ++ipch )
  {
     const int iipch{ ipch };
     surface_field[ ipch ] = this->h_[ ipch ] * IONCH_AREA( iipch );
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
void icc_fortran::compute_total_surface_charge(double & charget, double & areat) const 
{
  areat = 0.0;
  charget = 0.0;
  for (std::size_t ipch = 0; ipch != this->size(); ++ipch)
  {
    const double area{ IONCH_AREA( ipch ) };
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
void icc_fortran::create_amx(const geometry::membrane_cylinder_region & rgn)
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
void icc_fortran::create_amx(double zl1, double rl1, double rlvest)
{
double pi_{ core::constants::pi() };
CCALLSFSUB11(__patch_MOD_MATRIX,__patch_MOD_matrix,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,INT,DOUBLE,DOUBLE,DOUBLE,DOUBLE,DOUBLE,pi_,this->dx_radial_,this->dx_zaxis_,this->epsw_,this->epspr_,this->nsub0_,zl1,rl1,this->protein_radius_,rlvest,this->membrane_arc_radius_);
int sz = IONCH_NPATCH();
this->c_.resize( sz );
this->h_.resize( sz );
this->hnew_.resize( sz );
if( not this->get_patch_filename().empty() )
{
  char * fname{ const_cast< char* >( this->get_patch_filename().c_str() ) };
  IONCH_WPATCH(fname);
}
if( not this->get_a_matrix_filename().empty() )
{
  char * fname{ const_cast< char* >( this->get_a_matrix_filename().c_str() ) };
  IONCH_WAMX(fname);
}

}

//  Called after the result of the trial is known.
void icc_fortran::on_conclude_trial(const particle::change_set & changes)
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
void icc_fortran::set_a_matrix_filename(std::string value)
{
  a_matrix_filename_ = value;

}

//  Set the alfa scaling factor:
//  alfa = k{temperature} . echg / (4 * pi * eps0 * angstrom * {permittivity} )
//
// \param temperature : in kelvin
// \param permittivity : relative permittivity.
void icc_fortran::set_alfa(double temperature)
{
  this->alfa_ = core::constants::electron_charge() / std::sqrt(4 * core::constants::pi() * core::constants::epsilon_0() * core::constants::boltzmann_constant() * temperature * core::constants::angstrom() );
  

}

// Set the patch/tile delta to divide up the circumference
// of circles normal to and centred on the Z-axis (only 
// useful before generating A matrix)
//
// \pre empty
void icc_fortran::set_dxf(double val)
{
  UTILITY_REQUIRE( this->empty(), "Cannot change integration settings after defining the geometry." );
  this->dx_radial_ = val;
}


// Set the patch/tile delta that divides lines and arcs that
// are parallel or radial to the z-axis.  (only useful before 
// generating A matrix) 
//
// \pre empty
void icc_fortran::set_dxw(double val)
{
  UTILITY_REQUIRE( this->empty(), "Cannot change integration settings after defining the geometry." );
  this->dx_zaxis_ = val;
}


// Set the radial delta 
//
// \pre empty
void icc_fortran::set_nsub0(std::size_t val)
{
  UTILITY_REQUIRE( this->empty(), "Cannot change integration settings after defining the geometry." );
  this->nsub0_ = val;
}


// Set filename to save patch information. If not set then no patch
// data is saved.
void icc_fortran::set_patch_filename(std::string value)
{
  patch_filename_ = value;

}

//  Dump data from current object
void icc_fortran::dump(std::string fname) const
{
  //std::ofstream dos( fname );
  //
  //dos << "# alfa : " << this->alfa_ << "\n";
  //dos << "# patch name : " << this->patch_filename_ << "\n";
  //dos << "# amx name : " << this->a_matrix_filename_ << "\n";
  //dos << "#  C  CNEW H HNEW RIPNEW\n";
  //for ( std::size_t ii = 0; ii != this->size(); ++ii )
  //{
  //   dos << c_[ ii ] << " " << cnew_[ ii ] << " " 
  //      << h_[ ii ] << " " << hnew_[ ii ] << " "
  //      << ripnew_[ 0 ][ ii ] << "\n";
  //}
  //
  //dos << "# RIP\n";
  //std::array< std::size_t, 2 > idx;
  //for ( std::size_t ii = 0; ii != this->rip_.shape()[0]; ++ii)
  //{
  //   idx[ 0 ] = ii;
  //   for (std::size_t ipch = 0; ipch != this->size(); ++ipch)
  //   {
  //      idx[ 1 ] = ipch;
  //      dos << ii << " " << ipch << " " << this->rip_( idx ) << "\n";
  //   }
  //}
  //dos << "# A MATRIX\n";
  //this->amx_.write_a_matrix( dos );
  //dos << "# GRID\n";
  //this->grid_->write_grid( dos );
  //

}

// Get the patch centre point and normal
void icc_fortran::get_patch_coordinate(std::size_t idx, geometry::coordinate & pos, geometry::coordinate & norm)
{
  const int iidx{ idx + 1 };
  pos.x = IONCH_PRX( iidx );
  pos.y = IONCH_PRY( iidx );
  pos.z = IONCH_PRZ( iidx );
  norm.x = IONCH_PUX( iidx );
  norm.y = IONCH_PUY( iidx );
  norm.z = IONCH_PUZ( iidx );

}

double icc_fortran::gaussbox() const
{
  return IONCH_GAUSSBOX(const_cast< double* >(this->h_.data()));
}


} // namespace evaluator
