#ifndef IONCH_UTILITY_MACHINE_HPP
#define IONCH_UTILITY_MACHINE_HPP

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

#include <string>
#include <cstddef>
#include "utility/unique_ptr.hpp"

#include "utility/config.hpp"
struct fenv_;
namespace utility {

//Access specific details of the machine environment
//
//This includes:
//Host name and type
//CPU cache information
//Operating system type
//Numeric limits
//
//This does NOT include:
//Library version information
//OpenMP information
//Run date
//Compiler information
//
//Notes:
//* CPU specific details all refer to the first CPU.
//* Cache information only considers data caches
class machine_env
{
   public:
    //The machines "hostname", as supplied by uname->nodename
    const std::string hostname;

    //The gross family of processor (x86, x86_64 etc), as supplied by uname
    const std::string family;

    //OS Name, as supplied by uname
    const std::string os_name;

    //OS version, as supplied by uname
    const std::string os_version;

    //OS release, as supplied by uname
    const std::string os_release;

    //Largest cache levels, may be 0, 1, 2 or 3 for no, L1, L2 and L3 caches
    //or -1 if no information could be found.
    const std::size_t cache_depth;

    //The L1 cache line size
    const std::size_t cache_line_size[3];

    //Cache size (in bytes)
    const int64_t cache_size[3];


   private:
    //The name the program was compiled as.
    //
    
    static std::string compilation_program_;

    static std::string compilation_date_;

    static std::string compilation_version_;

    static std::string compiler_name_;

    static std::string compiler_flags_;

    static std::string compiler_clg_;

    machine_env(const std::string & hname, const std::string & fam, const std::string & osname, const std::string & osver, const std::string & osrel, int depth, int l1lsz, int l1sz, int l2lsz, int l2sz, int l3lsz, int l3sz)
    : hostname (hname)
    , family (fam)
    , os_name (osname)
    , os_version (osver)
    , os_release (osrel)
    , cache_depth (std::min(3,depth))
    , cache_line_size ()
    , cache_size ()
    {
           const_cast< std::size_t& >(cache_line_size[0])=l1lsz;
           const_cast< int64_t& >(cache_size[0])=l1sz;
           const_cast< std::size_t& >(cache_line_size[1])=l2lsz;
           const_cast< int64_t& >(cache_size[1])=l2sz;
           const_cast< std::size_t& >(cache_line_size[2])=l3lsz;
           const_cast< int64_t& >(cache_size[2])=l3sz;
    }


   public:
    //Create a machine_env object.
    static std::unique_ptr< machine_env > create();

    //Print copyright information and host/compilation settings.
    void description(std::ostream & a_os) const;

    ~machine_env();

};
//Manage Floating Point environment
class fp_env {
   public:
    //rounding direction
    static const uint32_t Downward;

    //rounding direction
    static const uint32_t ToNearest;

    //rounding direction
    static const uint32_t TowardZero;

    //rounding direction
    static const uint32_t Upward;

    //exception flag
    static const uint32_t DivByZero;

    //exception flag
    static const uint32_t Inexact;

    //exception flag
    static const uint32_t Invalid;

    //exception flag
    static const uint32_t Overflow;

    //exception flag
    static const uint32_t Underflow;

    //exception flag
    static const uint32_t AnyException;

    //Singleton FP environment
    static fp_env env_;

struct content;    // Use RAII to define a scope with FP non-stop mode (ie
    // FP exceptions are not raised until the end of the scope).
    // Note that all FP exceptions are reset at the end of the scope
    // so if they are of interest they must be examined before reaching
    // the end of the scope.
    class fp_nonstop_scope
     {
       private:
        fp_env::content* content_;


       public:
        fp_nonstop_scope();

        ~fp_nonstop_scope();

        fp_nonstop_scope(const fp_nonstop_scope & source) = delete;

        fp_nonstop_scope(fp_nonstop_scope && source) = delete;

        fp_nonstop_scope & operator=(const fp_nonstop_scope & source) = delete;


    };
    

   private:
    //Initialise FP environment
    fp_env() {}

    ~fp_env() {}

    fp_env(const fp_env & source);

    fp_env & operator=(const fp_env & source);


   public:
    //Test if the floating point environment was initialised properly.
    bool ensure();
    //Get description of the current error. Should be called after ensure
    //or no_except returned false.  Will return only the first exception
    std::string error_message();

    //Return which exceptions occured.  Foreach exception that
    //has been raised the corresponding constant will be ORed to
    //the result.
    uint32_t except();

    //Test if a floating point exception occured
    bool no_except();

    //reset the exceptions in the current environment
    //
    //\throw std::runtime if reset operation failed
    void reset();

    //Report the floating point environment
    void report(std::ostream & a_os) const;

    //Get the floating point rounding mode. 
    //
    //\return equal to Downward, ToNearest, TowardZero or
    //Upward.
    int round() const;

    //Set the floating point rounding mode. 
    //
    //\throw std::domain_error if mode is not
    //equal to Downward, ToNearest, TowardZero or
    //Upward.
    void set_round(uint32_t mode);

};

} // namespace utility
#endif
