

#ifndef DEBUG
#define DEBUG 0
#endif

#include "platform/imc_meta.hpp"
#include "observable/base_observable.hpp"
#include "core/input_base_reader.hpp"
#include "platform/simulation.hpp"

// manual includes
#include "core/input_help.hpp"
#include "core/strngs.hpp"
#include "utility/utility.hpp"
//=
namespace platform {

std::map< std::string, observable::sampler_definition::sampler_generator_fn > imc_meta::type_to_object_;

std::size_t imc_meta::counter_ = 0;

imc_meta::imc_meta() 
: input_base_meta (core::strngs::imc_label(), false, false)
, parameter_set_()
, missing_required_tags_ ()
, type_ ()
{
  this->missing_required_tags_.set();
  if (0 == counter_)
  {
     const std::string seclabel( core::strngs::imc_label() );
     // ----------------------------------------
     // add section
     // ----------------------------------------
  
     //   Specie input section definition
     core::input_help::exemplar().add_section( seclabel,
           "Super-looper input section definition.  In addition to the listed options, each individual "
           "sampler subtype may have its own specific options." );
  
     // ----------------------------------------
     // add parameters
     // ----------------------------------------
  
     //  type : [required] trial subtype
     {
        const std::string description("[required, string] sampler subtype (see documentation for valid subtypes).");
        core::input_help::exemplar().add_option( seclabel, core::strngs::fstype(), description );
     }
  }
  ++counter_;
  

}

imc_meta::~imc_meta() 
{}

// Get a list of definition labels/keys
std::vector< std::string > imc_meta::definition_key_list()
{
  std::vector< std::string > result;
  for( auto const& entry : this->type_to_object_ )
  {
    result.push_back( entry.label() );
  }
  return result;

}

//Read an entry in the input file. Return true if the entry was processed.
//
//throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool imc_meta::do_read_entry(core::input_base_reader & reader)
{
  if (reader.name().find(core::strngs::fstype()) == 0)
  {
     // --------------------
     // Evaluator type
     UTILITY_INPUT(not reader.value().empty(), "Sampler type must have a value.", this->section_label(), &reader);
     std::string val (reader.dequote(reader.value()));
     UTILITY_INPUT(type_to_object_.count(val) != 0, "Sampler type ["+val+"] does not name a valid sampler for this application (see documentation).", this->section_label(), &reader);
     this->type_ = val;
     missing_required_tags_.reset(SAMPLER_TYPE);
  }
  else
  {
     // --------------------
     // Choice specific parameters
     UTILITY_INPUT(0 == this->parameter_set_.count (reader.name()), ("keyword \""+reader.name()+"\" appears more than once in a single in specie section"), core::strngs::fsspec(), &reader);
     this->parameter_set_.insert (std::make_pair(reader.name(), reader.value()));
  }
  return true;

}

// Perform checks at the end of reading a section before creating an IGCMC object
// and adding it to the simulation. This does not reset the meta data as 'multiple'
// is false.
void imc_meta::do_read_end(simulation & sim)
{
  UTILITY_INPUT( not this->missing_required_tags_.any(), "Not all required tags were present.", this->section_label(), nullptr );
  
  // Call the generator functional to create and add
  // super-looper object to simulator.
  this->type_to_object_[ this->type_ ]( this->parameter_set_, sim );

}

// Reset the object
void imc_meta::do_reset()
{
  this->parameter_set_.clear();
  this->missing_required_tags_.reset();

}


} // namespace platform
