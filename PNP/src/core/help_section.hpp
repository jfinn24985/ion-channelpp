#ifndef IONCH_CORE_HELP_SECTION_HPP
#define IONCH_CORE_HELP_SECTION_HPP

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

#include "core/help_subtype.hpp"
#include <boost/serialization/map.hpp>
#include <string>
#include <cstddef>
#include <iostream>

namespace core { class fixed_width_output_filter; } 

namespace core {

class help_section : private help_subtype
 {
   public:
    typedef std::map< std::string, help_subtype >::iterator iterator;

    typedef std::map< std::string, help_subtype >::const_iterator const_iterator;


   private:
    // optional subtypes for this stanza type
    std::map< std::string, help_subtype > subtypes_;


   public:
    help_section(std::string label, std::string desc)
    : help_subtype( label, desc )
    {}
    

    help_section() = default;

    ~help_section() = default;

    help_section(help_section && source)
    : help_subtype( std::move( source ) )
    , subtypes_( std::move( source.subtypes_ ) )
    {}
    help_section(const help_section & source)
    : help_subtype( source )
    , subtypes_( source.subtypes_ )
    {}

    help_section & operator=(help_section source)
    {
      this->swap( source );
      return *this;
    }

    void swap(help_section & other)
    {
      this->help_subtype::swap( other );
      std::swap( this->subtypes_, other.subtypes_ );
    }
  friend class boost::serialization::access;

   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< help_subtype >(*this);
        ar & this->subtypes_;
      }


public:
  // Methods we want to make public from help_subtype
  using help_subtype::description;
  using help_subtype::title;
  using help_subtype::add_entry;
  using help_subtype::has_entry;
  using help_subtype::get_entry;

   public:
    help_subtype::iterator entry_begin()
    {
      return this->help_subtype::begin();
    }

    help_subtype::const_iterator entry_begin() const
    {
      return this->help_subtype::begin();
    }

    help_subtype::iterator entry_end()
    {
      return this->help_subtype::end();
    }

    help_subtype::const_iterator entry_end() const
    {
      return this->help_subtype::end();
    }

    bool entry_empty() const
    {
      return this->help_subtype::empty();
    }

    std::size_t entry_size() const
    {
      return this->help_subtype::size();
    }

    iterator subtype_begin()
    {
      return this->subtypes_.begin();
    }

    const_iterator subtype_begin() const
    {
      return this->subtypes_.begin();
    }

    iterator subtype_end()
    {
      return this->subtypes_.end();
    }

    const_iterator subtype_end() const
    {
      return this->subtypes_.end();
    }

    bool subtype_empty() const
    {
      return this->subtypes_.empty();
    }

    std::size_t subtype_size() const
    {
      return this->subtypes_.size();
    }

    // Write help description to destination stream using the given filter.
    // Include descriptions for child entries.
    void write(fixed_width_output_filter & filt, std::ostream & dest) const;

    // Write help description for the named parameter to destination stream 
    // using the given filter. If not an immediate entry, try subtype entries.
    bool write(fixed_width_output_filter & filt, std::string param, std::ostream & dest) const;

    // Add a new help entry to this master entry
    //
    // \pre not has_subtype( entry.title )
    // \post has_subtype( entry.title )
    void add_subtype(help_subtype entry);

    // Does this master entry have an entry with the given title?
    bool has_subtype(std::string title) const
    {
      return this->subtypes_.find( title ) != this->subtypes_.end();
    }

    // Get subtype entry with the given title.
    //
    // \pre has_subtype( title )
    help_subtype& get_subtype(std::string title);

    // Get subtype entry with the given title.
    //
    // \pre has_subtype( title )
    const help_subtype& get_subtype(std::string title) const;


};

} // namespace core
#endif
