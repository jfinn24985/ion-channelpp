#ifndef IONCH_PLATFORM_RANGE_T_HPP
#define IONCH_PLATFORM_RANGE_T_HPP

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

#include <boost/range/iterator_range.hpp>

#include <boost/ptr_container/serialize_ptr_vector.hpp>

namespace evaluator { class base_evaluator; } 

namespace platform {

typedef boost::iterator_range<boost::ptr_vector< evaluator::base_evaluator >::iterator> range_t;
typedef boost::iterator_range<boost::ptr_vector< evaluator::base_evaluator >::const_iterator> const_range_t;

} // namespace platform
#endif
