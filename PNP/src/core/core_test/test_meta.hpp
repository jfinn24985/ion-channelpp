#ifndef IONCH__TEST_META_HPP
#define IONCH__TEST_META_HPP

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
#include <boost/serialization/map.hpp>
#include <string>
#include <cstddef>

namespace core { class input_base_reader; } 
namespace core { class input_help; } 

//Helper class to test input_base_meta/input_delegater relationship
class test_meta : public core::input_base_meta
 {
   private:
    // Map of entries.
    std::map< std::string, std::string > entries_;

    // The number of times do_read_entry has been called
    mutable std::size_t publish_help_count_;

    // The number of times do_read_entry has been called
    std::size_t read_end_count_;

    // The number of times do_read_entry has been called
    std::size_t read_entry_count_;

    // The number of times do_read_entry has been called
    std::size_t reset_count_;


   public:
    //4 arg version to match base class ctor.
    test_meta(std::string label, bool multiple = false, bool req = false)
    : input_base_meta (label, multiple, req)
    , entries_()
    , publish_help_count_()
    , read_end_count_()
    , read_entry_count_()
    , reset_count_()
    {}


   private:
    test_meta() = delete;


   public:
    virtual ~test_meta();


   protected:
    //Read an entry in the input file. Return true if the entry was processed.
    //
    //throw an error if input file is incorrect (using UTILITY_INPUT macro)
    virtual bool do_read_entry(core::input_base_reader & reader) override;

    // Perform checks at the end of reading a section.
    virtual void do_read_end(const core::input_base_reader & reader) override;

    // Perform checks at the end of reading a section.
    virtual void do_reset() override;


   public:
    virtual void publish_help(core::input_help & helper) const override
    {
       ++this->publish_help_count_;
    }

    // Get the map of entries
    std::map< std::string,  std::string > const& entry_map() const
    {
      return this->entries_;
    }

    inline const std::size_t get_publish_help_count() const;

    inline const std::size_t get_read_entry_count() const;

    inline const std::size_t get_read_end_count() const;

    std::size_t get_reset_count() const
    {
      return this->reset_count_;
    }


};
inline const std::size_t test_meta::get_publish_help_count() const
{
  return publish_help_count_;
}

inline const std::size_t test_meta::get_read_entry_count() const
{
  return this->read_entry_count_;
}

inline const std::size_t test_meta::get_read_end_count() const
{
  return this->read_end_count_;
}


#endif
