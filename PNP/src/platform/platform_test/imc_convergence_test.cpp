

#ifndef DEBUG
#define DEBUG 0
#endif

#define BOOST_TEST_MODULE imc_convergence_test
#include <boost/test/unit_test.hpp>

#include "platform/platform_test/imc_convergence_test.hpp"
#include "platform/imc_simulation.hpp"
#include "platform/simulation.hpp"
#include "platform/simulator_meta.hpp"

// Manuals
#include "core/input_document.hpp"
#include "core/input_delegater.hpp"
//#include "core/input_help.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
#include "evaluator/evaluator_manager.hpp" // for simulation serialization
//#include "geometry/geometry_manager.hpp"   // for simulation serialization
#include "geometry/periodic_cube_region.hpp"// For sims
//#include "observable/base_observable.hpp"  // for simulation serialization
#include "observable/gdbm_sink.hpp"          // For sims
//#include "observable/report_manager.hpp"   // for simulation serialization
//#include "particle/ensemble.hpp"           // for simulation serialization
#include "particle/particle_manager.hpp"   // for simulation serialization
#include "platform/serial_storage.hpp"     // for sims
//#include "trial/choice_manager.hpp"        // for simulation serialization
//#include "trial/base_chooser.hpp"          // for simulation serialization
//#include "trial/choice.hpp"                // for simulation serialization
//#include "utility/XXrandomXX.hpp"
//#include "utility/XXutilityXX.hpp"
// -
#include <fstream>
// - 
namespace platform {

// Build and run a simulation.
BOOST_AUTO_TEST_CASE( malasics_simulation_main_test )
{
  const std::string input_filename{ "malasics.inp" };
  if( boost::filesystem::exists( "0005" ) )
  {
    boost::filesystem::remove_all( "0005" );
  }
  if( boost::filesystem::exists( input_filename ) )
  {
    boost::filesystem::remove( input_filename );
  }
  {
    {
      const std::string input_file_contents{ 
  "\nsimulator\ntype imc\nupdate malasics\nniter 100\ninner 250\nname Test simulation\nnstep 5000\nnaver 1000\nisave 100\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Ka\nz 1\nd 1.41\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nchex 0.0\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n\ntrial\nrate 0.5\ntype \"individual\"\nend\n"
   };
      std::ofstream out( "malasics.inp" );
      out.write( input_file_contents.data(), input_file_contents.size() );
    }
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::ostream > slog( new std::ofstream( "malasics.log" ) );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
    char a1[] { "ionch" };
    char a2[] { "--input" };
    std::vector< char >a3 ( input_filename.begin(), input_filename.end() );
    a3.push_back( '\0' );
    char a4[] { "--run" };
    char a5[] { "5" };
    char* argv[] { &a1[0], &a2[0], a3.data(), &a4[0], &a5[0] };
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
    BOOST_REQUIRE( boost::filesystem::exists( "0005" ) );
    BOOST_REQUIRE( boost::filesystem::is_directory( "0005" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0005/checkpoint.arc" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0005/output.dbm" ) );
    try
    {
      observable::gdbm_sink db( "0005/output.dbm", observable::base_sink::READ_ONLY );
      BOOST_REQUIRE( db.exists( "igcmc.dat" ) );
      BOOST_REQUIRE( db.exists( "specie-count.dat" ) );
      BOOST_REQUIRE( db.exists( "widom.dat" ) );
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception working on db \"0005/output.dbm\": " ) + err.what() );
    }
  }
  //if( boost::filesystem::exists( "0005" ) )
  //{
  //  boost::filesystem::remove_all( "0005" );
  //}
  //if( boost::filesystem::exists( input_filename ) )
  //{
  //  boost::filesystem::remove( input_filename );
  //}
  

}

// Build and run a simulation.
BOOST_AUTO_TEST_CASE( lamperski_simulation_main_test )
{
  const std::string input_filename{ "lamperski.inp" };
  if( boost::filesystem::exists( "0006" ) )
  {
    boost::filesystem::remove_all( "0006" );
  }
  if( boost::filesystem::exists( input_filename ) )
  {
    boost::filesystem::remove( input_filename );
  }
  {
    {
      const std::string input_file_contents{
  "\nsimulator\ntype imc\nupdate lamperski\ndelta 0.5\nuse-random true\nniter 100\ninner 250\nname Test simulation\nnstep 2000\nnaver 100\nisave 100\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Ka\nz 1\nd 1.41\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nchex 0.0\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n\ntrial\nrate 0.5\ntype \"individual\"\nend\n"
   };
      std::ofstream out( input_filename );
      out.write( input_file_contents.data(), input_file_contents.size() );
    }
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::ostream > slog( new std::ofstream( "lamperski.log" ) );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
    char a1[] { "ionch" };
    char a2[] { "--input" };
    std::vector< char >a3 ( input_filename.begin(), input_filename.end() );
    a3.push_back( '\0' );
    char a4[] { "--run" };
    char a5[] { "6" };
    char* argv[] { &a1[0], &a2[0], a3.data(), &a4[0], &a5[0] };
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
    BOOST_REQUIRE( boost::filesystem::exists( "0006" ) );
    BOOST_REQUIRE( boost::filesystem::is_directory( "0006" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0006/checkpoint.arc" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0006/output.dbm" ) );
    try
    {
      observable::gdbm_sink db( "0006/output.dbm", observable::base_sink::READ_ONLY );
      BOOST_REQUIRE( db.exists( "igcmc.dat" ) );
      BOOST_REQUIRE( db.exists( "specie-count.dat" ) );
      BOOST_REQUIRE( db.exists( "widom.dat" ) );
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception working on db \"0006/output.dbm\": " ) + err.what() );
    }
  }
  //if( boost::filesystem::exists( "0006" ) )
  //{
  //  boost::filesystem::remove_all( "0006" );
  //}
  //if( boost::filesystem::exists( input_filename ) )
  //{
  //  boost::filesystem::remove( input_filename );
  //}
  

}

// Build and run a simulation that outputs trial log data.
BOOST_AUTO_TEST_CASE( trial_log_simulation_test )
{
  const std::string input_filename{ "trial-log.inp" };
  if( boost::filesystem::exists( "0007" ) )
  {
    boost::filesystem::remove_all( "0007" );
  }
  if( boost::filesystem::exists( input_filename ) )
  {
    boost::filesystem::remove( input_filename );
  }
  {
    {
      const std::string input_file_contents{ 
  "\nsimulator\ntype standard\ninner 250\nname Test simulation\nnstep 1000\nnaver 100\nisave 100\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype trial-log\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Ka\nz 1\nd 1.41\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.05\nchex 0.0\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nchex 0.0\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n\ntrial\nrate 0.5\ntype \"individual\"\nend\n"
   };
      std::ofstream out( input_filename );
      out.write( input_file_contents.data(), input_file_contents.size() );
    }
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
    boost::shared_ptr< platform::storage_manager > stor( new platform::serial_storage );
    boost::shared_ptr< std::ostream > slog( new std::ofstream( input_filename + ".log" ) );
    stor->set_log( slog );
    boost::shared_ptr< platform::simulation > sim( new platform::simulation( regn, stor ) );
  
    char a1[] { "ionch" };
    char a2[] { "--input" };
    std::vector< char >a3 ( input_filename.begin(), input_filename.end() );
    a3.push_back( '\0' );
    char a4[] { "--run" };
    char a5[] { "7" };
    char* argv[] { &a1[0], &a2[0], a3.data(), &a4[0], &a5[0] };
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
    BOOST_REQUIRE( boost::filesystem::exists( "0007" ) );
    BOOST_REQUIRE( boost::filesystem::is_directory( "0007" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0007/checkpoint.arc" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0007/output.dbm" ) );
    try
    {
      observable::gdbm_sink db( "0007/output.dbm", observable::base_sink::READ_ONLY );
      BOOST_REQUIRE( db.exists( "trial-log.dat" ) );
      BOOST_REQUIRE( db.exists( "specie-count.dat" ) );
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception working on db \"0007/output.dbm\": " ) + err.what() );
    }
  }
  //if( boost::filesystem::exists( "0007" ) )
  //{
  //  boost::filesystem::remove_all( "0007" );
  //}
  //if( boost::filesystem::exists( input_filename ) )
  //{
  //  boost::filesystem::remove( input_filename );
  //}
  

}


} // namespace platform
