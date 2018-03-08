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

#define BOOST_TEST_MODULE mpi_manager_test
#include <boost/test/unit_test.hpp>

#include "platform/platform_test/mpi_storage_test.hpp"
#include "platform/mpi_storage.hpp"
#include "platform/simulation.hpp"
#include "platform/simulator_meta.hpp"
#include "platform/simulation_manager.hpp"
#include "platform/standard_simulation.hpp"
#include "platform/storage_meta.hpp"

// Manuals
#include "core/input_delegater.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
#include "evaluator/evaluator_manager.hpp" // for simulation serialization
#include "geometry/geometry_manager.hpp"   // for simulation serialization
#include "geometry/periodic_cube_region.hpp"
#include "observable/base_observable.hpp"  // for simulation serialization
#include "observable/gdbm_sink.hpp"          // For sims
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

// Test input file content.
std::string mpi_storage_test::canon_input()
{
  const std::string result( "\n\nsimulator\ntype standard\ninner 100\nname Test simulation\nnstep 2000\nnaver 100\nisave 500\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype metropolis\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.1\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n\n" );
  return result;

}

//Test of mpi execution storage manager.

BOOST_AUTO_TEST_CASE( mpi_storage_lifetime_test )
{
  {
    // Test for virtual noncopy pattern
    BOOST_CHECK( std::is_default_constructible< platform::mpi_storage >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< platform::mpi_storage >::type {} );
    BOOST_CHECK( not std::is_move_constructible< platform::mpi_storage >::type {} );
    BOOST_CHECK( not( std::is_assignable< platform::mpi_storage, platform::mpi_storage >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< platform::mpi_storage >::type {} );
  }
  // static methods
  const std::string cname( "checkpoint.arc" );
  const std::string fbase( "input.\%1$03d.inp" );
  const std::string dbase( "\%1$03d" );
  const std::string oname( "output.dbm" );
  const std::size_t runnum( 1ul );
  {
    BOOST_CHECK_EQUAL( platform::storage_manager::default_checkpoint_name(), cname );
    BOOST_CHECK_EQUAL( platform::storage_manager::default_filename_base(), fbase );
    BOOST_CHECK_EQUAL( platform::storage_manager::default_output_dir_fmt(), dbase );
    BOOST_CHECK_EQUAL( platform::storage_manager::default_output_name(), oname );
    BOOST_CHECK_EQUAL( platform::storage_manager::default_run_number(), runnum );
    BOOST_CHECK_EQUAL( platform::mpi_storage::type_label_(), "mpi" );
  }
  {
    // Public ctor that inherits from base storage_manager
    boost::shared_ptr< platform::mpi_storage > var1( new platform::mpi_storage );
    BOOST_CHECK_EQUAL( var1->checkpoint_name(), cname );
    BOOST_CHECK_EQUAL( var1->filename_base(), fbase );
    BOOST_CHECK_EQUAL( var1->output_dir_fmt(), dbase );
    BOOST_CHECK_EQUAL( var1->output_name(), oname );
    BOOST_CHECK_EQUAL( var1->run_number(), runnum );
  }
  

}

// Use minimal derived class to test base class storage manager operations.
//
//Tested methods
//  * checkpoint_name
//  * default_checkpoint_name
//  * default_filename_base
//  * default_output_dir_fmt
//  * default_output_name
//  * default_run_number
//  * filename_base
//  * output_dir_fmt
//  * output_name
//  * run_number
//  * serialize
//  * set_checkpoint_name
//  * set_filename_base
//  * set_output_dir_fmt
//  * set_output_name
//  * set_run_number
//Not tested (undefined in simple_storage_manager)
//  * checkpoint_path
//  * compute_output_dir
//  * find_input_filename
//  * output_path
//  * open_output
//  * open_input
//  * open_checkpoint
//  ** open_log
//  ** get_log (should see message on cout!)
//

BOOST_AUTO_TEST_CASE( mpi_storage_method_test )
{
  //Tested methods
  //  * checkpoint_name
  //  * default_checkpoint_name
  //  * default_filename_base
  //  * default_output_dir_fmt
  //  * default_output_name
  //  * default_run_number
  //  * filename_base
  //  * get_label
  //  * output_dir_fmt
  //  * output_name
  //  * run_number
  //  * serialize
  //  * set_checkpoint_name
  //  * set_filename_base
  //  * set_output_dir_fmt
  //  * set_output_name
  //  * set_run_number
  //Not tested (undefined in simple_storage_manager)
  //  * checkpoint_path
  //  * compute_output_dir
  //  * find_input_filename
  //  * output_path
  //  * open_output
  //  * open_input
  //  * open_checkpoint
  //  ** open_log
  //  ** get_log (undefined because open_log is undefined.)
  
  const std::string cname( "checkpoint.arc" );
  const std::string test_cname( "check.dat" );
  const std::string fbase( "input.\%1$03d.inp" );
  const std::string test_fbase( "channel.\%1$04d.inp" );
  const std::string dbase( "\%1$03d" );
  const std::string outpath( "001/000" );
  const std::string outdir( "001/000/" );
  const std::string test_dbase( "\%1$04d.dat" );
  const std::string test_outpath( "5999.dat/000" );
  const std::string test_outdir( "5999.dat/000/" );
  const std::string oname( "output.dbm" );
  const std::string test_oname( "result.zip" );
  const std::size_t runnum( 1ul );
  const std::size_t test_runnum( 5999ul );
  
  std::stringstream store;
  {
    // Public ctor 1
    boost::shared_ptr< platform::mpi_storage > var1( new platform::mpi_storage );
    BOOST_CHECK_EQUAL( var1->get_label(), "mpi" );
    BOOST_CHECK_EQUAL( var1->checkpoint_name(), cname );
    BOOST_CHECK_EQUAL( var1->filename_base(), fbase );
    BOOST_CHECK_EQUAL( var1->output_dir_fmt(), dbase );
    BOOST_CHECK_EQUAL( var1->output_dir_fmt(), dbase );
    BOOST_CHECK_EQUAL( var1->output_name(), oname );
    BOOST_CHECK_EQUAL( var1->run_number(), runnum );
    // compute_output_dir gives "001/000"
    BOOST_CHECK_EQUAL( var1->compute_output_dir(), outpath );
    BOOST_CHECK_EQUAL( var1->checkpoint_path(), outdir + cname );
    BOOST_CHECK_EQUAL( var1->output_path(), outdir + oname );
  
    var1->set_checkpoint_name( test_cname );
    var1->set_filename_base( test_fbase );
    var1->set_output_dir_fmt( test_dbase );
    var1->set_output_name( test_oname );
    var1->set_run_number( test_runnum );
  
    BOOST_CHECK_EQUAL( var1->checkpoint_name(), test_cname );
    BOOST_CHECK_EQUAL( var1->filename_base(), test_fbase );
    BOOST_CHECK_EQUAL( var1->output_dir_fmt(), test_dbase );
    BOOST_CHECK_EQUAL( var1->output_name(), test_oname );
    BOOST_CHECK_EQUAL( var1->run_number(), test_runnum );
    // compute_output_dir gives "5999.dat/000"
    BOOST_CHECK_EQUAL( var1->compute_output_dir(), test_outpath );
    BOOST_CHECK_EQUAL( var1->checkpoint_path(), test_outdir + test_cname );
    BOOST_CHECK_EQUAL( var1->output_path(), test_outdir + test_oname );
  
    // write class instance to archive
    boost::archive::text_oarchive oa( store );
    oa << var1;
  }
  {
    boost::shared_ptr< platform::mpi_storage > var2;
    // read class instance from archive
    boost::archive::text_iarchive ia( store );
    ia >> var2;
    BOOST_CHECK_EQUAL( var2->checkpoint_name(), test_cname );
    BOOST_CHECK_EQUAL( var2->filename_base(), test_fbase );
    BOOST_CHECK_EQUAL( var2->output_dir_fmt(), test_dbase );
    BOOST_CHECK_EQUAL( var2->output_name(), test_oname );
    BOOST_CHECK_EQUAL( var2->run_number(), test_runnum );
    BOOST_CHECK_EQUAL( var2->compute_output_dir(), test_outpath );
    BOOST_CHECK_EQUAL( var2->checkpoint_path(), test_outdir + test_cname );
    BOOST_CHECK_EQUAL( var2->output_path(), test_outdir + test_oname );
    try
    {
      std::string fn;
      fn = var2->find_input_filename();
      BOOST_ERROR( "Unexpected \"var2->find_input_filename()\" found file named " + fn );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Unable to find input file(s) named: channel.5999.inp channel.inp in section \"simulator\".\n** Check input file name or provide --inputpattern commandline option. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"var2->find_input_filename()\" was not expected type: " ) + err.what() );
    }
    {
      auto ipath( boost::filesystem::unique_path( "input%%%%%.inp" ) );
      const std::string spath( ipath.native() );
  
      // USE PATH AS INPUT FILE NAME
      //////////////////////////////
      BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
      var2->set_filename_base( spath );
      BOOST_CHECK_EQUAL( var2->filename_base(), spath );
      // Try to find input after generating file
      try
      {
        {
          std::ofstream tmp( spath );
          tmp << "\n";
        }
        BOOST_CHECK_EQUAL( var2->find_input_filename(), spath );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"var2->find_input_filename()\" was not expected:" ) + err.what() );
      }
      // Try to open input
      try
      {
        auto reader_ptr = var2->open_input();
        BOOST_CHECK_EQUAL( reader_ptr->current_filename(), boost::filesystem::absolute( spath ).native() );
        BOOST_CHECK_EQUAL( reader_ptr->current_line_number(), 0 );
        BOOST_CHECK_EQUAL( reader_ptr->next(), false );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"var2->open_input()\" was not expected:" ) + err.what() );
      }
      if( boost::filesystem::exists( ipath ) )
      {
        boost::filesystem::remove( ipath );
      }
      var2->set_filename_base( test_fbase );
      // USE PATH AS OUTPUT DIR NAME
      //////////////////////////////
      BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
      var2->set_output_dir_fmt( spath );
      BOOST_CHECK_EQUAL( var2->output_dir_fmt(), spath );
      const std::string gen_cpath = var2->checkpoint_path();
      BOOST_CHECK_EQUAL( gen_cpath, spath + "/000/" + test_cname );
      const std::string gen_opath = var2->output_path();
      BOOST_CHECK_EQUAL( gen_opath, spath + "/000/" + test_oname );
      // Path should not be created until we open a file.
      BOOST_CHECK( not boost::filesystem::exists( ipath ) );
      try
      {
        {
          auto sink_ptr = var2->open_output();
          BOOST_CHECK( boost::filesystem::exists( ipath ) );
          BOOST_CHECK_EQUAL( sink_ptr->filename(), gen_opath );
          // Put entry in sink so that file is generated
          // if lazy opening is used.
          sink_ptr->write( "entry1", "Some value" );
        }
        // check for file after sink is closed.
        BOOST_CHECK( boost::filesystem::exists( gen_opath ) );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"var2->open_output()\" was not expected:" ) + err.what() );
      }
      if( boost::filesystem::exists( ipath ) )
      {
        boost::filesystem::remove_all( ipath );
      }
      try
      {
        {
          auto os_ptr = var2->open_checkpoint();
          BOOST_CHECK( boost::filesystem::exists( ipath ) );
          BOOST_CHECK( os_ptr.get() != nullptr );
          // Put text in checkpoint so that file is generated
          // if lazy opening is used.
          *( os_ptr ) << "Write some text\n";
        }
        // check for file after checkpoint is closed.
        BOOST_CHECK( boost::filesystem::exists( gen_cpath ) );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( std::string( "exception thrown by \"var2->open_checkpoint()\" was not expected:" ) + err.what() );
      }
      if( boost::filesystem::exists( ipath ) )
      {
        boost::filesystem::remove_all( ipath );
      }
    }
    {
      boost::shared_ptr< std::stringstream > ss( new std::stringstream );
      var2->set_log( ss );
      var2->get_log() << "TEST";
      BOOST_CHECK_EQUAL( "TEST", ss->str() );
    }
    var2->get_log() << "TEST2";
    BOOST_CHECK_EQUAL( "TESTTEST2", dynamic_cast< std::stringstream& >(var2->get_log()).str() );
  }
  

}

// Test what happens when input file resolves to no files.
BOOST_AUTO_TEST_CASE( mpi_storage_input_filename_noexist )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "input%%%%%.inp" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS INPUT FILE NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_filename_base( spath );
    BOOST_CHECK_EQUAL( sman_ptr->filename_base(), spath );
    // Try to find input before generating file
    try
    {
      std::string fn;
      fn = sman_ptr->find_input_filename();
      BOOST_ERROR( "Unexpected \"sman_ptr->find_input_filename()\" found file named " + fn );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Unable to find input file(s) named: " + spath + " in section \"simulator\".\n** Check input file name or provide --inputpattern commandline option. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->find_input_filename()\" was not expected type: " ) + err.what() );
    }
  }

}

// Test what happens when input file resolves to directory.
BOOST_AUTO_TEST_CASE( mpi_storage_input_filename_is_dir )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "input%%%%%.inp" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS INPUT FILE NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_filename_base( spath );
    BOOST_CHECK_EQUAL( sman_ptr->filename_base(), spath );
    boost::filesystem::create_directories( ipath );
    try
    {
      std::string fn;
      fn = sman_ptr->find_input_filename();
      BOOST_ERROR( "Unexpected \"sman_ptr->find_input_filename()\" found file named " + fn );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Unable to find input file(s) named: " + spath + " in section \"simulator\".\n** Check input file name or provide --inputpattern commandline option. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->find_input_filename()\" was not expected type: " ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove_all( ipath );
    }
  }
  

}

// Test what happens when input file not accessible,
BOOST_AUTO_TEST_CASE( mpi_storage_input_filename_is_not_accessible )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "input%%%%%.inp" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS INPUT FILE NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_filename_base( spath );
    BOOST_CHECK_EQUAL( sman_ptr->filename_base(), spath );
    // Try to find input after generating inaccessible file
    try
    {
      {
        std::ofstream tmp( spath );
        tmp << "\n";
      }
      boost::filesystem::permissions( spath, boost::filesystem::no_perms );
      BOOST_CHECK_EQUAL( sman_ptr->find_input_filename(), spath );
      BOOST_ERROR( "Unexpected \"sman_ptr->find_input_filename()\" found file named " + spath );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Unable to find input file(s) named: " + spath + " in section \"simulator\".\n** Check input file name or provide --inputpattern commandline option. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->find_input_filename()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove( ipath );
    }
  }

}

// Test what happens when input file not readable
BOOST_AUTO_TEST_CASE( mpi_storage_input_filename_is_not_readable )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "input%%%%%.inp" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS INPUT FILE NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_filename_base( spath );
    BOOST_CHECK_EQUAL( sman_ptr->filename_base(), spath );
    // Try to find input after generating inaccessible file
    try
    {
      {
        std::ofstream tmp( spath );
        tmp << "\n";
      }
      boost::filesystem::permissions( spath, boost::filesystem::owner_write );
      BOOST_CHECK_EQUAL( sman_ptr->find_input_filename(), spath );
      BOOST_ERROR( "Unexpected \"sman_ptr->find_input_filename()\" found file named " + spath );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Unable to find input file(s) named: " + spath + " in section \"simulator\".\n** Check input file name or provide --inputpattern commandline option. **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->find_input_filename()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove( ipath );
    }
  }

}

// Test what happens when input file not readable
BOOST_AUTO_TEST_CASE( mpi_storage_open_output_output_directory_is_not_directory )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output\%\%\%\%\%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      {
        boost::filesystem::create_directory( ipath );
        std::ofstream tmp( spath + "/000" );
        tmp << "\n";
      }
      auto os_str = sman_ptr->open_output();
      BOOST_REQUIRE( os_str );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_output()\" found file named " + spath );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "The filesystem object at target output directory path \"" + spath + "/000\" is not a directory in section \"simulator\".\n** Delete filesystem object or set/change parameter \"outputdir\". **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_output()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir not readable
BOOST_AUTO_TEST_CASE( mpi_storage_open_output_output_directory_is_not_accessible )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output\%\%\%\%\%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::no_perms );
      auto os_str = sman_ptr->open_output();
      BOOST_REQUIRE( os_str );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_output()\" found file named " + spath );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Problem openning output file " + spath + "/000/" + sman_ptr->output_name() ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_output()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_all );
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir only writable
BOOST_AUTO_TEST_CASE( mpi_storage_open_output_output_directory_is_0W0 )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_write );
      auto os_str = sman_ptr->open_output();
      BOOST_REQUIRE( os_str );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_output()\" found file named " + spath );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Problem openning output file " + spath + "/000/" + sman_ptr->output_name() ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_output()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_all );
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir only readable + writable
BOOST_AUTO_TEST_CASE( mpi_storage_open_output_output_directory_is_RW0 )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_write | boost::filesystem::owner_read );
      auto os_str = sman_ptr->open_output();
      BOOST_REQUIRE( os_str );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_output()\" found file named " + spath );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Problem openning output file " + spath + "/000/" + sman_ptr->output_name() ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_output()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir only exec + writable (PASSES)
BOOST_AUTO_TEST_CASE( mpi_storage_open_output_output_directory_is_0WX )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_write | boost::filesystem::owner_exe );
      auto os_str = sman_ptr->open_output();
      BOOST_REQUIRE( os_str );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_output()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_read | boost::filesystem::owner_write | boost::filesystem::owner_exe );
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir only readable + writable
BOOST_AUTO_TEST_CASE( mpi_storage_open_output_output_directory_is_R0X )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_exe | boost::filesystem::owner_read );
      auto os_str = sman_ptr->open_output();
      BOOST_REQUIRE( os_str );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_output()\" found file named " + spath );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Problem openning output file " + spath + "/000/" + sman_ptr->output_name() ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_output()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when input file not readable
BOOST_AUTO_TEST_CASE( mpi_storage_open_checkpoint_output_directory_is_not_directory )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      {
        boost::filesystem::create_directory( ipath );
        std::ofstream tmp( spath + "/000" );
        tmp << "\n";
      }
      auto os_str = sman_ptr->open_checkpoint();
      BOOST_REQUIRE( *(os_str) );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_checkpoint()\" found file named " + spath );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "The filesystem object at target output directory path \"" + spath + "/000\" is not a directory in section \"simulator\".\n** Delete filesystem object or set/change parameter \"outputdir\". **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_checkpoint()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir not readable
BOOST_AUTO_TEST_CASE( mpi_storage_open_checkpoint_output_directory_is_not_accessible )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::no_perms );
      auto os_str = sman_ptr->open_checkpoint();
      BOOST_REQUIRE( *(os_str) );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_checkpoint()\" found file named " + spath );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Problem openning checkpoint file " + spath + "/000/" + sman_ptr->checkpoint_name() ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_checkpoint()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_all );
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir only writable
BOOST_AUTO_TEST_CASE( mpi_storage_open_checkpoint_output_directory_is_0W0 )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_write );
      auto os_str = sman_ptr->open_checkpoint();
      BOOST_REQUIRE( *(os_str) );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_checkpoint()\" found file named " + spath );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Problem openning checkpoint file " + spath + "/000/" + sman_ptr->checkpoint_name() ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_checkpoint()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_all );
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir only readable + writable
BOOST_AUTO_TEST_CASE( mpi_storage_open_checkpoint_output_directory_is_RW0 )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_write | boost::filesystem::owner_read );
      auto os_str = sman_ptr->open_checkpoint();
      BOOST_REQUIRE( *(os_str) );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_checkpoint()\" found file named " + spath );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Problem openning checkpoint file " + spath + "/000/" + sman_ptr->checkpoint_name() ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_checkpoint()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir only exec + writable (PASSES)
BOOST_AUTO_TEST_CASE( mpi_storage_open_checkpoint_output_directory_is_0WX )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_write | boost::filesystem::owner_exe );
      auto os_str = sman_ptr->open_checkpoint();
      BOOST_REQUIRE( *(os_str) );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_checkpoint()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_read | boost::filesystem::owner_write | boost::filesystem::owner_exe );
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test what happens when output dir only readable + writable
BOOST_AUTO_TEST_CASE( mpi_storage_open_checkpoint_output_directory_is_R0X )
{
  boost::shared_ptr< platform::mpi_storage > sman_ptr( new platform::mpi_storage );
  {
    auto ipath( boost::filesystem::unique_path( "output%%%%%" ) );
    const std::string spath( ipath.native() );
  
    // USE PATH AS OUTPUT DIR NAME
    //////////////////////////////
    BOOST_REQUIRE( not boost::filesystem::exists( ipath ) );
    sman_ptr->set_output_dir_fmt( spath );
    BOOST_CHECK_EQUAL( sman_ptr->output_dir_fmt(), spath );
    // Try to get directory after generating file with same name
    try
    {
      boost::filesystem::create_directories( ipath / "000" );
      boost::filesystem::permissions( ipath / "000", boost::filesystem::owner_exe | boost::filesystem::owner_read );
      auto os_str = sman_ptr->open_checkpoint();
      BOOST_REQUIRE( *(os_str) );
      BOOST_ERROR( "Unexpected \"sman_ptr->open_checkpoint()\" found file named " + spath );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      // std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Problem openning checkpoint file " + spath + "/000/" + sman_ptr->checkpoint_name() ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"sman_ptr->open_checkpoint()\" was not expected:" ) + err.what() );
    }
    if( boost::filesystem::exists( ipath ) )
    {
      boost::filesystem::remove_all( ipath );
    }
  }

}

// Test combination of storage_meta and simple_storage_manager.
BOOST_AUTO_TEST_CASE( run_input_test )
{
  core::input_delegater dg( 1 );
  const std::string type_label( "test" );
  const std::string cname( "checkpoint.arc" );
  const std::string test_cname( "check.dat" );
  const std::string fbase( "input.\%1$03d.inp" );
  const std::string test_fbase( "channel.\%1$04d.inp" );
  const std::string dbase( "\%1$03d" );
  const std::string outpath( "001" );
  const std::string outdir( "001/" );
  const std::string test_dbase( "\%1$04d.dat" );
  const std::string test_outpath( "5999.dat" );
  const std::string test_outdir( "5999.dat/" );
  const std::string oname( "output.dbm" );
  const std::string test_oname( "result.zip" );
  boost::shared_ptr< std::stringstream > log( new std::stringstream );
  boost::shared_ptr< platform::storage_manager > fstype( new platform::mpi_storage );
  fstype->set_log( log );
  {
    boost::shared_ptr< platform::storage_meta > meta( new platform::storage_meta( fstype ) );
    platform::mpi_storage::add_definition( *meta );
    // no extra parameters
    BOOST_REQUIRE( meta->has_type( "mpi" ) );
  
    dg.add_input_delegate( meta );
  }
  
  BOOST_CHECK_EQUAL( fstype->checkpoint_name(), cname );
  BOOST_CHECK_EQUAL( fstype->filename_base(), fbase );
  BOOST_CHECK_EQUAL( fstype->output_dir_fmt(), dbase );
  BOOST_CHECK_EQUAL( fstype->output_name(), oname );
   
  // Should be valid input
  /////////////////////////////
  std::string canon_input( "run\ntype \"mpi\"\ninput \"channel.\%1$04d.inp\"\noutputdir \"\%1$04d.dat\"\ncheckname check.dat\noutname result.zip\nend\n\n" );
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
  
  BOOST_CHECK_EQUAL( fstype->checkpoint_name(), test_cname );
  BOOST_CHECK_EQUAL( fstype->filename_base(), fbase );
  BOOST_CHECK_EQUAL( fstype->output_dir_fmt(), test_dbase );
  BOOST_CHECK_EQUAL( fstype->output_name(), test_oname );
  const std::string slog( log->str() );
  BOOST_CHECK( slog.find( fbase ) < slog.size() );
  BOOST_CHECK( slog.find( test_fbase ) < slog.size() );
  //std::cout << "\n" << slog << "\n";

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( run_input_type_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< platform::storage_manager > fstype( new platform::mpi_storage );
  {
    boost::shared_ptr< platform::storage_meta > meta( new platform::storage_meta( fstype ) );
    platform::mpi_storage::add_definition( *meta );
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "run\ntype #\"mpi\"\noutdir \"%1$07d\"\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Run subtype parameter \"type\" in section \"run\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>type #\"mpi\"<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( run_input_type_bad_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< platform::storage_manager > fstype( new platform::mpi_storage );
  {
    boost::shared_ptr< platform::storage_meta > meta( new platform::storage_meta( fstype ) );
    platform::mpi_storage::add_definition( *meta );
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "run\ntype \"standard\"\ninput \"channel.\%1$04d.inp\"\noutputdir \"\%1$04d.dat\"\ncheckname check.dat\noutname result.zip\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Run subtype parameter \"type\" with value (\"standard\") in section \"run\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>type \"standard\"<<\n** A value from the list (mpi) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( run_input_duplicate_type_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< platform::storage_manager > fstype( new platform::mpi_storage );
  {
    boost::shared_ptr< platform::storage_meta > meta( new platform::storage_meta( fstype ) );
    platform::mpi_storage::add_definition( *meta );
    dg.add_input_delegate( meta );
  }
  // Type is repeated
  /////////////////////////////
  std::string canon_input( "run\ntype \"mpi\"\ntype \"mpi\"\ninput \"channel.\%1$04d.inp\"\noutputdir \"\%1$04d.dat\"\ncheckname check.dat\noutname result.zip\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Parameter \"type\" appears more than once in section \"run\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>type \"mpi\"<<\n** Parameter \"type\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( run_input_checkname_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< platform::storage_manager > fstype( new platform::mpi_storage );
  {
    boost::shared_ptr< platform::storage_meta > meta( new platform::storage_meta( fstype ) );
    platform::mpi_storage::add_definition( *meta );
    dg.add_input_delegate( meta );
  }
  // Checkname is hidden by comment character
  /////////////////////////////
  std::string canon_input( "run\ntype \"mpi\"\ninput \"channel.\%1$04d.inp\"\noutputdir \"\%1$04d.dat\"\ncheckname #check.dat\noutname result.zip\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Checkpoint filename parameter \"checkname\" in section \"run\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>checkname #check.dat<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( run_input_outname_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< platform::storage_manager > fstype( new platform::mpi_storage );
  {
    boost::shared_ptr< platform::storage_meta > meta( new platform::storage_meta( fstype ) );
    platform::mpi_storage::add_definition( *meta );
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "run\ntype \"mpi\"\ninput \"channel.\%1$04d.inp\"\noutputdir \"\%1$04d.dat\"\ncheckname check.dat\noutname #result.zip\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Output filename parameter \"outname\" in section \"run\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>outname #result.zip<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( run_input_outdir_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< platform::storage_manager > fstype( new platform::mpi_storage );
  {
    boost::shared_ptr< platform::storage_meta > meta( new platform::storage_meta( fstype ) );
    platform::mpi_storage::add_definition( *meta );
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "run\ntype \"mpi\"\ninput \"channel.\%1$04d.inp\"\noutputdir #\"\%1$04d.dat\"\ncheckname check.dat\noutname result.zip\nend\n\n" );
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
    // std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Output directory parameter \"outputdir\" in section \"run\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>outputdir #\"\%1$04d.dat\"<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( run_input_inputpattern_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< platform::storage_manager > fstype( new platform::mpi_storage );
  {
    boost::shared_ptr< platform::storage_meta > meta( new platform::storage_meta( fstype ) );
    platform::mpi_storage::add_definition( *meta );
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "run\ntype \"mpi\"\ninput #\"channel.\%1$04d.inp\"\noutputdir \"\%1$04d.dat\"\ncheckname check.dat\noutname result.zip\nend\n\n" );
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
    // std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Input file pattern parameter \"input\" in section \"run\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>input #\"channel.\%1$04d.inp\"<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
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
  char a6[] { "--seed" };
  char a7[] { "212121" };
  char* argv[] { &a1[0], &a2[0], &a3[0], &a4[0], &a5[0], &a6[0], &a7[0] };
  int argc = 7;
  
  auto getseed = [](std::string logname)->size_t
  {
    std::ifstream is( logname );
    for (std::string line; std::getline(is, line); )
    {
      if( line.find("Random seed value :") != std::string::npos )
      {
        std::string l{ line.substr(line.find(':') + 1) };
        return std::stoul( l );
      }
    }
    return 0ul;
  };
  
  
  if( boost::filesystem::exists( "0002" ) )
  {
    boost::filesystem::remove_all( "0002" );
  }
  if( boost::filesystem::exists( a3 ) )
  {
    boost::filesystem::remove( a3 );
  }
  boost::shared_ptr< platform::mpi_storage > stor( new platform::mpi_storage );
  {
    {
      const std::string input_file_contents
      {
        "simulator\ntype standard\ninner 10\nname Test simulation\nnstep 200\nnaver 100\nisave 50\nntarg 50\nepsw 70.0\nkelvin 305.0\nend\n\nrun\ntype standard\noutputdir \"\%1$04d\"\nend\n\nevaluator\ntype coulomb\nend\n\nsampler\ntype specie-count\nend\n\nsampler\ntype metropolis\nend\n\nsampler\ntype widom\niwidom 10\nend\n\nregion\ntype cube\nname centre\nwidth 5\norigin 2.5 2.5 2.5\nend\n\nspecie\nname Na\nz 1\nd 1.01\nctarg 0.1\nend\n\nspecie\nname Cl\nz -1\nd 1.78\nctarg 0.1\nend\n\ntrial\nrate 0.5\ntype move\ndelta 1.0\nend\n\ntrial\nrate 0.5\ntype jump\nend\n"
      };
      std::ofstream out( a3 );
      out.write( input_file_contents.data(), input_file_contents.size() );
    }
    boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 10.0 ) );
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
    BOOST_REQUIRE( boost::filesystem::exists( "0002/000/checkpoint.arc" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0002/000/output.dbm" ) );
    BOOST_REQUIRE( boost::filesystem::exists( "0002/000/log" ) );
    if( stor->size() > 1 )
    {
      BOOST_REQUIRE( boost::filesystem::exists( "0002/001/checkpoint.arc" ) );
      BOOST_REQUIRE( boost::filesystem::exists( "0002/001/output.dbm" ) );
      BOOST_REQUIRE( boost::filesystem::exists( "0002/001/log" ) );
      std::size_t sd1, sd2;
      sd1 = getseed( "0002/000/log" );
      sd2 = getseed( "0002/001/log" );
      BOOST_CHECK_EQUAL( sd1 + 1, sd2 );
    }
    try
    {
      //std::cout << "RANK " << stor->rank() << "\n";
      observable::gdbm_sink db( stor->compute_output_dir() + "/output.dbm", observable::base_sink::READ_ONLY );
      BOOST_REQUIRE( db.exists( "metropolis.dat" ) );
      BOOST_REQUIRE( db.exists( "specie-count.dat" ) );
      BOOST_REQUIRE( db.exists( "widom.dat" ) );
    }
    catch( std::exception const& err )
    {
      std::cout << err.what() << "\n";
      BOOST_ERROR( std::string( "Unexpected exception working on db \"0002/000/output.dbm\": " ) + err.what() );
    }
  }
  stor->barrier();
  if( stor->rank() == 0 )
  {
    if( boost::filesystem::exists( "0002" ) )
    {
      boost::filesystem::remove_all( "0002" );
    }
    if( boost::filesystem::exists( a3 ) )
    {
      boost::filesystem::remove( a3 );
    }
  }

}


} // namespace platform
