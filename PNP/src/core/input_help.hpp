#ifndef IONCH_CORE_INPUT_HELP_HPP
#define IONCH_CORE_INPUT_HELP_HPP

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

#include <boost/noncopyable.hpp>

#include <boost/serialization/map.hpp>
#include <string>
#include "core/help_section.hpp"
#include <cstddef>
#include "core/input_base_reader.hpp"
#include <iostream>

// manuals
#include "core/fixed_width_output_filter.hpp"
//-
#ifndef HAS_NO_BACKTRACE
extern "C"
{
#include <execinfo.h>
}
#endif

namespace core {


#ifndef HAS_NO_BACKTRACE
using ::backtrace_symbols;
#else // don't HAVE_BACKTRACE so provide dummy implementations
static char** backtrace_symbols (void*const*, int)
{
   return 0;
}
#endif

//  GLOBAL INPUT FILE DESCRIPTION  HELPER
//
//  This class provides a consistent mechanism to generate a formatted
//  help message for the program input file sections.
//
// PATTERN : Singleton
//
// Class to format help messages on input file parameters
// 
// Formatter for information about the input file. 
class input_help : public boost::noncopyable
 {
   private:
    // The input section descriptions.
    std::map< std::string, help_section > sections_;


   public:
    static input_help & exemplar();

    input_help();

    ~input_help();

    // Are there any section description objects defined?
    bool empty() const
    {
      return this->sections_.empty();
    }

    // How many section descriptions are there?
    std::size_t size() const
    {
      return this->sections_.size();
    }

    // Add a section description.
    //
    // \pre not has_section( sect.title() )
    // \post has_section( sect.title() )
    void add_section(help_section sect);

    // Is there a section description set for this section label
    bool has_section(std::string label) const;

    help_section & get_section(std::string label);

    help_section const& get_section(std::string label) const;

    // Create an error message and throw an exception.
    void do_assert(char const* test, std::string mesg, std::string section, input_base_reader const* const reader, char const* func, char const* filename, int linenum, void* const* backtrace, int backsz) const;

    // Write help information for the name section to errlog.
    //
    // \pre not section.empty
    void write(std::string section, std::ostream & errlog) const;

    // Write the whole help information to errlog.
    void write(std::ostream & errlog) const;

    // Write help information for the named parameter (optionally
    // in the given section) to errlog.
    //
    // \pre not param.empty (but section.empty is okay.)
    
    void write(std::string section, std::string param, std::ostream & errlog) const;


};

} // namespace core
#endif
