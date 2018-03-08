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

#include "utility/machine.hpp"


#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <boost/format.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "core/strngs.hpp"
extern "C"
{
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fenv.h>

  /* --------------------------------------------------
  Set up in "C" to enable using the floating point 
  environment.
  -------------------------------------------------- */

static fenv_t original_env = { 0 }; /* Environment on program entry */
static int err_setup = -1; /* Whether an error occured during set up */

#ifdef __GNUC__
#define CONSTRUCTOR __attribute__ ((constructor))
#else
#define CONSTRUCTOR
#endif

static void CONSTRUCTOR fpu_setup(void)
{
  err_setup = fegetenv (&original_env);
  err_setup = -1 == err_setup ? -2 : err_setup;
#if defined(DEBUG) && DEBUG > 0
  /* Raise a signal for FP errors in DEBUG mode */
  feenableexcept(FE_INVALID   | 
                   FE_DIVBYZERO | 
                   FE_OVERFLOW  | 
                   FE_UNDERFLOW);
#endif
}


}
#include <stdexcept>
// end include

namespace utility {

// Include file that contains the actual compilation version
// information.
//#include "vers.cpp"

//The name the program was compiled as.
//

std::string machine_env::compilation_program_ = "gcc";

std::string machine_env::compilation_date_ = "date";

std::string machine_env::compilation_version_ = "version";

std::string machine_env::compiler_name_ = "gcc";

std::string machine_env::compiler_flags_ = "ddd";

std::string machine_env::compiler_clg_ = "ccc";

//Create a machine_env object.
std::unique_ptr< machine_env > machine_env::create()
{
  class helper_
  {
  public:
    static std::string file_to_string (std::string dirname, const char* fname)
    {
      std::string result;
      dirname.append ("/");
      dirname.append (fname);
      std::ifstream ifs_(dirname.c_str ());
      if (ifs_) std::getline (ifs_, result);
      return result;
    }
    static int file_to_number (std::string dirname, const char* fname)
    {
      int result = -1;
      std::string s_;
      dirname.append ("/");
      dirname.append (fname);
      std::ifstream ifs_(dirname.c_str ());
      if (ifs_)
      {
        ifs_ >> result;
        if (ifs_)
        {
          ifs_ >> s_;
          if (not s_.empty ())
          {
            if ('K' == s_[0] or 'k' == s_[0]) result *= 1024;
            else if ('M' == s_[0] or 'm' == s_[0]) result *= 1024*1024;
          }
        }
      }
      return result;
    }
  };
  utsname uname_;
  uname (&uname_);
  int cache_max = 0;
  const char num_to_char[] = { "0123456789" };
  int cache_sz[3] = { 0, 0, 0 };
  int cache_lnsz[3] = { 0, 0, 0  };
  for (int index = 0; index < 10; ++index)
  {
    std::string dirname ("/sys/devices/system/cpu/cpu0/cache/index_");
    dirname[dirname.size () - 1] = num_to_char[index];
    // std::cerr << "Checking directory \"" << dirname << "\"\n";
    struct stat st;
    if(stat (dirname.c_str (),&st) != 0)
    {
      // std::cerr << "Stat failed\n";
      break;
    }
    const std::string tmp_ (helper_::file_to_string (dirname, "type"));
    // std::cerr << "Got type \"" << tmp_ << "\"\n";
    if (std::string::npos != tmp_.find ("ata")
        or std::string::npos != tmp_.find ("nified"))
    {
      // Have data cache
      int level = helper_::file_to_number (dirname, "level") - 1;
      // std::cerr << "Found level \"" << level << "\"\n";
      if (0 <= level and level > 2) break; // Can`t handle more than three cache levels
      cache_max = std::max (cache_max, level);
      cache_lnsz[level] = helper_::file_to_number (dirname, "coherency_line_size");
      cache_sz[level] = helper_::file_to_number (dirname, "size");
      // std::cerr << "Found cache line \"" << cache_lnsz[level] << "\"\n";
      // std::cerr << "Found cache size \"" << cache_sz[level] << "\"\n";
    }
  }
  return std::unique_ptr< machine_env > (new machine_env (uname_.nodename
    , uname_.machine, uname_.sysname, uname_.version, uname_.release
    , cache_max
    , cache_lnsz[0], cache_sz[0]
    , cache_lnsz[1], cache_sz[1]
    , cache_lnsz[2], cache_sz[2]));

}

//Print copyright information and host/compilation settings.
void machine_env::description(std::ostream & a_os) const 
{
// host and environment information

const boost::format row_format("%1$20s : %2$6d");
a_os << "\n Program Information.\n";
a_os << " --------------------\n";

a_os << boost::format(row_format) % "program" % this->compilation_program_ << "\n";
a_os << boost::format(row_format) % "compiled version" % this->compilation_version_ << "\n";
a_os << boost::format(row_format) % "compiled date" % this->compilation_date_ << "\n";

// run date information
a_os << boost::format(row_format) % "run date" % boost::gregorian::to_simple_string(boost::gregorian::day_clock::local_day()) << "\n\n";

// program compilation information
a_os << " Compiler Information.\n";
a_os << " ---------------------\n";
a_os << boost::format(row_format) % "cpp compiler" % this->compiler_name_ << "\n";
a_os << boost::format(row_format) % "cpp target" % this->compiler_clg_ << "\n";
a_os << boost::format(row_format) % "cpp flags" % this->compiler_flags_ << "\n\n";

// host and environment information
a_os << " Host information.\n";
a_os << " -----------------\n";
// Print some machine information
a_os << " Host machine\n";
a_os << boost::format (row_format) % "name" % this->hostname << "\n";
a_os << " Operating system.\n";
a_os << boost::format (row_format) % "name" % this->os_name << "\n";
a_os << boost::format (row_format) % "version" % this->os_version << "\n";
a_os << boost::format (row_format) % "release" % this->os_release << "\n";
a_os << " Processor\n";
a_os << boost::format (row_format) % "type" % this->family << "\n";

if (0 != this->cache_depth)
{
  static const boost::format labelfmt ("L\%1\% cache");
  for (std::size_t lvl = 0; lvl != this->cache_depth; ++lvl)
  {
    const std::string label ( (boost::format(labelfmt) % (lvl + 1)).str () );
    if (cache_size[lvl] != -1)
    {
      a_os << boost::format (row_format) % (label + " size") % this->cache_size[lvl] << "\n";
      a_os << boost::format (row_format) % (label + " line size") % this->cache_line_size[lvl] << "\n";
    }
    else
    {
      a_os << boost::format (row_format) % label % "unknown or not present\n";
    }
  }
}
else
{
  a_os << boost::format (row_format) % "cache" % "unknown or not present\n";
}
a_os << boost::format (row_format) % "machine integer size" % (sizeof(int)*8) << "\n\n";
a_os << " Floating point numbers.\n";
a_os << " -----------------------\n";
a_os << boost::format (row_format) % "epsilon" % std::numeric_limits<double>::epsilon() << "\n";
a_os << boost::format (row_format) % "huge" % std::numeric_limits<double>::max() << "\n";
a_os << boost::format (row_format) % "denorm" % std::numeric_limits<double>::denorm_min() << "\n";
a_os << boost::format (row_format) % "IEC559/IEEE754" % (std::numeric_limits<double>::is_iec559 ? "true" : "false") << "\n";
utility::fp_env::env_.report (a_os);

}

machine_env::~machine_env() 
{}

//rounding direction
const uint32_t fp_env::Downward = FE_DOWNWARD;

//rounding direction
const uint32_t fp_env::ToNearest = FE_TONEAREST;

//rounding direction
const uint32_t fp_env::TowardZero = FE_TOWARDZERO;

//rounding direction
const uint32_t fp_env::Upward = FE_UPWARD;

//exception flag
const uint32_t fp_env::DivByZero = FE_DIVBYZERO;

//exception flag
const uint32_t fp_env::Inexact = FE_INEXACT;

//exception flag
const uint32_t fp_env::Invalid = FE_INVALID;

//exception flag
const uint32_t fp_env::Overflow = FE_OVERFLOW;

//exception flag
const uint32_t fp_env::Underflow = FE_UNDERFLOW;

//exception flag
const uint32_t fp_env::AnyException = FE_ALL_EXCEPT;

//Singleton FP environment
fp_env fp_env::env_;

struct fp_env::content
{
  std::fenv_t env_;
};fp_env::fp_nonstop_scope::fp_nonstop_scope()
: content_(new fp_env::content)
{
  feholdexcept(&(content_->env_));
}

fp_env::fp_nonstop_scope::~fp_nonstop_scope()
{
  // We must reset all errors to avoid throwing an exception out of this dtor
  feclearexcept(FE_ALL_EXCEPT);
  feupdateenv( &(content_->env_) );
  delete content_;
}

//Test if the floating point environment was initialised properly.
bool fp_env::ensure() 
{
  return err_setup == 0;
}


//Get description of the current error. Should be called after ensure
//or no_except returned false.  Will return only the first exception
std::string fp_env::error_message() 
{
  if (err_setup == -1)
  {
    return "Floating point environment setup function not called.";
  }
  else if (err_setup != 0)
  {
    return "Non-zero return value setting up floating point environment.";
  }
  std::string errormsg;
  if (fetestexcept(DivByZero))
  {
    errormsg += " DivByZero";
  }
  if (fetestexcept(Inexact))
  {
    errormsg += " Inexact";
  }
  if (fetestexcept(Invalid))
  {
    errormsg += " Invalid";
  }
  if (fetestexcept(Overflow))
  {
    errormsg += " Overflow";
  }
  if (fetestexcept(Underflow))
  {
    errormsg += " Underflow";
  }
  if (not errormsg.empty ())
  {
    return "floating point exceptions:" + errormsg;
  }
  return errormsg;

}

//Return which exceptions occured.  Foreach exception that
//has been raised the corresponding constant will be ORed to
//the result.
uint32_t fp_env::except() 
{
  unsigned int result_ = 0;
  if (fetestexcept(FE_DIVBYZERO))
    {
      result_ |= DivByZero;
    }
  if (fetestexcept(FE_INEXACT))
    {
      result_ |= Inexact;
    }
  if (fetestexcept(FE_INVALID))
    {
      result_ |= Invalid;
    }
  if (fetestexcept(FE_OVERFLOW))
    {
      result_ |= Overflow;
    }
  if (fetestexcept(FE_UNDERFLOW))
    {
      result_ |= Underflow;
    }
  return result_;

}

//Test if a floating point exception occured
bool fp_env::no_except() 
{
  return 0 == fetestexcept(FE_ALL_EXCEPT);

}

//reset the exceptions in the current environment
//
//\throw std::runtime if reset operation failed
void fp_env::reset() 
{
  if (0 != feclearexcept(FE_ALL_EXCEPT))
  {
    throw std::runtime_error ("could not reset floating point exceptions");
  }

}

//Report the floating point environment
void fp_env::report(std::ostream & a_os) const 
{
  a_os << "Floating point rounding mode: ";
  switch (fegetround ())
    {
    case FE_DOWNWARD:
      a_os << "Downward";
      break;
    case FE_TONEAREST:
      a_os << "To Nearest";
      break;
    case FE_TOWARDZERO:
      a_os << "Toward Zero";
      break;
    case FE_UPWARD:
      a_os << "Upward";
      break;
    default:
      a_os << "UNKNOWN";
      break;
    }
  a_os << "\nCurrent floating point exceptions: ";
  const int excpts (fetestexcept(FE_ALL_EXCEPT));
  if (excpts & FE_DIVBYZERO)
    {
      a_os << "DivByZero ";
    }
  if (excpts & FE_INEXACT)
    {
      a_os << "Inexact ";
    }
  if (excpts & FE_INVALID)
    {
      a_os << "Invalid ";
    }
  if (excpts & FE_OVERFLOW)
    {
      a_os << "Overflow ";
    }
  if (excpts & FE_UNDERFLOW)
    {
      a_os << "Underflow ";
    }
  a_os << "\n";

}

//Get the floating point rounding mode. 
//
//\return equal to Downward, ToNearest, TowardZero or
//Upward.
int fp_env::round() const 
{
  return fegetround();

}

//Set the floating point rounding mode. 
//
//\throw std::domain_error if mode is not
//equal to Downward, ToNearest, TowardZero or
//Upward.
void fp_env::set_round(uint32_t mode) 
{
  switch (mode)
    {
    case Downward:
    case Upward:
    case ToNearest:
    case TowardZero:
      fesetround (mode);
      break;
    default:
      throw std::domain_error("Attempt to set floating point rounding to unknown mode");
    }

}


} // namespace utility
