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

#include "observable/d3_distribution.hpp"
#include "observable/sampler_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "observable/report_manager.hpp"
#include "observable/base_sink.hpp"
#include "core/input_document.hpp"

// Manuals
#include "core/input_base_reader.hpp"
#include "core/input_error.hpp"
#include "core/strngs.hpp"
#include "observable/field_format.hpp"
#include "observable/output_series_datum.hpp"
#include "observable/output_series.hpp"
#include "particle/ensemble.hpp"
#include "particle/specie_key.hpp"
// -
#include <boost/tokenizer.hpp>
// - 
namespace observable {

// For serialization
d3_distribution::d3_distribution()
: distributions_()
, regions_()
, specie_labels_()
, stepsize_()
{}

// Add definition for generating objects of this 
// class to the meta object.
void d3_distribution::add_definition(sampler_meta & meta)
{
  std::string desc( "Sample the 3D distribution of particles." );
  std::unique_ptr< sampler_definition > result( new sampler_definition( type_label_(), desc, &d3_distribution::make_sampler ) );
  // two extra parameters
  result->add_definition( { "stepsize", "number in Angstrom", ">0", std::to_string( default_bin_width() ), "Subsampling cube width of population histogram." } );
  result->add_definition( { core::strngs::fsregn(), "list", "specie:region labels", "all species and whole cell", "List of specie label : region label pairs. Use to restrict sampling of a specie to a specific region of the simulation cell. Default is to use all species and the whole cell." } );
  meta.add_sampler_type( result );
  

}

//Log message descibing the observable and its parameters
void d3_distribution::description(std::ostream & os) const 
{
  os << " " << this->type_label_() << "\n";
  os << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  os << "    Collect 3D density distributions for each specie.\n";
  os << "    - Region-specific distributions per-specie.\n";
  for( auto const& val : this->regions_ )
  {
    os << "      - " << val.first << "  : " << val.second << "\n";
  }
  os << "    - sample bin width : " << this->stepsize_ << " (default ";
  os << d3_distribution::default_bin_width() << ")\n";

}

//Retrieve the radial distribution profiles
//
///return vector< estimate_array > const*const where
//  rdf(i, j) is at index [i x specie_count + j]
//
//
//

boost::any d3_distribution::get_value() const
{
  boost::any result = &this->distributions_;
  return result;

}

boost::shared_ptr< base_observable > d3_distribution::make_sampler(std::vector< core::input_parameter_memo > const& paramset)
{
  std::unique_ptr<d3_distribution> result( new d3_distribution );
  result->stepsize_ = d3_distribution::default_bin_width();
  for( auto& param : paramset )
  {
    if( core::strngs::fsregn() == param.name() )
    {
      std::string region_list = param.get_text( "3D distribution " + d3_distribution::type_label_(), core::strngs::sampler_label() );
      result->regions_ = d3_distribution::process_region_list( region_list );
      if( result->regions_.empty() )
      {
        throw core::input_error::parameter_value_error( "Volume selection", core::strngs::sampler_label(), param, "Unable to convert value into a list of region labels." );
      }
    }
    else if ( "stepsize" == param.name())
    {
      result->stepsize_ = param.get_float( d3_distribution::type_label_(), core::strngs::sampler_label(), true, false );
    }
    else
    {
      UTILITY_REQUIRE( param.name() == core::strngs::fsend(), "Parameter names should have been checked in meta.do_read_end" );
    }
  }
  boost::shared_ptr< base_observable > tmp(result.release());
  return tmp;

}

// Prepare the evaluator for a simulation.
void d3_distribution::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  if( not this->distributions_.empty() )
  {
    this->distributions_.clear();
  }
  this->specie_labels_.clear();
  // regions_ is a map of specie label to region. It is
  // optional in that species not in the map will use the
  // system region. It is an error for a named region not to
  // exist.
  if( not this->regions_.empty() )
  {
    for( auto const& entry : this->regions_ )
    {
      if( gman.region_key( entry.second ) == gman.region_count() )
      {
        // error, region label not found.
        std::stringstream region_os;
        region_os << "(";
        for( std::size_t ireg = 0; ireg != gman.region_count(); ++ireg )
        {
          region_os << gman.get_region( ireg ).label() << ",";
        }
        std::string reg_list( region_os.str() );
        reg_list.back() = ')';
        core::input_parameter_memo tmp;
        tmp.set_name( core::strngs::fsregn() );
        tmp.set_value( entry.second );
        throw core::input_error::parameter_value_error( "Volume selection", core::strngs::sampler_label(), tmp, (core::input_error::bad_key_message_format() % reg_list).str() );
      }
    }
  }
  this->specie_labels_.resize( pman.specie_count() );
  for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
  {
    const std::string label = pman.get_specie( ispec ).label();
    this->specie_labels_[ ispec ] = label;
    if( this->regions_.count( label ) == 1 )
    {
      // specie has specific region target
      geometry::coordinate bbmin, bbmax;
      gman.get_region( gman.region_key( this->regions_[ label ] ) ).extent( bbmin, bbmax, pman.get_specie( ispec ).radius() );
      this->distributions_.push_back( { { bbmin, bbmax, this->stepsize_ } } );
    }
    else
    {
      // specie uses global region
      geometry::coordinate bbmin, bbmax;
      gman.system_region().extent( bbmin, bbmax, pman.get_specie( ispec ).radius() );
      this->distributions_.push_back( { { bbmin, bbmax, this->stepsize_ } } );
    }
  }

}

// Save 3d average particle counts to a data sink.
void d3_distribution::on_report(std::ostream & out, base_sink & sink)
{
  // Write out 3D data
  for (std::size_t ispec = 0; ispec != this->specie_labels_.size(); ++ispec)
  {
    if( this->distributions_[ ispec ].count() > 0 )
    {
      std::string label( "d3df-"+this->specie_labels_[ ispec ]+".dat" );
      auto & data = this->distributions_[ ispec ];
      if( not sink.has_dataset( label ) )
      {
        std::unique_ptr< observable::output_series >definition{ new observable::output_series( label, false ) };
        definition->set_title( "3D density distribution histogram for specie "+ this->specie_labels_[ ispec ] );
        {
          std::unique_ptr< observable::output_field > field( new observable::digitizer_3d_output( "XMID YMID ZMID", "ANGSTROM ANGSTROM ANGSTROM", data.axis(), observable::digitizer_3d_output::USE_MID ) );
          definition->push_back_field( std::move( field ) );
        }
        {
          std::unique_ptr< observable::output_field > field( new observable::mean_variance_output( "P.mean P.var", "Rate Rate2" ) );
          definition->push_back_field( std::move( field ) );
        }
        sink.add_dataset( std::move( definition ) );
      }
      {
        std::unique_ptr< observable::output_series_datum > datum( new observable::output_series_datum{ 0ul, new utility::estimate_array( std::move( data.release_data() ) ) } );
        sink.receive_data( label, std::move( datum ) );
      }
    }
  }

}

// Sample particle density
void d3_distribution::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  for( auto & histo : this->distributions_ )
  {
    histo.begin_sample();
  }
  auto const& ens = pman.get_ensemble();
  for( std::size_t ith = 0; ith != ens.size(); ++ith )
  {
    const std::size_t ispec( ens.key( ith ) );
    if( ispec != particle::specie_key::nkey )
    {
      geometry::coordinate pos = ens.position( ith );
      if( this->distributions_[ ispec ].in_range( pos ) )
      {
        this->distributions_[ispec].sample_datum( pos );
      }
    }
  }
  for( auto & histo : this->distributions_ )
  {
    histo.end_sample();
  }

}

std::string d3_distribution::type_label_()
{
  return std::string("3d-distribution");
}

// Write an input file section.
//
// only throw possible should be from os.write() operation
void d3_distribution::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );
  if( not this->regions_.empty() )
  {
    std::stringstream os;
    for( auto const& entry : this->regions_ )
    {
      if( os.str().size() != 0 )
      {
        os << " ";
      }
      os << entry.first << ":" << entry.second;
    }
    wr[ ix ].add_entry( core::strngs::fsregn(), os.str() );
  }
  wr[ ix ].add_entry( "stepsize", this->stepsize_ );
  

}

// Parse a list of "label:region" pairs into the map of specie
// label to regions.

std::map< std::string, std::string > d3_distribution::process_region_list(std::string list)
{
  std::map< std::string, std::string > regions;
  boost::tokenizer<> tok { list };
  std::string label;
  for( std::string word_or_pair : tok )
  {
    if( label.empty() )
    {
      // specie label part of pair
      if( 2 != word_or_pair.size() )
      {
        throw core::input_error::parameter_value_error( "Specie-region pair list ", core::strngs::sampler_label(), core::strngs::fsregn(), list, nullptr, "Specie label \""+word_or_pair+"\" should have two exactly two letters." ); 
      }
      label = word_or_pair;
    }
    else
    {
      // region part of pair
      auto check_unique = regions.insert( { label, word_or_pair } );
      if( not check_unique.second )
      {
        throw core::input_error::parameter_value_error( "Specie-region pair list ", core::strngs::sampler_label(), core::strngs::fsregn(), list, nullptr, "Specie label \""+label+"\" appears more than once in list" ); 
      }
      label.clear();
    }
  }
  if( not label.empty() )
  {
    throw core::input_error::parameter_value_error( "Specie-region pair list ", core::strngs::sampler_label(), core::strngs::fsregn(), list, nullptr, "Specie label \""+label+"\" has no matching region name." ); 
  }
  return regions;
  

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::d3_distribution );