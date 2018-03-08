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

#include "cscpbc/periodic_system.hpp"
#include "core/strngs.hpp"
#include "platform/imc_simulation.hpp"
#include "utility/archive.hpp"

#include "core/input_delegater.hpp"
#include "core/input_document.hpp"
#include "geometry/grid.hpp"
#include "geometry/coordinate.hpp"
#include "geometry/coordinate_set.hpp"

// manual includes
#include "cscpbc/accept_igcmc.hpp"
#include "cscpbc/alternater_chooser.hpp"
#include "cscpbc/bulk_simulator_meta.hpp"
#include "cscpbc/gc_choice_pbc.hpp"
#include "cscpbc/imc_meta.hpp"
#include "cscpbc/jump_choice_pbc.hpp"
#include "cscpbc/lamperski_igcmc.hpp"
#include "cscpbc/malasics_igcmc.hpp"
#include "cscpbc/widom.hpp"
#include "core/constants.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "evaluator/coulomb.hpp"
#include "observable/density_zaxis.hpp"
#include "observable/metropolis_sampler.hpp"
#include "observable/acceptance_observable.hpp"
#include "observable/rdf_sampler.hpp"
#include "observable/sampler_meta.hpp"
#include "observable/specie_count.hpp"
#include "particle/specie_meta.hpp"
#include "particle/ensemble.hpp"
#include "trial/base_chooser.hpp"
#include "trial/choice_meta.hpp"
#include "trial/chooser.hpp"
#include "trial/grid.hpp"
#include "trial/move_choice.hpp"
// -
namespace periodic_cube {

periodic_system::~periodic_system() = default;
// Register the input file reader classes for this simulation.
//
// The base method registers a set of classes that should be
// simulation cell independent. Derived classes should therefore
// call this method as well as adding simulation cell dependent
// parts.
void periodic_system::build_reader(core::input_delegater & decoder) 
{
  {
     // Add input decoder class to handle the base application options
     boost::shared_ptr< core::input_base_meta > delegate( new periodic_cube::bulk_simulator_meta( *this ) );
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
     tmp_->type_to_object_[ evaluator::coulomb::type_label_() ] = &evaluator::coulomb::make_evaluator;
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
     tmp_->type_to_object_[ observable::acceptance_observable::type_label_() ]= &observable::acceptance_observable::make_sampler;
     // PBC specific samplers
     tmp_->type_to_object_[ periodic_cube::widom::type_label_() ]= &periodic_cube::widom::make_sampler;
  
     boost::shared_ptr< core::input_base_meta > delegate( tmp_.release() );
     decoder.add_input_delegate( delegate );
  }
  // PBC super loopers
  {
     std::unique_ptr< periodic_cube::imc_meta > tmp_( new periodic_cube::imc_meta );
     tmp_->type_to_object_[ periodic_cube::malasics_igcmc::type_label_() ]= &periodic_cube::malasics_igcmc::make_super_looper;
     tmp_->type_to_object_[ periodic_cube::lamperski_igcmc::type_label_() ]= &periodic_cube::lamperski_igcmc::make_super_looper;
     tmp_->type_to_object_[ periodic_cube::accept_igcmc::type_label_() ]= &periodic_cube::accept_igcmc::make_super_looper;
     boost::shared_ptr< core::input_base_meta > delegate( tmp_.release() );
     decoder.add_input_delegate( delegate );
  }
  // Choices
  {
     std::unique_ptr< trial::choice_meta > tmp_( new trial::choice_meta );
     tmp_->type_to_object_[ "move" ]= &trial::chooser< trial::move_choice >::make_chooser;
     tmp_->type_to_object_[ "gc" ]= &trial::chooser< periodic_cube::gc_choice_pbc >::make_chooser;
     tmp_->type_to_object_[ "accept" ]= &periodic_cube::alternater_chooser::make_chooser;
     tmp_->type_to_object_[ "jump" ]= &trial::chooser< periodic_cube::jump_choice_pbc >::make_chooser;
     boost::shared_ptr< core::input_base_meta > delegate( tmp_.release() );
     decoder.add_input_delegate( delegate );
  
  }

}

// (how to output IMC)
// Details about the current geometry to be written to the
// log at the start of the simulation.
void periodic_system::do_description(std::ostream & os) const 
{
  this->simulator::do_description( os );
  os << "[simulator] periodic cube\n";
  os << "       volume : " << this->volume(0) << "\n";
  os << "  cube length : " << this->length_ << "\n";
  os << " permittivity : " << this->permittivity() << "\n";
  if ( this->imc_ )
  {
     this->imc_->description( os );
  }

}

//Implement in derived classes to write the name of 
//the program and any citation information.
void periodic_system::do_license(std::ostream & os) const
{
  os << core::strngs::horizontal_bar () << "\n";
  os << "  Charge-Space Competition Monte Carlo Simulator\n\n";
  os << "  Simulation program for computing properties of\n";
  os << "  ions in bulk/periodic cube\n";

}

// (todo: how to output IMC)
// Write an input file section.
//
// only throw possible should be from os.write() operation
void periodic_system::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fsepsw(), this->permittivity() );
  if ( this->imc_ )
  {
     this->imc_->write_document( wr );
  }

}

//Generate an initial ensemble to run a simulation.
//
//NOTE: region object that accepts 'set_volume' or 
//a region factory that takes a volume parameter 
//could be passed as an argument.
std::unique_ptr< geometry::grid_generator > periodic_system::do_generate_simulation(std::ostream & os)
{
  // calculate dimensions
  const double sumconc = this->ionic_strength();
  this->length_ = std::cbrt( ( this->target_count() * core::constants::to_SI() ) / sumconc );
  // Generate initial ensemble on a grid
  return std::unique_ptr< trial::grid_generator >{ new trial::cubic_grid( this->length_, this->target_count(), this->get_random() ) };

}

// Derived class implementation of run method.
void periodic_system::do_run(std::ostream & oslog)
{
  // do thermalization
  this->run_loop(this->equilibration_interval(), oslog);
  oslog << "\nEND OF THERMALISATION\n\n";
  // do main simulation
  if ( this->imc_ )
  {
    while (not this->imc_->at_end())
    {
      this->imc_->prepare( *this );
      this->run_loop(this->production_interval(), oslog);
      oslog << "END OF PRODUCTION PHASE " << (this->imc_->count() + 1) << "\n";
      this->imc_->on_super_loop (*this, oslog);
    }
  }
  else
  {
    this->run_loop(this->production_interval(), oslog);
  }
  oslog << "\nEND OF PRODUCTION\n\n";

}

//  The maximum input file version to be understood by this program
std::size_t periodic_system::get_max_input_version() const
{
  return 1;
}

//Is a particle at the given position and radius in a valid position
//in the system?
//
//For PBC system, the coordinate is adjusted within the cube and true
//always returned.
bool periodic_system::is_valid_position(geometry::coordinate & coord, std::size_t ispec) const 
{
  const double box (this->length_);
  coord.x = (coord.x < 0 ? coord.x + box : (coord.x < box ? coord.x : coord.x - box));
  coord.y = (coord.y < 0 ? coord.y + box : (coord.y < box ? coord.y : coord.y - box));
  coord.z = (coord.z < 0 ? coord.z + box : (coord.z < box ? coord.z : coord.z - box));
  return true;

}

// Compute square of distance between two coordinates in 
// a periodic box.
double periodic_system::distance_sqr(const geometry::coordinate & lhs, const geometry::coordinate & rhs) const 
{
  const double box (this->length_);
  const double hbox (box/2);
  double dx = lhs.x - rhs.x;
  double dy = lhs.y - rhs.y;
  double dz = lhs.z - rhs.z;
  dx = (dx > hbox ? dx - box : (dx < -hbox ? dx + box : dx));
  dy = (dy > hbox ? dy - box : (dy < -hbox ? dy + box : dy));
  dz = (dz > hbox ? dz - box : (dz < -hbox ? dz + box : dz));
  return dx*dx + dy*dy + dz*dz;

}

//Compute the distances between any new position and existing positions.
//
//\post rij.size == ens.size
//\post rij[0:start_index] == 0.0

void periodic_system::do_compute_distances(const geometry::coordinate & position, const geometry::coordinate_set & coords, platform::simulator::array_type & rij, std::size_t endindex, std::size_t startindex) const
{
  const geometry::coordinate newpos(position);
  const double box = this->length_;
  const double hbox = box/2;
  for (std::size_t j = startindex; j != endindex; ++j)
  {
     // --------------
     // Calculate r_ij
     double dx = newpos.x - coords.x( j );
     double dy = newpos.y - coords.y( j );
     double dz = newpos.z - coords.z( j );
     dx += (dx > hbox ? -box : ( dx < -hbox ? box : 0.0));
     dy += (dy > hbox ? -box : ( dy < -hbox ? box : 0.0));
     dz += (dz > hbox ? -box : ( dz < -hbox ? box : 0.0));
     rij[j] = std::sqrt(dx*dx + dy*dy + dz*dz);
  }

}

// Return the current concentration of the given specie.
//
// For a periodic system, the volume is independent of specie, so ispec
// is always ignored (ie a valid ispec is not checked)
double periodic_system::volume(std::size_t ispec) const
{
  return std::pow( this->length_, 3 );
}

// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > periodic_system::range_x() const
{
  return std::pair< double, double >( 0.0, this->length_ );
}

// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > periodic_system::range_y() const
{
  return std::pair< double, double >( 0.0, this->length_ );
}
// Get the minimum and maximum dimension along the z coordinate
std::pair< double, double > periodic_system::range_z() const
{
  return std::pair< double, double >( 0.0, this->length_ );
}

} // namespace periodic_cube

BOOST_CLASS_EXPORT_IMPLEMENT(periodic_cube::periodic_system);

// Need to register the template types derived from base_chooser
BOOST_CLASS_EXPORT_GUID(trial::chooser< trial::move_choice >, "trial::chooser< trial::move_choice >");
BOOST_CLASS_EXPORT_GUID(trial::chooser< periodic_cube::gc_choice_pbc >, "trial::chooser< periodic_cube::gc_choice_pbc >");
BOOST_CLASS_EXPORT_GUID(trial::chooser< periodic_cube::jump_choice_pbc >, "trial::chooser< periodic_cube::jump_choice_pbc >");

