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

#include "trial/choice.hpp"
#include "particle/change_set.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "utility/random.hpp"

// Manuals
#include "core/strngs.hpp"
#include "particle/ensemble.hpp"
#include "platform/simulator.hpp"
// -
namespace trial {

base_choice::~base_choice() = default;


} // namespace trial

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(trial::base_choice, "trial::base_choice");


