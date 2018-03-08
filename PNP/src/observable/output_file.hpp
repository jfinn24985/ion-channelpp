#ifndef IONCH_OBSERVABLE_OUTPUT_FILE_HPP
#define IONCH_OBSERVABLE_OUTPUT_FILE_HPP

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
#include <cstddef>
#include "utility/unique_ptr.hpp"

namespace utility { class estimate_array; } 
namespace observable { class base_sink; } 
namespace observable { struct output_datum; } 

namespace observable {

// A column of data in the output.
class output_field
 {
   private:
    // Single word labelling the content of the field.
    std::string label_;

    // Single word indicating units of field
    std::string unit_;


   public:
    // Simple default ctor
    output_field();

    // Main ctor
    output_field(std::string name, std::string unit);
    

    virtual ~output_field();


   private:
    // NO copy ctor
    output_field(const output_field & source) = delete;

    // Simple  movement ctor
    output_field(output_field && source) = delete;


  friend class boost::serialization::access;    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & label_;
      ar & unit_;
    }

    // Simple assignment operation
    output_field & operator=(const output_field& source) = delete;


   public:
    // Does using this field consume an array element?
    // Default value is false.
    virtual bool consume_value() const
    {
      return false;
    }

    // The user readable name of the field
    std::string label() const
    {
       return this->label_;
    }

    //  Set the user readable field label
    void set_label(std::string name)
    {
      this->label_ = name;
    }

    //  Set the field's unit label
    void set_unit(std::string name)
    {
      this->unit_ = name;
    }

    // The human readable label for the units of the field
    std::string unit() const
    {
       return this->unit_;
    }

    //  Check that we have label, unit and formatter function set.
    bool valid() const
    {
      return not ( this->label_.empty() or this->unit_.empty() );
    }

    // Write out formatted field data to the stream. For data
    // sets, this could be the field in the idxth row of the data.
    // For data series the idx can be ignored.
    void write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const;


   private:
    // Write formatted output to the given stream. Output
    // may or may not include information from the given 
    // parameters.
    virtual void do_write(std::ostream & os, const utility::estimate_array & arr, std::size_t idx, std::size_t rank) const = 0;


};
//  Base output dataset class.
class output_dataset
 {
   private:
    // Unique label to identify each data_sink object, also used as data key when saving to sink
    // object.
    std::string label_;

    //  Human readable title/description of the file content
    std::string title_;


   protected:
    // Simple default ctor
    output_dataset()
    : label_()
    , title_()
    {}


   public:
    // Main ctor
    output_dataset(std::string label)
    : label_( label )
    , title_()
    {}

    // Simple default dtor
    virtual ~output_dataset() {}


   private:
    // No copy ctor
    output_dataset(const output_dataset & source) = delete;

    // No move ctor
    output_dataset(output_dataset & source) = delete;


   public:
    // No assignment operator
    output_dataset& operator=(const output_dataset & source) = delete;


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & title_;
      ar & label_;
    }


   public:
    // Get the title of the dataset
    std::string label() const
    {
      return this->label_;
    }

    // Get the title of the dataset
    std::string title() const
    {
      return this->title_;
    }

    // Create file if it doesn't exist and write the header and body. If
    // the file exists append new data.
    virtual void write(base_sink & sink) const = 0;

    // Attempt to accept a datum.
    virtual void accept(std::unique_ptr< output_datum > datum) = 0;


   private:
    // Transfer arbitrary data as a character buffer. This version 
    void receive_data(std::string buf);


   public:
    // Set the title
    void set_title(std::string val)
    {
      this->title_ = val;
    }


};

} // namespace observable
#endif
