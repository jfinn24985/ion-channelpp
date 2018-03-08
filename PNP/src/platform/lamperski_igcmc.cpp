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

#include "platform/lamperski_igcmc.hpp"
#include "core/constants.hpp"
#include "platform/specie_monitor.hpp"
#include "platform/simulation.hpp"
#include "utility/archive.hpp"

#include "core/input_document.hpp"
#include "platform/simulator_meta.hpp"
#include "platform/imc_update.hpp"
#include "core/input_parameter_memo.hpp"

//manual includes
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "geometry/geometry_manager.hpp"
#include "observable/report_manager.hpp"
#include "particle/particle_manager.hpp"
#include "utility/estimate_array.hpp"
#include "utility/fuzzy_equals.hpp"
//-
namespace platform {

//Log message descibing the observable and its parameters
void lamperski_igcmc::description(std::ostream & os) const 
{
  os << " Update Method [" << this->type_label_() << "]\n";
  os << " Estimate the excess chemical potential parameters required\n";
  os << " to maintain the target concentrations of solute species.\n\n";
  os << "   S. Lamperski, Mol. Simul. 33, 1193, 2007\n\n";

}

// Update the chemical potentials (in derived classes)
void lamperski_igcmc::do_update(simulation & sys, std::ostream & oslog)
{
  // Lamperski:
  //  Add or subtract "delta" depending on whether average
  // density is greater or less than desired
  //
  // Use mean concentration from specie_count sampler.
  //
  // NOTE: we could use specie_count::type_label_()
  auto & pman = sys.particles();
  
  for (size_t ispec = 0; ispec != pman.specie_count(); ++ispec)
  {
    particle::specie & spcobj = pman.get_specie(ispec);
    // only look at solute species
    if (spcobj.is_solute())
    {
      const std::size_t localidx{ this->counts().local_index( ispec ) };
      if( localidx != this->counts().size() )
      {
        // NOTE: localidx = this->counts().size() if specie not recorded.
        const double volume{ this->counts().volume( localidx ) };
        if ( volume != 0.0 )
        {
          double actual_concentration = this->counts().mean( localidx )*core::constants::to_SI()/volume;
          if (not utility::feq(actual_concentration,0.0))
          {
            const double change = (this->use_random_ ? this->delta_ * sys.get_random().uniform(): this->delta_ );
            const double chempi = spcobj.excess_potential() + (spcobj.concentration() > actual_concentration? change : -change);
            spcobj.set_excess_potential( chempi );
          }
        }
      }
    }
  }

}

//Prepare the evaluator for use with the given simulator and
//stepper.
void lamperski_igcmc::do_prepare(simulation & sim) 
{}

std::string lamperski_igcmc::type_label() const 
{
  return type_label_();
}

std::string lamperski_igcmc::type_label_()

{
  return std::string("lamperski");
}

lamperski_igcmc::lamperski_igcmc() 
: chemical_potential_update()
, delta_(0.1)
, use_random_(false)
{}

lamperski_igcmc::~lamperski_igcmc() = default;
//Add type and delta information to an input file section.
//
//only throw possible should be from os.write() operation
void lamperski_igcmc::do_write_part_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( "delta", this->delta_ );
  wr[ ix ].add_entry( "use-random", this->use_random_ );

}

// Add updater specific options to the definition object and
// return the "updater" type label for this class.
std::string lamperski_igcmc::add_to_definition(simulation_definition & defn)
{
  defn.add_definition( { "delta", "number", ">0", "0.1", "(lamperski update) The magnitude of the change in chemical potential to use in updates." } );
  defn.add_definition( { "use-random", "boolean", "", "false", "(lamperski update) Scale the magnitude of the change in chemical potential by a random factor during updates.." } );
  return type_label_();

}

boost::shared_ptr< imc_update > lamperski_igcmc::make_updater(const std::vector< core::input_parameter_memo >& params)
{
  std::unique_ptr< lamperski_igcmc > smplr(new lamperski_igcmc);
  // Check parameters for usable values
  for (auto const& item : params)
  {
    if( item.name() == "delta" )
    {
      smplr->delta_ = item.get_float( "Lamperski IMC", core::strngs::simulator_label(), true, false );
    }
    else if (item.name() == "use-random")
    {
      smplr->use_random_ = item.get_bool( "Lamperski IMC", core::strngs::simulator_label(), true, false );
    }
    else if( core::strngs::fsend() != item.name() )
    {
      throw core::input_error::invalid_parameter_subtype_error( core::strngs::simulator_label(), type_label_(), item );
    }
  }
  return boost::shared_ptr< imc_update >( smplr.release() );

}


} // namespace platform

BOOST_CLASS_EXPORT_IMPLEMENT( platform::lamperski_igcmc );
