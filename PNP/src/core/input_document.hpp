#ifndef IONCH_CORE_INPUT_DOCUMENT_HPP
#define IONCH_CORE_INPUT_DOCUMENT_HPP

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

#include <boost/serialization/map.hpp>
#include <string>
#include <cstddef>
#include <iostream>
#include <boost/serialization/vector.hpp>

// manual includes
#include <sstream>
// -
namespace core {

//  Data for a section in an input_document.
class input_section
 {
   private:
    // Name/value pairs
    std::map<std::string, std::string> content_;

    // The label of the section
    std::string label_;


   public:
    input_section() = default;

    input_section(std::string lbl)
    : content_()
    , label_( lbl )
    {}

    ~input_section() = default;

    input_section(const input_section & source)
    : content_( source.content_ )
    , label_( source.label_ )
    {}
    input_section(input_section && source)
    : content_( std::move( source.content_ ) )
    , label_( std::move( source.label_ ) )
    {}
   input_section & operator=(input_section source)
   {
     this->swap( source );
     return *this;
   }

    void swap(input_section & source)
    {
       std::swap( this->content_, source.content_ );
       std::swap( this->label_, source.label_ );
    }

  friend class boost::serialization::access;    //Read/write object to an archive.
    template< class Archive > void serialize(Archive & ar, std::size_t a_ver)
    {
      ar & this->content_;
      ar & this->label_;
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value. 
    
    void add_entry(std::string name, std::string value)
    {
      this->content_[ name ] = value;
    }
    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, bool value)
    {
      this->content_[ name ] = ( value ? "true" : "false" );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, int value)
    {
      this->content_[ name ] = std::to_string( value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, int64_t value)
    {
      this->content_[ name ] = std::to_string( value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, long long value)
    {
      this->content_[ name ] = std::to_string( value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, uint32_t value)
    {
      this->content_[ name ] = std::to_string( value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, unsigned long value)
    {
      this->content_[ name ] = std::to_string( value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, unsigned long long value)
    {
      this->content_[ name ] = std::to_string( value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, float value)
    {
      if (value < 1.0E6 and value > 1.0E-6)
        this->content_[ name ]= std::to_string( value );
      else
        this->add_entry< float >( name, value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, double value)
    {
      if (value < 1.0E6 and value > 1.0E-6)
        this->content_[ name ]= std::to_string( value );
      else
        this->add_entry< double >( name, value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    void add_entry(std::string name, long double value)
    {
      if (value < 1.0E6 and value > 1.0E-6)
        this->content_[ name ]= std::to_string( value );
      else
        this->add_entry< long double >( name, value );
    }

    // Add name/value pair to section
    //
    // Calls to this method with the same 'name' parameter
    // will replace any previous value even if the type of value
    // is different.
    
    template< class T > void add_entry(std::string name, T value)
    {
      std::stringstream ss;
      ss << value;
      this->content_[ name ] = ss.str();
    }

    std::size_t empty() const
    {
      return this->content_.empty();
    }

    // Get value of entry with the given name.
    //
    //  \pre has_entry( name )
    std::string get_entry(std::string name) const
    {
      return this->content_.at( name );
    }

    // Does this input section have entry with the given name.
    bool has_entry(std::string name) const
    {
      return 0 != this->content_.count( name );
    }

    const std::string& label() const
    {
      return this->label_;
    }

    void set_label(std::string lbl)
    {
      this->label_ = lbl;
    }
    // How many entries?
    std::size_t size() const
    {
      return this->content_.size();
    }

    //  Write the content in input file format to the given stream.
    void write(std::ostream & os) const;


};
// Manage writing of an input file
//
// Designed to allow the following "safe" usage. Note 
//
// void f(input_file_writer &wr)
// {
//   std::size_t ix = wr.add_section( "a" );
//   wr[ix].add_entry( "name", "value1" );
//   wr[ix].add_entry( "name2", "value2" );
//   // allow 'g' to update current section as well as adding new sections
//   g(wr, ix);
// }
//
// void g(input_file_writer &wr, std::size_t ix)
// {
//   // note overwrite of existing name in map
//   // silently over-writes existing entry
//   wr[ix].add_entry( "name", "value3" );
//   std::size_t ix2 = wr.add_section( "b" );
//   wr[ix2].add_entry( "name", "value1" );
// }

class input_document
 {
   private:
    // The content of the input file.
    std::vector< input_section > content_;

    // The version of the input file
    std::size_t version_;


   public:
    // Construct writer with the given input file version.
    explicit input_document(std::size_t ver)
    : content_()
    , version_( ver )
    {}

   private:
    // Default ctor only for serialization.
    input_document() = default;


   public:
    ~input_document() = default;

    input_document(const input_document & source)
    : content_( source.content_ )
    , version_( source.version_ )
    {}
    input_document(input_document && source)
    : content_( std::move( source.content_ ) )
    , version_( std::move( source.version_ ) )
    {}
    input_document & operator=(input_document source)
    {
      this->swap( source );
      return *this;
    }

    void swap(input_document & source)
    {
       std::swap( content_, source.content_ );
       std::swap( version_, source.version_ );
    }


  friend class boost::serialization::access;    //Read/write object to an archive.
    template< class Archive > void serialize(Archive & ar, std::size_t a_ver)
    {
      ar & content_;
      ar & version_;
    }

    // Create and add an new section with the given label and return
    // the index to it.
    
    std::size_t add_section(std::string label);

    // Remove a section with the given index.
    //
    // This make all indices higher than idx invalid.
    void remove_section(std::size_t idx);

    // Test if there is any content.
    bool empty() const
    {
       return this->content_.empty();
    }

    // Get a reference to the section by index.
    //
    //  \pre ix > size is undefined
    input_section& operator[](std::size_t ix)
    {
      return this->content_[ ix ];
    }

    // Return number of sections.
    std::size_t size() const
    {
       return this->content_.size();
    }

    // Return number of sections.
    std::size_t version() const
    {
       return this->version_;
    }

    //  Write the content in input file format to the given stream.
    void write(std::ostream & os);


};

} // namespace core
#endif
