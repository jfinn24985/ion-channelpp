#ifndef IONCH_UTILITY_UTILITY_HPP
#define IONCH_UTILITY_UTILITY_HPP

//----------------------------------------------------------------------
//HELPER FUNCTIONS
//
//This contains a set of generic routines that can be used anywhere
//in the program.
//
//
// log - the global logging stream
//
// (Unit test suite "utility_test_suite")
//
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


// --
#include "utility/config.hpp"
#include "utility/mathutil.hpp"
#include <string>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <boost/shared_ptr.hpp>

// ----------------------------------------------------------------------
// IN-CODE TESTING.
//
//  Constants that control the inclusion of debugging and
//  design-by-contract code in the program.
//
// The simulator in use in this program consists of four checking
// conditionals and an input value conditional. All have the same
// interface of two arguments.  The first argument is an if test
// conditional that is expected to be true.  The second argument is a
// message giving details specific to the failed test. In addition the
// program reports information about the code location of the error
// (possibly including a call stack).
//
// Checking conditionals that may be optionally compiled into the
// program:
//
// * UTILITY_ALWAYS : This test is non-optional and should be used to
// verify conditions that are dependent on conditions while the progam
// is running.  Ideally it should not be used as it indicates an error
// in code that should have raised an exception for the error itself.
// In practice it might may be used for such things as checking
// C-library calls succeeded where you don't want to handle the error
// cases yourself.
//
// * UTILITY_INPUT : Check that runtime inputs to the program (from the
// user) are within allowed values.  This is also used to check data
// derived from input; for example to check that dividing some length
// into intervals gives a number of segments below a program limit.
// As input is never controlled by the program this test is always
// available.  In addition to the information provided with
// UTILITY_ALWAYS, this method calls a locally defined input_helper
// (std::ostream&,std::string) method.  This method can be used to
// create a detailed feedback message describing the expected input
// for the entire module or class in addition to the error-specific
// message provided in the method call.
// 
// * UTILITY_REQUIRE : Check the arguments to a method fit within the
// method's advertised input domain. As we do not necessarily
// completely know how the method will be called this is usually the
// last test to be removed by conditional compilation.
//
// * UTILITY_ENSURE : Test the programmer's logic has led to a method's
// result within the domain advertised by the method (and so expected
// by the method's caller). It should be used in any non-trivial
// method to ensure the method's post-conditions are met. Ideally if
// the method's input is within the advertised domain (as checked by
// UTILITY_REQUIRE) the results of a well-tested method should always be
// correct, so this is conditionally removed before UTILITY_REQUIRE
// by conditional compilation.
//
// * UTILITY_CHECK : Testing the programmer's logic.  This lets the
// progammer assert what they think should be true is actually true!
// It is intended to allow the programmer to insert a check of
// intermediate results during the development of a method. As a
// mainly developement test it is usually the first test to be
// removed by conditional compilation.
//
// For those c-libraries that provide a means to inspect the current
// call stack we use two methods 'backtrace' and 'backtrace_symbols'
// to generate a messag containing this information.  We use the same
// interface as the GNU libc methods.

#ifndef HAS_NO_BACKTRACE
extern "C"
{
#include <execinfo.h>
}
namespace utility
{
  using ::backtrace;
}
#else // HAS_NO_BACKTRACE so provide dummy implementations
namespace utility
{
  static inline int backtrace(void**,int) { return 0; }
}
#endif

#define UTILITY_ALWAYS(X,Y) do { if (!(X)) {				\
      void * TRACE[20]; const int sz_ = utility::backtrace (&TRACE[0], 20); \
      utility::do_assert(#X,(Y),__func__,__FILE__, __LINE__, &TRACE[0], sz_); }} while( false )

#if(DEBUG > 0)
#define UTILITY_CHECK UTILITY_ALWAYS
#define UTILITY_ENSURE UTILITY_ALWAYS
#define UTILITY_ENSURE_OLD(X) X
#define UTILITY_INDEX(X,Y,Z) UTILITY_ALWAYS(utility::mathutil::in_half_up_range(std::size_t(0),(Y),(X)),Z)
#define UTILITY_RANGE(W,X,Y,Z) UTILITY_ALWAYS(utility::mathutil::in_closed_range((X),(Y),(W)),Z)
#else
#define UTILITY_CHECK(X,Y) while( false ){}
#define UTILITY_ENSURE(X,Y) while( false ){}
#define UTILITY_ENSURE_OLD(X)  while( false ){}
#define UTILITY_INDEX(X,Y,Z) while( false ){}
#define UTILITY_RANGE(W,X,Y,Z) while( false ){}
#endif
#define UTILITY_REQUIRE UTILITY_ALWAYS

#define UTILITY_INPUT(X,Y,S,R) do { if (!(X)) {				\
      void * TRACE[20];	const int sz_ = utility::backtrace (&TRACE[0], 20); \
      utility::do_assert_input(#X,(Y),(S),(R),__func__,__FILE__, __LINE__, &TRACE[0], sz_); }} while( false )

#define UTILITY_STDC_ERROR(X, Y) do { if (!(X)) {			\
      void * TRACE[20];	const int sz_ = utility::backtrace (&TRACE[0], 20); \
      std::string msg_((Y)); msg_.append("\n"); msg_.append(strerror(errno)); \
      utility::do_assert(#X,msg_,__func__,__FILE__, __LINE__, &TRACE[0], sz_); }} while( false )

namespace core
{
  class input_base_reader;
};
namespace utility {

  // Assert method behind UTILITY_... macros
  void do_assert(const char * a_test, std::string a_msg, const char * a_fn, const char * a_filename, int a_linenum, void * const* a_backtrace, int a_backsz);

  void do_assert_input(const char * a_test, std::string a_msg, std::string a_section, core::input_base_reader const*const reader, const char * a_fn, const char * a_filename, int a_linenum, void * const* a_backtrace, int a_backsz);


} // namespace utility
#endif
