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

#include "observable/specie_region_count.hpp"
#include "observable/output_series.hpp"
#include "core/strngs.hpp"
#include "observable/sampler_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "observable/base_sink.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "observable/report_manager.hpp"
#include "core/input_document.hpp"

// manual includes
#include "observable/field_format.hpp"
#include "observable/output_series_datum.hpp"
#include "particle/ensemble.hpp"
// -
#include <boost/format.hpp>
// --
namespace observable {

specie_region_count::~specie_region_count() = default;

// Add definition for generating objects of this 
// class to the meta object.
void specie_region_count::add_definition(sampler_meta & meta)
{
  std::string desc( "Observe and report the average number of particles per region with periodic sampling." );
  std::unique_ptr< sampler_definition > result( new sampler_definition( type_label_(), desc, &specie_region_count::make_sampler ) );
  // no extra parameters
  meta.add_sampler_type( result );
  

}

//Log message descibing the observable and its parameters
void specie_region_count::description(std::ostream & out) const 
{
  out << " " << this->type_label_() << "\n";
  out << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  out << "    Sample average per-region specie numbers.\n";

}

//Retrieve the current specie count estimates
//
// \return type estimater_array const*const
boost::any specie_region_count::get_value() const
{
  boost::any result = &this->count_;
  return result;

}

// Generate specie_count object using data in paramset. As this
// sampler takes no special parameters the paramset should be
// empty.
boost::shared_ptr< base_observable > specie_region_count::make_sampler(std::vector< core::input_parameter_memo > const& paramset)
{
  UTILITY_REQUIRE( paramset.size() == 1, "This sampler requires no parameters." );
  boost::shared_ptr< base_observable > smplr(new specie_region_count);
  return smplr;

}

// Write the average specie counts to out and the data sink.
void specie_region_count::on_report(std::ostream & out, base_sink & sink)
{
  if( this->count_.count() > 0 )
  {
    // LOG OUTPUT
    // ----------
    boost::format fmt_head( " %3s %3s %10s %10s %10s" );
    out << boost::format( fmt_head ) % "REG" % "SPC" % "<COUNT>" % "VAR" % "<[]>" << "\n";
    const static boost::format fmt_row( " %3d %3d %10.5f %10.4f %10.7f" );
    const std::size_t nreg = this->count_.size() / this->nspec_;
    UTILITY_CHECK( ( this->count_.size() % this->nspec_ ) == 0, "Array size is not a multiple of the number of species." );
    for( size_t ireg = 0; ireg != nreg; ++ireg )
    {
      const std::size_t offset = ireg * this->nspec_;
      for( size_t ispec = 0; ispec != this->nspec_; ++ispec )
      {
        const double count_to_conc = this->volume( offset + ispec ) / core::constants::to_SI();
        out << boost::format( fmt_row ) % ireg % ispec % this->mean( offset + ispec ) % this->variance( offset + ispec ) % ( this->mean( offset + ispec ) * count_to_conc ) << "\n";
      }
    }  
    // SINK OUTPUT
    // -----------
    const std::string label( "specie-region-count.dat" );
    if( not sink.has_dataset( label ) )
    {
      // need to give a data set definition
      std::unique_ptr< observable::output_series >definition{ new observable::output_series( label, true ) };
      definition->set_title( "Time series specie / region counts." );
      {
        std::unique_ptr< observable::output_field > field{ new observable::rank_output( "INDEX", "ORDINAL" ) };
        definition->push_back_field( std::move( field ) );
      }
      {
        std::unique_ptr< observable::output_field > field{ new observable::sample_count_output( "SAMPLES", "ORDINAL" ) };
        definition->push_back_field( std::move( field ) );
      }
      {
        std::unique_ptr< observable::combined_output > combo{ new observable::combined_output };
        {
          std::unique_ptr< observable::key_output > field{ new observable::key_output( "REG:SPC", "INDEX", false ) };
          // Add keys.
          const std::size_t nreg = this->count_.size() / this->nspec_;
          UTILITY_CHECK( ( this->count_.size() % this->nspec_ ) == 0, "Array size is not a multiple of the number of species." );
          for( size_t ireg = 0; ireg != nreg; ++ireg )
          {
            for( size_t idx = 0; idx != this->nspec_; ++idx )
            {
              std::stringstream hashval;
              hashval << ireg << ":" << idx;
              field->push_back( hashval.str() );
            }
          }
          combo->push_back_field( std::move( field ) );
        }
        {
          std::unique_ptr< observable::element_output > field{ new observable::element_output( "REG:SPC:VOL", "ANGSTROM3", false ) };
          // Add keys.
          const std::size_t nreg = this->count_.size() / this->nspec_;
          UTILITY_CHECK( ( this->count_.size() % this->nspec_ ) == 0, "Array size is not a multiple of the number of species." );
          for( size_t ireg = 0; ireg != nreg; ++ireg )
          {
            const std::size_t offset = ireg * this->nspec_;
            for( size_t idx = 0; idx != this->nspec_; ++idx )
            {
              field->push_back( this->volume( offset + idx ) );
            }
          }
          combo->push_back_field( std::move( field ) );
        }
        {
          std::unique_ptr< observable::mean_variance_output > field{ new observable::mean_variance_output( "Count.mean Count.var", "Rate Rate2" ) };
          combo->push_back_field( std::move( field ) );
        }
        definition->push_back_field( std::move( combo ) );
      }
      sink.add_dataset( std::move( definition ) );
    }
    std::unique_ptr< observable::output_series_datum > datum( new observable::output_series_datum{ this->rank_, new utility::estimate_array{ this->count_.size() } } );
    datum->arr->swap( this->count_ );
    sink.receive_data( label, std::move( datum ) );
    ++this->rank_;
  }

}

// Sample average particle numbers.
void specie_region_count::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  auto const& ens = pman.get_ensemble();
  const std::size_t nspec = pman.specie_count();
  const std::size_t nreg = gman.region_count();
  std::vector< std::size_t > sample( this->count_.size() );
  std::vector< double > radii( nspec );
  UTILITY_CHECK( sample.size() == nspec * nreg, "Estimate array is wrong size for simulation" );
  // Region '0' is always all of cell
  for( std::size_t ispec = 0; ispec != nspec; ++ispec )
  {
    sample[ ispec ] = ens.specie_count( ispec );
    radii[ ispec ] = pman.get_specie( ispec ).radius();
  }
  if( nreg > 1 )
  {
    for( std::size_t ireg = 1; ireg != nreg; ++ireg )
    {
      const std::size_t offset = ireg * nspec;
      auto const& reg = gman.get_region( ireg );
      for( std::size_t idx = 0; idx != ens.size(); ++idx )
      {
        const std::size_t ispec = ens.key( idx );
        if( ispec != particle::specie_key::nkey )
        {
          if( reg.is_inside( ens.position( idx ), radii[ ispec ] ) )
          {
            ++sample[ offset + ispec ];
          }
        }
      }
    }
  }
  // we use "unsafe" append as we know "sample" is right size.
  this->count_.append( sample.begin(), sample.end() );

}

// Prepare the sampler for a simulation run. Reset any existing data.
void specie_region_count::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  this->count_.clear();
  this->nspec_ = pman.specie_count();
  const std::size_t nreg = gman.region_count();
  this->volume_.resize( this->nspec_ * nreg );
  std::vector< double > radii( this->nspec_ );
  for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
  {
    radii[ ispec ] = pman.get_specie( ispec ).radius();
  }
  for( std::size_t ireg = 0; ireg != gman.region_count(); ++ireg )
  {
    const std::size_t offset = ireg * this->nspec_;
    auto const& reg = gman.get_region( ireg );
    for( std::size_t ispec = 0; ispec != this->nspec_; ++ispec )
    {
      this->volume_[ offset + ispec ] = reg.volume( radii[ ispec ] );
    }
  }
  this->count_.resize( this->nspec_ * nreg );
  this->rank_ = 0ul;

}

std::string specie_region_count::type_label_()
{
  return std::string("specie-region-count");
}

// Write an input file section.
//
// only throw possible should be from os.write() operation
void specie_region_count::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );
  

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::specie_region_count );