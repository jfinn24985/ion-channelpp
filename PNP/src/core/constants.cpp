//Authors: Justin Finnerty
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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "core/constants.hpp"
#include <boost/format.hpp>


// -- manual includes
#include "core/strngs.hpp"
#include <boost/format.hpp>
// --

namespace core {

// The input file version number
constexpr int constants::filver;

// The maximum input file version number the program 
// understands
constexpr int constants::fvermx;

//Report the values of constants used in this program.
void constants::description(std::ostream & a_os)

{
  const boost::format line1_("%1$20s : %2$12.8g");
  a_os << " Physical Constants as used in program.\n";
  a_os << " --------------------------------------\n";
  a_os << boost::format(line1_) % "pi" % pi () << "\n";
  a_os << boost::format(line1_) % "Anstrom (/m)" % angstrom () << "\n";
  a_os << boost::format(line1_) % "Epsilon 0" % epsilon_0 () << "\n";
  a_os << boost::format(line1_) % "Avogadro Number" % avogadro_number () << "\n";
  a_os << boost::format(line1_) % "Boltzmann Constant" % boltzmann_constant () << "\n";
  a_os << boost::format(line1_) % "Electron Charge" % electron_charge () << "\n";
  a_os << boost::format(line1_) % "n/Ang^3 to Molar" % to_SI () << "\n";

}


} // namespace core
