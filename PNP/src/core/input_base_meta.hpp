#ifndef IONCH_CORE_INPUT_BASE_META_HPP
#define IONCH_CORE_INPUT_BASE_META_HPP

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

#include <string>

namespace core { class input_base_reader; } 
namespace core { class input_help; } 

namespace core {

// Manage information between input file format and real classes.
//
// Lifetime pattern: virtual, no copy
class input_base_meta : public boost::noncopyable
 {
   private:
    //The name of the input section.
    std::string section_label_;

    //Can this section appear more than once
    bool multiple_;

    //is this section required?
    bool required_;


   public:
    input_base_meta(std::string label, bool multiple, bool required);

    virtual ~input_base_meta() {}

    //Can this section appear multiple times?
    bool multiple() const
    {
      return this->multiple_;
    }

    // Interpret a section  of the input file.
    //
    // throw an error if input file is incorrect (using UTILITY_INPUT macro)
    void read_section(input_base_reader & reader);

    virtual void publish_help(input_help & helper) const = 0;


   private:
    //Read a name/value entry in the input file. Return true if the entry was processed.
    //
    //throw an error if input file is incorrect (using UTILITY_INPUT macro)
    virtual bool do_read_entry(input_base_reader & reader) = 0;

    // Perform checks at the end of reading a section and create/add result to
    // simulations. If multiple sections may be read, this method must also
    // reset the meta object state.
    virtual void do_read_end(const input_base_reader & reader) = 0;

    // Reset the object state on entry into read_seaction. This
    // provides a stronger exception safety if the delegate is
    // reused after raising an exception in read_section.
    virtual void do_reset() = 0;


   public:
    //Is an instance of this section required?
    bool required() const
    {
      return this->required_;
    }

    std::string section_label() const
    {
      return this->section_label_;
    }


};

} // namespace core
#endif
