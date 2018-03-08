#ifndef IONCH_PARTICLE_SPECIE_KEY_HPP
#define IONCH_PARTICLE_SPECIE_KEY_HPP

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

// manual includes
#include <cstddef>
// -
namespace particle {

// Define important specie key values
enum specie_key {
  nkey = ~(std::size_t(0)),// Invalid specie key value
  first = 0// The lowest possible key value (should be zero)

};

} // namespace particle
#endif
