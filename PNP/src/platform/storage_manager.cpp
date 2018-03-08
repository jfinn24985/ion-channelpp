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

#include "platform/storage_manager.hpp"
#include "utility/archive.hpp"

#include "platform/simulation.hpp"
#include "core/input_document.hpp"
#include "core/input_parameter_memo.hpp"
#include "core/input_base_reader.hpp"
#include "core/input_delegater.hpp"

// manual includes
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "observable/archive_file.hpp"
#include "observable/base_sink.hpp"
#include "observable/gdbm_sink.hpp"
#include "observable/memory_sink.hpp"
#include "platform/serial_storage.hpp"
#include "platform/storage_meta.hpp"
// -
#include <boost/program_options.hpp>
#include <boost/random/random_device.hpp>
#include <fstream>
// -
namespace platform {

storage_manager::storage_manager()
: checkpoint_name_( storage_manager::default_checkpoint_name() )
, filename_base_( storage_manager::default_filename_base() )
, output_dir_fmt_( storage_manager::default_output_dir_fmt() )
, output_name_( storage_manager::default_output_name() )
, run_number_( storage_manager::default_run_number() )
, seed_value_()
, log_( new std::stringstream )
, cutover_log_( false )
{}
storage_manager::~storage_manager()
{}

// Get the default name of the checkpoint file
std::string storage_manager::default_checkpoint_name()
{
  static const std::string result( "checkpoint.arc" );
  return result;
}

// Get the default template for the input filename.
std::string storage_manager::default_filename_base()
{
  static const std::string result( "input.\%1$03d.inp" );
  return result;
}

// Get the default name of the output file
std::string storage_manager::default_output_name()
{
  static const std::string result( "output.dbm" );
  return result;
}

std::string storage_manager::default_output_dir_fmt()
{
  static const std::string result( "\%1$03d" );
  return result;
}

// Details about the current simulation to be written to the
// log at the start of the simulation.
void storage_manager::description(std::ostream & os) const 
{
  os << core::strngs::horizontal_bar() << "\n";
  os << " Simulation Runtime Settings\n";
  os << "----------------------------\n";
  os << "                 run index : " << this->run_number() << "\n";
  os << " output directory template : " << this->output_dir_fmt() << "\n";
  os << "    input filename pattern : " << this->filename_base() << "\n";
  os << "      checkpoint file name : " << this->checkpoint_name() << "\n";
  os << "          output file name : " << this->output_name() << "\n";

}

// Return the simulations log output stream.  If no
// log stream is currently open then this calls open_log.
// Therefore this may throw any exception as open_log.
std::ostream & storage_manager::get_log()
{
  if( not this->log_ )
  {
    this->log_ = this->open_log();
  }
  return *(this->log_);

}

// Generate a random value to seed the random number generator.
//
// This uses an operating system provided random source to generate
// a single random unsigned int suitable to seed the psuedo-random
// number generator. 

uint32_t storage_manager::random_seed_value()
{
  // Get random seed value from system's random device:
  // requires #include <boost/random/random_device.hpp>
  boost::random::random_device seedgenr;
  return seedgenr() - seedgenr.min();

}

// Return a reference to the system error log.
//
// On UNIX systems this will be the standard error stream.
std::ostream & storage_manager::system_log() const
{
  return std::cerr;
}

// Write contents of simulation as an input document
void storage_manager::write_document(core::input_document & wr) 
{
  // Add storage information section
  std::size_t ix = wr.add_section( platform::storage_meta::storage_label() );
  auto &sec = wr[ ix ];
  sec.add_entry( core::strngs::fstype(), this->get_label() );
  sec.add_entry( core::strngs::outputdir_label(), this->output_dir_fmt() );
  sec.add_entry( core::strngs::inputpattern_label(), this->filename_base() );
  sec.add_entry( platform::storage_meta::output_name_label(), this->output_name() );
  sec.add_entry( platform::storage_meta::checkpoint_name_label(), this->checkpoint_name() );
  this->do_write_document( wr, ix );

}

// Signal that we can switch from in-memory log to output log.
void storage_manager::cutover_log()
{
  if( not this->cutover_log_ )
  {
    boost::shared_ptr< std::ostream > newlog{ this->open_log() };
    if( newlog )
    {
      std::copy( std::istreambuf_iterator<char>( this->log_->rdbuf() ),
                 std::istreambuf_iterator<char>(),
                 std::ostreambuf_iterator<char>( newlog->rdbuf() ) );
      std::swap( this->log_, newlog );
      this->cutover_log_ = true;
    }
  }

}

// Set the name of the checkpoint file.
//
// \require not fn.empty
void storage_manager::set_checkpoint_name(std::string fn)
{
  UTILITY_REQUIRE( not fn.empty(), "Cannot set checkpoint name to an empty string." );
  this->checkpoint_name_ = fn;
}

// Set the template/regular expression for deriving input filenames.
void storage_manager::set_filename_base(std::string fmt)
{
  UTILITY_REQUIRE( not fmt.empty(), "Cannot set checkpoint name to an empty string." );
  this->filename_base_ = fmt;
}

// Function to set log to an externally defined output stream.
//
// \return The previous stream.
boost::shared_ptr< std::ostream > storage_manager::set_log(boost::shared_ptr< std::ostream > os_ptr)
{
  if( not this->cutover_log_ )
  {
    std::copy( std::istreambuf_iterator<char>( this->log_->rdbuf() ),
               std::istreambuf_iterator<char>(),
               std::ostreambuf_iterator<char>( os_ptr->rdbuf() ) );
    this->cutover_log_ = true;
  }
  boost::shared_ptr< std::ostream > tmp{ this->log_ };
  this->log_ = os_ptr;
  return tmp;

}

//Set template string used to create directory path for 
//output files.
//
//Formatting is based on Boost Format so the string can
//be in Boost Format, Posix printf or printf style.
void storage_manager::set_output_dir_fmt(std::string fmt)
{
  UTILITY_REQUIRE( not fmt.empty(), "Cannot set checkpoint name to an empty string." );
  this->output_dir_fmt_ = fmt;
}

// Set the name of the output file. If an extension is
// given it should be used to select the file type and
// use an appropriate base_sink subclass.
//
// \require has_valid_extension( fmt )
void storage_manager::set_output_name(std::string fmt)
{
  UTILITY_REQUIRE( not fmt.empty(), "Cannot set output file to an empty string." );
  UTILITY_REQUIRE( has_valid_extension( fmt ), "Output file extension is not one of (zip,dbm,mem)" );
  this->output_name_ = fmt;

}

// Test the name of the output file to see if it has a 
// known extension. (Having no extension is interpreted
// as using the default extension and is therefore also
// true)
bool storage_manager::has_valid_extension(std::string fmt)
{
  boost::filesystem::path fpath( fmt );
  std::string ext = fpath.extension().native();
  return ( ext.empty() or ext.find( "zip" ) < ext.size() or ext.find( "dbm" ) < ext.size() or ext.find( "mem" ) < ext.size() );

}

// Use file extension (or default extension) to open a sink object.
//
// \pre has_valid_extension( path.native )
boost::shared_ptr< observable::base_sink > storage_manager::open_sink_by_extension(const boost::filesystem::path & fpath, std::string def_ext)
{
  std::string ext = fpath.extension().native();
  boost::shared_ptr< observable::base_sink > result; 
  if ( ext.empty() )
  {
    ext = def_ext;
  }
  if( ext.find( "zip" ) < ext.size() )
  {
    // ZIP archive
    result.reset( new observable::archive_file( fpath.native() ) );
  }
  else if( ext.find( "dbm" ) < ext.size() )
  {
    // GDBM/DBM archive
    result.reset( new observable::gdbm_sink( fpath.native() ) );
  }
  else if( ext.find( "mem" ) < ext.size() )
  {
    // in-memory sink
    result.reset( new observable::memory_sink( fpath.native() ) );
  }
  return result;

}

// Generate path to the checkpoint file.
//
// Default implementation :
//
// \return compute_output_dir + checkpoint_name
//
// \pre not checkpoint_name.empty

std::string storage_manager::checkpoint_path() const
{
  boost::filesystem::path cpath( this->compute_output_dir() );
  cpath /= this->checkpoint_name();
  return cpath.native();

}

// Ensure the output directory exists, creating if necessary.
// Return the path to the directory. Throws error if output
// directory path exists but is not a directory.

boost::filesystem::path storage_manager::ensure_output_dir()
{
  const boost::filesystem::path dpath( this->compute_output_dir() );
  if( not boost::filesystem::exists( dpath ) )
  {
    boost::filesystem::create_directories( dpath );
  }
  else
  {
    if( not boost::filesystem::is_directory( dpath ) )
    {
      throw core::input_error::input_logic_error( "The filesystem object at target output directory path \""+dpath.native()+"\" is not a directory", core::strngs::simulator_label(), "Delete filesystem object or set/change parameter \"" + core::strngs::outputdir_label() + "\"." );
    }
  }
  return dpath;

}

// Generate a series of possible filenames and return
// the first filename that is found. Throw an error if no
// filenames are found. The template string is processed
// using 'printf' or boost::format like function.
//
// For a filename_base of "input.%1$03d.inp" and run number
// of 11 the filenames checked for will be:
//
//  * input.011.inp
//
//  * input.inp
//
// If no "%" is present it assumes the string is directly
// the filename.
//
// \post exists(return val)

std::string storage_manager::find_input_filename() const
{
  // Filenames to generate
  //   input.%03d.inp % run_index
  //   input.inp
  std::string result;
  std::string errmsg( "Unable to find input file(s) named:" );
  
  auto test_fn = [](std::string fn)->bool{
   return boost::filesystem::exists( fn )
     and boost::filesystem::is_regular_file( fn )
     and std::ifstream( fn );
  };
    
  if( not this->filename_base().empty() )
  {
    // Check for '%' to see if filename base is a format string
    if( std::string::npos != this->filename_base().find( '%' ) )
    {
      // TRY ONE : filename base is format string
      // -------
      // Assume filename_base is a format string.
      boost::format res( this->filename_base() );
      // Ignore error of too many arguments.
      res.exceptions( boost::io::all_error_bits ^ boost::io::too_many_args_bit );
      res % this->run_number();
      result = res.str();
      // Check if file exists
      if( test_fn( result ) )
      {
        return result;
      }
      else
      {
        // Filename not found
        errmsg.append( " " );
        errmsg.append( result );
      }
      // TRY TWO : remove format code from filename base
      // -------
      result.assign( this->filename_base() );
      // Remove everything from the '%' to the next '.'
      std::size_t pos1 = result.find( '%' ); // We know this is not npos!
      std::size_t pos2 = result.find( '.', pos1 );
      if( std::string::npos != pos2 )
      {
        result.erase( pos1, pos2 - pos1 + 1 );
      }
      else
      {
        result.erase( pos1 );
      }
      // Check if file exists
      if( test_fn( result ) )
      {
        return result;
      }
      else
      {
        // Filename not found
        errmsg.append( " " );
        errmsg.append( result );
      }
    }
    else
    {
      // TRY THREE : filename base is simple string
      // ---------
      result = this->filename_base();
      // Check if file exists
      if( test_fn( result ) )
      {
        return result;
      }
      else
      {
        // Filename not found
        errmsg.append( " " );
        errmsg.append( result );
      }
    }
  }
  // If we got here no files where found.
  throw core::input_error::input_logic_error( errmsg, core::strngs::simulator_label(), "Check input file name or provide --inputpattern commandline option." );
  result.clear();
  return result; // To stop compiler complaints
  

}

// Generate path to the output file.
//
// Default implementation :
//
// \return compute_output_dir + output_name
//
// \pre not checkpoint_name.empty

std::string storage_manager::output_path() const
{
  boost::filesystem::path cpath( this->compute_output_dir() );
  cpath /= this->output_name();
  return cpath.native();

}

// Adds factory methods and input parsers for all the types that can be instantiated from 
// the input file.
void storage_manager::build_input_delegater(boost::shared_ptr< storage_manager > fstype, core::input_delegater & delegate)
{
  // build storage manager meta and add to delegater
  boost::shared_ptr< platform::storage_meta > fsmeta { new platform::storage_meta( fstype ) };
  
  // Monitor trial occurence and success
  platform::serial_storage::add_definition( *fsmeta );
  
  
  delegate.add_input_delegate( fsmeta );

}

// Process the command line.
//
// return is one of:
//
// * bad options (0)
//
// * help (1)
//
// * input (2)
//
// * restart (3)
// 

int storage_manager::process_command_line(int argc, char ** argv)
{
  if( argc > 1 )
  {
    namespace po = boost::program_options;
    po::options_description cmdln_opts( "Allowed Options" );
    cmdln_opts.add_options()
    ( "run", po::value<unsigned int>(), "Simulation run number (positive integer)." )
    ( core::strngs::inputpattern_label().c_str(), po::value<std::string>(), "Plain or printf style format string for generating input filename" )
    ( "seed", po::value<unsigned int>(), "Simulation random seed value (positive integer)." )
    ( "restart", "Restart simulation (from checkpoint)" )
    ( core::strngs::outputdir_label().c_str(), po::value<std::string>(), "Output directory (may be name or format string)" )
    ( "help", "Produce help message" );
    po::variables_map vm;
    try
    {
      po::store( po::parse_command_line( argc, argv, cmdln_opts ), vm );
      po::notify( vm );
    }
    catch( std::exception &err )
    {
      this->get_log() << "Error : " << err.what() << "\n"
                         << cmdln_opts << "\n";
      return 0;
    }
  
    // After here, 'vm' will contain typed values as
    // specified via po::value<> statement or an error
    // would have been raised.
    if( vm.count( "help" ) )
    {
      this->get_log() << cmdln_opts << "\n";
      return 1;
    }
  
    if( vm.count( "restart" ) )
    {
      // Restart, ignore all other options.
      // ----------------------------------
      if( vm.count( "seed" ) )
      {
        this->get_log() << core::strngs::horizontal_bar() << "\n"
            << "Random seed value on command line is ignored with restart.\n";
      }
      if( vm.count( core::strngs::inputpattern_label() ) )
      {
        this->get_log() << core::strngs::horizontal_bar() << "\n"
            << "Input filename on command line is ignored with restart.\n";
      }
      if( vm.count( core::strngs::outputdir_label() ) )
      {
        this->set_output_dir_fmt( vm[core::strngs::outputdir_label()].as<std::string>() );
      }
      if( vm.count( "run" ) )
      {
        this->set_run_number( vm["run"].as<unsigned int>() );
      }
      return 3;
    }
    else
    {
      // Process command line options.
      // -----------------------------
      if( vm.count( core::strngs::inputpattern_label() ) )
      {
        this->set_filename_base( vm[core::strngs::inputpattern_label()].as<std::string>() );
      }
      if( vm.count( "run" ) )
      {
        this->set_run_number( vm["run"].as<unsigned int>() );
      }
      if( vm.count( core::strngs::outputdir_label() ) )
      {
        this->set_output_dir_fmt( vm[core::strngs::outputdir_label()].as<std::string>() );
      }
  
      // set random generator seed
      this->set_seed_value( vm.count( "seed" ) ? this->compute_seed( vm["seed"].as<unsigned int>() ) : this->random_seed_value() );
      this->get_log() << core::strngs::horizontal_bar()
          << "\nRandom seed value : " << this->get_seed_value() << ".\n";
      return 2;
    }
  }
  else
  {
    // No command line arguments, using defaults.
    // ------------------------------------------
  
    // set random generator seed
    this->set_seed_value( this->random_seed_value() );
    this->get_log() << core::strngs::horizontal_bar()
        << "\nRandom seed value : " << this->get_seed_value() << ".\n";
    return 2;
  }

}


} // namespace platform
