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

#include "observable/base_observable.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "observable/report_manager.hpp"
#include "observable/base_sink.hpp"
#include "core/input_document.hpp"
#include "particle/change_set.hpp"

// Manual includes
#include "core/strngs.hpp"
// -
namespace observable {

base_observable::~base_observable()  = default;

// Add an input file section to wr.
//
// Output of this method is something like
//
// sampler
// <call do_write_input_section>
// end

void base_observable::write_document(core::input_document & wr) const 
{
  std::size_t ix = wr.add_section(core::strngs::sampler_label());
  this->do_write_document(wr, ix);

}

tracked_observable::~tracked_observable() = default;


} // namespace observable

BOOST_CLASS_EXPORT_IMPLEMENT( observable::tracked_observable );