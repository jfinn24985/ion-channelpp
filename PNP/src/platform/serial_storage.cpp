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

#include "platform/serial_storage.hpp"
#include "utility/archive.hpp"

#include "core/input_document.hpp"
#include "core/input_parameter_memo.hpp"
#include "platform/simulation.hpp"
#include "observable/base_sink.hpp"
#include "core/input_base_reader.hpp"
#include "platform/storage_meta.hpp"

// manuals
#include "core/input_error.hpp"
#include "core/input_reader.hpp"
#include "core/strngs.hpp"
// -
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/traits.hpp>
#include <fstream>
// -
namespace platform {

serial_storage::serial_storage()
: storage_manager()
{}
serial_storage::~serial_storage()
{}

// Details about the current storage object.
void serial_storage::do_description(std::ostream & os) const 
{}

// Write extra data of derived simulator object into the input
// document object at the given section.
void serial_storage::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );

}

// Throw an exception as this manager takes no extra
// parameters.
void serial_storage::set_parameters(const std::vector< core::input_parameter_memo >& params)
{
  UTILITY_ALWAYS( params.size() == 1, "serial storage class has no extra parameters." );
}

// Computer the output directory for the current simulator object
// from the output_dir_fmt template.
std::string serial_storage::compute_output_dir() const
{
  boost::format res( this->output_dir_fmt() );
  // Ignore error of too many arguments.
  res.exceptions( boost::io::all_error_bits ^ boost::io::too_many_args_bit );
  res % this->run_number();
  return res.str();

}

bool serial_storage::main(int argc, char ** argv, simulation & sim)
{
  // NOTE: The input decoders are not required to check that
  // the final system is valid.
  try
  {
    switch( this->process_command_line( argc, argv ) )
    {
    case 0:
      // Bad command line options.
      // (help message already written to log)
      this->system_log() << this->get_log().rdbuf();
      return false;
    case 1:
    {
      // Help message requested.
      const std::string nul_string{};
      this->system_log() << this->get_log().rdbuf();
      sim.generate_help( nul_string, nul_string, this->system_log() );
      return true;
    }
    case 2:
    {
      // Read input file.
      {
        auto reader = this->open_input();
        this->get_log() << core::strngs::horizontal_bar() << "\n"
          << "Reading input file \""
          << reader->current_filename() << "\"\n";
        sim.read_input( *reader );
      }
      // Now our log is well defined, so write temporary log out.
      this->cutover_log();
      this->get_log() << core::strngs::horizontal_bar() << "\n";
      sim.set_random_seed( this->get_seed_value() );
      sim.generate_simulation();
      break;
    }
    case 3:
    {
      // Restart from checkpoint
      if( not sim.restart() )
      {
        this->system_log() << this->get_log().rdbuf() << "\n";
        return false;
      }
      // Now our log is well defined, so write temporary log out.
      this->cutover_log();
      this->get_log() << core::strngs::horizontal_bar() << "\n";
      break;
    }
    default:
      {
      // Logic error
        UTILITY_CHECK( false, "Should never get here!" );
        break;
      }
    }
    // Initialisation from input complete. (We could still get some
    // input errors after this point during call to 'prepare'.)
    return sim.main();
  }
  catch(const core::input_error &err)
  {
    // flush cached log
    if( not this->is_cutover() )
    {
      this->system_log() << this->get_log().rdbuf() << "\n";
    }
    this->system_log() << err.what() << "\n";
    sim.generate_help( err.section(), err.parameter(), this->system_log() );
    return false;
  }
  catch(const std::runtime_error &err)
  {
    // Crashing...
    if( not this->is_cutover() )
    {
      this->system_log() << this->get_log().rdbuf() << "\n";
    }
    this->system_log() << err.what() << "\n";
    return false;
  }

}

// Create/open the simulation's output sink.
//
// If extension is not given then a sink of type gdbm_sink
// is returned.
boost::shared_ptr< observable::base_sink > serial_storage::open_output()
{
  this->ensure_output_dir();
  const std::string fn( this->output_path() );
  {
    // attempt to open file in 
    std::ofstream test( fn );
    UTILITY_ALWAYS( test, "Problem openning output file "+fn );
  }
  return this->open_sink_by_extension( fn, "dbm" );

}

// Create/open the simulation's input stream.
//
// This uses find_input_filename to identify the
// input file then opens it using a input_reader
// type reader object.
//
// \throw as per find_input_filename
boost::shared_ptr< core::input_base_reader > serial_storage::open_input()
{
  const std::string inputfn = this->find_input_filename();
  boost::shared_ptr< core::input_base_reader > reader( new core::input_reader( inputfn ) );
  return reader;

}

// Create/open the simulation's checkpoint stream.
boost::shared_ptr< std::ostream > serial_storage::open_checkpoint()
{
  this->ensure_output_dir();
  const std::string fn( this->checkpoint_path() );
  boost::shared_ptr< std::ostream > result( new std::ofstream( fn ) );
  UTILITY_ALWAYS( *(result), "Problem openning checkpoint file "+fn );
  return result;

}

// Create the system's log output stream. In this case
// the log file is to standard out.
boost::shared_ptr< std::ostream > serial_storage::open_log()
{
  std::unique_ptr< boost::iostreams::filtering_ostream > tmp( new boost::iostreams::filtering_ostream );
  tmp->push( std::cout );
  boost::shared_ptr< std::ostream > result( tmp.release() );
  return result;

}

void serial_storage::add_definition(storage_meta & meta)
{
  std::string desc( "Use local storage and serial (non-parallel) execution." );
  std::unique_ptr< storage_definition > result( new storage_definition( type_label_(), desc ) );
  // no extra parameters
  meta.add_type( result );

}

// The string "local" used to identify this storage
// manager in the input file.
std::string serial_storage::type_label_()
{
  const std::string result( "standard" );
  return result;

}


} // namespace platform

BOOST_CLASS_EXPORT_IMPLEMENT( platform::serial_storage );
