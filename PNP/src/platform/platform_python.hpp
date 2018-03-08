#ifndef IONCH_PLATFORM_PLATFORM_PYTHON_HPP
#define IONCH_PLATFORM_PLATFORM_PYTHON_HPP

//Python wrappings for platform namespace
//
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

#include "particle/specie.hpp"
#include "geometry/coordinate.hpp"
#include "particle/ensemble.hpp"
#include "platform/simulator.hpp"
#include <boost/python.hpp>

using namespace boost::python;
using namespace platform;

particle::specie& (simulator::*get_specie_const)(size_t) = &simulator::get_specie;

BOOST_PYTHON_MODULE(platform)
{
}
#endif
