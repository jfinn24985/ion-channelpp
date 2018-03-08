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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "utility/utility.hpp"


//--
#include <stdexcept>
#include <sstream>
#include "core/input_help.hpp"
#include "utility/config.hpp"

#ifndef HAS_NO_BACKTRACE
extern "C"
{
#include <execinfo.h>
}
#endif

namespace utility {


#ifndef HAS_NO_BACKTRACE
using ::backtrace_symbols;
#else // don't HAVE_BACKTRACE so provide dummy implementations
static char** backtrace_symbols (void*const*, int)
{
   return 0;
}
#endif

//Assert method
void do_assert(const char * a_test, std::string a_msg, const char * a_fn, const char * a_filename, int a_linenum, void * const* a_backtrace, int a_backsz)
{
   //    static const int s_size (4096);
   //    char what_buffer_[s_size];
   //    boost::iostreams::array_sink what_sink_ (&what_buffer_[0], s_size);
   //    boost::iostreams::stream< boost::iostreams::array_sink > errlog_ (what_sink_);
   std::string result_;
   {
      std::stringstream errlog_;
      // --
      errlog_ << "\n================== CONTRACT FAILURE ==================\n"
              << "REASON: \"" << a_msg << "\"\n"
              << "FAILED TEST: (" << a_test << ")\n"
              << "FILE: " << a_filename << ", LINE: " << a_linenum << ".\n";

      if (NULL != a_fn)
      {
         errlog_ << "FUNCTION: " << a_fn << ".\n";
      }

      if (NULL != a_backtrace)
      {
         errlog_ << "BACKTRACE: \n";
         boost::shared_ptr< char * > trace_ (utility::backtrace_symbols (a_backtrace, a_backsz), &std::free);

         for (int l_ = 0; l_ < a_backsz; ++l_)
         {
            errlog_ << * (trace_.get () + l_) << "\n";
         }
      }

      errlog_ << "================== CONTRACT FAILURE ==================\n";
      result_ = errlog_.str ();
   }
   throw std::runtime_error (result_);
}

void do_assert_input(const char * a_test, std::string a_msg, std::string a_section, core::input_base_reader const*const a_reader, const char * a_fn, const char * a_filename, int a_linenum, void * const* a_backtrace, int a_backsz)
{
   //    static const int s_size (4096);
   //    char what_buffer_[s_size];
   //    boost::iostreams::array_sink what_sink_ (&what_buffer_[0], s_size);
   //    boost::iostreams::stream< boost::iostreams::array_sink > errlog_ (what_sink_);
   std::string result_;
   {
      std::stringstream errlog_;
      // --
      errlog_ << "\n================== INPUT FAILURE ==================\n"
              << "NIPUT FILE SECTION \"" << a_section << "\"\n"
              << "REASON: \"" << a_msg << "\"\n"
              << "FAILED TEST: (" << a_test << ")\n"
              << "FILE: " << a_filename << ", LINE: " << a_linenum << ".\n";
      result_ = errlog_.str ();
   }
   throw std::runtime_error (result_);
}




} // namespace utility
