

#ifndef DEBUG
#define DEBUG 0
#endif

#include "platform/simulator_meta.hpp"
#include "platform/simulation_manager.hpp"
#include "core/input_parameter_memo.hpp"
#include "platform/simulation.hpp"
#include "core/input_base_reader.hpp"
#include "core/input_help.hpp"

// Manual incls
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "utility/utility.hpp"
//-
#include <boost/iostreams/filtering_stream.hpp>
//-
namespace platform {

// Read an entry in the input file
//
// throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool simulator_meta::do_read_entry(core::input_base_reader & reader) 
{
  // Example input section
  //
  // simulator
  // name Some title
  // outputdir somewhere/here
  // inputname some regular expression
  // nstep 100000
  // naver 1000
  // inner 1000 # Number or "auto"
  // isave 1000
  // ntarg 300
  // kelvin 300
  // type standard
  // end
  //
  // process entry
  if (reader.name().find(core::strngs::fsname()) == 0)
  {
     // --------------------
     //run title "name ##"
     reader.duplicate_test( this->sim_->run_title().empty(), this->section_label() );
     this->sim_->set_run_title( reader.get_text( "Simulation title", this->section_label() ) );
  }
  else if (reader.name().find(core::strngs::fsnstp()) == 0)
  {
     // --------------------
     // Number of production step "nstep ##"
     reader.duplicate_test( not this->required_tags_[ SIMULATOR_PROD ], this->section_label() );
     this->nstep_ = reader.get_ordinal( "Simulation production step", this->section_label() );
     this->required_tags_.set( SIMULATOR_PROD );
  }
  else if (reader.name().find(core::strngs::fsnavr()) == 0)
  {
     // --------------------
     // Number of equilibrium step "naver ##"
     reader.duplicate_test( not this->required_tags_[ SIMULATOR_THERM ], this->section_label() );
     this->naver_ = reader.get_ordinal( "Simulation equilibration step", this->section_label() );
     this->required_tags_.set( SIMULATOR_THERM );
  }
  else if (reader.name().find(core::strngs::inner_label()) == 0)
  {
     // --------------------
     // Number of inner loop trials per step "inner ##"
     reader.duplicate_test( not this->required_tags_[ SIMULATOR_INNER ], this->section_label() );
     this->sim_->set_inner_loop_size( reader.get_ordinal( "Simulation inner loop", this->section_label() ) );
     this->required_tags_.set( SIMULATOR_INNER );
  }
  else if (reader.name().find(core::strngs::fsisav()) == 0)
  {
     // --------------------
     // Checkpoint interval "isave ##"
     reader.duplicate_test( not this->required_tags_[ SIMULATOR_CHECK ], this->section_label() );
     this->sim_->set_report_interval( reader.get_ordinal( "Simulation checkpoint/report interval", this->section_label() ) );
     this->required_tags_.set( SIMULATOR_CHECK );
  }
  else if (reader.name().find(core::strngs::fsntrg()) == 0)
  {
     // --------------------
     // Target particle number "ntarg ##"
    //reader.duplicate_test( 0 == this->sim_->target_count(), this->section_label() );
     this->sim_->set_target_count( reader.get_ordinal( "Simulation target particle number", this->section_label() ) );
  }
  else if (reader.name().find(core::strngs::fsepsw()) == 0)
  {
     // --------------------
     // Relative permittivity of media "epsw ##.#"
     // (can not test for duplicate?)
     this->sim_->set_solvent_permittivity( reader.get_float( "Simulation media permittivity", this->section_label(), true, false ) );
  }
  else if (reader.name().find(core::strngs::fstsi()) == 0)
  {
     // --------------------
     // Temperature "kelvin ##.#"
     const double temperature = reader.get_float( "Simulation temperature (in kelvin)", this->section_label(), true, false );
     this->sim_->set_temperature( temperature );
     if (temperature < 273.0)
     {
      this->sim_->get_log() << core::strngs::horizontal_bar() << "\n";
      {
        namespace io = boost::iostreams;
        core::fixed_width_output_filter filt( 2, 1, 68 );
        io::filtering_ostream os;
        os.push( filt );
        os.push( this->sim_->get_log() );
        os << "WARNING: Requested simulation temperature ("
           << temperature << ") is below the freezing point of water.";
      }
      this->sim_->get_log() << "\n" << core::strngs::horizontal_bar() << "\n";
     }
  }
  else if( reader.name().find( core::strngs::fstype() ) == 0 )
  {
    // --------------------
    // Simulation manager subtype
    reader.duplicate_test( this->type_.empty(), this->section_label() );
    std::vector< std::string > keys{ this->definition_key_list() };
    const std::size_t idx = reader.get_key(  "Simulation manager \"type\"", this->section_label(), keys );
    this->type_ = keys.at( idx );
    this->required_tags_.set( SIMULATOR_TYPE );
  }
  else
  {
    // --------------------
    // Simulation manager specific parameters?
    const std::string lname{ reader.name() };
    reader.duplicate_test( 0 == std::count_if( this->params_.begin(), this->params_.end(), [&lname](const core::input_parameter_memo& param){ return lname == param.name(); } ), this->section_label() );
    this->params_.push_back( { reader } );
  }
  return true;

}

// Check for completeness of input at end of section. It does not
// reset the meta data as 'multiple' is false.
void simulator_meta::do_read_end(const core::input_base_reader & reader)
{
  {
    std::string missing{};
    if( not this->required_tags_[SIMULATOR_PROD] )
    {
      missing += (missing.empty() ? "" : " " ) + core::strngs::fsnstp();
    }
    if( not this->required_tags_[SIMULATOR_THERM] )
    {
      missing += (missing.empty() ? "" : " " ) + core::strngs::fsnavr();
    }
    if( not this->required_tags_[SIMULATOR_CHECK] )
    {
      missing += (missing.empty() ? "" : " " ) + core::strngs::fsisav();
    }
    if( not this->required_tags_[SIMULATOR_INNER] )
    {
      missing += (missing.empty() ? "" : " " ) + core::strngs::inner_label();
    }
    if( not this->required_tags_[SIMULATOR_TYPE] )
    {
      missing += (missing.empty() ? "" : " " ) + core::strngs::fstype();
    }
    if( not missing.empty() )
    {
      reader.missing_parameters_error( "Simulation configuration", this->section_label(), missing );
    }
  }
  // create sim manager
  for( auto const& defn : this->types_ )
  {
    if( defn.label() == this->type_ )
    {
      // check parameters.
      for( auto const& entry : this->params_ )
      {
        if( not defn.has_definition( entry.name() ) )
        {
          throw core::input_error::invalid_parameter_subtype_error( this->section_label(), this->type_, entry );
        }
      }
      // push "end" tag onto parameter set.
      this->params_.push_back( { reader } );
  
      boost::shared_ptr< simulation_manager > sman;
      sman = defn( this->params_ );
      sman->set_equilibration_interval( this->naver_ );
      sman->set_production_interval( this->nstep_ );
      this->sim_->set_manager( sman );
      return;
    }
  }
  UTILITY_CHECK( false, "Programming error, should never get here." );

}

// Get a list of definition labels/keys
std::vector< std::string > simulator_meta::definition_key_list()
{
  std::vector< std::string > result;
  for( auto const& entry : this->types_ )
  {
    result.push_back( entry.label() );
  }
  return result;

}

// Add ctor for input "simulation" section
//
// \pre not has_type( ctor.type_name_ )
// \post has_type( ctor.type_name_ )
void simulator_meta::add_type(std::unique_ptr< simulation_definition >& ctor)
{
  UTILITY_REQUIRE( not this->has_type( ctor->label() ), ("Attempt to add more than one chooser factory for type \"" + ctor->label() + "\".") );
  this->types_.push_back( ctor.release() );

}

// Is there a ctor to match input "simulation" section with
// "type = [label]"
bool simulator_meta::has_type(std::string label)
{
  for( auto const& defn : this->types_ )
  {
    if( defn.label() == label )
    {
      return true;
    }
  }
  return false;
  

}

simulator_meta::simulator_meta(simulation & sim) 
: input_base_meta (core::strngs::simulator_label(), false, true)
, sim_(&sim)
, required_tags_()
, params_()
, type_()
, naver_()
, nstep_()
{}

simulator_meta::~simulator_meta() 
{}

// Reset the object
void simulator_meta::do_reset()
{
  this->required_tags_.reset();

}

void simulator_meta::publish_help(core::input_help & helper) const
{
  const std::string seclabel( core::strngs::simulator_label() );
  // ----------------------------------------
  // add section
  // ----------------------------------------
  //   Specie input section definition
  helper.add_section( { seclabel, "Define global simulation parameters." } );
  auto &sect = helper.get_section( seclabel );
  
  
  // ----------------------------------------
  // add parameters { name, type, range, default, description } 
  // ----------------------------------------
  
  //XX // nstep : [required, number] production steps
  //XX {
  //XX   const std::string description( "Number of steps in production phase of simulation." );
  //XX   sect.add_entry( { core::strngs::fsnstp(), "integer", "> 0", "required", description } );
  //XX }
  //XX // naver : [required, number] equilibrium steps
  //XX {
  //XX   const std::string description( "Number of steps in equilibration/thermalization phase of simulation." );
  //XX   sect.add_entry( { core::strngs::fsnavr(), "integer", "> 0", "required", description } );
  //XX }
  // isave : [required, number] steps between system checkpoints
  {
    const std::string description( "Number of steps between checkpoints and reports of the system." );
    sect.add_entry( { core::strngs::fsisav(), "integer", "> 0", "required", description } );
  }
  // inner : [optional, number] steps between system checkpoints
  {
    const std::string description( "Number of trials within a step. TODO: Default is 'auto' which gives a loop size that scales with the problem size." );
    sect.add_entry( { core::strngs::inner_label(), "integer", "> 0", "required", description } );
  }
  
  // kelvin : [optional, number] simulation temperature (in Kelvin)
  {
    const std::string description( "Simulation temperature in Kelvin. The default is room temperature. For the simulation to be meaningful the temperature should be above the freezing point of the media." );
    sect.add_entry( { core::strngs::fstsi(), "number", "> 0.0", std::to_string( evaluator::evaluator_manager::standard_room_temperature() ), description } );
  }
  // name : [optional, string] User provided descriptive title
  {
    const std::string description( "Title or descriptive text for the simulation." );
    sect.add_entry( { core::strngs::fsname(), "text", "", "optional", description } );
  }
  // ntarg : [required, integer] target average particle count.
  {
    const std::string description( "Target average number of solute particles. If no value (or zero) is given then the simulation will attempt to calculate the particle number from the target concentration and initial volume.  (If no concentrations are given and no ntarg then the particle defined in the input file specie sections are used as-is.)" );
    sect.add_entry( { core::strngs::fsntrg(), "integer", ">0", "optional", description } );
  }
  // epsw : [optional, number > 0.0] The relative permittivity of the media 
  {
    const std::string description( "The relative permittivity of the media. The default is the standard relative permittivity of water." );
    sect.add_entry( { core::strngs::fsepsw(), "number", "> 0.0", std::to_string( evaluator::evaluator_manager::standard_aqueous_permittivity() ), description } );
  }
  

}


} // namespace platform
