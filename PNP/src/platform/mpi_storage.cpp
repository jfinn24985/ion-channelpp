

#ifndef DEBUG
#define DEBUG 0
#endif

#include "platform/mpi_storage.hpp"
#include "utility/archive.hpp"

#include "core/input_document.hpp"
#include "core/input_parameter_memo.hpp"
#include "platform/simulation.hpp"
#include "observable/base_sink.hpp"
#include "core/input_base_reader.hpp"
#include "platform/storage_meta.hpp"

// manuals
#include "core/input_error.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
// -
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/mpi.hpp>
#include <cstdlib>
#include <fstream>
// -
namespace platform {

mpi_storage::mpi_storage()
: storage_manager()
, rank_(0)
, sibling_count_(1)
, env_()
, world_()
{}
mpi_storage::mpi_storage(std::size_t rank)
: storage_manager()
, rank_( rank )
{}
mpi_storage::~mpi_storage()
{}

// Details about the current storage object.
void mpi_storage::do_description(std::ostream & os) const 
{}

// Write extra data of derived simulator object into the input
// document object at the given section.
void mpi_storage::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );

}

// Throw an exception as this manager takes no extra
// parameters.
void mpi_storage::set_parameters(const std::vector< core::input_parameter_memo >& params)
{
  UTILITY_ALWAYS( 1 == params.size(), "serial storage class has no extra parameters." );
}

// Abort the MPI system with the given error code.
void mpi_storage::abort(int err_num)
{
  this->world_->abort( err_num );
}

// Wait for all other MPI jobs
void mpi_storage::barrier()
{
  if( this->world_ ) this->world_->barrier();
}

// Computer the output directory for the current simulator object
// from the output_dir_fmt template.
std::string mpi_storage::compute_output_dir() const
{
  boost::format res( this->output_dir_fmt() + "/\%2$03d" );
  // Ignore error of too many arguments.
  res.exceptions( boost::io::all_error_bits ^ boost::io::too_many_args_bit );
  res % this->run_number() % this->rank_;
  return res.str();

}

bool mpi_storage::main(int argc, char ** argv, simulation & sim)
{
  // NOTE: The input decoders are not required to check that
  // the final system is valid.
  namespace mpi = boost::mpi;
  // HACK TO TEST FOR OPENMPI SERVER
  if( std::getenv( "OMPI_COMM_WORLD_SIZE" ) )
  {
    const bool abort_on_exception{ true };
    this->env_.reset( new mpi::environment( argc, argv, abort_on_exception ) );
    this->world_.reset( new mpi::communicator );
    this->rank_ = this->world_->rank();
    this->sibling_count_ = this->world_->size();
  }
  try
  {
    switch( this->process_command_line( argc, argv ) )
    {
    case 0:
      // Bad command line options.
      this->system_log() << this->get_log().rdbuf();
      this->world_->abort( 1 );
      return false;
    case 1:
    {
      // Help message requested.
      const std::string nul_string{};
      this->system_log() << this->get_log().rdbuf();
      sim.generate_help( nul_string, nul_string, this->system_log() );
      this->world_->abort( 0 );
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
        this->world_->abort( 1 );
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
    if( not this->is_cutover() )
    {
      this->system_log() << this->get_log().rdbuf() << "\n";
    }
    this->system_log() << err.what() << "\n";
    sim.generate_help( err.section(), err.parameter(), this->system_log() );
    this->world_->abort( 1 );
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
    this->world_->abort( 1 );
    return false;
  }

}

// Create/open the simulation's output sink.
//
// If extension is not given then a sink of type gdbm_sink
// is returned.
boost::shared_ptr< observable::base_sink > mpi_storage::open_output()
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
boost::shared_ptr< core::input_base_reader > mpi_storage::open_input()
{
  boost::shared_ptr< core::input_base_reader > reader;
  if( this->rank_ == 0 )
  {
    const std::string inputfn = this->find_input_filename();
    reader.reset( new core::input_preprocess );
    reader->add_include( inputfn );
  }
  if( this->sibling_count_ > 1 )
  {
    // Only broadcast input file if we have siblings.
    namespace mpi = boost::mpi;
    std::string buf;
    if( this->rank_ == 0 )
    {
      std::stringstream store;
      // write class instance to archive
      boost::archive::text_oarchive oa( store );
      oa << reader;
      buf.assign( std::move( store.str() ) );
    }
    mpi::broadcast( *this->world_, buf, 0 );
  
    if (this->rank_ != 0)
    {
      std::stringstream store( buf );
      // read class instance from archive
      boost::archive::text_iarchive ia( store );
      ia >> reader;
    }
  }
  return reader;

}

// Create/open the simulation's checkpoint stream.
boost::shared_ptr< std::ostream > mpi_storage::open_checkpoint()
{
  this->ensure_output_dir();
  const std::string fn( this->checkpoint_path() );
  boost::shared_ptr< std::ostream > result( new std::ofstream( fn ) );
  UTILITY_ALWAYS( *(result), "Problem openning checkpoint file "+fn );
  return result;

}

// Create the system's log output stream. In this case
// the log file is to standard out.
boost::shared_ptr< std::ostream > mpi_storage::open_log()
{
  this->ensure_output_dir();
  std::string fn { this->compute_output_dir() + "/log" };
  boost::shared_ptr< std::ostream > result( new std::ofstream( fn.c_str(), std::ios_base::ate ) );
  return result;

}

void mpi_storage::add_definition(storage_meta & meta)
{
  std::string desc( "Use local storage for MPI parallel execution." );
  std::unique_ptr< storage_definition > result( new storage_definition( type_label_(), desc ) );
  // no extra parameters
  meta.add_type( result );

}

// The string "local" used to identify this storage
// manager in the input file.
std::string mpi_storage::type_label_()
{
  const std::string result( "mpi" );
  return result;

}


} // namespace platform

BOOST_CLASS_EXPORT_IMPLEMENT( platform::mpi_storage );