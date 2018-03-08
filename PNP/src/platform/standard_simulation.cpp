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

#include "platform/standard_simulation.hpp"
#include "core/input_document.hpp"
#include "platform/simulator_meta.hpp"
#include "platform/simulation.hpp"
#include "core/input_parameter_memo.hpp"

// manual includes
#include "core/strngs.hpp"
#include "utility/utility.hpp"
// -
// -
namespace platform {

standard_simulation::~standard_simulation()
{}

// Details about the current storage object.
void standard_simulation::do_description(std::ostream & os) const
{
  os << " Simple Monte Carlo Simulation.\n"; 
  os << " ------------------------------\n"; 
  

}

//Implement in derived classes to write the name of 
//the program and any citation information.
void standard_simulation::license(std::ostream & os) const
{}

// Label that identifies this simulation manager subtype in
// the input file (virtual access).
std::string standard_simulation::type_label() const
{
  return standard_simulation::type_label_();
}

// Label that identifies this simulation manager subtype in
// the input file.
std::string standard_simulation::type_label_()
{
  std::string result{ "standard" };
  return result;
}

// Write extra data of derived simulator object into the input
// document object at the given section.
void standard_simulation::do_write_part_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label() );

}

void standard_simulation::add_definition(simulator_meta & meta)
{
  std::string desc( "Standard Monte Carlo simulation. This runs a single equilibration phase then a single production phase." );
  std::unique_ptr< simulation_definition > result( new simulation_definition( type_label_(), desc, &standard_simulation::make_simulation_manager ) );
  // no extra parameters
  meta.add_type( result );

}

// Perform the simulation. This class does:
//
// sim.prepare + sim.run_loop(equil_int)
// sim.prepare + sim.run_loop(prod_int)

bool standard_simulation::run(simulation & sim, std::ostream & oslog)
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
  // do main simulation
  oslog << "\n -------------------";
  oslog << "\n Start of Production";
  oslog << "\n -------------------\n\n";
  sim.prepare();
  sim.run_loop( this->production_interval() );
  oslog << "\n -----------------";
  oslog << "\n End of Production";
  oslog << "\n -----------------\n\n";
  return true;
  

}

boost::shared_ptr< simulation_manager > standard_simulation::make_simulation_manager(const std::vector< core::input_parameter_memo >& params)
{
  UTILITY_CHECK( params.size() == 1, "Standard simulation manager requires no parameters." );
  return std::unique_ptr< simulation_manager >{ new standard_simulation };

}


} // namespace platform

BOOST_CLASS_EXPORT_IMPLEMENT( platform::standard_simulation );