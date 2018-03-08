#ifndef IONCH_CORE_HELP_ENTRY_HPP
#define IONCH_CORE_HELP_ENTRY_HPP

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

#include "utility/archive.hpp"

#include <string>
#include <iostream>

namespace core { class fixed_width_output_filter; } 

namespace core {

class help_entry
 {
   private:
    // The one word title/label this description applies to.
    std::string title_;

    // Description of the expected type.
    std::string type_desc_;

    // Description of the expected range.
    std::string range_;

    // Description of any default value.
    std::string default_value_;

    // Description of the particular option.
    std::string description_;


   public:
    help_entry() = default;

    help_entry(std::string label, std::string type_desc, std::string range, std::string def_value, std::string desc)
    : title_( label )
    , type_desc_( type_desc )
    , range_( range )
    , default_value_( def_value )
    , description_( desc )
    {}

    ~help_entry() = default;

    help_entry(help_entry && source) = default;

    help_entry(const help_entry & source) = default;

    help_entry & operator=(help_entry source)
    {
      this->swap( source );
      return *this;
    }

    void swap(help_entry & other)
    {
      std::swap( this->title_, other.title_ );
      std::swap( this->type_desc_, other.type_desc_ );
      std::swap( this->range_, other.range_ );
      std::swap( this->default_value_, other.default_value_ );
      std::swap( this->description_, other.description_ );
    }
  friend class boost::serialization::access;

   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & this->title_;
        ar & this->type_desc_;
        ar & this->range_;
        ar & this->default_value_;
        ar & this->description_;
      }


   public:
    // Get the corresponding attribute.
    const std::string& default_value() const
    {
      return this->default_value_;
    }

    // Set the corresponding attribute.
    void default_value(std::string value)
    {
      this->default_value_ = value;
    }
    

    // Get the corresponding attribute.
    const std::string& description() const
    {
      return this->description_;
    }

    // Set the corresponding attribute.
    void description(std::string value)
    {
      this->description_ = value;
    }
    

    // Get the corresponding attribute.
    const std::string& range() const
    {
      return this->range_;
    }

    // Set the corresponding attribute.
    void range(std::string value)
    {
      this->range_ = value;
    }
    

    // Get the corresponding attribute.
    const std::string& title() const
    {
      return this->title_;
    }

    // Set the corresponding attribute.
    void title(std::string value)
    {
      this->title_ = value;
    }
    

    // Get the corresponding attribute.
    const std::string& type_desc() const
    {
      return this->type_desc_;
    }

    // Set the corresponding attribute.
    void type_desc(std::string value)
    {
      this->type_desc_ = value;
    }
    

    // Write help description to destination stream using the given filter.
    void write(fixed_width_output_filter & filt, std::ostream & dest) const;


};

} // namespace core
#endif
