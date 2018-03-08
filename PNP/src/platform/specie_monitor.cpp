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

#include "platform/specie_monitor.hpp"
#include "core/strngs.hpp"
#include "observable/base_sink.hpp"
#include "core/input_document.hpp"

namespace platform {

specie_monitor::~specie_monitor() = default;

//Log message descibing the observable and its parameters
void specie_monitor::description(std::ostream & out) const 
{
  out << " " << this->get_label() << "\n";
  out << " " << std::string( this->get_label().size(), '-' ) << "\n";
  out << "    Sample average specie numbers for a client observer. This\n";
  out << "    observer can not be created from the input file.\n";

}

// Produces no report.
void specie_monitor::on_report(std::ostream & out, observable::base_sink & sink)
{

}

// Remove input file section for this observer.
void specie_monitor::do_write_document(core::input_document & wr, std::size_t ix) const
{
  // Not from input file, delete the section.
  wr.remove_section( ix );
  

}


} // namespace platform

BOOST_CLASS_EXPORT_IMPLEMENT( platform::specie_monitor );