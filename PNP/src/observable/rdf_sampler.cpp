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

#include "observable/rdf_sampler.hpp"
#include "utility/histogram.hpp"
#include "observable/sampler_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "observable/base_sink.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "observable/report_manager.hpp"
#include "core/input_document.hpp"

// Manuals
#include "core/strngs.hpp"
#include "core/input_base_reader.hpp"
#include "observable/field_format.hpp"
#include "observable/output_series_datum.hpp"
#include "observable/output_series.hpp"
#include "particle/ensemble.hpp"
// - 
namespace observable {

// For serialization and factory only
rdf_sampler::rdf_sampler() = default;
rdf_sampler::~rdf_sampler() = default;

// Add definition for generating objects of this 
// class to the meta object.
void rdf_sampler::add_definition(sampler_meta & meta)
{
  std::string desc( "Sample the radial distribution function around each ion collated by ion specie." );
  std::unique_ptr< sampler_definition > result( new sampler_definition( type_label_(), desc, &rdf_sampler::make_sampler ) );
  // two extra parameters
  result->add_definition( { "stepsize", "distance in Angstrom", ">0", std::to_string( default_bin_width() ), "Bin width of pairwise radial population histogram." } );
  result->add_definition( { "width", "distance in Angstrom", ">0", std::to_string( default_width() ), "Range of population histogram (excludes particle radii)." } );
  meta.add_sampler_type( result );
  

}

//Log message descibing the observable and its parameters
//
//TODO: describe specific RDF functions (if not default)
void rdf_sampler::description(std::ostream & out) const 
{
  out << " " << this->type_label_() << "\n";
  out << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  out << "    Collect and report one dimensional density profiles of species\n";
  out << "    - Default histogram bin width : " << rdf_sampler::default_bin_width() << "\n";

}

//Retrieve the radial distribution profiles
//
///return vector< estimate_array > const*const where
//  rdf(i, j) is at index [i x specie_count + j]
//
//
//

boost::any rdf_sampler::get_value() const
{
  boost::any result = &this->data_sets_;
  return result;

}

// Make an rdf_observable (independent of type) from input file
//
// parameters : "region" "stepsize"
boost::shared_ptr< base_observable > rdf_sampler::make_sampler(std::vector< core::input_parameter_memo > const& param_set)
{
  double width = rdf_sampler::default_width();
  double stepsize = rdf_sampler::default_bin_width();
  for( auto& param : param_set )
  {
    if( "width" == param.name() )
    {
      width = param.get_float( "Radial Distribution function total width", core::strngs::sampler_label(), true, false );
    }
    else if( "stepsize" == param.name() )
    {
      stepsize = param.get_float( "Radial Distribution histgram bin width", core::strngs::sampler_label(), true, false );
    }
    else
    {
      UTILITY_REQUIRE( param.name() == core::strngs::fsend(), "Parameter names should have been checked in meta.do_read_end" );
    }
  }
  std::unique_ptr< rdf_sampler > result(new rdf_sampler);
  result->width_ = width;
  result->stepsize_ = stepsize;
  boost::shared_ptr< base_observable > tmp(result.release());
  return tmp;

}

// Report radial distribution/density profiles
void rdf_sampler::on_report(std::ostream & out, base_sink & sink)
{
  // Write out RDF histograms
  for( std::size_t ispec = 0; ispec != this->specie_count(); ++ispec )
  {
    for( std::size_t jspec = ispec; jspec != this->specie_count(); ++jspec )
    {
      auto & histo = this->data_sets_[ this->get_index( ispec, jspec ) ];
      const std::string label { "rdf-"+this->specie_labels_[ispec]+"-"+this->specie_labels_[jspec]+".dat" };
      if( not sink.has_dataset( label ) )
      {
        // need to give a data set definition
        std::unique_ptr< observable::output_series >definition{ new observable::output_series( label, false ) };
        definition->set_title( "Radial distribution histogram for species "+this->specie_labels_[ispec]+" and "+this->specie_labels_[jspec] );
        {
          std::unique_ptr< observable::output_field > field( new observable::digitizer_output( "RMIN RMID RMAX", "ANGSTROM ANGSTROM ANGSTROM", histo.axis(), observable::digitizer_output::USE_ALL ) );
          definition->push_back_field( std::move( field ) );
        }
        {
          std::unique_ptr< observable::output_field > field( new observable::mean_variance_output( "P.mean P.var", "Rate Rate2" ) );
          definition->push_back_field( std::move( field ) );
        }
        sink.add_dataset( std::move( definition ) );
      }
  
      if( histo.count() > 0 )
      {
        std::unique_ptr< observable::output_series_datum > datum( new observable::output_series_datum{ 0ul, new utility::estimate_array( std::move( histo.release_data() ) ) } );
        sink.receive_data( label, std::move( datum ) );
      }
    }
  }

}

void rdf_sampler::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  {
    for( auto & entry : this->data_sets_ )
    {
      entry.begin_sample();
    }
    // Compute and sample rij 
    const particle::ensemble &ens( pman.get_ensemble() );
    std::vector< double > rij( ens.size() );
    std::vector< std::size_t > offsets( pman.specie_count() );
    // M(i,j) = data_sets_[ (2N+1-i)*i/2 + j ]
    //
    // offset_base = 2N+1
    std::size_t offset_base = (2 * pman.specie_count() + 1);
    for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
    {
      offsets[ ispec ] = ( ispec == 0 ? 0ul : ((offset_base - ispec) * ispec) / 2 );
    }
    for ( std::size_t ith = 0; ith != ens.size() - 1; ++ith )
    {
      const std::size_t ispec( ens.key( ith ) );
      if ( ispec != particle::specie_key::nkey )
      {
        gman.calculate_distances( ens.position( ith ), ens.get_coordinates(), rij, ith + 1, ens.size() );
        for ( std::size_t jth = ith + 1; jth != ens.size(); ++jth )
        {
          const std::size_t jspec( ens.key( jth ) );
          if ( jspec != particle::specie_key::nkey )
          {
            std::size_t ii = std::min( ispec, jspec );
            std::size_t jj = std::max( ispec, jspec );
            auto & histo = this->data_sets_[ offsets[ ii ] + (jj - ii) ];
            if( histo.in_range( rij[ jth ] ) )
            {
              histo.sample_datum( rij[ jth ] );
            }
          }
        }
      }
    }
    for( auto & entry : this->data_sets_ )
    {
      entry.end_sample();
    }
  }

}

// Prepare the sampler for a simulation.
//
// Initialize histogram matrix and capture specie labels.
void rdf_sampler::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  this->specie_labels_.clear();
  this->specie_labels_.resize( pman.specie_count() );
  std::vector< double > radii( pman.specie_count() );
  for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
  {
    auto const& spc = pman.get_specie( ispec );
    this->specie_labels_[ ispec ] = spc.label();
    radii[ ispec ] = spc.radius();
  }
  this->data_sets_.clear();
  for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
  {
    for( std::size_t jspec = ispec; jspec != pman.specie_count(); ++jspec )
    {
      const double min = radii[ ispec ] + radii[ jspec ];
      const double max = min + this->width_;
      this->data_sets_.push_back( { min, max, this->stepsize_, false } );
    }
  }
  // calculate expected data set size
  UTILITY_CHECK( this->data_sets_.size() == ((pman.specie_count() * (pman.specie_count() + 1)) / 2), "Size not what was expected." );

}

//Type label is "rdf-specie"
std::string rdf_sampler::type_label_()

{
  return std::string("rdf-specie");
}

// Add sampler type, width and stepsize to wr[ix]
void rdf_sampler::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );
  wr[ ix ].add_entry( "width", this->width_ );
  wr[ ix ].add_entry( "stepsize", this->stepsize_ );

}

// Get histogram for specie pair (i,j)
//
// \pre ispec < specie_count and jspec < specie_count
const utility::histogram& rdf_sampler::get_histogram(std::size_t ispec, std::size_t jspec) const
{
  UTILITY_REQUIRE( ispec < this->specie_count(), "Specie index out of range." );
  UTILITY_REQUIRE( jspec < this->specie_count(), "Specie index out of range." );
  if( ispec > jspec )
  {
    std::swap( ispec, jspec );
  }
  return this->data_sets_[ this->get_index( ispec, jspec ) ];

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::rdf_sampler );