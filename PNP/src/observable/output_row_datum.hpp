#ifndef IONCH_OBSERVABLE_OUTPUT_ROW_DATUM_HPP
#define IONCH_OBSERVABLE_OUTPUT_ROW_DATUM_HPP

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
#include "utility/unique_ptr.hpp"
#include <cstddef>

namespace observable { class output_row; } 
namespace observable { class output_series; } 
namespace observable { class output_text; } 
namespace observable { class output_rows; } 

namespace observable {

struct output_row_datum : public output_datum 
{
    std::unique_ptr< output_row > row;

    std::size_t count;

    // serialization only
    output_row_datum()
    : output_datum()
    , row()
    , count()
    {}

    // Main only
    output_row_datum(std::unique_ptr< output_row > buf, std::size_t rank)
    : output_datum()
    , row( std::move( buf ) )
    , count( rank )
    {}

    virtual ~output_row_datum() {}


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_datum >(*this);
      ar & this->row;
      ar & this->count;
    }

    void visit(output_series & sink) override;

    void visit(output_text & sink) override;


   public:
    void visit(output_rows & sink) override;


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::output_row_datum );
#endif
