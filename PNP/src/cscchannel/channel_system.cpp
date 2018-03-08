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

#include "cscchannel/channel_system.hpp"
#include "core/input_delegater.hpp"
#include "geometry/grid.hpp"
#include "core/input_document.hpp"
#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"

// manual includes
#include "cscchannel/cell_grid.hpp"
#include "cscchannel/gc_choice_cell.hpp"
#include "cscchannel/jump_choice_cell.hpp"
#include "cscchannel/cell_simulator_meta.hpp"
//#include "cscchannel/widom.hpp"
#include "core/constants.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "evaluator/coulomb.hpp"
#include "evaluator/induced_charge.hpp"
#include "evaluator/localizer.hpp"
#include "observable/density_zaxis.hpp"
#include "observable/metropolis_sampler.hpp"
#include "observable/rdf_sampler.hpp"
#include "observable/sampler_meta.hpp"
#include "observable/specie_count.hpp"
#include "particle/specie_meta.hpp"
#include "particle/ensemble.hpp"
#include "trial/base_chooser.hpp"
#include "trial/choice_meta.hpp"
#include "trial/chooser.hpp"
#include "trial/chooser_by_simulator.hpp"
#include "trial/grid.hpp"
#include "trial/move_choice.hpp"
#include "utility/fuzzy_equals.hpp"
// -
namespace ion_channel {

channel_system::channel_system()
: simulator()
, region_names_{ "ZLIM", "FILT", "CHAN", "BULK" }
, volumes_()
, cell_hlength_()
, cell_radius_()
, membrane_arc_radius_()
, pore_hlength_()
, pore_radius_()
, protein_radius_()
, structural_hlength_()
, vestibule_arc_radius_()
, epsw_( 80.0 )
, epspr_( 10.0 )
, use_boda_bounds_( false )
{}


channel_system::~channel_system() 
{}

// Register the input file reader classes for this simulation.
//
// The base method registers a set of classes that should be
// simulation cell independent. Derived classes should therefore
// call this method as well as adding simulation cell dependent
// parts.
void channel_system::build_reader(core::input_delegater & decoder) 
{
  {
     // Add input decoder class to handle the base application options
     boost::shared_ptr< core::input_base_meta > delegate( new ion_channel::cell_simulator_meta( *this ) );
     decoder.add_input_delegate( delegate );
  }
  {
     // Add input decoder class to handle the specie definition options
     boost::shared_ptr< core::input_base_meta > delegate( new particle::specie_meta());
     decoder.add_input_delegate( delegate );
  }
  {
     // Add input decoder class to handle the energy evaluator options.  This
     // handles instantiation of the requested evaluator.
     std::unique_ptr< evaluator::evaluator_meta > tmp_( new evaluator::evaluator_meta );
     // Add the evaluator types that we want this application to handle
     tmp_->type_to_object_[ evaluator::localizer::type_label_() ] = &evaluator::localizer::make_evaluator;
     tmp_->type_to_object_[ evaluator::coulomb::type_label_() ] = &evaluator::coulomb::make_evaluator;
     tmp_->type_to_object_[ evaluator::induced_charge::type_label_() ] = &evaluator::induced_charge::make_evaluator;
     boost::shared_ptr< core::input_base_meta > delegate( tmp_.release() );
     decoder.add_input_delegate( delegate );
  }
  {
     // Add input decoder class for the generic observable types. This handles the
     // instantiation of the requested observables
     std::unique_ptr< observable::sampler_meta > tmp_( new observable::sampler_meta );
     tmp_->type_to_object_[ observable::density_zaxis::type_label_() ]= &observable::density_zaxis::make_sampler;
     tmp_->type_to_object_[ observable::metropolis_sampler::type_label_() ]= &observable::metropolis_sampler::make_sampler;
     tmp_->type_to_object_[ observable::rdf_sampler::type_label_() ]= &observable::rdf_sampler::make_sampler;
     tmp_->type_to_object_[ observable::specie_count::type_label_() ]= &observable::specie_count::make_sampler;
     // PBC specific samplers
     //tmp_->type_to_object_[ ion_channel::widom::type_label_() ]= &ion_channel::widom::make_sampler;
  
     boost::shared_ptr< core::input_base_meta > delegate( tmp_.release() );
     decoder.add_input_delegate( delegate );
  }
  // Choices
  {
     std::unique_ptr< trial::choice_meta > tmp_( new trial::choice_meta );
     tmp_->type_to_object_[ "move" ]= &trial::chooser< trial::move_choice >::make_chooser;
     tmp_->type_to_object_[ "gc" ]= &trial::chooser_by_simulator< ion_channel::gc_choice_cell, ion_channel::channel_system >::make_chooser;
     tmp_->type_to_object_[ "jump" ]= &trial::chooser_by_simulator< ion_channel::jump_choice_cell, ion_channel::channel_system >::make_chooser;
     boost::shared_ptr< core::input_base_meta > delegate( tmp_.release() );
     decoder.add_input_delegate( delegate );
  
  }

}

//Generate an initial ensemble to run a simulation.
//
//NOTE: region object that accepts 'set_volume' or 
//a region factory that takes a volume parameter 
//could be passed as an argument.
std::unique_ptr< geometry::grid_generator > channel_system::do_generate_simulation(std::ostream & os)
{
  const double sumconc = this->ionic_strength();
  
  // Calculate average solute specie raduis
  //
  // NOTE: if there are no solute species then ri.mean() will be well-defined and
  // have value 0.0
  utility::estimater ri;
  for (auto const& spc : this->get_species() )
  {
     if ( spc.is_solute() )
     {
        ri.append( spc.radius () );
     }
  }
  // Compute simulation geometry parameters or the target number of particles.
  if ( this->target_count() != 0 )
  {
     // Non-zero particle number means calculate the volume based on this number
     // and the target ionic strength.
     //
     // Total volume
     const double volume = this->target_count() * core::constants::to_SI() / sumconc;
     // subtract inner cylinder and vestibules
     double bulk_volume = volume - (2.0 * core::constants::pi()
                                    * std::pow( this->pore_radius() - ri.mean(), 2 )
                                    * this->pore_hlength())
                          // pore volume
                          - volume_of_rotation( this->pore_radius()
                                + this->vestibule_arc_radius()
                                , this->vestibule_arc_radius() + ri.mean() )
                          ; // vestibule volume
     UTILITY_INPUT(bulk_volume > 0, "Target particle number too small for the given pore dimensions and salt concentrations.", core::strngs::simulator_label(), nullptr );
  
     // For simulation with a channel we want sim box to be as long as diameter
     // (ie. zl4 - zl2 = rl5).  We subtract the channel volume when calculating
     // cell radius and length from the total volume.  We increase the radius and
     // length by the average solute specie radius because we substract the
     // specie radius when calculating the volume for each specie
     double cell_radius = std::cbrt( bulk_volume / (core::constants::pi() * 2.0) );
  
     this->set_cell_radius( cell_radius + ri.mean() );
     this->set_cell_hlength( this->pore_hlength() + this->vestibule_arc_radius()
                             + cell_radius + (2 * ri.mean()) );
     UTILITY_INPUT( (this->cell_radius() > this->protein_radius()) and (this->cell_hlength() > this->pore_hlength() + this->vestibule_arc_radius())
               , "Target particle number too small for the given protein dimensions and salt concentrations."
               , core::strngs::simulator_label(), nullptr );
  
  }
  else
  {
     // Calculate the number of particles from the volume components : pore
     // volume, vestibule volume and bulk volume
     double volume = (2.0 * core::constants::pi()
                      * std::pow( this->pore_radius() - ri.mean(), 2 )
                      * this->pore_hlength())
                     // pore volume
                     + volume_of_rotation( this->pore_radius()
                                           + this->vestibule_arc_radius()
                                           , this->vestibule_arc_radius() + ri.mean() )
                     // vestibule volume
                     + (2.0 * core::constants::pi()
                        * std::pow( this->cell_radius() - ri.mean(), 2 )
                        * (this->cell_hlength()
                           - (this->pore_hlength() + this->vestibule_arc_radius()
                              + (2 * ri.mean()))))
                     ; // bulk volume
  
  
     this->set_target_count( volume * sumconc / core::constants::to_SI() );
  
  }
  
  this->volumes_.resize( this->specie_count() );
  // Definition of accessible volume for each specie 
  for (size_t ispec = 0; ispec != this->specie_count(); ++ispec)
  {
     particle::specie const& spec( this->get_specie( ispec ) );
     const double sphere_radius( spec.radius() );
     // Total accessible volume of a specie. Three parts in the following sum are:
     //   1. Volume within cell but outside membrane.
     //   2. Volume in cylindrical part of pore.
     //   3. Volume in vestibule of pore.
     this->volumes_[ispec] = std::pow( this->cell_radius() - sphere_radius , 2 ) * core::constants::pi()
                       * (2 * (this->cell_hlength() - (2 * sphere_radius) - (this->pore_hlength() + this->vestibule_arc_radius())))
                       // bulk volume
                       + std::pow( this->pore_radius() - sphere_radius, 2 ) * core::constants::pi() * (2*this->pore_hlength())
                       // pore volume
                       + volume_of_rotation(this->pore_radius() + this->vestibule_arc_radius(), this->vestibule_arc_radius() + sphere_radius)
                       ; // vestibule volume
  }
  
  // Generate initial ensemble on a grid
  return std::unique_ptr< trial::grid_generator >{ new ion_channel::cell_grid( this->pore_hlength()  + this->vestibule_arc_radius()
           , this->cell_hlength() - (this->pore_hlength()  + this->vestibule_arc_radius())
           , this->cell_radius(), this->target_count(), this->get_random() ) };

}

//Details about the current geometry to be written to the
//log at the start of the simulation.
void channel_system::do_description(std::ostream & os) const 
{
  this->simulator::do_description( os );
  os << "[simulator] cylindrical cell with membrane\n";
  const double allvolume{ 2 * this->cell_hlength_ * core::constants::pi() * std::pow( this->cell_radius_, 2 ) };
  os << "          volume : " << allvolume << "\n";
  os << "     half length : " << this->cell_hlength_ << "\n";
  os << "     cell radius : " << this->cell_radius_ << "\n";
  os << "  membrane width : " << (2*(this->pore_hlength_ + this->vestibule_arc_radius_)) << "\n";
  os << "  pore cyl. len. : " << (2*this->pore_hlength_) << "\n";
  os << "     pore radius : " << this->pore_radius_ << "\n";
  os << "  pore entry arc : " << this->vestibule_arc_radius_ << "\n";
  os << "    permittivity : " << this->permittivity() << "\n";

}

//Implement in derived classes to write the name of 
//the program and any citation information.
void channel_system::do_license(std::ostream & os) const
{
  os << core::strngs::horizontal_bar () << "\n";
  os << "  Charge-Space Competition Monte Carlo Simulator\n\n";
  os << "  Simulation program for computing properties of\n";
  os << "  ions in model ion channel\n";

}

void channel_system::do_write_document(core::input_document & wr, std::size_t ix) const
{
wr[ix].add_entry( core::strngs::fsgzl1(), this->pore_hlength() );
wr[ix].add_entry( core::strngs::fsgzl4(), this->cell_hlength() );
wr[ix].add_entry( core::strngs::fsgrl1(), this->pore_radius() );
wr[ix].add_entry( core::strngs::fsgrl4(), this->protein_radius() );
wr[ix].add_entry( core::strngs::fsgrl5(), this->cell_radius() );
wr[ix].add_entry( core::strngs::fsgrlv(), this->vestibule_arc_radius() );
wr[ix].add_entry( core::strngs::fsgrlc(), this->membrane_arc_radius() );
wr[ix].add_entry( core::strngs::fsgzlm(), this->structural_hlength() );
wr[ix].add_entry( core::strngs::fsepsw(), this->permittivity() );
wr[ix].add_entry( core::strngs::fsepsp(), this->protein_permittivity() );
wr[ix].add_entry( core::strngs::fsnsrt(), this->use_boda_bounds() );
std::string names;
for (auto s : this->region_names_)
{
   names += s + " ";
}
names.pop_back();
wr[ix].add_entry( core::strngs::fsregn(), names);


}

//  The maximum input file version to be understood by this program
std::size_t channel_system::get_max_input_version() const
{
  return 1;
}

// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > channel_system::range_x() const
{
  return std::pair< double, double >( -this->cell_radius_, this->cell_radius_ );
}

// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > channel_system::range_y() const
{
  return std::pair< double, double >( -this->cell_radius_, this->cell_radius_ );
}
// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > channel_system::range_z() const
{
  return std::pair< double, double >( -this->cell_hlength_, this->cell_hlength_ );
}
// Calculate the volume of rotation _underneath_ an arc between z_0
// and z_1.
//
// This calculates the volume of rotation underneath an arc whose
// centrepoint is (axial_disp,radial_disp) and has the given arc_radius.
// The volume is the slice between the z_0 and z_1 .
//
// \pre radial_disp > arc_radius

double channel_system::volume_of_rotation(double axial_disp, double radial_disp, double arc_radius, double z_0, double z_1)
{
  UTILITY_REQUIRE(arc_radius < radial_disp, "The arc radius must be smaller than the radial offset.");
  UTILITY_REQUIRE(not utility::feq(z_0,z_1), "Z0 can not equal Z1.");
  UTILITY_REQUIRE(z_0 < z_1, "Z0 must be less than Z1.");
  const double z1bar_ (z_1 - axial_disp);
  const double z0bar_ (z_0 - axial_disp);
  UTILITY_REQUIRE(z0bar_ >= axial_disp - arc_radius, "Z0 must be within one arc_radius of axial_disp.");
  UTILITY_REQUIRE(z1bar_ <= axial_disp + arc_radius, "Z0 must be within one arc_radius of axial_disp.");
  
  const double arc_r2 (std::pow(arc_radius,2));
  if (utility::feq(z0bar_, 0.0))
  {
     // Handle case where x0 = xc --> z0bar_ = 0
     return core::constants::pi()
            * (
               (std::pow(radial_disp,2) + arc_r2) * (z_1 - z_0)
               - std::pow(z1bar_,3)/3.0
               - radial_disp * (z1bar_ * std::sqrt(arc_r2 - std::pow(z1bar_,2))
                                + arc_r2 * asin(z1bar_/arc_radius))
            );
  }
  else if (utility::feq(z1bar_, arc_radius))
  {
     // handle case where x1 = xc + r --> z1bar_ = r)
     return core::constants::pi()
            * (
               (std::pow(radial_disp,2) + arc_r2) * (z_1 - z_0)
               + (std::pow(z0bar_,3) - std::pow(z1bar_,3))/3.0
               - radial_disp * (arc_r2 * core::constants::pi() / 4.0)
               + radial_disp * (z0bar_ * std::sqrt(arc_r2 - std::pow(z0bar_,2))
                                + arc_r2 * std::asin(z0bar_/arc_radius))
            );
  }
  else
  {
     return core::constants::pi()
            * (
               (std::pow(radial_disp,2) + arc_r2) * (z_1 - z_0)
               + (std::pow(z0bar_,3) - std::pow(z1bar_,3))/3.0
               - radial_disp * (z1bar_ * std::sqrt(arc_r2 - std::pow(z1bar_,2))
                                + arc_r2 * std::asin(z1bar_/arc_radius))
               + radial_disp * (z0bar_ * sqrt(arc_r2 - std::pow(z0bar_,2))
                                + arc_r2 * std::asin(z0bar_/arc_radius))
            );
  }
  

}

//Compute the distances between given position and existing positions.
//
//\pre rij.size <= ens.size
//\post rij[0:startindex] === 0
//\post rij[ens.size:] undefined
void channel_system::do_compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, platform::simulator::array_type & rij, std::size_t endindex, std::size_t startindex) const
{
  const geometry::coordinate newpos(position);
  for (std::size_t j = startindex; j != endindex; ++j)
  {
     // --------------
     // Calculate r_ij
     const double dx = newpos.x - coords.x( j );
     const double dy = newpos.y - coords.y( j );
     const double dz = newpos.z - coords.z( j );
     rij[j] = std::sqrt(dx*dx + dy*dy + dz*dz);
  }

}

//Compute distance between two coordinates.
double channel_system::distance_sqr(const geometry::coordinate & lhs, const geometry::coordinate & rhs) const
{
  const double dx = lhs.x - rhs.x;
  const double dy = lhs.y - rhs.y;
  const double dz = lhs.z - rhs.z;
  return std::sqrt(dx*dx + dy*dy + dz*dz);
}

//Is a particle at the given position and radius in a valid position
//in the system?
bool channel_system::is_valid_position(geometry::coordinate & coord, std::size_t ispec) const 
{
  // get sphere radius
  const double radius{ this->get_specie( ispec ).radius() };
  // Absolute Z position
  const double drznew{ std::abs(coord.z) };
  // Radial position
  const double r2inew{ std::sqrt(coord.x * coord.x + coord.y * coord.y) };
  // pore extreme in radial direction
  const double rl2{ this->pore_radius_ + this->vestibule_arc_radius_ };
  
  // Outside of membrane, check if outside extreme of bulk in r and z?
  if (drznew >= (this->pore_hlength_ + this->vestibule_arc_radius_ + radius))
  {
    return not (r2inew > (this->cell_radius_  - radius)
           or drznew > (this->cell_hlength_ - radius));
  }
  
  // (else inside membrane and) Within channel cylinder inner-radius?
  if (r2inew <= (this->pore_radius_ - radius))
  {
    return true;
  }
  
  // Inside channel (and outside inner cylinder radius) OR Outside maximum
  // vestibule radius means overlap
  if (drznew < this->pore_hlength_ or r2inew > rl2)
  {
    return false;
  }
  
  // Check (sqr) distance from vestibule arc centerpoint.
  const double r2minimum{ std::pow( this->vestibule_arc_radius_ + radius, 2 ) };
  const double r2actual{ std::pow( rl2 - r2inew, 2 )
                   + std::pow( drznew - this->pore_hlength_, 2 ) };
  return (r2actual >= r2minimum);

}

// Generate a cylinder object containing the allowable sphere
// centrepoint region for a given specie.
geometry::cylinder_region channel_system::region_geometry(std::size_t subregion, std::size_t ispec) const
{
  auto const& spec = this->get_specie( ispec );
  const double sphere_radius{ spec.radius() };
  
  double hlength{};
  double radius{};
  
  switch( subregion )
  {
  case IBULK: // all of sim
  {
     radius = this->cell_radius_ - sphere_radius;
     hlength = this->cell_hlength_ - sphere_radius;
  }
  break;
  
  case ICHAN:
  {
     if (this->use_boda_bounds_)
     {
        // Total channel insertion
        hlength = this->pore_hlength_ + this->vestibule_arc_radius_;
     }
     else
     {
        // Total channel insertion region (any part of particle in channel)
        hlength = this->pore_hlength_ + this->vestibule_arc_radius_ + sphere_radius;
     }
     radius = this->pore_radius_ + this->vestibule_arc_radius_;
  
  }
  break;
  
  case IFILT:
  {
     if ( this->use_boda_bounds_ )
     {
        // Centre of particle in filter
        hlength = this->pore_hlength_;
     }
     else
     {
        // Any of particle in filter
        hlength = this->pore_hlength_ + sphere_radius;
     }
     radius = this->pore_radius_ - sphere_radius;
  }
  break;
  
  case IZLIM:
  {
     if ( this->use_boda_bounds_ and not ( spec.is_mobile() or spec.is_channel_only() ) )
     {
        // Centre of particle in zlimit
        hlength = this->structural_hlength_;
     }
     else
     {
        // All of particle inside zlimit
        hlength = this->structural_hlength_ - sphere_radius;
     }
     radius = this->pore_radius_ - sphere_radius;
  }
  break;
  
  default:
  {
     const bool invalid_region_index{ false };
     UTILITY_REQUIRE( invalid_region_index, "Region index out of range." );
  }
  break;
  }
  
  return cylinder( radius, hlength );
  

}


} // namespace ion_channel

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(ion_channel::channel_system, "ion_channel::channel_system");

// Template classes defined in build_reader
#include <boost/preprocessor/punctuation/comma.hpp>
BOOST_CLASS_EXPORT_GUID(trial::chooser< trial::move_choice >, "trial::chooser< trial::move_choice >");
BOOST_CLASS_EXPORT_KEY2(trial::chooser_by_simulator< ion_channel::gc_choice_cell BOOST_PP_COMMA() ion_channel::channel_system >, "trial::chooser_by_simulator< ion_channel::gc_choice_cell, ion_channel::channel_system >");
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser_by_simulator< ion_channel::gc_choice_cell BOOST_PP_COMMA() ion_channel::channel_system >);
BOOST_CLASS_EXPORT_KEY2(trial::chooser_by_simulator< ion_channel::jump_choice_cell BOOST_PP_COMMA()  ion_channel::channel_system >, "trial::chooser_by_simulator< ion_channel::jump_choice_cell, ion_channel::channel_system >");
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser_by_simulator< ion_channel::jump_choice_cell BOOST_PP_COMMA()  ion_channel::channel_system >);