#ifndef IONCH_CORE_CONSTANTS_HPP
#define IONCH_CORE_CONSTANTS_HPP

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

#include <iostream>

namespace core {

//  -----------------------------------------------
//  PHYSICAL CONSTANTS
//
//  The set of physical constants used in this
//  program.  There are standard and derived
//  constants.
//
//  The physical constants are all accessed using
//  a function interface.
//
//  -----------------------------------------------
//  ARRAY AND LOOP SIZES (C++ only)
//
//  Public definitions of array sizes that must
//  be known in multiple modules.
//
//  The sizes are defined using an anonymous
//  enumeration.
//
//  -----------------------------------------------
//  SET INDICES (C++ only)
//
//  Fixed indices used across modules. These
//  include named constants for specific regions
//  and for file ids.
//
//  The indices are defined using an anonymous
//  enumeration.
//

class constants
 {
   private:
    enum 
     {
      izlim =  0
	// Region in channel defined by +/-zl1
	,
      ifilt =  1
	// Region from channel end to end
	,
      ichan =  2
	// Region outside channel
	,
      ibulk =  3
      

    };

    enum 
     {
      nionmx =  2048

	// Maximum number of particles`
	,
      ntotmx =  8192

	// Maximum number of ICC patches
	,
      npchmx =  2048

	// Maximum number of species
	,
      nspcmx =  16

	// Maximum number of salts
	,
      nsltmx =  4

	// Maximum nuber of particles per salt
	,
      nnewmx =  4

	// Maximum nuber of regions
	,
      nrgnmx =  4

	// Maximum number of histogram bins in z direction
	,
      nzgmx =  4096

	// Default random seed 
	,
      mag =  12584210
      

    };


   public:
    // Value of pi as used in program.
    //
    // (A fixed value is used to ensure common(ish) 
    // value across programmes)
    constexpr static double pi()
    {
      // 'pi'
      return 3.141592653589793;
    };

    // Conversion from particles/per Ang**3 to 
    // Molar {to S.I.}
    // [ (10E-30)m3 / (N_av)mol * L / (10E-3)m3 ] 
    // [ (10E-27/N_av) L/mol ]
    //
    // Units (psuedo SI) l{1} mol{-1}
    // .:. reciprocal molar or molal
    constexpr static double to_SI()
    {
      return 1660.539276735512625080121;
    };

    // Convert Amgstrom to meters
    //
    // Units (m{1} Angstrom{-1})
    constexpr static double angstrom()
    {
      return 1.E-10;
    }

    // Permittivity of a vacuum
    //
    // Epsilon_0 
    // Units : m{-3} C{2} kg{-1} s{2}
    // (equiv) : m{-3} kg{-1} s{4} A{2}
    constexpr static double epsilon_0()
    {
       return 8.8542E-12;
    };

    // Avogardo's Number (not used in program, but listed as it is the
    // value used in calculating 'tosi' (N)
    //
    //Units # mol{-1}
    constexpr static double avogadro_number()
    {
      return 6.02214E23;
    };

    // Boltzmann's constant
    //
    // Units: (J{1} K{-1})
    // (equiv) : m{2} kg{1} s{-2} K{-1}
    constexpr static double boltzmann_constant()
    {
      return 1.3806E-23;
    };

    // Charge of an electron
    //
    // units (C{1})
    constexpr static double electron_charge()
    {
      return 1.6021917E-19;
    };

    // The input file version number
    static constexpr int filver = 1;

    // The maximum input file version number the program 
    // understands
    static constexpr int fvermx = 1;

    //Report the values of constants used in this program.
    static void description(std::ostream & a_os);


};

} // namespace core
#endif
