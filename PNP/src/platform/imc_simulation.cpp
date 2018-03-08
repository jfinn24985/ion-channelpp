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

#include "platform/imc_simulation.hpp"
#include "platform/lamperski_igcmc.hpp"
#include "platform/malasics_igcmc.hpp"
#include "platform/imc_update.hpp"
#include "platform/simulator_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "platform/simulation.hpp"
#include "utility/archive.hpp"

#include "core/input_document.hpp"

// manuals
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
//-
namespace platform {

void imc_simulation::add_definition(simulator_meta & meta)
{
  std::string desc( "Iterative Monte Carlo simulation. This runs a single equilibration phase then multiple production phases." );
  std::unique_ptr< simulation_definition > result( new simulation_definition( type_label_(), desc, &imc_simulation::make_simulation_manager ) );
  // extra parameters
  std::stringstream labels;
  labels << "(";
  labels <<  malasics_igcmc::add_to_definition( *result );
  labels << "|";
  labels << lamperski_igcmc::add_to_definition( *result );
  labels << ")";
  result->add_definition( { "update", "label", labels.str(), "None", "The type of iterative simulation to perform." } );
  result->add_definition( { "niter", "number", ">1", "100", "The number of iterations to perform in a iterative simulation." } );
  
  meta.add_type( result );

}

// Log message descibing the simulation and its parameters
void imc_simulation::do_description(std::ostream & os) const 
{
  os << " Iterative Monte Carlo Simulation.\n"; 
  os << " ---------------------------------\n"; 
  this->updater_->description( os );
  os << "    number of iterations : " << this->super_loop_size_ << "\n"; 
  os << "     current iterations* : " << this->super_loop_count_ + 1; 
  os << " (*will be 1 except after restart.)\n";

}

imc_simulation::imc_simulation(boost::shared_ptr< imc_update > up)
: updater_( up )
, super_loop_size_( 100 )
, super_loop_count_( 0 )
{}

// For serialization.
imc_simulation::imc_simulation()
: updater_()
, super_loop_size_(100)
, super_loop_count_(0)
{}

imc_simulation::~imc_simulation() = default;
//Implement in derived classes to write the name of 
//the program and any citation information.
void imc_simulation::license(std::ostream & os) const
{}

boost::shared_ptr< simulation_manager > imc_simulation::make_simulation_manager(const std::vector< core::input_parameter_memo >& params)
{
  std::unique_ptr< imc_simulation > result{ new imc_simulation };
  // make copy of params so we can remove used parameters
  std::vector< core::input_parameter_memo > lparams{ params };
  // -------------------
  // Find number of iterations (optional)
  // -------------------
  {
    const std::string number_of_iterations_label{ "niter" };
    for( std::size_t niter_idx = 0; niter_idx != lparams.size(); ++niter_idx )
    {
      if( number_of_iterations_label == lparams[ niter_idx ].name() )
      {
        std::size_t niter = lparams[ niter_idx ].get_ordinal( "Iterative Monte Carlo", core::strngs::simulator_label() );
        if( niter <= 1 )
        {
          throw core::input_error::parameter_value_error( "Iterative Monte Carlo", core::strngs::simulator_label(), lparams[ niter_idx ], "Iterative procedure requires more than one iteration." );
        }
        result->set_loop_size( niter );
        auto iter = lparams.begin();
        std::advance( iter, niter_idx );
        lparams.erase( iter );
        break;
      }
    }
  }
  // -------------------
  // Set updater type (required)
  // -------------------
  {
    const std::string updater_label{ "update" };
    std::size_t updater_idx{};
    for( updater_idx = 0; updater_idx != lparams.size(); ++updater_idx )
    {
      if( updater_label == lparams[ updater_idx ].name() )
      {
        const std::vector< std::string > keys{ malasics_igcmc::type_label_(), lamperski_igcmc::type_label_() };
        const std::size_t idx = lparams[ updater_idx ].get_key( "Iterative Monte Carlo", core::strngs::simulator_label(), keys );
        auto iter = lparams.begin();
        std::advance( iter, updater_idx );
        lparams.erase( iter );
        switch( idx )
        {
        case 0:
          result->updater_ = malasics_igcmc::make_updater( lparams );
          break;
        case 1:
          result->updater_ = lamperski_igcmc::make_updater( lparams );
          break;
        }
        break;
      }
    }
    if( updater_idx == lparams.size() )
    {
      throw core::input_error::missing_parameters_error( "Simulation configuration", core::strngs::simulator_label(), updater_label, lparams.back().filename(), lparams.back().line_number() );
    }
  }
  return boost::shared_ptr< simulation_manager >( result.release() );

}

//Update the chemical potentials.
//
//* Set do_repeat to true if loop count less than loop size.
//* Update chemical potential by calling (defined in derived classes)
//do_on_super_loop.
//* Report chemical potentials.
bool imc_simulation::run(simulation & sim, std::ostream & oslog) 
{
  // do thermalization
  oslog << "\n -----------------------";
  oslog << "\n Start of Thermalisation";
  oslog << "\n -----------------------\n\n";
  sim.prepare();
  sim.run_loop( this->equilibration_interval() );
  oslog << "\n ---------------------";
  oslog << "\n End of Thermalisation";
  oslog << "\n ---------------------\n\n";
  
  this->updater_->prepare( sim );
  // Perform iterations on thermalised simulation.
  for( ; this->super_loop_count_ < this->super_loop_size_; ++this->super_loop_count_)
  {
    // do main simulation
    oslog << "\n ---------------------------";
    oslog << "\n Start of Iteration [" << (this->super_loop_count_ + 1) << "]\n";
    oslog << "\n ---------------------------\n\n";
    sim.prepare();
    sim.run_loop( this->production_interval() );
    oslog << "\n -------------------------";
    oslog << "\n Update [" << (this->super_loop_count_ + 1) << "]\n";
    oslog << "\n -------------------------\n\n";
  
    // Perform iteration update
    this->updater_->update( sim, oslog );
    oslog << "\n -------------------------";
    oslog << "\n End of Iteration [" << (this->super_loop_count_ + 1) << "]\n";
    oslog << "\n -------------------------\n\n";
  
  }
  
  return true;

}

// Label that identifies this simulation manager subtype in
// the input file (virtual access).
std::string imc_simulation::type_label_()
{
  const std::string label( "imc" );
  return label;
}

// Write extra data of derived simulator object into the input
// document object at the given section.
void imc_simulation::do_write_part_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label() );
  wr[ ix ].add_entry( core::strngs::fsnstp(), this->super_loop_size_);
  this->updater_->do_write_part_document( wr, ix );

}


} // namespace platform

BOOST_CLASS_EXPORT_IMPLEMENT( platform::imc_simulation );