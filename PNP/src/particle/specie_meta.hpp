#ifndef IONCH_PARTICLE_SPECIE_META_HPP
#define IONCH_PARTICLE_SPECIE_META_HPP

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


#include "core/input_base_meta.hpp"
#include <boost/serialization/vector.hpp>
#include "utility/bitset.hpp"
#include <boost/serialization/shared_ptr.hpp>
#include "particle/specie.hpp"
#include <string>

namespace core { class strngs; } 
namespace core { class input_help; } 
namespace core { class help_entry; } 
namespace particle { class particle_manager; } 
namespace core { class input_base_reader; } 

namespace particle {

// Interpret specie definitions in the input file.
//
// (NOTE: inherits from noncopyable)
//
// Echo individual spec section
//
// Write out the interpreted specie data from the input
// file in the same format as an input file.  This includes program
// default values for optional input data and normalised rate
// values.
//
// Echo input section for this specie
//
// Write out the interpreted specie data from the input file in the
// same format as an input file.  This includes program default
// values for optional input data, normalised rate values and
// particle position information provided by conf module.
class specie_meta : public core::input_base_meta
 {
   public:
    // Option indices in missing_required_flags_
    enum
     {
      SPECIE_LABEL = 0,// Index of specie label option.
      SPECIE_TYPE = 1,// Index of specie type option 
      SPECIE_DIAMETER = 2,// Index of specie diameter option
      SPECIE_VALENCY = 3,// Index of specie valency option
      SPECIE_TAG_COUNT = 4// The number of required tags

    };


   private:
    //The set of valid keywords
    static std::vector<core::help_entry> known_keywords_;

    // Flags to check for missing options in input file
    std::bitset<SPECIE_TAG_COUNT> missing_required_tags_;

    boost::shared_ptr< particle_manager > manager_;

    //The current specie object
    specie specie_obj_;

    // No default ctor
    specie_meta() = delete;


   public:
    // Standard constructor.
    specie_meta(boost::shared_ptr< particle_manager > man);

    virtual ~specie_meta();

    // Add a parameter keyword to the known keywords
    //
    // \pre not is_standard_keyword(entry.title)
    // \pre not has_keyword(entry.title)
    // \post has_keyword(entry.title)
    static void add_keyword(const core::help_entry & entry);

    // Test if 'name' matches a standard keyword.
    static bool is_standard_keyword(std::string name);

    // Test if 'name' is already registered as a parameter keyword
    static bool has_keyword(std::string name);

    virtual void publish_help(core::input_help & helper) const override;


   private:
    //Read an entry in the input file. Return true if the entry was processed.
    //
    //throw an error if input file is incorrect (using UTILITY_INPUT macro)
    virtual bool do_read_entry(core::input_base_reader & reader) override;

    // Perform checks at the end of reading a section.
    virtual void do_read_end(const core::input_base_reader & reader) override;

    // Perform checks at the end of reading a section.
    virtual void do_reset() override;


};

} // namespace particle
#endif
