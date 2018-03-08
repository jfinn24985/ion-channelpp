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

#include "platform/storage_meta.hpp"
#include "platform/storage_manager.hpp"
#include "core/input_parameter_memo.hpp"
#include "core/input_base_reader.hpp"
#include "core/input_help.hpp"

// manual includes
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "utility/utility.hpp"
// -
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/algorithm/string.hpp>
// - 
namespace platform {

// Read an entry in the input file
//
// throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool storage_meta::do_read_entry(core::input_base_reader & reader) 
{
  // Example input section
  //
  // run
  // outputdir somewhere/here
  // inputname some regular expression
  // outname output.ext
  // checkname check.arc
  // type keyword
  // ... # type specific options
  // end
  //
  // process entry
  if( reader.name().find( core::strngs::outputdir_label() ) == 0 )
  {
    // --------------------
    // Output/data directory name "outputdir 'abc/def'"
    // (can not test for duplicate entry)
    std::string fmt{ reader.get_text( "Output directory", this->section_label() ) };
    boost::algorithm::trim( fmt );
    if( fmt.empty() )
    {
      reader.parameter_value_error( "Output directory", this->section_label(), core::input_error::non_empty_value_message() );
    }
    this->fstype_->set_output_dir_fmt( fmt );
  }
  else if( reader.name().find( core::strngs::inputpattern_label() ) == 0 )
  {
    // --------------------
    // Input file recognition pattern "inputpattern 'abc.%03d.def'"
    // (can not test for duplicate entry)
    const std::string fbase = reader.get_text( "Input file pattern", this->section_label() );
    if( this->fstype_->filename_base() != fbase )
    {
      // TODO: issue warning.
      this->fstype_->get_log() << core::strngs::horizontal_bar() << "\n";
      {
        namespace io = boost::iostreams;
        core::fixed_width_output_filter filt( 2, 1, 68 );
        io::filtering_ostream os;
        os.push( filt );
        os.push( this->fstype_->get_log() );
        os << "WARNING: Value for input file recognition pattern parameter \""
           << core::strngs::inputpattern_label()
           << "\" in input file (" << fbase
           << ") {file: " << reader.current_filename()
           << ", line: " << reader.current_line_number()
           << "} is different to that used ("
           << this->fstype_->filename_base()
           << ").  Value in input file ignored.";
      }
      this->fstype_->get_log() << "\n" << core::strngs::horizontal_bar() << "\n";
    }
  }
  else if( reader.name().find( this->output_name_label() ) == 0 )
  {
    // --------------------
    // outname {path} : output filename
    // (can not test for duplicate entry)
    std::string fmt{ reader.get_text( "Output filename", this->section_label() ) };
    boost::algorithm::trim( fmt );
    if( fmt.empty() )
    {
      reader.parameter_value_error( "Output filename", this->section_label(), core::input_error::non_empty_value_message() );
    }
    this->fstype_->set_output_name( fmt );
   }
  else if( reader.name().find( this->checkpoint_name_label() ) == 0 )
  {
    // --------------------
    // checkname {path} : checkpoint filename
    // (can not test for duplicate entry)
    std::string fmt{ reader.get_text( "Checkpoint filename", this->section_label() ) };
    boost::algorithm::trim( fmt );
    if( fmt.empty() )
    {
      reader.parameter_value_error( "Checkpoint filename", this->section_label(), core::input_error::non_empty_value_message() );
    }
    this->fstype_->set_checkpoint_name( fmt );
   }
  else if( reader.name().find( core::strngs::fstype() ) == 0 )
  {
    // --------------------
    // Storage manager subtype
    reader.duplicate_test( this->type_.empty(), this->section_label() );
    std::vector< std::string > keys{ this->definition_key_list() };
    const std::size_t idx = reader.get_key( "Run subtype", this->section_label(), keys );
    this->type_ = keys.at( idx );
  }
  else
  {
    // --------------------
    // Storage specific parameters
    const std::string lname{ reader.name() };
    reader.duplicate_test( 0 == std::count_if( this->parameter_set_.begin(), this->parameter_set_.end(), [&lname](const core::input_parameter_memo& param){ return lname == param.name(); } ), this->section_label() );
    this->parameter_set_.push_back( { reader } );
  }
  return true;

}

// Check for completeness of input at end of section. It does not
// reset the meta data as 'multiple' is false.
void storage_meta::do_read_end(const core::input_base_reader & reader)
{
  if( this->type_.empty() )
  {
    reader.missing_parameters_error( "Simulation configuration", this->section_label(), core::strngs::fstype() );
  }
  // Call functor method to create sampler and add to simulator
  for( const auto& defn : this->types_ )
  {
    if( defn.label() == this->type_ )
    { 
      // check parameters.
      for( auto const& entry : this->parameter_set_ )
      {
        if( not defn.has_definition( entry.name() ) )
        {
          throw core::input_error::invalid_parameter_subtype_error( this->section_label(), this->type_, entry );
        }
      }
      // push "end" tag onto parameter set.
      this->parameter_set_.push_back( { reader } );
  
      this->fstype_->set_parameters( this->parameter_set_ );
      return;
    }
  }
  // error to get here
  UTILITY_CHECK( false, "should never reach this point in the code." );

}

// Get a list of definition labels/keys
std::vector< std::string > storage_meta::definition_key_list()
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
void storage_meta::add_type(std::unique_ptr< storage_definition >& ctor)
{
  UTILITY_REQUIRE( not this->has_type( ctor->label() ), ("Attempt to add more than one chooser factory for type \"" + ctor->label() + "\".") );
  this->types_.push_back( ctor.release() );

}

// Is there a ctor to match input "simulation" section with
// "type = [label]"
bool storage_meta::has_type(std::string label)
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

storage_meta::storage_meta(boost::shared_ptr< storage_manager > fstype) 
: input_base_meta( storage_meta::storage_label(), false, false )
, fstype_( fstype )
, types_()
, parameter_set_()
, type_()
{}

storage_meta::~storage_meta() 
{}

// Reset the object
void storage_meta::do_reset()
{
  this->type_.clear();
  this->parameter_set_.clear();

}

void storage_meta::publish_help(core::input_help & helper) const
{
  const std::string seclabel( storage_label() );
  // ----------------------------------------
  // add section
  // ----------------------------------------
  //   Specie input section definition
  helper.add_section( { seclabel, "Define parameters defining file and directory names and parallelism." } );
  auto &sect = helper.get_section( seclabel );
  
  // ----------------------------------------
  // add parameters
  // ----------------------------------------
  
  // outputdir : [optional, filename regular expression] directory for output files
  {
    const std::string description( "Defines the directory for result/output files (will be created if not present)." );
    sect.add_entry( { core::strngs::outputdir_label(), "directory name or template", "", "%1$03d", description } );
  }
  // inputname : [optional, filename regular expression] how to locate input.
  {
    const std::string description( "Pattern to generate filenames so as to recognise an input file. This information is only useful on the command-line but is shown in output as documentation." );
    sect.add_entry( { core::strngs::inputpattern_label(), "filename regular expression", "", "input.%1$03d.inp", description } );
  }
  // checkname : [optional, filename] Name of checkpoint.
  {
    const std::string description( "The filename to use for the checkpoint file. This will be writen in the output directory." );
    sect.add_entry( { storage_meta::checkpoint_name_label(), "filename", "", "checkpoint.arc", description } );
  }
  // outputname : [optional, filename] Name of output file.
  {
    const std::string description( "The name of the output file to be writen in the output directory. The extension on the given filename determines the type of output file created. dbm|zip: create container files that hold all the output. mem: stores output in memory and writes no output file" );
    sect.add_entry( { storage_meta::output_name_label(), "filename", "*.(dbm|zip|mem)", "required", description } );
  }

}

// Name of checkpoint name parameter in input file
std::string storage_meta::checkpoint_name_label()
{
  const std::string label( "checkname" );
  return label;
}

// Name of checkpoint name parameter in input file
std::string storage_meta::output_name_label()
{
  const std::string label( "outname" );
  return label;
}

// The name of the storage section in the input file.
std::string storage_meta::storage_label()
{
  const std::string result( "run" );
  return result;
}


} // namespace platform
