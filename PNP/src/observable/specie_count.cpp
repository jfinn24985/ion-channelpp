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

#include "observable/specie_count.hpp"
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

specie_count::~specie_count() = default;

// Add definition for generating objects of this 
// class to the meta object.
void specie_count::add_definition(sampler_meta & meta)
{
  std::string desc( "Observe and report the average number of particles from periodic sampling." );
  std::unique_ptr< sampler_definition > result( new sampler_definition( type_label_(), desc, &specie_count::make_sampler ) );
  // no extra parameters
  meta.add_sampler_type( result );
  

}

//Log message descibing the observable and its parameters
void specie_count::description(std::ostream & out) const 
{
  out << " " << this->type_label_() << "\n";
  out << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  out << "    Sample average specie numbers.\n";

}

//Retrieve the current specie count estimates
//
// \return type estimater_array const*const
boost::any specie_count::get_value() const
{
  boost::any result = &this->count_;
  return result;

}

// Generate specie_count object using data in paramset. As this
// sampler takes no special parameters the paramset should be
// empty.
boost::shared_ptr< base_observable > specie_count::make_sampler(std::vector< core::input_parameter_memo > const& paramset)
{
  UTILITY_REQUIRE( paramset.size() == 1, "This sampler requires no parameters." );
  boost::shared_ptr< base_observable > smplr(new specie_count);
  return smplr;

}

// Write the average specie counts to out and the data sink.
void specie_count::on_report(std::ostream & out, base_sink & sink)
{
  if( this->count_.count() > 0 )
  {
    boost::format fmt_head( " %3s %10s %10s %10s" );
    out << boost::format( fmt_head ) % "SPC" % "<COUNT>" % "VAR" % "<[]>" << "\n";
    const static boost::format fmt_row( " %3d %10.5f %10.4f %10.7f" );
    for( size_t idx = 0; idx != this->specie_list_.size(); ++idx )
    {
      const double count_to_conc = this->volume( idx ) / core::constants::to_SI();
      out << boost::format( fmt_row ) % this->specie_key( idx ) % this->mean( idx ) % this->variance( idx ) % (this->mean( idx ) * count_to_conc) << "\n";
    }
    const std::string label( "specie-count.dat" );
    if( not sink.has_dataset( label ) )
    {
      // need to give a data set definition
      std::unique_ptr< observable::output_series >definition{ new observable::output_series( label, true ) };
      definition->set_title( "Time series specie counts." );
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
          std::unique_ptr< observable::key_output > field{ new observable::key_output( "SPC", "INDEX", false ) };
          // Add keys.
          for( size_t idx = 0; idx != this->specie_list_.size(); ++idx )
          {
            std::stringstream hashval;
            hashval << this->specie_key( idx );
            field->push_back( hashval.str() );
          }
          combo->push_back_field( std::move( field ) );
        }
        {
          std::unique_ptr< observable::element_output > field{ new observable::element_output( "SPC.VOL", "ANGSTROM3", false ) };
          // Add keys.
          for( size_t idx = 0; idx != this->specie_list_.size(); ++idx )
          {
            field->push_back( this->volume( idx ) );
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
void specie_count::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  auto const& ens = pman.get_ensemble(); 
  std::vector< double > sample( this->specie_list_.size() );
  for( std::size_t idx = 0; idx != this->specie_list_.size(); ++idx )
  {
    sample[ idx ] = double( ens.specie_count( this->specie_list_[ idx ].first ) );
  }
  UTILITY_CHECK( sample.size() == this->count_.size(), "Mismatched array sizes" );
  this->count_.append( sample );

}

// Prepare the sampler for a simulation run. Reset any existing data.
void specie_count::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  this->specie_list_.clear();
  this->count_.clear();
  for( std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec )
  {
    if( pman.get_specie( ispec ).is_solute() )
    {
      std::pair< std::size_t, double > entry( ispec, gman.system_region().volume( pman.get_specie( ispec ).radius() ) );
      this->specie_list_.push_back( entry );
    }
  }
  // is no solute an error?
  if( not this->specie_list_.empty() )
  {
    this->count_.resize( this->specie_list_.size() );
  }
  this->rank_ = 0ul;

}

std::string specie_count::type_label_()
{
  return std::string("specie-count");
}

// Write an input file section.
//
// only throw possible should be from os.write() operation
void specie_count::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );
  

}

// Convert specie index/key into a local index.
//
// If specie is not in list return "size"
//
// \post idx <= size
std::size_t specie_count::local_index(std::size_t ispec) const
{
  for( std::size_t idx = 0; idx < this->size(); ++idx )
  {
    if ( this->specie_list_[ idx ].first == ispec ) return idx;
  }
  return this->size();
}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::specie_count );