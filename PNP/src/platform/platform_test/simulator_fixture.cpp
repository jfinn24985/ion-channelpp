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

#include "platform/platform_test/simulator_fixture.hpp"
#include "platform/platform_test/simulator_test_dummy.hpp"

namespace platform {

simulator_fixture::simulator_fixture()
: sim( new platform::simulator_test_dummy )
{
  sim->set_report_interval( 10 );
  sim->set_equilibration_interval( 10 );
  sim->set_inner_loop_size( 10 );
  sim->set_production_interval( 20 );
  sim->set_run_number( 2 );
  sim->set_target_count( 8 );
  sim->set_temperature( 300 );

}

simulator_fixture::~simulator_fixture()
{}

// Add solute specie definitions with predefined particles.
void simulator_fixture::add_solute_species()
{
  {
     particle::specie spc;
     spc.set_label( "Na" );
     spc.set_valency( 1.0 );
     spc.set_radius( 1.0 );
     spc.set_rate( 1.0 );
     spc.set_excess_potential( 0.0 );
     spc.set_concentration( 0.055 );
     spc.set_type( particle::specie::SOLUTE );
     spc.append_position( { 1.62127193214, 9.29177995257, 5.25370757073 } );
     spc.append_position( { 6.12488878987, 0.936444546808, 7.72035117147 } );
     spc.set_count( 2 );
     sim->add_specie( spc );
  }
  {
     particle::specie spc;
     spc.set_label( "Ka" );
     spc.set_valency( 1.0 );
     spc.set_radius( 1.35 );
     spc.set_rate( 1.0 );
     spc.set_excess_potential( 0.0 );
     spc.set_concentration( 0.055 );
     spc.set_type( particle::specie::SOLUTE );
     spc.append_position( { 0.629669015988, 9.4781017778, 0.436567124972 } );
     spc.append_position( { 8.8153046596, 8.25296759285, 9.77222366556 } );
     spc.set_count( 2 );
     sim->add_specie( spc );
  }
  {
     particle::specie spc;
     spc.set_label( "Cl" );
     spc.set_valency( -1.0 );
     spc.set_radius( 1.4 );
     spc.set_rate( 1.0 );
     spc.set_excess_potential( 0.0 );
     spc.set_concentration( 0.11 );
     spc.set_type( particle::specie::SOLUTE );
     spc.append_position( { 1.81235688249, 3.29878476378, 11.05346341778 } );
     spc.append_position( { 9.86409500135, 11.03444072878, 4.96346891359 } );
     spc.append_position( { 4.02705270915, 4.9878484368, 9.33967304135 } );
     spc.append_position( { 2.81632820001, 3.43431356454, 0.380214873216 } );
     spc.set_count( 4 );
     sim->add_specie( spc );
  }
  

}

// Add a localized specie (with predefined location/centroid)
void simulator_fixture::add_localized_species()
{
  particle::specie spc;
  
  spc.set_label( "G0" );
  spc.set_valency( 1.0 );
  spc.set_radius( 1.4 );
  spc.set_rate( 1.0 );
  spc.set_excess_potential( 0.0 );
  spc.set_concentration( 0.0 );
  spc.set_type( particle::specie::MOBILE );
  spc.append_position( { 6.09067367981, 5.54122699628, 4.77693360195 },
  { 1.0, 6.09067367981, 5.54122699628, 4.77693360195 } );
  spc.append_position( { 3.50504548229, 2.3265422555, 2.89757050576 },
  { 1.0, 3.50504548229, 2.3265422555, 2.89757050576 } );
  spc.set_count( 2 );
  sim->add_specie( spc );
  

}

// Add solute specie definitions with predefined particles.
void simulator_fixture::add_solute_specie_definitions()
{
  {
     particle::specie spc;
     spc.set_label( "Na" );
     spc.set_valency( 1.0 );
     spc.set_radius( 1.0 );
     spc.set_rate( 1.0 );
     spc.set_excess_potential( 0.0 );
     spc.set_concentration( 0.055 );
     spc.set_type( particle::specie::SOLUTE );
     spc.append_position( { 1.62127193214, 9.29177995257, 5.25370757073 } );
     spc.append_position( { 6.12488878987, 0.936444546808, 7.72035117147 } );
     spc.set_count( 2 );
     sim->add_specie( spc );
  }
  {
     particle::specie spc;
     spc.set_label( "Na" );
     spc.set_label( "Ka" );
     spc.set_valency( 1.0 );
     spc.set_radius( 1.35 );
     spc.set_rate( 1.0 );
     spc.set_excess_potential( 0.0 );
     spc.set_concentration( 0.055 );
     spc.set_type( particle::specie::SOLUTE );
     spc.append_position( { 0.629669015988, 9.4781017778, 0.436567124972 } );
     spc.append_position( { 8.8153046596, 8.25296759285, 9.77222366556 } );
     spc.set_count( 2 );
     sim->add_specie( spc );
  }
  {
     particle::specie spc;
     spc.set_label( "Na" );
     spc.set_label( "Cl" );
     spc.set_valency( -1.0 );
     spc.set_radius( 1.4 );
     spc.set_rate( 1.0 );
     spc.set_excess_potential( 0.0 );
     spc.set_concentration( 0.11 );
     spc.set_type( particle::specie::SOLUTE );
     spc.append_position( { 1.81235688249, 3.29878476378, 11.05346341778 } );
     spc.append_position( { 9.86409500135, 11.03444072878, 4.96346891359 } );
     spc.append_position( { 4.02705270915, 4.9878484368, 9.33967304135 } );
     spc.append_position( { 2.81632820001, 3.43431356454, 0.380214873216 } );
     spc.set_count( 4 );
     sim->add_specie( spc );
  }
  

}


} // namespace platform
