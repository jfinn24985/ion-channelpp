

#ifndef DEBUG
#define DEBUG 0
#endif

#include "cscpbc/bulk_simulator_meta.hpp"
#include "cscpbc/periodic_system.hpp"
#include "core/input_base_reader.hpp"
#include "platform/simulator.hpp"

// Manual incs
#include "core/strngs.hpp"
#include "core/input_help.hpp"
#include "utility/utility.hpp"
// -
namespace periodic_cube {

// Read an entry in the input file
//
// throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool bulk_simulator_meta::do_read_entry(core::input_base_reader & reader) 
{
  // Example input section
  //
  // simulator
  // name Some title
  // checkdir somewhere/else
  // outputdir somewhere/here
  // inputname some regular expression
  // nstep 100000
  // naver 1000
  // inner 1000 # Number or "auto"
  // isave 1000
  // ntarg 300
  // kelvin 300
  //
  // ## PLUS ##
  // epsw 80.0
  // end
  //
  // First pass to base class
  if (this->simulator_meta::do_read_entry( reader ))
  {
     return true;
  }
  if (reader.name().find(core::strngs::fsepsw()) == 0)
  {
     // --------------------
     // Number of production step "epsw ##.#"
     UTILITY_INPUT(not reader.value().empty(), "Permittivity of media '"+core::strngs::fsepsw()+"' must have a value.", this->section_label(), &reader);
     try
     {
        this->bulk_sim_->set_permittivity( boost::lexical_cast< double  >(reader.value()) );
        UTILITY_INPUT(this->bulk_sim_->permittivity() > 0ul, "Permittivity must be grater than zero.", this->section_label(), &reader);
        this->missing_required_tags_.reset( SIMULATOR_EPSW );
     }
     catch (boost::bad_lexical_cast const& err)
     {
        const bool convert_succeed(false);
        UTILITY_INPUT(convert_succeed, "Invalid number for " + core::strngs::fsepsw() + " : " + err.what(), this->section_label(), &reader);
     }
  }
  else
  {
     // Option not understood
     return false;
  }
  return true;

}

// Check for completeness of input at end of section
void bulk_simulator_meta::do_read_end(platform::simulator & sim)
{
  UTILITY_INPUT(not this->missing_required_tags_.any(), "Not all required tags were present.", this->section_label(), nullptr);
  
  this->simulator_meta::do_read_end( sim );

}

bulk_simulator_meta::bulk_simulator_meta(periodic_system & sim) 
: simulator_meta( sim )
, bulk_sim_( &sim )
, missing_required_tags_()
{
  this->missing_required_tags_.set();
  if (1 == counter) // Base class should increment counter before we get here.
  {
    const std::string seclabel( core::strngs::simulator_label() );
    // ----------------------------------------
    // NOTE: reuse 'simulator' section
    // ----------------------------------------
  
    // epsw XX.X
    // move_rate XX.X
    // jump_rate XX.X
    // gc_rate XX.X
  
    // ----------------------------------------
    // add parameters
    // ----------------------------------------
  
    // epsw : [required, number] permittivity of medium
    {
      const std::string description("[required, number > 0] The relative permittivity of the media.");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsepsw(), description );
    }
  }

}

bulk_simulator_meta::~bulk_simulator_meta() 
{}


} // namespace periodic_cube
