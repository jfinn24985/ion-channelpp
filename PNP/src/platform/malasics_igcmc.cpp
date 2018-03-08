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

#include "platform/malasics_igcmc.hpp"
#include "core/constants.hpp"
#include "platform/specie_monitor.hpp"
#include "core/strngs.hpp"
#include "platform/simulation.hpp"
#include "utility/archive.hpp"

#include "core/input_document.hpp"
#include "platform/simulator_meta.hpp"
#include "platform/imc_update.hpp"
#include "core/input_parameter_memo.hpp"

// Manuals
#include "core/input_error.hpp"
#include "geometry/geometry_manager.hpp"
#include "observable/report_manager.hpp"
#include "particle/particle_manager.hpp"
#include "utility/fuzzy_equals.hpp"
#include "utility/estimate_array.hpp"
// -
namespace platform {

//Log message descibing the observable and its parameters
void malasics_igcmc::description(std::ostream & os) const 
{
  os << " Update Method [" << this->type_label_() << "]\n";
  os << " Estimate the excess chemical potential parameters required\n";
  os << " to maintain the target concentrations of solute species.\n\n";
  os << "   Attila Malasics, Dirk Gillespie and Dezso¨ Boda \"Simulating\n";
  os << "   prescribed particle densities in the grand canonical\n";
  os << "   ensemble using iterative algorithms\", The Journal of Chemical\n";
  os << "   Physics, 2008, 128, 124102\n\n";

}

//Update the chemical potentials (in derived classes)
void malasics_igcmc::do_update(simulation & sys, std::ostream & oslog)
{
  // Malasics
  //   Update with  mu_ex = (mu_ideal - mu_ex) - log(<dens>)
  //
  // Get mean concentration from specie_count sampler.
  //
  // NOTE: we could use specie_count::type_label_()
  particle::particle_manager& pman = sys.particles();
  
  for (size_t ispec = 0; ispec != pman.specie_count(); ++ispec)
  {
    particle::specie & spcobj = pman.get_specie(ispec);
    // only solute species have interesting chemical potentials
    if (spcobj.is_solute())
    {
      const std::size_t localidx{ this->counts().local_index( ispec ) };
      if( localidx != this->counts().size() )
      {
        // NOTE: localidx = this->counts().size() if specie not recorded.
        const double volume{ this->counts().volume( localidx ) };
        if ( volume != 0.0 )
        {
          const double bulk_density = this->counts().mean( localidx )/volume;
          if (not utility::feq(bulk_density,0.0))
          {
            spcobj.set_excess_potential(spcobj.chemical_potential() - std::log(bulk_density));
          }
        }
      }
    }
  }

}

malasics_igcmc::~malasics_igcmc() = default;

//Prepare the evaluator for use with the given simulator and
//stepper.
void malasics_igcmc::do_prepare(simulation & sim) 
{}

std::string malasics_igcmc::type_label() const 
{
  return type_label_();
}

std::string malasics_igcmc::type_label_()

{
  return std::string("malasics");
}

//Write an input file section.
//
//only throw possible should be from os.write() operation
void malasics_igcmc::do_write_part_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( "update", this->type_label_() );

}

// Add updater specific options to the definition object and
// return the "updater" type label for this class.
std::string malasics_igcmc::add_to_definition(simulation_definition & defn)
{
  return type_label_();

}

boost::shared_ptr< imc_update > malasics_igcmc::make_updater(const std::vector< core::input_parameter_memo >& params)
{
  if( not params.size() == 1 )
  {
    for (auto & item : params)
    {
      if( core::strngs::fsend() != item.name() )
      {
        throw core::input_error::invalid_parameter_subtype_error( core::strngs::simulator_label(), type_label_(), item );
      }
    }
  }
  return boost::shared_ptr< imc_update > { new malasics_igcmc };

}


} // namespace platform

BOOST_CLASS_EXPORT_IMPLEMENT( platform::malasics_igcmc );
