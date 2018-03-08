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

#include "observable/widom_row.hpp"

// manual includes
#include "utility/utility.hpp"
// -
// -
namespace observable {

// Output a space separated list of the field labels.
void widom_row::labels(std::ostream & os) const
{
  os << "INDEX ";
  for( auto &dat : this->data_ )
  {
    dat.labels( os );
    os << " ";
  }

}

// Output a space separated list of the field units.
void widom_row::units(std::ostream & os) const
{
  os << "ORDINAL ";
  for( auto &dat : this->data_ )
  {
    dat.units( os );
    os << " ";
  }

}

// Output a space separated list of the field entries.
void widom_row::row(std::ostream & os) const
{
  os << loopindex_ << " ";
  for( auto &dat : this->data_ )
  {
    dat.row( os );
    os << " ";
  }

}

// Merge the given row into this row.
//
// \pre size = source.size and loopindex = loopindex
void widom_row::merge(std::unique_ptr< output_row > source)
{
  widom_row* source_ptr = dynamic_cast< widom_row* >( source.get() );
  UTILITY_REQUIRE( source_ptr != nullptr, "Data is not of required subtype." ); 
  UTILITY_REQUIRE( this->size() == source_ptr->size(), "Data size mismatch." );
  UTILITY_REQUIRE( this->loopindex() == source_ptr->loopindex(), "Loop index mismatch." );
  for( std::size_t idx = 0; idx != this->data_.size(); ++idx )
  {
    this->data_[ idx ].merge( source_ptr->data_[ idx ] );
  }

}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::widom_row );