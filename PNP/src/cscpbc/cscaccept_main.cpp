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


#include "platform/simulator.hpp"
#include "particle/specie.hpp"
#include "observable/accept_igcmc.hpp"
#include "observable/specie_count.hpp"
#include "observable/metropolis_sampler.hpp"
#include "observable/rdf_sampler.hpp"
#include "observable/widom.hpp"
#include "core/strngs.hpp"
#include "evaluator/coulomb.hpp"
#include <iostream>

#ifndef DEBUG
#define DEBUG 0
#endif


int main(int argc, char** argv)
{
  // stepwise test of cmc
  platform::simulator sim;
  sim.set_delta( 3.0 );
  sim.set_equilibration_interval( 10 );
  sim.set_production_interval( 100 );
  sim.set_inner_loop_size( 1000 ); // #X 0
  sim.set_target_particles( 50 );

  {
    particle::specie spc1;
    spc1.set_label( "Na" );
    spc1.set_radius( 1.9/2 );
    spc1.set_valency( 1.0 );
    spc1.set_excess_potential( 0.0 );
    spc1.set_concentration( 0.1 );
    spc1.set_type ( particle::specie::SOLUTE );
    sim.add_specie(spc1);
  }
  {
    particle::specie spc1;
    spc1.set_label( "Cl" );
    spc1.set_radius( 3.62/2 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.0 );
    spc1.set_concentration( 0.2 );
    spc1.set_type ( particle::specie::SOLUTE );
    sim.add_specie(spc1);
  }
  {
    particle::specie spc1;
    spc1.set_label( "Ca" );
    spc1.set_radius( 1.98/2 );
    spc1.set_valency( 2.0 );
    spc1.set_excess_potential( 0.0 );
    spc1.set_concentration( 0.05 );
    spc1.set_type ( particle::specie::SOLUTE );
    sim.add_specie(spc1);
  }
  observable::igcmc *imc;
  {
    std::map< std::string, std::string > params;
    params["nstep"] = "50";
    imc = observable::accept_igcmc::make_sampler( params, sim );
  }
  {
    std::map< std::string, std::string > params;
    observable::specie_count::make_sampler( params, sim );
    observable::metropolis_sampler::make_sampler( params, sim );
    observable::rdf_sampler::make_sampler( params, sim );
  }
  {
    std::map< std::string, std::string > params;
    params[core::strngs::fsiwid()] = "50";
    params[core::strngs::fswidm()] = "true";
    observable::widom::make_sampler( params, sim );
  }
  {
    std::map< std::string, std::string > params;
    evaluator::coulomb::make_evaluator( params, sim );
  }
  // build initial system
  sim.initialize();
  sim.do_enrol();
  sim.description( std::cout );
  // Run IGCMC simulation
  for (size_t scalar = 5; scalar != 11; ++scalar )
  {
    // Start out with low GC rate.
    sim.set_trial_rates( { 1., 1., scalar/10.0 } );
    sim.run( std::cout );
    imc->reset_count();
  }
  sim.description( std::cout );
}
