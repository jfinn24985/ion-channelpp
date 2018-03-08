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

#include "cscchannel/cell_simulator_meta.hpp"
#include "cscchannel/channel_system.hpp"
#include "core/input_base_reader.hpp"
#include "platform/simulator.hpp"

// Manual incs
#include "core/strngs.hpp"
#include "core/input_help.hpp"
#include "utility/utility.hpp"
// -
namespace ion_channel {

// Read an entry in the input file
//
// throw an error if input file is incorrect (using UTILITY_INPUT macro)
bool cell_simulator_meta::do_read_entry(core::input_base_reader & reader) 
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
  try
  {
     if (reader.name().find(core::strngs::fsepsw()) == 0)
     {
        // --------------------
        // Permittivity of solvent "epsw ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Permittivity of media '"+core::strngs::fsepsw()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_permittivity( boost::lexical_cast< double  >(reader.value()) );
        UTILITY_INPUT(this->cell_sim_->permittivity() > 0ul, "Permittivity must be greater than zero.", this->section_label(), &reader);
        this->missing_required_tags_.reset( SIMULATOR_EPSW );
     }
     else if (reader.name().find( core::strngs::fsepsp() ) == 0)
     {
        // --------------------
        // Permittivity of membrane protein "epspr ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Permittivity of protein '"+core::strngs::fsepsp()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_protein_permittivity( boost::lexical_cast< double  >(reader.value() ) );
        UTILITY_INPUT(this->cell_sim_->protein_permittivity() > 0ul, "Permittivity must be greater than zero.", this->section_label(), &reader);
     }
     else if (reader.name().find( core::strngs::fsgzl1() ) == 0 )
     {
        // --------------------
        // Length of central part of filter "zl1 ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Pore half length '"+core::strngs::fsgzl1()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_pore_hlength( boost::lexical_cast< double >( reader.value() ) );
        UTILITY_INPUT(this->cell_sim_->pore_hlength() > 0ul, "Pore half length must be greater than zero.", this->section_label(), &reader);
        this->missing_required_tags_.reset( SIMULATOR_ZL1 );
     }
     else if (reader.name().find( core::strngs::fsgzl4() ) == 0 )
     {
        // --------------------
        // Optional half length of the cell "zl4 ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Cell half length '"+core::strngs::fsgzl4()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_cell_hlength( boost::lexical_cast< double >( reader.value() ) );
        UTILITY_INPUT(this->cell_sim_->cell_hlength() > 0ul, "Cell half length must be greater than zero.", this->section_label(), &reader);
     }
     else if (reader.name().find( core::strngs::fsgrl1() ) == 0 )
     {
        // --------------------
        // Radius of the filter "rl1 ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Pore radius '"+core::strngs::fsgrl1()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_pore_radius( boost::lexical_cast< double >( reader.value() ) );
        UTILITY_INPUT(this->cell_sim_->pore_radius() > 0ul, "Pore radius must be greater than zero.", this->section_label(), &reader);
        this->missing_required_tags_.reset( SIMULATOR_RL1 );
     }
     else if (reader.name().find( core::strngs::fsgrl4() ) == 0 )
     {
        // --------------------
        // Radius of the protein "rl4 ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Protein outer radius '"+core::strngs::fsgrl4()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_protein_radius( boost::lexical_cast< double >( reader.value() ) );
        UTILITY_INPUT(this->cell_sim_->protein_radius() > 0ul, "Protein outer radius must be greater than zero.", this->section_label(), &reader);
        this->missing_required_tags_.reset( SIMULATOR_RL4 );
     }
     else if (reader.name().find( core::strngs::fsgrl5() ) == 0 )
     {
        // --------------------
        // Optional radius of the cell "rl5 ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Cell radius '"+core::strngs::fsgrl5()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_cell_radius( boost::lexical_cast< double >( reader.value() ) );
        UTILITY_INPUT(this->cell_sim_->cell_radius() > 0ul, "Cell radius must be greater than zero.", this->section_label(), &reader);
     }
     else if (reader.name().find( core::strngs::fsgrlv() ) == 0 )
     {
        // --------------------
        // Vestibule arc radius "rlvest ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Vestibule arc radius '"+core::strngs::fsgrlv()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_vestibule_arc_radius( boost::lexical_cast< double >( reader.value() ) );
        UTILITY_INPUT(this->cell_sim_->vestibule_arc_radius() > 0ul, "Vestibule arc radius must be greater than zero.", this->section_label(), &reader);
        this->missing_required_tags_.reset( SIMULATOR_RLVEST );
     }
     else if (reader.name().find( core::strngs::fsgrlc() ) == 0 )
     {
        // --------------------
        // Toriod outer arc radius "rlcurv ##.#"
        UTILITY_INPUT(not reader.value().empty(), "Membrane arc radius '"+core::strngs::fsgrlc()+"' must have a value.", this->section_label(), &reader);
        this->cell_sim_->set_membrane_arc_radius( boost::lexical_cast< double >( reader.value() ) );
        UTILITY_INPUT(this->cell_sim_->membrane_arc_radius() > 0ul, "Membrane arc radius must be greater than zero.", this->section_label(), &reader);
        this->missing_required_tags_.reset( SIMULATOR_RLCURV );
     }
     else
     {
     // Option not understood
        return false;
     }
  }
  catch (boost::bad_lexical_cast const& err)
  {
     const bool convert_succeed(false);
     UTILITY_INPUT(convert_succeed, "Invalid value [" + reader.value() + "] for " + reader.name() + " : " + err.what(), this->section_label(), &reader);
  }
  
  return true;

}

// Check for completeness of input at end of section
void cell_simulator_meta::do_read_end(platform::simulator & sim)
{
  UTILITY_INPUT( not this->missing_required_tags_.any(), "Not all required tags were present.", this->section_label(), nullptr );
  // Input requirements
  //   ** (zlimit <= zl1) : structural ions limited to central cylinder
  //   ** (rlvest + rlcurv <= rl4 - rl1) : radial width of toroid is greater than arcs
  //   ** (rlcurv <= zl1 + rlvest) : z-width of toroid is greater  than outer arc.
  //   ** if target_particle_number_ == 0 then protein must be in defined cell boundaries
  UTILITY_INPUT( this->cell_sim_->structural_hlength() <= this->cell_sim_->pore_hlength()
               , "Invalid geometry: structural ion z-limit is greater than zl1"
               , this->section_label(), nullptr );
  UTILITY_INPUT( (this->cell_sim_->vestibule_arc_radius() + this->cell_sim_->membrane_arc_radius()) <= (this->cell_sim_->protein_radius() - this->cell_sim_->pore_radius())
               , "Invalid geometry: rlvest + rlcurv greater than rl4 - rl1"
               , this->section_label(), nullptr );
  UTILITY_INPUT( this->cell_sim_->membrane_arc_radius() < (this->cell_sim_->vestibule_arc_radius() + this->cell_sim_->pore_hlength())
               , "Invalid geometry: rlcurv greater than zl1 + rlvest"
               , this->section_label(), nullptr );
  UTILITY_INPUT( this->cell_sim_->target_count() > 0 or ((this->cell_sim_->cell_radius() > this->cell_sim_->protein_radius()) and (this->cell_sim_->cell_hlength() > this->cell_sim_->pore_hlength() + this->cell_sim_->vestibule_arc_radius()))
               , "Invalid geometry: channel protein extends outside simulation box boundary"
               , this->section_label(), nullptr );
  
  
  this->simulator_meta::do_read_end( sim );

}

cell_simulator_meta::cell_simulator_meta(channel_system & sim) 
: simulator_meta( sim )
, cell_sim_( &sim )
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
    {
      const std::string description("[required, number > 0] The relative permittivity of the protein.");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsepsp(), description );
    }
    {
      const std::string description("[required, number > " + core::strngs::fsgzlm() + "] The pore half length.");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsgzl1(), description );
    }
    {
      const std::string description("[optional, number > 0] The cell half length (optional if " + core::strngs::fsntrg() + "is given).");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsgzl4(), description );
    }
    {
      const std::string description("[required, number > 0] The pore radius.");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsgrl1(), description );
    }
    {
      const std::string description("[required, number > 0] The protein outer radius, must be larger than " + core::strngs::fsgrl1() + " + " + core::strngs::fsgrlv() + " + " + core::strngs::fsgrlc() + ".");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsgrl4(), description );
    }
    {
      const std::string description("[optional, number > 0] The cell radius (optional if " + core::strngs::fsntrg() + "is given).");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsgrl5(), description );
    }
    {
      const std::string description("[required, number > 0] The radius of the arc at the vestibule.");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsgrlv(), description );
    }
    {
      const std::string description("[required, number > 0] The radius of the arc at the outer edge of protein.");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsgrlc(), description );
    }
    {
      const std::string description("[required, 0 < number < " + core::strngs::fsgzl1() + "] The structural ion limit.");
      core::input_help::exemplar().add_option( seclabel, core::strngs::fsgzlm(), description );
    }
  }

}

cell_simulator_meta::~cell_simulator_meta() 
{}


} // namespace ion_channel
