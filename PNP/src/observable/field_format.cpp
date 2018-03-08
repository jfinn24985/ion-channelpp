//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or (at
//your option) any later version.
//
//This program is distributed in the hope that it will be useful, but
//WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------


#ifndef DEBUG
#define DEBUG 0
#endif

#include "observable/field_format.hpp"
#include "utility/estimate_array.hpp"

namespace observable {

// Main ctor, use_rank choses rank or index as key selector in write.
element_output::element_output(std::string name, std::string unit, bool use_rank)
: output_field( name, unit )
, keys_()
, use_rank_( use_rank )
{}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void element_output::do_write(std::ostream & os, const utility::estimate_array &, std::size_t idx, std::size_t rank) const
{
  os << this->keys_.at( this->use_rank_ ? rank : idx );
}

// Main ctor.
index_output::index_output(std::string name, std::string unit)
: output_field( name, unit )
{}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void index_output::do_write(std::ostream & os, const utility::estimate_array &, std::size_t idx, std::size_t) const
{
  os << idx;
}

// Main ctor, use_rank choses rank or index as key selector in write.
key_output::key_output(std::string name, std::string unit, bool use_rank)
: output_field( name, unit )
, keys_()
, use_rank_( use_rank )
{}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void key_output::do_write(std::ostream & os, const utility::estimate_array &, std::size_t idx, std::size_t rank) const
{
  os << this->keys_.at( this->use_rank_ ? rank : idx );
}

// Main ctor.
mean_output::mean_output(std::string name, std::string unit)
: output_field( name, unit )
{}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void mean_output::do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t) const
{
  os << arr.mean( idx );
}

// Main ctor.
mean_variance_output::mean_variance_output(std::string name, std::string unit)
: output_field( name, unit )
{}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void mean_variance_output::do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t) const
{
  os << arr.mean( idx ) << " " << arr.variance( idx );
}

// Main ctor.
rank_output::rank_output(std::string name, std::string unit)
: output_field( name, unit )
{}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void rank_output::do_write(std::ostream & os, const utility::estimate_array &, std::size_t, std::size_t rank) const
{
  os << rank;
}

// Main ctor.
sample_count_output::sample_count_output(std::string name, std::string unit)
: output_field( name, unit )
{}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void sample_count_output::do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t, std::size_t) const
{
  os << arr.count();
}

// Main ctor.
variance_output::variance_output(std::string name, std::string unit)
: output_field( name, unit )
{}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void variance_output::do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t) const
{
  os << arr.variance( idx );
}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void digitizer_output::do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t) const
{
  bool prior = false;
  if( 0 != (this->parts_ & USE_MIN) )
  {
    os << this->axis_.bin_minimum( idx );
    prior = true;
  }
  if( 0 != (this->parts_ & USE_MID) )
  {
    if( prior )
    {
      os << " ";
    }
    os << this->axis_.bin_midpoint( idx );
    prior = true;
  }
  if( 0 != (this->parts_ & USE_MAX) )
  {
    if( prior )
    {
      os << " ";
    }
    os << this->axis_.bin_maximum( idx );
  }

}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void digitizer_3d_output::do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t) const
{
  bool prior = false;
  if( 0 != (this->parts_ & USE_MIN) )
  {
    os << this->axis_.bin_minimum( idx );
    prior = true;
  }
  if( 0 != (this->parts_ & USE_MID) )
  {
    if( prior )
    {
      os << " ";
    }
    os << this->axis_.bin_midpoint( idx );
    prior = true;
  }
  if( 0 != (this->parts_ & USE_MAX) )
  {
    if( prior )
    {
      os << " ";
    }
    os << this->axis_.bin_maximum( idx );
  }

}

combined_output::~combined_output()
{}

// Add an output field to the data set row
void combined_output::push_back_field(std::unique_ptr< output_field > field)
{
  if( this->children_.empty() )
  {
    this->set_label( field->label() );
    this->set_unit( field->unit() );
  }
  else
  {
    this->set_label( this->label() + " " + field->label() );
    this->set_unit( this->unit() + " " + field->unit() );
  }
  this->children_.push_back( field.release() );

}

// Does using this field consume an array element?
// Default value is false.
bool combined_output::consume_value() const
{
  // true if any fld returns true.
  for( auto &fld : this->children_ ) { if (fld.consume_value()) return true; }
  return false;
}

// Write formatted output to the given stream. Output
// may or may not include information from the given 
// parameters.
void combined_output::do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const
{
  for( auto & fld : this->children_ )
  {
    fld.write( os, arr, idx, rank );
    os << " ";
  }

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::combined_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::element_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::index_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::key_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::mean_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::mean_variance_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::rank_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::sample_count_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::variance_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::digitizer_output );
BOOST_CLASS_EXPORT_IMPLEMENT( observable::digitizer_3d_output );
