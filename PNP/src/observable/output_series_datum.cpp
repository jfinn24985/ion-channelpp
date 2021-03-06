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

#include "observable/output_series_datum.hpp"
#include "utility/estimate_array.hpp"
#include "observable/output_series.hpp"
#include "observable/output_text.hpp"
#include "observable/output_rows.hpp"

namespace observable {

void output_series_datum::visit(output_series & sink)
{
  sink.receive_data( this->count, std::move( this->arr ) );
}

void output_series_datum::visit(output_text & sink)
{
  UTILITY_REQUIRE( false, "This output datum can not update an output_text object." );
}

void output_series_datum::visit(output_rows & sink)
{
  UTILITY_REQUIRE( false, "This output datum can not update an output_rows object." );
}


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::output_series_datum );