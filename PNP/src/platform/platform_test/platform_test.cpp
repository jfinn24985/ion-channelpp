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
#define BOOST_TEST_MODULE platform_test
#include <boost/test/unit_test.hpp>

#include "platform/platform_test/platform_test.hpp"
#include "platform/serial_storage.hpp"
#include "platform/simulation.hpp"
#include "platform/simulator_meta.hpp"
#include "platform/simulation_manager.hpp"
#include "platform/standard_simulation.hpp"
#include "platform/storage_meta.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"

// Manuals
#include "core/input_document.hpp"
#include "core/input_delegater.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
#include "evaluator/evaluator_manager.hpp" // for simulation serialization
#include "geometry/geometry_manager.hpp"   // for simulation serialization
#include "geometry/periodic_cube_region.hpp"
#include "observable/base_observable.hpp"  // for simulation serialization
#include "observable/gdbm_sink.hpp"
#include "observable/report_manager.hpp"   // for simulation serialization
#include "particle/ensemble.hpp"           // for simulation serialization
#include "particle/particle_manager.hpp"   // for simulation serialization
#include "trial/choice_manager.hpp"        // for simulation serialization
#include "trial/base_chooser.hpp"          // for simulation serialization
#include "trial/choice.hpp"                // for simulation serialization
//#include "utility/XXrandomXX.hpp"
//#include "utility/XXutilityXX.hpp"
// -
#include <fstream>
// - 
namespace platform {

// Mockup set of realistic species.
//
//  specie_count = 5
//  solute = "Na" (x2 particles) and "Cl" (x1 particle)
//  mobile = "CA" (x1 particle)
//  flexible = "CO" (x1 particle)
//  channel only = "OX" (x1 particle)
//  
boost::shared_ptr< particle::particle_manager > platform_test::mockup_particle_manager()
{
  boost::shared_ptr< particle::particle_manager > pman( new particle::particle_manager );
  //XX {
  //XX   particle::specie spc1;
  //XX   spc1.set_label( "CA" );
  //XX   spc1.set_concentration( 1.0 );
  //XX   spc1.set_radius( 0.11 );
  //XX   spc1.set_rate( 0.2 );
  //XX   spc1.set_valency( 1.0 );
  //XX   spc1.set_excess_potential( 0.123 );
  //XX   spc1.set_type( particle::specie::MOBILE );
  //XX   spc1.append_position( geometry::coordinate( 2.0, 2.0, 0.0 ), geometry::centroid( 3.0, 2.0, 2.0, 0.0 ) );
  //XX   pman->add_specie( spc1 );
  //XX }
  //XX {
  //XX   particle::specie spc1;
  //XX   spc1.set_label( "CO" );
  //XX   spc1.set_concentration( 1.0 );
  //XX   spc1.set_radius( 0.12 );
  //XX   spc1.set_rate( 0.2 );
  //XX   spc1.set_valency( -1.0 );
  //XX   spc1.set_excess_potential( 0.123 );
  //XX   spc1.set_type( particle::specie::FLEXIBLE );
  //XX   spc1.append_position( geometry::coordinate( 0.0, 0.0, 2.0 ), geometry::centroid( 3.0, 0.0, 0.0, 2.0 ) );
  //XX   pman->add_specie( spc1 );
  //XX }
  //XX {
  //XX   particle::specie spc1;
  //XX   spc1.set_label( "OX" );
  //XX   spc1.set_concentration( 1.0 );
  //XX   spc1.set_radius( 0.12 );
  //XX   spc1.set_rate( 0.2 );
  //XX   spc1.set_valency( -1.0 );
  //XX   spc1.set_excess_potential( 0.123 );
  //XX   spc1.set_type( particle::specie::CHANNEL_ONLY );
  //XX   spc1.append_position( geometry::coordinate( 2.0, 0.0, 2.0 ) );
  //XX   pman->add_specie( spc1 );
  //XX }
  //XX {
  //XX   particle::specie spc1;
  //XX   spc1.set_label( "Na" );
  //XX   spc1.set_concentration( 1.0 );
  //XX   spc1.set_radius( 0.12 );
  //XX   spc1.set_rate( 0.2 );
  //XX   spc1.set_valency( 1.0 );
  //XX   spc1.set_excess_potential( 0.123 );
  //XX   spc1.set_type( particle::specie::SOLUTE );
  //XX   spc1.append_position( geometry::coordinate( 0.0, 0.0, 0.0 ) );
  //XX   spc1.append_position( geometry::coordinate( 2.0, 0.0, 0.0 ) );
  //XX   pman->add_specie( spc1 );
  //XX }
  //XX {
  //XX   particle::specie spc1;
  //XX   spc1.set_label( "Cl" );
  //XX   spc1.set_concentration( 1.0 );
  //XX   spc1.set_radius( 0.2 );
  //XX   spc1.set_rate( 0.2 );
  //XX   spc1.set_valency( -1.0 );
  //XX   spc1.set_excess_potential( 0.3123 );
  //XX   spc1.set_type( particle::specie::SOLUTE );
  //XX   spc1.append_position( geometry::coordinate( 0.0, 2.0, 0.0 ) );
  //XX   pman->add_specie( spc1 );
  //XX }
  //XX pman->add_predefined_particles();
  return pman;

}

// Create geometry manager with PBC cube "cell" and 
// open cube "bulk" regions.
boost::shared_ptr< geometry::geometry_manager > platform_test::mockup_geometry_manager()
{
  boost::shared_ptr< geometry::geometry_manager > gman;
  //XX boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 5.0 ) );
  //XX boost::shared_ptr< geometry::geometry_manager > gman( new geometry::geometry_manager( regn ) );
  //XX {
  //XX   boost::shared_ptr< geometry::base_region > bulk( new geometry::cube_region( "bulk", 4.0, geometry::coordinate( 0.0, 0.0, 0.0 ), true ) );
  //XX   gman->add_region( bulk );
  //XX }
  return gman;

}

// Test input file content.
std::string platform_test::canon_input()
{
  const std::string result( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype metropolis\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.1\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n\n" );
  return result;

}

// Test simulation ctor
BOOST_AUTO_TEST_CASE( simulation_lifetime_test )
{
  {
    // Test for non-virtual no-copy object pattern
    BOOST_CHECK( not std::is_default_constructible< platform::simulation >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< platform::simulation >::type {} );
    BOOST_CHECK( not std::is_move_constructible< platform::simulation >::type {} );
    BOOST_CHECK( not ( std::is_assignable< platform::simulation, platform::simulation >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< platform::simulation >::type {} );
  }
  {
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > var( new platform::simulation( regn, stor ) );
    BOOST_CHECK_EQUAL( var->energy(), 0.0 );
    BOOST_CHECK_EQUAL( var->inner_loop_size(), 0ul );
    BOOST_CHECK_EQUAL( var->get_outer_loop_index(), 0ul );
    BOOST_CHECK_EQUAL( var->report_interval(), 0ul );
    BOOST_CHECK_EQUAL( var->compute_degrees_of_freedom(), 0ul );
    BOOST_CHECK( var->run_title().empty() );
    BOOST_CHECK( not var->has_manager() );
  }

}

// Test simulation access/modify methods
BOOST_AUTO_TEST_CASE( simulation_modify_method_test )
{
  std::stringstream store;
  const std::string title( "Test simulation" );
  const std::size_t inner( 100ul );
  const std::size_t prod( 2000ul );
  const std::size_t therm( 100ul );
  const std::size_t report( 500ul );
  const std::size_t ntarg( 50ul );
  const double epsw( 70.0 );
  const double tmptr( 305.0 );
  {
    boost::shared_ptr< platform::simulation_manager > simtype( new platform::standard_simulation );
    simtype->set_production_interval( prod );
    simtype->set_equilibration_interval( therm );
  
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > var( new platform::simulation( regn, stor ) );
  
    BOOST_CHECK_EQUAL( var->energy(), 0.0 );
    BOOST_CHECK_EQUAL( var->inner_loop_size(), 0ul );
    BOOST_CHECK_EQUAL( var->get_outer_loop_index(), 0ul );
    BOOST_CHECK_EQUAL( var->report_interval(), 0ul );
    BOOST_CHECK( var->run_title().empty() );
    BOOST_CHECK( not var->has_manager() );
  
    var->set_inner_loop_size( inner );
    var->set_report_interval( report );
    var->set_run_title( title );
    var->set_target_count( ntarg );
    var->set_solvent_permittivity( epsw );
    var->set_temperature( tmptr );
    var->set_manager( simtype );
  
    BOOST_CHECK_EQUAL( var->inner_loop_size(), 100ul );
    BOOST_CHECK_EQUAL( var->get_outer_loop_index(), 0ul );
    BOOST_CHECK_EQUAL( var->report_interval(), 500ul );
    BOOST_CHECK_EQUAL( var->run_title(), title );
    BOOST_CHECK_EQUAL( var->particles().target_count(), ntarg );
    BOOST_CHECK_EQUAL( var->evaluators().temperature(), tmptr );
    BOOST_CHECK_EQUAL( var->evaluators().permittivity(), epsw );
    BOOST_REQUIRE( var->has_manager() );
    BOOST_CHECK_EQUAL( var->manager().equilibration_interval(), therm );
    BOOST_CHECK_EQUAL( var->manager().production_interval(), prod );
  
    // write class instance to archive
    boost::archive::text_oarchive oa( store );
    oa << var;
  }
  {
     boost::shared_ptr< platform::simulation > var;
     // read class instance from archive
    boost::archive::text_iarchive ia( store );
    ia >> var;
  
    BOOST_CHECK_EQUAL( var->inner_loop_size(), 100ul );
    BOOST_CHECK_EQUAL( var->get_outer_loop_index(), 0ul );
    BOOST_CHECK_EQUAL( var->report_interval(), 500ul );
    BOOST_CHECK_EQUAL( var->run_title(), title );
    BOOST_CHECK_EQUAL( var->manager().equilibration_interval(), therm );
    BOOST_CHECK_EQUAL( var->manager().production_interval(), prod );
    BOOST_CHECK_EQUAL( var->particles().target_count(), ntarg );
    BOOST_CHECK_EQUAL( var->evaluators().temperature(), tmptr );
    BOOST_CHECK_EQUAL( var->evaluators().permittivity(), epsw );
  
  }

}

// Test license method (without/with sim_manager)
BOOST_AUTO_TEST_CASE( simulation_license_test )
{
  std::stringstream store;
  const std::string title( "Test simulation" );
  const std::size_t inner( 100ul );
  const std::size_t prod( 2000ul );
  const std::size_t therm( 100ul );
  const std::size_t report( 500ul );
  const std::size_t ntarg( 50ul );
  const double epsw( 70.0 );
  const double tmptr( 305.0 );
  {
    boost::shared_ptr< platform::simulation_manager > simtype( new platform::standard_simulation );
    simtype->set_production_interval( prod );
    simtype->set_equilibration_interval( therm );
  
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > var( new platform::simulation( regn, stor ) );
    BOOST_CHECK_EQUAL( var->energy(), 0.0 );
    BOOST_CHECK_EQUAL( var->inner_loop_size(), 0ul );
    BOOST_CHECK_EQUAL( var->get_outer_loop_index(), 0ul );
    BOOST_CHECK_EQUAL( var->report_interval(), 0ul );
    BOOST_CHECK( var->run_title().empty() );
    BOOST_CHECK( not var->has_manager() );
  
    {
      std::stringstream os;
      var->license( os );
      const std::string lic( os.str() );
      //std::cout << lic;
      BOOST_CHECK( lic.find( "GNU General Public License" ) < lic.size() );
      BOOST_CHECK( lic.find( "Free Software Foundation" ) < lic.size() );
      BOOST_CHECK( lic.find( "http://www.gnu.org/licenses" ) < lic.size() );
    }
  
    var->set_inner_loop_size( inner );
    var->set_report_interval( report );
    var->set_run_title( title );
    var->set_target_count( ntarg );
    var->set_solvent_permittivity( epsw );
    var->set_temperature( tmptr );
    var->set_manager( simtype );
  
    {
      std::stringstream os;
      var->license( os );
      const std::string lic( os.str() );
      //std::cout << lic;
      BOOST_CHECK( lic.find( "GNU General Public License" ) < lic.size() );
      BOOST_CHECK( lic.find( "Free Software Foundation" ) < lic.size() );
      BOOST_CHECK( lic.find( "http://www.gnu.org/licenses" ) < lic.size() );
    }
  
    BOOST_CHECK_EQUAL( var->inner_loop_size(), 100ul );
    BOOST_CHECK_EQUAL( var->get_outer_loop_index(), 0ul );
    BOOST_CHECK_EQUAL( var->report_interval(), 500ul );
    BOOST_CHECK_EQUAL( var->run_title(), title );
    BOOST_CHECK_EQUAL( var->particles().target_count(), ntarg );
    BOOST_CHECK_EQUAL( var->evaluators().temperature(), tmptr );
    BOOST_CHECK_EQUAL( var->evaluators().permittivity(), epsw );
    BOOST_REQUIRE( var->has_manager() );
    BOOST_CHECK_EQUAL( var->manager().equilibration_interval(), therm );
    BOOST_CHECK_EQUAL( var->manager().production_interval(), prod );
  }

}

// Test simulation access/modify methods
BOOST_AUTO_TEST_CASE( simulation_program_description_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > var( new platform::simulation( regn, stor ) );
  std::stringstream os;
  var->program_description( os );
  const std::string lic( os.str() );
  //std::cout << lic;
  BOOST_CHECK( lic.find( "3.1415927" ) < lic.size() );
  BOOST_CHECK( lic.find( "8.8542e-12" ) < lic.size() );
  BOOST_CHECK( lic.find( "6.02214e+23" ) < lic.size() );
  BOOST_CHECK( lic.find( "1.3806e-23" ) < lic.size() );
  BOOST_CHECK( lic.find( "1.6021917e-19" ) < lic.size() );
  BOOST_CHECK( lic.find( "1660.5393" ) < lic.size() );
  BOOST_CHECK( lic.find( "machine integer size" ) < lic.size() );
  BOOST_CHECK( lic.find( "Host machine" ) < lic.size() );
  BOOST_CHECK( lic.find( "compiled version" ) < lic.size() );

}

// Test simulation_definition ctor
BOOST_AUTO_TEST_CASE( simulation_definition_lifetime_test )
{
  {
    // Test for virtual no-copy object pattern
    BOOST_CHECK( not std::is_default_constructible< platform::simulation_definition >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< platform::simulation_definition >::type {} );
    BOOST_CHECK( not std::is_move_constructible< platform::simulation_definition >::type {} );
    BOOST_CHECK( not ( std::is_assignable< platform::simulation_definition, platform::simulation_definition >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< platform::simulation_definition >::type {} );
  }
  {
    auto dummy = [](const std::vector< core::input_parameter_memo > p)->std::unique_ptr<platform::simulation_manager>{ return std::unique_ptr<platform::simulation_manager>{}; };
    boost::shared_ptr< platform::simulation_definition > var( new platform::simulation_definition( "standard", "some desc", dummy ) );
    BOOST_CHECK_EQUAL( var->label(), "standard" );
    BOOST_CHECK_EQUAL( var->description(), "some desc" );
    BOOST_CHECK_EQUAL( var->size(), 0ul );
    BOOST_CHECK( var->empty() );
  }

}

// Test simulator meta ctor
BOOST_AUTO_TEST_CASE( simulator_meta_lifetime_test )
{
  {
    // Test for virtual no-copy object pattern
    BOOST_CHECK( not std::is_default_constructible< platform::simulator_meta >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< platform::simulator_meta >::type {} );
    BOOST_CHECK( not std::is_move_constructible< platform::simulator_meta >::type {} );
    BOOST_CHECK( not ( std::is_assignable< platform::simulator_meta, platform::simulator_meta >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< platform::simulator_meta >::type {} );
  }
  {
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > dummy( new platform::simulation( regn, stor ) );
  
    boost::shared_ptr< platform::simulator_meta > var( new platform::simulator_meta( *dummy ) );
    BOOST_CHECK_EQUAL( var->section_label(), "simulator" );
    BOOST_CHECK_EQUAL( var->multiple(), false );
    BOOST_CHECK_EQUAL( var->required(), true );
    BOOST_CHECK_EQUAL( var->size(), 0ul );
    BOOST_CHECK( var->empty() );
  }

}

// Test defining simulation object from input.
//
// * using standard_simulation simulation manager.
BOOST_AUTO_TEST_CASE( simulator_meta_input_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  const std::string title( "Test simulation" );
  const std::size_t inner( 100ul );
  const std::size_t prod( 2000ul );
  const std::size_t therm( 100ul );
  const std::size_t report( 500ul );
  const std::size_t ntarg( 50ul );
  const double epsw( 70.0 );
  const double tmptr( 305.0 );
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }
  
  BOOST_CHECK_EQUAL( sim->run_title(), title );
  BOOST_CHECK_EQUAL( sim->get_outer_loop_index(), 0ul );
  BOOST_CHECK_EQUAL( sim->inner_loop_size(), inner );
  BOOST_CHECK_EQUAL( sim->report_interval(), report );
  BOOST_CHECK_EQUAL( sim->has_manager(), true );
  BOOST_CHECK_EQUAL( sim->manager().equilibration_interval(), therm );
  BOOST_CHECK_EQUAL( sim->manager().production_interval(), prod );
  BOOST_CHECK_EQUAL( sim->manager().type_label(), "standard" );
  BOOST_CHECK_EQUAL( sim->particles().target_count(), ntarg );
  BOOST_CHECK_EQUAL( sim->evaluators().temperature(), tmptr );
  BOOST_CHECK_EQUAL( sim->evaluators().permittivity(), epsw );
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_type_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
  boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
  platform::standard_simulation::add_definition( *meta );
  
  BOOST_REQUIRE( meta->has_type( "standard" ) );
  dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype \ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
  dg.read_input( reader );
  BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation manager \"type\" parameter \"type\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>type <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_duplicate_type_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"type\" appears more than once in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>type standard<<\n** Parameter \"type\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_type_bad_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype parallel\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation manager \"type\" parameter \"type\" with value (parallel) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>type parallel<<\n** A value from the list (standard) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_inner_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner #100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation inner loop parameter \"inner\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>inner #100<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_inner_bad_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner ten\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation inner loop parameter \"inner\" with value (ten) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>inner ten<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_name_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname #Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation title parameter \"name\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>name #Test simulation<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_name_empty_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname \"\"\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation title parameter \"name\" with value (\"\") in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>name \"\"<<\n** Expected a non-empty value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_nstep_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep #2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation production step parameter \"nstep\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>nstep #2000<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_nstep_bad_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep twenty\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation production step parameter \"nstep\" with value (twenty) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>nstep twenty<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_naver_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver #100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation equilibration step parameter \"naver\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>naver #100<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_naver_bad_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver ten\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation equilibration step parameter \"naver\" with value (ten) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>naver ten<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_isave_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave #500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation checkpoint/report interval parameter \"isave\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 9.\n   >>isave #500<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_isave_bad_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave ten\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation checkpoint/report interval parameter \"isave\" with value (ten) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 9.\n   >>isave ten<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_ntarg_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg #50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation target particle number parameter \"ntarg\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 10.\n   >>ntarg #50<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_ntarg_bad_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg ten\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation target particle number parameter \"ntarg\" with value (ten) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 10.\n   >>ntarg ten<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_epsw_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw #70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation media permittivity parameter \"epsw\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 11.\n   >>epsw #70.0<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_epsw_bad_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw seventy\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation media permittivity parameter \"epsw\" with value (seventy) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 11.\n   >>epsw seventy<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_epsw_negative_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw -70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation media permittivity parameter \"epsw\" with value (-70.0) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 11.\n   >>epsw -70.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_epsw_zero_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 0.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation media permittivity parameter \"epsw\" with value (0.0) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 11.\n   >>epsw 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_kelvin_no_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin #305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation temperature (in kelvin) parameter \"kelvin\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n   >>kelvin #305.0<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_kelvin_bad_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin \"STP\"\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation temperature (in kelvin) parameter \"kelvin\" with value (\"STP\") in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n   >>kelvin \"STP\"<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_kelvin_negative_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin -305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation temperature (in kelvin) parameter \"kelvin\" with value (-305.0) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n   >>kelvin -305.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_kelvin_zero_value_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 0.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation temperature (in kelvin) parameter \"kelvin\" with value (0.0) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n   >>kelvin 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_type_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation configuration section \"simulator\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 12.\n** Add the following parameters to the section: type **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_inner_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation configuration section \"simulator\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 12.\n** Add the following parameters to the section: inner **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_nstep_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnaver 2000\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation configuration section \"simulator\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 12.\n** Add the following parameters to the section: nstep **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_naver_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation configuration section \"simulator\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 12.\n** Add the following parameters to the section: naver **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_isave_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Simulation configuration section \"simulator\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 12.\n** Add the following parameters to the section: isave **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_ntarg_test_PASS )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_epsw_test_PASS )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_kelvin_test_PASS )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_no_name_test_PASS )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< platform::simulator_meta > meta( new platform::simulator_meta( *sim ) );
    platform::standard_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype standard\ninner 100\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Create simulation stage 1: read full input.
BOOST_AUTO_TEST_CASE( full_simulation_build_input_delegate_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_delegater dg( platform::simulation::get_max_input_version() );
  BOOST_REQUIRE_EQUAL( dg.max_version(), 1ul );
  sim->build_input_delegater( dg );
  
  BOOST_REQUIRE( dg.has_section( core::strngs::simulator_label() ) );
  BOOST_REQUIRE( dg.has_section( core::strngs::evaluator_label() ) );
  BOOST_REQUIRE( dg.has_section( core::strngs::sampler_label() ) );
  BOOST_REQUIRE( dg.has_section( core::strngs::fsspec() ) );
  BOOST_REQUIRE( dg.has_section( core::strngs::fsregn() ) );
  BOOST_REQUIRE( dg.has_section( core::strngs::fstry() ) );
  BOOST_REQUIRE( dg.has_section( platform::storage_meta::storage_label() ) );
  
  //{
  //  core::input_help hlp;
  //  dg.get_documentation( hlp );
  //  hlp.write( std::cout );
  //}

}

// Create simulation stage 1: read full input.
BOOST_AUTO_TEST_CASE( full_simulation_read_input_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  const std::string title( "Test simulation" );
  const std::size_t inner( 100ul );
  const std::size_t prod( 2000ul );
  const std::size_t therm( 100ul );
  const std::size_t report( 500ul );
  const std::size_t ntarg( 50ul );
  const double epsw( 70.0 );
  const double tmptr( 305.0 );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", platform_test::canon_input() );
  try
  {
    sim->read_input( reader );
    // test post-conditions
    BOOST_REQUIRE( sim->has_manager() );
    BOOST_REQUIRE( sim->report().has_sink() );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what() << "\n";
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }
  
  BOOST_CHECK_EQUAL( sim->run_title(), title );
  BOOST_CHECK_EQUAL( sim->get_outer_loop_index(), 0ul );
  BOOST_CHECK_EQUAL( sim->inner_loop_size(), inner );
  BOOST_CHECK_EQUAL( sim->report_interval(), report );
  BOOST_CHECK_EQUAL( sim->has_manager(), true );
  BOOST_CHECK_EQUAL( sim->manager().equilibration_interval(), therm );
  BOOST_CHECK_EQUAL( sim->manager().production_interval(), prod );
  BOOST_CHECK_EQUAL( sim->manager().type_label(), "standard" );
  BOOST_CHECK_EQUAL( sim->particles().target_count(), ntarg );
  BOOST_CHECK_EQUAL( sim->evaluators().temperature(), tmptr );
  BOOST_CHECK_EQUAL( sim->evaluators().permittivity(), epsw );
  BOOST_CHECK_EQUAL( sim->storage().checkpoint_name(), platform::storage_manager::default_checkpoint_name() );
  BOOST_CHECK_EQUAL( sim->storage().filename_base(), platform::storage_manager::default_filename_base() );
  BOOST_CHECK_EQUAL( sim->storage().output_name(), platform::storage_manager::default_output_name() );
  BOOST_CHECK_EQUAL( sim->storage().run_number(), platform::storage_manager::default_run_number() );
  BOOST_CHECK_EQUAL( sim->storage().output_dir_fmt(), "\%1$04d" );
  BOOST_CHECK_EQUAL( sim->evaluators().size(), 2ul );
  BOOST_CHECK_EQUAL( sim->evaluators().get_evaluators()[0].type_label(), "coulomb" );
  BOOST_CHECK_EQUAL( sim->evaluators().get_evaluators()[1].type_label(), "hard-sphere" );
  BOOST_CHECK_EQUAL( sim->report().size(), 3ul );
  BOOST_CHECK( sim->report().has_sample( "specie-count" ) );
  BOOST_CHECK( sim->report().has_tracked( "widom" ) );
  BOOST_CHECK( sim->report().has_tracked( "metropolis" ) );
  BOOST_CHECK_EQUAL( sim->particles().get_ensemble().count(), 0ul );
  BOOST_CHECK_EQUAL( sim->particles().specie_count(), 2ul );
  BOOST_CHECK_EQUAL( sim->particles().target_ionic_strength(), 0.2 );
  BOOST_CHECK_EQUAL( sim->particles().get_ensemble().count(), 0ul );
  BOOST_CHECK_EQUAL( sim->particles().get_ensemble().known_keys(), 0ul );
  BOOST_CHECK( sim->particles().has_specie( "Na" ) );
  BOOST_CHECK( sim->particles().has_specie( "Cl" ) );
  BOOST_CHECK_EQUAL( sim->regions().size(), 2ul );
  BOOST_CHECK_EQUAL( sim->regions().region_key( "cell" ), 0ul );
  BOOST_CHECK_EQUAL( sim->regions().region_key( "centre" ), 1ul );
  BOOST_CHECK_EQUAL( sim->choices().size(), 0ul );
  BOOST_CHECK_EQUAL( sim->choices().size_chooser(), 2ul );
  BOOST_CHECK( sim->choices().has_chooser( "move" ) );
  BOOST_CHECK( sim->choices().has_chooser( "jump" ) );
  BOOST_CHECK( sim->choices().empty() );

}

// Generate simulation stage 2: generate starting configuration.
BOOST_AUTO_TEST_CASE( full_simulation_generate_sim_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  const std::size_t ntarg( 50ul );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", platform_test::canon_input() );
  try
  {
    sim->read_input( reader );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what() << "\n";
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }
  
  BOOST_CHECK_EQUAL( sim->particles().get_ensemble().count(), 0ul );
  BOOST_CHECK_EQUAL( sim->particles().get_ensemble().known_keys(), 0ul );
  try
  {
    sim->generate_simulation();
  
    BOOST_CHECK_EQUAL( sim->particles().get_ensemble().count(), ntarg );
    BOOST_CHECK_EQUAL( sim->particles().get_ensemble().specie_count( 0 ), ntarg/2 );
    BOOST_CHECK_EQUAL( sim->particles().get_ensemble().specie_count( 1 ), ntarg/2 );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception." );
  }

}

// Create simulation stage 3: get description and echo input.
BOOST_AUTO_TEST_CASE( full_simulation_echo_input_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
  stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", platform_test::canon_input() );
  try
  {
    sim->read_input( reader );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what() << "\n";
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }
  try
  {
    sim->generate_simulation();
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception." );
  }
  
  {
    core::input_document wr( 1 );
    sim->write_document( wr );
    //wr.write( std::cout );
    BOOST_CHECK_EQUAL( wr.size(), 13ul );
    std::map< std::string, std::size_t > expect { { "trial", 2ul }, { "simulator", 1ul }, { "run", 1 }, { "evaluator", 2 }, { "sampler", 3 }, { "region", 2 }, { "specie", 2 } };
    std::map< std::string, std::size_t > found;
    for( std::size_t ii = 0; ii != wr.size(); ++ii )
    {
      found[ wr[ ii ].label() ] += 1;
      if( wr[ ii ].label() == "simulator" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "epsw" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "epsw" ), "70.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "inner" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "inner" ), "100" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "isave" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "isave" ), "500" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "kelvin" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "kelvin" ), "305.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
        const std::string xtitle( "\"Test simulation\"" );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), xtitle );
        BOOST_REQUIRE( wr[ ii ].has_entry( "naver" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "naver" ), "100" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "nstep" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "nstep" ), "2000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "ntarg" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "ntarg" ), "50" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "standard" );
      }
      else if( wr[ ii ].label() == "run" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "checkname" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "checkname" ), "checkpoint.arc" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "input" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "input" ), "input.\%1$03d.inp" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "outname" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "outname" ), "output.dbm" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "outputdir" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "outputdir" ), "\%1$04d" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "standard" );
      }
      else if( wr[ ii ].label() == "evaluator" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string label( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( (label == "coulomb") or (label == "hard-sphere") );
      }
      else if( wr[ ii ].label() == "sampler" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "metropolis" or stype == "widom" or stype == "specie-count" ) );
        if( stype == "widom" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "iwidom" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "iwidom" ), "10" );
        }
      }
      else if( wr[ ii ].label() == "region" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "periodic-cube" or stype == "cube" ) );
        if( stype == "cube" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), "centre" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "open" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "open" ), "true" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "origin" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "origin" ), "2.5 2.5 2.5" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "width" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "width" ), "5.000000" );
        }
        else
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), "cell" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "width" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "width" ), "74.598436" );
        }
      }
      else if( wr[ ii ].label() == "specie" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "rate" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "rate" ), "1.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "chex" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "chex" ), "0" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "ctarg" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "ctarg" ), "0.100000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "free" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
        const std::string stype( wr[ ii ].get_entry( "name" ) );
        BOOST_CHECK( ( stype == "\"Na\"" or stype == "\"Cl\"" ) );
        if( stype == "\"Na\"" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "z" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "z" ), "1.000000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "d" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "d" ), "1.010000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "n" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "n" ).substr( 0, 2 ), "25" );
        }
        else
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "z" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "z" ), "-1" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "d" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "d" ), "1.780000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "n" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "n" ).substr( 0, 2 ), "25" );
        }
      }
      else if( wr[ ii ].label() == "trial" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "rate" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "rate" ), "0.500000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "move" or stype == "jump" ) );
        if( stype == "move" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "delta" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "delta" ), "1.0" );
        }
      }
    }
    BOOST_CHECK( expect == found );
  }

}

// Create simulation stage 3: check reading of echoed input.
BOOST_AUTO_TEST_CASE( full_simulation_read_echo_test )
{
  std::stringstream echoinput;
  {
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", platform_test::canon_input() );
    try
    {
      sim->read_input( reader );
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
    }
    try
    {
      sim->generate_simulation();
    }
    catch( std::exception const& err )
    {
      std::cout << err.what();
      BOOST_ERROR( "Unexpected exception." );
    }
  
    {
      core::input_document wr( 1 );
      sim->write_document( wr );
      BOOST_CHECK_EQUAL( wr.size(), 13ul );
      wr.write( echoinput );
    }
  }
  {
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
    core::input_preprocess reader;
    const std::string canon_input( echoinput.str() );
    reader.add_buffer( "dummy", canon_input );
    try
    {
      sim->read_input( reader );
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
      return;
    }
    try
    {
      sim->generate_simulation();
    }
    catch( std::exception const& err )
    {
      std::cout << err.what();
      BOOST_ERROR( "Unexpected exception." );
      return;
    }
  
    {
      std::stringstream echo2;
      core::input_document wr( 1 );
      sim->write_document( wr );
      BOOST_CHECK_EQUAL( wr.size(), 13ul );
      wr.write( echo2 );
      BOOST_CHECK_EQUAL( echo2.str(), canon_input );
  
      std::map< std::string, std::size_t > expect { { "trial", 2ul }, { "simulator", 1ul }, { "run", 1 }, { "evaluator", 2 }, { "sampler", 3 }, { "region", 2 }, { "specie", 2 } };
      std::map< std::string, std::size_t > found;
      for( std::size_t ii = 0; ii != wr.size(); ++ii )
      {
        found[ wr[ ii ].label() ] += 1;
        if( wr[ ii ].label() == "simulator" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "epsw" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "epsw" ), "70.000000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "inner" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "inner" ), "100" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "isave" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "isave" ), "500" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "kelvin" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "kelvin" ), "305.000000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
          const std::string xtitle( "\"Test simulation\"" );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), xtitle );
          BOOST_REQUIRE( wr[ ii ].has_entry( "naver" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "naver" ), "100" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "nstep" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "nstep" ), "2000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "ntarg" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "ntarg" ), "50" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "standard" );
        }
        else if( wr[ ii ].label() == "run" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "checkname" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "checkname" ), "checkpoint.arc" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "input" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "input" ), "input.\%1$03d.inp" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "outname" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "outname" ), "output.dbm" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "outputdir" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "outputdir" ), "\%1$04d" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "standard" );
        }
        else if( wr[ ii ].label() == "evaluator" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
          const std::string label( wr[ ii ].get_entry( "type" ) );
          BOOST_CHECK( (label == "coulomb") or (label == "hard-sphere") );
        }
        else if( wr[ ii ].label() == "sampler" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
          const std::string stype( wr[ ii ].get_entry( "type" ) );
          BOOST_CHECK( ( stype == "metropolis" or stype == "widom" or stype == "specie-count" ) );
          if( stype == "widom" )
          {
            BOOST_REQUIRE( wr[ ii ].has_entry( "iwidom" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "iwidom" ), "10" );
          }
        }
        else if( wr[ ii ].label() == "region" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
          const std::string stype( wr[ ii ].get_entry( "type" ) );
          BOOST_CHECK( ( stype == "periodic-cube" or stype == "cube" ) );
          if( stype == "cube" )
          {
            BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), "centre" );
            BOOST_REQUIRE( wr[ ii ].has_entry( "open" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "open" ), "true" );
            BOOST_REQUIRE( wr[ ii ].has_entry( "origin" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "origin" ), "2.5 2.5 2.5" );
            BOOST_REQUIRE( wr[ ii ].has_entry( "width" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "width" ), "5.000000" );
          }
          else
          {
            BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), "cell" );
            BOOST_REQUIRE( wr[ ii ].has_entry( "width" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "width" ), "74.598436" );
          }
        }
        else if( wr[ ii ].label() == "specie" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "rate" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "rate" ), "1.000000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "chex" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "chex" ), "0" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "ctarg" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "ctarg" ), "0.100000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "free" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
          const std::string stype( wr[ ii ].get_entry( "name" ) );
          BOOST_CHECK( ( stype == "\"Na\"" or stype == "\"Cl\"" ) );
          if( stype == "\"Na\"" )
          {
            BOOST_REQUIRE( wr[ ii ].has_entry( "z" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "z" ), "1.000000" );
            BOOST_REQUIRE( wr[ ii ].has_entry( "d" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "d" ), "1.010000" );
            BOOST_REQUIRE( wr[ ii ].has_entry( "n" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "n" ).substr( 0, 2 ), "25" );
          }
          else
          {
            BOOST_REQUIRE( wr[ ii ].has_entry( "z" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "z" ), "-1" );
            BOOST_REQUIRE( wr[ ii ].has_entry( "d" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "d" ), "1.780000" );
            BOOST_REQUIRE( wr[ ii ].has_entry( "n" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "n" ).substr( 0, 2 ), "25" );
          }
        }
        else if( wr[ ii ].label() == "trial" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "rate" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "rate" ), "0.500000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
          const std::string stype( wr[ ii ].get_entry( "type" ) );
          BOOST_CHECK( ( stype == "move" or stype == "jump" ) );
          if( stype == "move" )
          {
            BOOST_REQUIRE( wr[ ii ].has_entry( "delta" ) );
            BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "delta" ), "1.0" );
          }
        }
      }
      BOOST_CHECK( expect == found );
    }
  }
  

}

// Create simulation stage 4: describe the system after being read.
BOOST_AUTO_TEST_CASE( full_simulation_description_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", platform_test::canon_input() );
  try
  {
    sim->read_input( reader );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what() << "\n";
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }
  try
  {
    sim->generate_simulation();
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception thrown by sim->generate_simulation()." );
  }
  try
  {
    std::stringstream desc;
    sim->description( desc );
    //std::cout << desc.str();
    const std::string sdesc( desc.str() );
    BOOST_CHECK( sdesc.find( "Simulation Runtime Settings" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "output directory template : \%1$04d" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "thermalization interval : 100" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "steps between reports : 500" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "trials per \"step\" : 100" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "chem. pot. :-9.71748" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "radius :0.505" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "radius :0.89" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "particle # :50" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "beta (1/k_bT): 2.37483e+20 J{-1}" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "Temperature: 305 (Kelvin)" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "Permittivity of media: 70 (relative to vacuum)" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "Scale factor: 167110 / ( permittivity * T )" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "             = 0" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "move   50.00" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "jump   50.00" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "metropolis\n ----------\n    Sample the metropolis factor of each trial" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( " widom\n -----\n    Sample the free energy distribution of the ensemble\n    - Trials per sampling loop : 10" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "specie-count\n ------------\n    Sample average specie numbers." ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "target particle count : 50" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "target ionic strength : 0.2" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "number of species : 2" ) < sdesc.size() );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception thrown by sim->prepare();." );
    return;
  }

}

// Create simulation stage 4: prepare system for a simulation.
BOOST_AUTO_TEST_CASE( full_simulation_prepare_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", platform_test::canon_input() );
  try
  {
    sim->read_input( reader );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what() << "\n";
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }
  try
  {
    sim->generate_simulation();
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception thrown by sim->generate_simulation()." );
  }
  try
  {
    sim->prepare();
    // std::cout << slog->str();
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception thrown by sim->prepare();." );
    return;
  }
  {
    core::input_document wr( 1 );
    sim->write_document( wr );
    // wr.write( std::cout );
    BOOST_CHECK_EQUAL( wr.size(), 13ul );
    std::map< std::string, std::size_t > expect { { "trial", 2ul }, { "simulator", 1ul }, { "run", 1 }, { "evaluator", 2 }, { "sampler", 3 }, { "region", 2 }, { "specie", 2 } };
    std::map< std::string, std::size_t > found;
    for( std::size_t ii = 0; ii != wr.size(); ++ii )
    {
      found[ wr[ ii ].label() ] += 1;
      if( wr[ ii ].label() == "simulator" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "epsw" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "epsw" ), "70.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "inner" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "inner" ), "100" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "isave" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "isave" ), "500" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "kelvin" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "kelvin" ), "305.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
        const std::string xtitle( "\"Test simulation\"" );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), xtitle );
        BOOST_REQUIRE( wr[ ii ].has_entry( "naver" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "naver" ), "100" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "nstep" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "nstep" ), "2000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "ntarg" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "ntarg" ), "50" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "standard" );
      }
      else if( wr[ ii ].label() == "run" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "checkname" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "checkname" ), "checkpoint.arc" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "input" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "input" ), "input.\%1$03d.inp" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "outname" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "outname" ), "output.dbm" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "outputdir" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "outputdir" ), "\%1$04d" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "standard" );
      }
      else if( wr[ ii ].label() == "evaluator" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string label( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( (label == "coulomb") or (label == "hard-sphere") );
      }
      else if( wr[ ii ].label() == "sampler" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "metropolis" or stype == "widom" or stype == "specie-count" ) );
        if( stype == "widom" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "iwidom" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "iwidom" ), "10" );
        }
      }
      else if( wr[ ii ].label() == "region" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "periodic-cube" or stype == "cube" ) );
        if( stype == "cube" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), "centre" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "open" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "open" ), "true" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "origin" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "origin" ), "2.5 2.5 2.5" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "width" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "width" ), "5.000000" );
        }
        else
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), "cell" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "width" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "width" ), "74.598436" );
        }
      }
      else if( wr[ ii ].label() == "specie" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "rate" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "rate" ), "1.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "chex" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "chex" ), "0" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "ctarg" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "ctarg" ), "0.100000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "free" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
        const std::string stype( wr[ ii ].get_entry( "name" ) );
        BOOST_CHECK( ( stype == "\"Na\"" or stype == "\"Cl\"" ) );
        if( stype == "\"Na\"" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "z" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "z" ), "1.000000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "d" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "d" ), "1.010000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "n" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "n" ).substr( 0, 2 ), "25" );
        }
        else
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "z" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "z" ), "-1" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "d" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "d" ), "1.780000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "n" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "n" ).substr( 0, 2 ), "25" );
        }
      }
      else if( wr[ ii ].label() == "trial" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "rate" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "rate" ), "0.500000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "move" or stype == "jump" ) );
        if( stype == "move" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "delta" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "delta" ), "1.0" );
        }
      }
    }
    BOOST_CHECK( expect == found );
  }

}

// Create simulation stage 4: describe the system after calling prepare.
BOOST_AUTO_TEST_CASE( full_simulation_prepare_description_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", platform_test::canon_input() );
  try
  {
    sim->read_input( reader );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what() << "\n";
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }
  try
  {
    sim->generate_simulation();
    sim->prepare();
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception thrown by sim->generate_simulation()." );
  }
  try
  {
    std::stringstream desc;
    sim->description( desc );
    //std::cout << desc.str();
    const std::string sdesc( desc.str() );
    BOOST_CHECK( sdesc.find( "Simulation Runtime Settings" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "output directory template : \%1$04d" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "thermalization interval : 100" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "steps between reports : 500" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "trials per \"step\" : 100" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "chem. pot. :-9.71748" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "radius :0.505" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "radius :0.89" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "particle # :50" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "beta (1/k_bT): 2.37483e+20 J{-1}" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "Temperature: 305 (Kelvin)" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "Permittivity of media: 70 (relative to vacuum)" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "Scale factor: 167110 / ( permittivity * T )" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "             = 7.82716" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "move   50.00" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "jump   50.00" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "   type spc. rate(%)\n      0    0   25.00\n      1    1   25.00\n      0    1   25.00\n      1    0   25.00" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "metropolis\n ----------\n    Sample the metropolis factor of each trial" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( " widom\n -----\n    Sample the free energy distribution of the ensemble\n    - Trials per sampling loop : 10" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "specie-count\n ------------\n    Sample average specie numbers." ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "target particle count : 50" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "target ionic strength : 0.2" ) < sdesc.size() );
    BOOST_CHECK( sdesc.find( "number of species : 2" ) < sdesc.size() );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception thrown by sim->prepare();." );
    return;
  }

}

// Create simulation stage 5: run a simulation.
BOOST_AUTO_TEST_CASE( full_simulation_run_loop_test )
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
  boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
  boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", platform_test::canon_input() );
  try
  {
    sim->read_input( reader );
  }
  catch( std::exception const& err )
  {
    std::cout << err.what() << "\n";
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }
  try
  {
    sim->generate_simulation();
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception thrown by sim->generate_simulation()." );
  }
  try
  {
    sim->prepare();
    slog->str( "" );
    BOOST_CHECK_EQUAL( sim->compute_degrees_of_freedom(), 152ul );
    sim->run_loop( sim->manager().equilibration_interval() );
    { 
      // look for DoF use.
      const std::string log( slog->str() );
      BOOST_CHECK( log.find( "degrees of freedom of the simulation (152) is larger than" ) < std::string::npos );
      //std::cout << log;
    }
    //std::cout << slog->str();
    sim->prepare();
    slog->str( "" );
    sim->run_loop( sim->manager().production_interval() );
    //std::cout << slog->str();
  }
  catch( std::exception const& err )
  {
    std::cout << err.what();
    BOOST_ERROR( "Unexpected exception thrown by sim->prepare() or sim->run_loop(...)." );
    return;
  }
  {
    core::input_document wr( 1 );
    sim->write_document( wr );
    // wr.write( std::cout );
    BOOST_CHECK_EQUAL( wr.size(), 13ul );
    std::map< std::string, std::size_t > expect { { "trial", 2ul }, { "simulator", 1ul }, { "run", 1 }, { "evaluator", 2 }, { "sampler", 3 }, { "region", 2 }, { "specie", 2 } };
    std::map< std::string, std::size_t > found;
    for( std::size_t ii = 0; ii != wr.size(); ++ii )
    {
      found[ wr[ ii ].label() ] += 1;
      if( wr[ ii ].label() == "simulator" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "epsw" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "epsw" ), "70.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "inner" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "inner" ), "100" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "isave" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "isave" ), "500" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "kelvin" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "kelvin" ), "305.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
        const std::string xtitle( "\"Test simulation\"" );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), xtitle );
        BOOST_REQUIRE( wr[ ii ].has_entry( "naver" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "naver" ), "100" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "nstep" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "nstep" ), "2000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "ntarg" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "ntarg" ), "50" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "standard" );
      }
      else if( wr[ ii ].label() == "run" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "checkname" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "checkname" ), "checkpoint.arc" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "input" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "input" ), "input.\%1$03d.inp" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "outname" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "outname" ), "output.dbm" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "outputdir" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "outputdir" ), "\%1$04d" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "standard" );
      }
      else if( wr[ ii ].label() == "evaluator" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string label( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( (label == "coulomb") or (label == "hard-sphere") );
      }
      else if( wr[ ii ].label() == "sampler" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "metropolis" or stype == "widom" or stype == "specie-count" ) );
        if( stype == "widom" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "iwidom" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "iwidom" ), "10" );
        }
      }
      else if( wr[ ii ].label() == "region" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "periodic-cube" or stype == "cube" ) );
        if( stype == "cube" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), "centre" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "open" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "open" ), "true" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "origin" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "origin" ), "2.5 2.5 2.5" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "width" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "width" ), "5.000000" );
        }
        else
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "name" ), "cell" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "width" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "width" ), "74.598436" );
        }
      }
      else if( wr[ ii ].label() == "specie" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "rate" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "rate" ), "1.000000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "chex" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "chex" ), "0" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "ctarg" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "ctarg" ), "0.100000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "type" ), "free" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "name" ) );
        const std::string stype( wr[ ii ].get_entry( "name" ) );
        BOOST_CHECK( ( stype == "\"Na\"" or stype == "\"Cl\"" ) );
        if( stype == "\"Na\"" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "z" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "z" ), "1.000000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "d" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "d" ), "1.010000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "n" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "n" ).substr( 0, 2 ), "25" );
        }
        else
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "z" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "z" ), "-1" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "d" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "d" ), "1.780000" );
          BOOST_REQUIRE( wr[ ii ].has_entry( "n" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "n" ).substr( 0, 2 ), "25" );
        }
      }
      else if( wr[ ii ].label() == "trial" )
      {
        BOOST_REQUIRE( wr[ ii ].has_entry( "rate" ) );
        BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "rate" ), "0.500000" );
        BOOST_REQUIRE( wr[ ii ].has_entry( "type" ) );
        const std::string stype( wr[ ii ].get_entry( "type" ) );
        BOOST_CHECK( ( stype == "move" or stype == "jump" ) );
        if( stype == "move" )
        {
          BOOST_REQUIRE( wr[ ii ].has_entry( "delta" ) );
          BOOST_CHECK_EQUAL( wr[ ii ].get_entry( "delta" ), "1.0" );
        }
      }
    }
    BOOST_CHECK( expect == found );
  }

}

// Build and run a simulation.
BOOST_AUTO_TEST_CASE( full_simulation_main_test )
{
  char a1[] { "ionch" };
  char a2[] { "--input" };
  char a3[] { "test.inp" };
  char a4[] { "--run" };
  char a5[] { "2" };
  char* argv[] { &a1[0], &a2[0], &a3[0], &a4[0], &a5[0] };
  int argc = 5;
  
  if( boost::filesystem::exists( "0002" ) )
  {
    boost::filesystem::remove_all( "0002" );
  }
  if( boost::filesystem::exists( a3 ) )
  {
    boost::filesystem::remove( a3 );
  }
  {
    {
      const std::string input_file_contents
      {
        "simulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype metropolis\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.1\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n"
      };
      std::ofstream out( a3 );
      out.write( input_file_contents.data(), input_file_contents.size() );
    }
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
    if( boost::filesystem::exists( "0002" ) )
    {
      boost::filesystem::remove_all( "0002" );
    }
    try
    {
      BOOST_CHECK( stor->main( argc, argv, *sim ) );
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception thrown by \"sim.main(...)\": " ) + err.what() );
    }
  }
  {
    BOOST_REQUIRE( boost::filesystem::exists( "0002" ) );
    BOOST_REQUIRE( boost::filesystem::is_directory( "0002" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0002/checkpoint.arc" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0002/output.dbm" ) );
    try
    {
      observable::gdbm_sink db( "0002/output.dbm", observable::base_sink::READ_ONLY );
      BOOST_REQUIRE( db.exists( "metropolis.dat" ) );
      BOOST_REQUIRE( db.exists( "specie-count.dat" ) );
      BOOST_REQUIRE( db.exists( "widom.dat" ) );
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception working on db \"0002/output.dbm\": " ) + err.what() );
    }
  }
  if( boost::filesystem::exists( "0002" ) )
  {
    boost::filesystem::remove_all( "0002" );
  }
  if( boost::filesystem::exists( a3 ) )
  {
    boost::filesystem::remove( a3 );
  }

}


} // namespace platform
