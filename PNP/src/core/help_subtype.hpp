#ifndef IONCH_CORE_HELP_SUBTYPE_HPP
#define IONCH_CORE_HELP_SUBTYPE_HPP

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

#include <boost/serialization/map.hpp>
#include <string>
#include "core/help_entry.hpp"
#include <cstddef>
#include <iostream>

namespace core { class fixed_width_output_filter; } 

namespace core {

class help_subtype
 {
   public:
    typedef std::map< std::string, help_entry >::iterator iterator;

    typedef std::map< std::string, help_entry >::const_iterator const_iterator;


   private:
    // Sub-options of this entry.
    std::map< std::string, help_entry > children_;

    // The one word title/label this description applies to.
    std::string title_;

    // Description of the particular option.
    std::string description_;


   public:
    help_subtype(std::string label, std::string desc)
    : title_( label )
    , description_( desc )
    {}
    

    help_subtype() = default;

    ~help_subtype() = default;

    help_subtype(help_subtype && source)
    : children_( std::move( source.children_ ) )
    , title_( std::move( source.title_ ) )
    , description_( std::move( source.description_ ) )
    {}
    help_subtype(const help_subtype & source)
    : children_( source.children_ )
    , title_( source.title_ )
    , description_( source.description_ )
    {}

    help_subtype & operator=(help_subtype source)
    {
      this->swap( source );
      return *this;
    }

    void swap(help_subtype & other)
    {
      std::swap( this->children_, other.children_ );
      std::swap( this->title_, other.title_ );
      std::swap( this->description_, other.description_ );
    }
  friend class boost::serialization::access;

   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & this->children_;
        ar & this->title_;
        ar & this->description_;
      }


   public:
    iterator begin()
    {
      return this->children_.begin();
    }

    const_iterator begin() const
    {
      return this->children_.begin();
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
    

    iterator end()
    {
      return this->children_.end();
    }

    const_iterator end() const
    {
      return this->children_.end();
    }

    bool empty() const
    {
      return this->children_.empty();
    }

    std::size_t size() const
    {
      return this->children_.size();
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
    

    // Write help description to destination stream using the given filter.
    // Include descriptions for child entries.
    void write(fixed_width_output_filter & filt, std::ostream & dest) const;

    // Write help description for a specific parameter to destination stream using the 
    // given filter, if the parameter exists. Otherwise do nothing.
    //
    // \return true if param was entry in this subtype (== has_entry)
    //
    // \pre not param.empty
    bool write(fixed_width_output_filter & filt, std::string param, std::ostream & dest) const;

    // Add a new help entry to this master entry
    //
    // \pre not has_entry( entry.title )
    // \post has_entry( entry.title )
    void add_entry(help_entry entry);

    // Does this master entry have an entry with the given title?
    bool has_entry(std::string title) const
    {
      return this->children_.find( title ) != this->children_.end();
    }

    // Get entry with the given title.
    //
    // \pre has_entry( title )
    help_entry& get_entry(std::string title);

    // Get entry with the given title.
    //
    // \pre has_entry( title )
    const help_entry& get_entry(std::string title) const;


};

} // namespace core
#endif
