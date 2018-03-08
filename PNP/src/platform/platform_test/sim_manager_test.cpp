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
#define BOOST_TEST_MODULE sim_manager_test
#include <boost/test/unit_test.hpp>

#include "platform/platform_test/sim_manager_test.hpp"
#include "platform/imc_simulation.hpp"
#include "platform/simulation.hpp"
#include "platform/simulator_meta.hpp"
#include "platform/simulation_manager.hpp"
#include "platform/standard_simulation.hpp"

// Manuals
#include "core/input_document.hpp"
#include "core/input_delegater.hpp"
#include "core/input_error.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
#include "evaluator/evaluator_manager.hpp" // for simulation serialization
#include "geometry/periodic_cube_region.hpp"// For sims
#include "observable/gdbm_sink.hpp"          // For sims
#include "particle/particle_manager.hpp"   // for simulation serialization
#include "platform/serial_storage.hpp"     // for sims
// -
#include <fstream>
// - 
namespace platform {

// Log message descibing the itearation update and its parameters
void sim_manager_test::simple_updater::description(std::ostream & os) const
{

}

// Add content of update into IMC section of an input file section.
//
//only throw possible should be from os.write() operation
void sim_manager_test::simple_updater::do_write_part_document(core::input_document & wr, std::size_t ix) const
{

}

//Prepare the updater before running a series of simulation steps
void sim_manager_test::simple_updater::prepare(simulation & sim)
{

}

// Perform update after each IMC cycle.
void sim_manager_test::simple_updater::update(simulation & sys, std::ostream & oslog)
{

}

// Test input file content.
std::string sim_manager_test::canon_input()
{
  const std::string result( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype metropolis\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.1\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n\n" );
  return result;

}

// (using standard_simulation)
// Methods tested
//  * equilibration_interval
//  * production_interval
//  * set_equilibration_interval
//  * set_production_interval
//  * serialize
//  * description
//  run
//  * license
//  write_part_document
//  * type_label

void sim_manager_test::simulation_manager_methods_test(boost::shared_ptr< simulation_manager > sman, std::string label, std::string lic_text, std::vector< std::string > entries)
{
  std::stringstream store;
  {
    BOOST_CHECK_EQUAL( sman->equilibration_interval(), 0ul );
    BOOST_CHECK_EQUAL( sman->production_interval(), 0ul );
  
    sman->set_equilibration_interval( 100ul );
    sman->set_production_interval( 1000ul );
  
    BOOST_CHECK_EQUAL( sman->equilibration_interval(), 100ul );
    BOOST_CHECK_EQUAL( sman->production_interval(), 1000ul );
  
    // write class instance to archive
    boost::archive::text_oarchive oa( store );
    oa << sman;
  }
  {
    boost::shared_ptr< platform::simulation_manager > var2;
    // read class instance from archive
    boost::archive::text_iarchive ia( store );
    ia >> var2;
  
    BOOST_CHECK_EQUAL( var2->equilibration_interval(), 100ul );
    BOOST_CHECK_EQUAL( var2->production_interval(), 1000ul );
    BOOST_CHECK_EQUAL( var2->type_label(), label );
  
    {
      std::stringstream desc;
      var2->description( desc );
      const std::string canon(  "\n thermalization interval : 100\n     production interval : 1000\n" );
      BOOST_CHECK( desc.str().find( canon ) != std::string::npos );
    }
    {
      std::stringstream lisc;
      var2->license( lisc );
      if( lic_text.empty() )
      {
        BOOST_CHECK( lisc.str().empty() );
      }
      else
      {
        BOOST_CHECK( lisc.str().find( lic_text ) != std::string::npos );
      }
    }
    {
      core::input_document out( 1 );
      const std::size_t ix = out.add_section( core::strngs::simulator_label() );
      var2->write_part_document( out, ix );
      BOOST_CHECK( out[ ix ].has_entry( core::strngs::fsnavr() ) );
      BOOST_CHECK( out[ ix ].has_entry( core::strngs::fsnstp() ) );
      for( auto item : entries )
      {
         BOOST_CHECK( out[ ix ].has_entry( item ) );
      }
    }
  }

}

// Test standard_simulation ctor (also simulation_manager)
BOOST_AUTO_TEST_CASE( standard_simulation_lifetime_test )
{
  {
    // Test for virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< platform::simulation_manager >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< platform::simulation_manager >::type {} );
    BOOST_CHECK( not std::is_move_constructible< platform::simulation_manager >::type {} );
    BOOST_CHECK( not ( std::is_assignable< platform::simulation_manager, platform::simulation_manager >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< platform::simulation_manager >::type {} );
  }
  {
    // Test for virtual object pattern
    BOOST_CHECK( std::is_default_constructible< platform::standard_simulation >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< platform::standard_simulation >::type {} );
    BOOST_CHECK( not std::is_move_constructible< platform::standard_simulation >::type {} );
    BOOST_CHECK( not ( std::is_assignable< platform::standard_simulation, platform::standard_simulation >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< platform::standard_simulation >::type {} );
  }
  {
    boost::shared_ptr< platform::standard_simulation > var( new platform::standard_simulation );
    BOOST_CHECK_EQUAL( var->equilibration_interval(), 0ul );
    BOOST_CHECK_EQUAL( var->production_interval(), 0ul );
  
    sim_manager_test::simulation_manager_methods_test( var, "standard", "", {} );
  }

}

// Test standard_simulation ctor (also simulation_manager)
BOOST_AUTO_TEST_CASE( imc_simulation_lifetime_test )
{
  {
    // Test for virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< platform::imc_simulation >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< platform::imc_simulation >::type {} );
    BOOST_CHECK( not std::is_move_constructible< platform::imc_simulation >::type {} );
    BOOST_CHECK( not ( std::is_assignable< platform::imc_simulation, platform::imc_simulation >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< platform::imc_simulation >::type {} );
  }
  {
    boost::shared_ptr< platform::imc_update > up( new platform::sim_manager_test::simple_updater );
    boost::shared_ptr< platform::imc_simulation > var( new platform::imc_simulation( up ) );
    BOOST_CHECK_EQUAL( var->equilibration_interval(), 0ul );
    BOOST_CHECK_EQUAL( var->production_interval(), 0ul );
  
    sim_manager_test::simulation_manager_methods_test( var, "imc", "", {} );
  }

}

// Test standard_simulation ctor (also simulation_manager)
BOOST_AUTO_TEST_CASE( imc_simulation_method_test )
{
  {
    boost::shared_ptr< platform::imc_update > up( new platform::sim_manager_test::simple_updater );
    boost::shared_ptr< platform::imc_simulation > var( new platform::imc_simulation( up ) );
    BOOST_CHECK_EQUAL( var->count(), 0ul );
    BOOST_CHECK_EQUAL( var->loop_size(), 100ul );
    var->set_loop_size( 200ul );
    BOOST_CHECK_EQUAL( var->loop_size(), 200ul );
  
  }

}

// Test defining simulation object from input.
//
// * using imc_simulation 
BOOST_AUTO_TEST_CASE( input_malasics_update_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    BOOST_REQUIRE( meta->has_type( "imc" ) );
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
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate malasics\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
  BOOST_CHECK_EQUAL( sim->manager().type_label(), "imc" );
  {
    std::stringstream out;
    sim->manager().description( out );
    BOOST_CHECK( out.str().find( "Update Method [malasics]" ) != std::string::npos );
  }
  BOOST_CHECK_EQUAL( sim->particles().target_count(), ntarg );
  BOOST_CHECK_EQUAL( sim->evaluators().temperature(), tmptr );
  BOOST_CHECK_EQUAL( sim->evaluators().permittivity(), epsw );
  
  

}

// Test defining simulation object from input.
//
// * using imc_simulation 
BOOST_AUTO_TEST_CASE( input_lamperski_update_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_type( "standard" ) );
    BOOST_REQUIRE( meta->has_type( "imc" ) );
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
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate lamperski\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
  BOOST_CHECK_EQUAL( sim->manager().type_label(), "imc" );
  {
    std::stringstream out;
    sim->manager().description( out );
    BOOST_CHECK( out.str().find( "Update Method [lamperski]" ) != std::string::npos );
  }
  BOOST_CHECK_EQUAL( sim->particles().target_count(), ntarg );
  BOOST_CHECK_EQUAL( sim->evaluators().temperature(), tmptr );
  BOOST_CHECK_EQUAL( sim->evaluators().permittivity(), epsw );
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_no_keywords_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Simulation configuration section \"simulator\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 13.\n** Add the following parameters to the section: update **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_missing_update_keyword_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nniter 100\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Simulation configuration section \"simulator\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 14.\n** Add the following parameters to the section: update **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_update_bad_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nupdate something\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Iterative Monte Carlo parameter \"update\" with value (something) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 10.\n   >>update something<<\n** A value from the list (malasics,lamperski) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_update_no_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nupdate \nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Iterative Monte Carlo parameter \"update\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 10.\n   >>update <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_niter_no_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate malasics\nniter \ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Iterative Monte Carlo parameter \"niter\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>niter <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_niter_negative_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate malasics\nniter -1\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
    //sim->description( std::cout );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Iterative Monte Carlo parameter \"niter\" with value (-1) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>niter -1<<\n** Input value must be greater than or equal to zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_niter_bad_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate malasics\nniter ten\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Iterative Monte Carlo parameter \"niter\" with value (ten) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>niter ten<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_niter_zero_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate malasics\nniter 0\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Iterative Monte Carlo parameter \"niter\" with value (0) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>niter 0<<\n** Iterative procedure requires more than one iteration. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_imc_niter_one_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate malasics\nniter 1\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Iterative Monte Carlo parameter \"niter\" with value (1) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>niter 1<<\n** Iterative procedure requires more than one iteration. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_lamperski_delta_no_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate lamperski\ndelta \ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Lamperski IMC parameter \"delta\" in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>delta <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_lamperski_delta_bad_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate lamperski\ndelta ten\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Lamperski IMC parameter \"delta\" with value (ten) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>delta ten<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( simulator_input_lamperski_delta_negative_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate lamperski\ndelta -0.1\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Lamperski IMC parameter \"delta\" with value (-0.1) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>delta -0.1<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( simulator_input_lamperski_delta_zero_value_test )
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
    platform::imc_simulation::add_definition( *meta );
  
    dg.add_input_delegate( meta );
  }
  
  std::string canon_input( "\n\nsimulator\ntype imc\nupdate lamperski\ndelta 0\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n" );
  
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
    BOOST_CHECK( msg.find( "Lamperski IMC parameter \"delta\" with value (0) in section \"simulator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>delta 0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Build and run a simulation.
BOOST_AUTO_TEST_CASE( malasics_simulation_main_test )
{
  if( boost::filesystem::exists( "0003" ) )
  {
    boost::filesystem::remove_all( "0003" );
  }
  if( boost::filesystem::exists( "testmalasics.inp" ) )
  {
    boost::filesystem::remove( "testmalasics.inp" );
  }
  {
    {
      const std::string input_file_contents{ 
  "\nsimulator\ntype imc\nupdate malasics\nniter 10\ninner 50\nname Test simulation\nnstep 500\nnaver 100\nisave 100\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Ka\nz 1\nd 1.41\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nchex 0.0\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n\ntrial\nrate 0.5\ntype \"individual\"\nend\n"
   };
      std::ofstream out( "testmalasics.inp" );
      out.write( input_file_contents.data(), input_file_contents.size() );
    }
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
    char a1[] { "ionch" };
    char a2[] { "--input" };
    char a3[] { "testmalasics.inp" };
    char a4[] { "--run" };
    char a5[] { "3" };
    char* argv[] { &a1[0], &a2[0], &a3[0], &a4[0], &a5[0] };
    int argc = 5;
  
    try
    {
      BOOST_CHECK( stor->main( argc, argv, *sim ) );
      //std::cout << slog->rdbuf();
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception thrown by \"sim.main(...)\": " ) + err.what() );
    }
  }
  {
    BOOST_REQUIRE( boost::filesystem::exists( "0003" ) );
    BOOST_REQUIRE( boost::filesystem::is_directory( "0003" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0003/checkpoint.arc" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0003/output.dbm" ) );
    try
    {
      observable::gdbm_sink db( "0003/output.dbm", observable::base_sink::READ_ONLY );
      BOOST_REQUIRE( db.exists( "igcmc.dat" ) );
      BOOST_REQUIRE( db.exists( "specie-count.dat" ) );
      BOOST_REQUIRE( db.exists( "widom.dat" ) );
      const std::string igcmctitle( "# TITLE: \"Series of chemical potential estimates.\"" );
      const std::string igcmcfields( "FIELDS: INDEX SPECIE Ka_CURRENT Ka_MEAN Ka_VAR SPECIE Na_CURRENT Na_MEAN Na_VAR SPECIE Cl_CURRENT Cl_MEAN Cl_VAR" );
      const std::string igcmcunits( "# UNITS: ORDINAL LABEL ENERGY ENERGY ENERGY LABEL ENERGY ENERGY ENERGY LABEL ENERGY ENERGY ENERGY" );
      const std::string uuidlabel( "# UUID:" );
  
      std::string buffer;
      BOOST_REQUIRE( db.read( "igcmc.dat", buffer ) );
      BOOST_REQUIRE_LT( buffer.find( igcmctitle ), buffer.size() );
      BOOST_REQUIRE_LT( buffer.find( igcmcfields ), buffer.size() );
      BOOST_REQUIRE_LT( buffer.find( igcmcunits ), buffer.size() );
      BOOST_REQUIRE_EQUAL( buffer.find( uuidlabel ), 0ul );
      //std::cout << buffer;
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception working on db \"0003/output.dbm\": " ) + err.what() );
    }
  }
  if( boost::filesystem::exists( "0003" ) )
  {
    boost::filesystem::remove_all( "0003" );
  }
  if( boost::filesystem::exists( "testmalasics.inp" ) )
  {
    boost::filesystem::remove( "testmalasics.inp" );
  }
  

}

// Build and run a simulation.
BOOST_AUTO_TEST_CASE( lamperski_simulation_main_test )
{
  if( boost::filesystem::exists( "0004" ) )
  {
    boost::filesystem::remove_all( "0004" );
  }
  if( boost::filesystem::exists( "testlamperski.inp" ) )
  {
    boost::filesystem::remove( "testlamperski.inp" );
  }
  {
    {
      const std::string input_file_contents{ 
  "\nsimulator\ntype imc\nupdate lamperski\ndelta 0.2\nniter 10\ninner 50\nname Test simulation\nnstep 500\nnaver 100\nisave 100\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Ka\nz 1\nd 1.41\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nchex 0.0\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n\ntrial\nrate 0.5\ntype \"individual\"\nend\n"
   };
      std::ofstream out( "testlamperski.inp" );
      out.write( input_file_contents.data(), input_file_contents.size() );
    }
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::stringstream > slog( new std::stringstream );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
    char a1[] { "ionch" };
    char a2[] { "--input" };
    char a3[] { "testlamperski.inp" };
    char a4[] { "--run" };
    char a5[] { "4" };
    char* argv[] { &a1[0], &a2[0], &a3[0], &a4[0], &a5[0] };
    int argc = 5;
  
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
    BOOST_REQUIRE( boost::filesystem::exists( "0004" ) );
    BOOST_REQUIRE( boost::filesystem::is_directory( "0004" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0004/checkpoint.arc" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0004/output.dbm" ) );
    try
    {
      observable::gdbm_sink db( "0004/output.dbm", observable::base_sink::READ_ONLY );
      BOOST_REQUIRE( db.exists( "igcmc.dat" ) );
      BOOST_REQUIRE( db.exists( "specie-count.dat" ) );
      BOOST_REQUIRE( db.exists( "widom.dat" ) );
      const std::string igcmctitle( "# TITLE: \"Series of chemical potential estimates.\"" );
      const std::string igcmcfields( "FIELDS: INDEX SPECIE Ka_CURRENT Ka_MEAN Ka_VAR SPECIE Na_CURRENT Na_MEAN Na_VAR SPECIE Cl_CURRENT Cl_MEAN Cl_VAR" );
      const std::string igcmcunits( "# UNITS: ORDINAL LABEL ENERGY ENERGY ENERGY LABEL ENERGY ENERGY ENERGY LABEL ENERGY ENERGY ENERGY" );
      const std::string uuidlabel( "# UUID:" );
  
      std::string buffer;
      BOOST_REQUIRE( db.read( "igcmc.dat", buffer ) );
      BOOST_CHECK_EQUAL( buffer.find( uuidlabel ), 0ul );
      BOOST_REQUIRE_LT( buffer.find( igcmctitle ), buffer.size() );
      BOOST_REQUIRE_LT( buffer.find( igcmcfields ), buffer.size() );
      BOOST_REQUIRE_LT( buffer.find( igcmcunits ), buffer.size() );
      //std::cout << buffer;
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception working on db \"0004/output.dbm\": " ) + err.what() );
    }
  }
  if( boost::filesystem::exists( "0004" ) )
  {
    boost::filesystem::remove_all( "0004" );
  }
  if( boost::filesystem::exists( "testlamperski.inp" ) )
  {
    boost::filesystem::remove( "testlamperski.inp" );
  }
  

}


} // namespace platform

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID( platform::sim_manager_test::simple_updater, "platform::sim_manager_test::simple_updater" );
