#ifndef IONCH_CORE_INPUT_PARAMETER_MEMO_HPP
#define IONCH_CORE_INPUT_PARAMETER_MEMO_HPP

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

#include "core/input_base_reader.hpp"
#include "core/base_input_parameter.hpp"
#include <string>
#include <cstddef>

namespace core {

// Concrete, canonical and serializable class with base_input_parameter interface.
class input_parameter_memo : public base_input_parameter
 {
   private:
    std::string name_;

    std::string value_;

    std::string line_;

    std::string filename_;

    std::size_t line_number_;


   public:
    // Empty constructor (useful for containers etc)
    input_parameter_memo()
    : name_(), value_(), line_(), filename_(), line_number_()
    {}

    // From a reader
    input_parameter_memo(const input_base_reader & reader)
    : name_( reader.name() )
    , value_( reader.value() )
    , line_( reader.line() )
    , filename_( reader.current_filename() )
    , line_number_( reader.current_line_number() )
    {}

    virtual ~input_parameter_memo() {}

    input_parameter_memo(input_parameter_memo && source)
    : name_( std::move( source.name_ ) )
    , value_( std::move( source.value_ ) )
    , line_( std::move( source.line_ ) )
    , filename_( std::move( source.filename_ ) )
    , line_number_( std::move( source.line_number_ ) )
    {}

    input_parameter_memo(const input_parameter_memo & source)
    : name_( source.name_ )
    , value_( source.value_ )
    , line_( source.line_ )
    , filename_( source.filename_ )
    , line_number_( source.line_number_ )
    {}
    // Explicitly copy other base_input_parameter objects to this
    // type.
    explicit input_parameter_memo(const base_input_parameter & source)
    : name_( source.name() )
    , value_( source.value() )
    , line_( source.line() )
    , filename_( source.filename() )
    , line_number_( source.line_number() )
    {}
    void swap(input_parameter_memo & source)
    {
      std::swap( name_, source.name_ );
      std::swap( value_, source.value_ );
      std::swap( line_, source.line_ );
      std::swap( filename_, source.filename_ );
      std::swap( line_number_, source.line_number_ );
    }
    input_parameter_memo & operator=(input_parameter_memo source)
    {
      this->swap( source );
      return *this;
    }


  friend class boost::serialization::access;    //Read/write object to an archive.
    template< class Archive > void serialize(Archive & ar, std::size_t a_ver)
    {
      ar & this->name_;
      ar & this->value_;
      ar & this->line_;
      ar & this->filename_;
      ar & this->line_number_;
    }

    // The parameter name
    virtual std::string name() const override
    {
      return this->name_;
    }

    // The parameter name
    virtual std::string value() const override
    {
      return this->value_;
    }

    // The parameter name
    virtual std::string line() const override
    {
      return this->line_;
    }

    // The parameter name
    virtual std::string filename() const override
    {
      return this->filename_;
    }

    // The parameter name
    virtual std::size_t line_number() const override
    {
      return this->line_number_;
    }

    // Alter the parameter name string.
    void set_name(std::string name)
    {
      this->name_ = name;
    }

    // Alter the parameter value string.  
    void set_value(std::string value)
    {
      this->value_ = value;
    }


};

} // namespace core
#endif
