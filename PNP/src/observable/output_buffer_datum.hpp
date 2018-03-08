#ifndef IONCH_OBSERVABLE_OUTPUT_BUFFER_DATUM_HPP
#define IONCH_OBSERVABLE_OUTPUT_BUFFER_DATUM_HPP

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

#include "observable/output_datum.hpp"
#include <string>

namespace observable { class output_series; } 
namespace observable { class output_text; } 
namespace observable { class output_rows; } 

namespace observable {

struct output_buffer_datum : public output_datum 
{
    // serialization only
    output_buffer_datum()
    : output_datum()
    , buffer()
    {}

    // Main only
    output_buffer_datum(std::string buf)
    : output_datum()
    , buffer( buf )
    {}

    virtual ~output_buffer_datum() {}


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_datum >(*this);
      ar & this->buffer;
    }

    void visit(output_series & sink) override;


   public:
    virtual void visit(output_text & sink) override;


   private:
    void visit(output_rows & sink) override;


   public:
    // A generic buffer for transferring information.
    std::string buffer;


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::output_buffer_datum );
#endif
