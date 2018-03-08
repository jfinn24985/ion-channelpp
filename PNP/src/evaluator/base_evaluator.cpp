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

#include "evaluator/base_evaluator.hpp"
#include "particle/particle_manager.hpp"
#include "geometry/geometry_manager.hpp"
#include "particle/change_set.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "core/input_document.hpp"

// manual includes
#include "core/strngs.hpp"
//-
namespace evaluator {

//Log message descibing the evaluator and its parameters
void base_evaluator::description(std::ostream & os) const
{
  os << core::strngs::evaluator_label() << " [" << this->type_label() << "]\n";
  this->do_description( os );

}

// Add an input file section.
//
// only throw possible should be from os.write() operation
//
// The output of this factory method is made up like
//
// evaluator
// type <type_label()>
// <call do_write_input_section>
// end

void base_evaluator::write_document(core::input_document & wr) const 
{
std::size_t ix = wr.add_section( core::strngs::evaluator_label() );
wr[ ix ].add_entry( core::strngs::fstype(), this->type_label() );
this->do_write_document( wr, ix );

}


} // namespace evaluator

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(evaluator::base_evaluator, "evaluator::base_evaluator");