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

#include "observable/density_zaxis.hpp"
#include "observable/sampler_meta.hpp"
#include "core/input_parameter_memo.hpp"
#include "observable/base_sink.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "observable/report_manager.hpp"
#include "core/input_document.hpp"

// Manuals
#include "core/input_base_reader.hpp"
#include "core/strngs.hpp"
#include "observable/field_format.hpp"
#include "observable/output_series_datum.hpp"
#include "observable/output_series.hpp"
#include "particle/ensemble.hpp"
// - 
namespace observable {

// For serialization and factory only
density_zaxis::density_zaxis() = default;
density_zaxis::~density_zaxis() = default;

// Add definition for generating objects of this 
// class to the meta object.
void density_zaxis::add_definition(sampler_meta & meta)
{
  std::string desc( "Sample particle density distributions along the z axis (=axis of rotation for cylinder regions)." );
  std::unique_ptr< sampler_definition > result( new sampler_definition( density_zaxis::type_label_(), desc, &density_zaxis::make_sampler ) );
  // extra parameters
  result->add_definition( { "stepsize", "distance in Angstrom", ">0", std::to_string( default_stepsize() ), "Bin width of the 1D population histogram." } );
  meta.add_sampler_type( result );
  

}

// Log message descibing the observable and its parameters
void density_zaxis::description(std::ostream & out) const 
{
  out << " " << this->type_label_() << "\n";
  out << " " << std::string( this->type_label_().size(), '-' ) << "\n";
  out << "    Collect and report 1D population histogram along z-axis\n";
  out << "    - Default histogram bin width   : " << this->stepsize_ << "\n";

}

//Retrieve the current z-axis density profiles.
//
//return value is of type const*const vector< estimate_array >
boost::any density_zaxis::get_value() const
{
  boost::any result = &this->data_sets_;
  return result;

}

// Make a z-axis population sampler
//
// Allowed parameters : "stepsize"
boost::shared_ptr< base_observable > density_zaxis::make_sampler(std::vector< core::input_parameter_memo > const& param_set)
{
  double stepsize = density_zaxis::default_stepsize();
  const std::string param_name = "stepsize";
  for( auto& param : param_set )
  {
    if ( param_name == param.name() )
    {
      stepsize = param.get_float( density_zaxis::type_label_(), core::strngs::sampler_label(), true, false );
    }
    else
    {
      UTILITY_REQUIRE( param.name() == core::strngs::fsend(), "Parameter names should have been checked in meta.do_read_end" );
    }
  }
  std::unique_ptr< density_zaxis > result( new density_zaxis );
  result->stepsize_ = stepsize;
  boost::shared_ptr< base_observable > tmp( result.release() );
  return tmp;

}

// Report 1D population histograms
void density_zaxis::on_report(std::ostream & out, base_sink & sink)
{
  // Write out gz data
  for( std::size_t ispec = 0; ispec != this->data_sets_.size(); ++ispec )
  {
    std::string label{ this->filenames_[ ispec ] };
    auto & data = this->data_sets_[ ispec ];
    if( not sink.has_dataset( label ) )
    {
      // need to give a data set definition
      std::unique_ptr< observable::output_series >definition{ new observable::output_series( label, false ) };
      definition->set_title( "Z-axial distribution histogram for specie "+std::to_string( ispec ) );
      {
        std::unique_ptr< observable::output_field > field( new observable::digitizer_output( "ZMIN ZMID ZMAX", "ANGSTROM ANGSTROM ANGSTROM", data.axis(), observable::digitizer_output::USE_ALL ) );
        definition->push_back_field( std::move( field ) );
      }
      {
         std::unique_ptr< observable::output_field > field( new observable::mean_variance_output( "P.mean P.var", "Rate Rate2" ) );
          definition->push_back_field( std::move( field ) );
      }
      sink.add_dataset( std::move( definition ) );
    }
     
    if( data.count() > 0 )
    {
      std::unique_ptr< observable::output_series_datum > datum( new observable::output_series_datum{ 0ul, new utility::estimate_array( std::move( data.release_data() ) ) } );
      sink.receive_data( label, std::move( datum ) );
    }
  }
  

}

void density_zaxis::on_sample(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman)
{
  // Sample the z axis
  const particle::ensemble &ens( pman.get_ensemble() );
  for( auto &histo : this->data_sets_ )
  {
    histo.begin_sample();
  }
  for( std::size_t ith = 0; ith != ens.size(); ++ith )
  {
    const std::size_t ispec = ens.key( ith );
    if( ispec != particle::specie_key::nkey )
    {
      this->data_sets_[ ispec ].sample_datum( ens.z( ith ) );
    }
  }
  for( auto &histo : this->data_sets_ )
  {
    histo.end_sample();
  }
  

}

// Prepare the observer for a simulation. Reset any existing data.
void density_zaxis::prepare(const particle::particle_manager & pman, const geometry::geometry_manager & gman, const evaluator::evaluator_manager & eman, const report_manager & sman)
{
  // Reset all
  this->data_sets_.clear();
  this->filenames_.clear();
  
  // get simulation extent
  geometry::coordinate llh, urh;
  
  gman.system_region().extent( llh, urh, 0.0 );
  
  double lower_bound = llh.z;
  double upper_bound = urh.z;
  UTILITY_CHECK( lower_bound < upper_bound, "Bad region extent definition." );
  
  // Create data sets.
  this->data_sets_.resize( pman.specie_count(), utility::histogram( lower_bound, upper_bound, this->stepsize_, false ) );
  this->filenames_.resize( pman.specie_count() );
  
  for (std::size_t ispec = 0; ispec != pman.specie_count(); ++ispec)
  {
    this->filenames_[ ispec ] = "gz-"+pman.get_specie(ispec).label()+".dat";
  }

}

//Type label is "population-zaxis"
std::string density_zaxis::type_label_()

{
  return std::string("population-zaxis");
}

// Add sampler type, width and stepsize to wr[ix]
void density_zaxis::do_write_document(core::input_document & wr, std::size_t ix) const
{
  wr[ ix ].add_entry( core::strngs::fstype(), this->type_label_() );
  wr[ ix ].add_entry( "stepsize", this->stepsize_ );

}

// Data set for specie ispec
//
// \pre ispec < size
utility::histogram density_zaxis::data_set(std::size_t ispec) const
{
  return this->data_sets_.at( ispec );
}

// The number of data sets.
std::size_t density_zaxis::size() const
{
  return this->data_sets_.size();
}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::density_zaxis );