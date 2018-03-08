
#ifndef IONCH_UTILITY_CONFIG_HPP
#define IONCH_UTILITY_CONFIG_HPP 1

//Compilation environment configuration
//
//This file defines a set of preprocessor macros that define aspects of 
//the compilation environment.  It generates no C++ code itself.
//
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


// \brief define the level of testing to perform
// level 0 is minimal, level 5 is all
#define IONCH_CONTRACT_LEVEL 5

// ------------------------------------------------------------
// C++ recently added the C99 <stdint.h> header as <cstdint>. Here we
// select the header to use dependent on what the compiler vendor
// provided.
#ifdef HAS_NO_CSTDINT
extern "C"
{
#include <stdint.h>
}
#else
#include <cstdint>
#endif

// -------------------------------------------------------------------
// C++11 includes a number of maths functions from C99 that were not
// in C++03. Here we promote the C99 methods into the std namespace
// when necessary.
#ifdef HAS_NO_STD_CBRT
extern "C"
{
#include <math.h>
}
namespace std
{
  using ::cbrt;
}
#endif
#ifdef HAS_NO_STD_NEARBYINT
extern "C"
{
#include <math.h>
}
namespace std
{
  using ::nearbyint;
}
#endif
#ifdef HAS_NO_LRINT
extern "C"
{
    long int lrint ();
    long int lround ();
    long int lrintf ();
    long int lroundf ();
}
namespace std
{
  using ::lrint;
  using ::lround;
  using ::lrintf;
  using ::lroundf;
}
#endif
#ifdef HAS_NO_LLRINT
extern "C"
{
  long long int llrint ();
  long long int llrintf ();
  long long int llrintl ();
  long long int llround ();
  long long int llroundf ();
  long long int llroundl ();
}

namespace std
{
  using ::llrint;
  using ::llround;
  using ::llrintf;
  using ::llroundf;
  using ::llrintl;
  using ::llroundl;
}
# endif
// now include cmath to get the C++ overrides of anything in math.h
#include <cmath>

// -------------------------------------------------------------------
// C++11 includes a nullptr object but not always implemented
// 
// Workaround from the official proposal: SC22/WG21/N2431 = J16/07-0301
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2431.pdf
#ifdef HAS_NO_NULLPTR
#ifdef __GNUC__
#define nullptr __null
#else
const                        // this is a const object...
class {
public:
  template<class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...
  template<class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }
private:
  void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr
#endif
#endif

#endif // IONCH_UTILITY_CONFIG_HPP