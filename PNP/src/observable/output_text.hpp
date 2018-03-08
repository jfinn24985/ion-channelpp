#ifndef IONCH_OBSERVABLE_OUTPUT_TEXT_HPP
#define IONCH_OBSERVABLE_OUTPUT_TEXT_HPP

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

#include "observable/output_file.hpp"
#include <string>
#include "utility/unique_ptr.hpp"
#include <iostream>

namespace observable { struct output_datum; } 
namespace observable { class base_sink; } 

namespace observable {

// This manages preformatted output data. Data can not be merged, instead it is 
// simply appended to the buffer. Also previous data is not stored in the object,
// with the internal buffer being reset each time it is written to the sink.
//
// This type is designed for generating some sort of data log. For example a
// log of some raw data from a simulation that we want to analyse after the
// simulation is complete.
class output_text : public output_dataset
 {
   private:
    // The labels of the data fields
    std::string field_labels_;

    // The units of the data fields
    std::string units_;

    // The current output
    mutable std::string buffer_;

    // If true then the data set is a series of estimate_arrays,
    // one for each report cycle.
    bool is_serial_;

    // Serialization constructor
    output_text()
    : output_dataset()
    , field_labels_()
    , units_()
    , buffer_()
    , is_serial_()
    {}


   public:
    // Main constructor
    output_text(std::string label, bool serial)
    : output_dataset( label )
    , field_labels_()
    , units_()
    , buffer_()
    , is_serial_( serial )
    {}

    // Simple dtor
    ~output_text();


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_dataset >(*this);
      ar & field_labels_;
      ar & units_;
      ar & buffer_;
      ar & is_serial_;
    }


   public:
    // Attempt to accept a datum.
    void accept(std::unique_ptr< output_datum > datum) override;

    // Write the header of the data file
    //
    // \pre not empty
    void do_header(std::ostream & os) const;

    // Whether to replace or append the buffer to the file
    bool is_serial() const
    {
      return this->is_serial_;
    }

    // Capture data.
    //
    // \pre not buf.empty
    void receive_data(std::string buf);

    // Set the field labels
    void set_field_labels(std::string val)
    {
      this->field_labels_ = val;
    }

    // Set the field labels
    void set_units(std::string val)
    {
      this->units_ = val;
    }

    // Create file if it doesn't exist and write the header and body. If
    // the file exists append new data.
    //
    // \pre not empty
    void write(base_sink & sink) const;


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::output_text );
#endif
