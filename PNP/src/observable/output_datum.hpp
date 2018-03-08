#ifndef IONCH_OBSERVABLE_OUTPUT_DATUM_HPP
#define IONCH_OBSERVABLE_OUTPUT_DATUM_HPP

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


namespace observable { class output_series; } 
namespace observable { class output_text; } 
namespace observable { class output_rows; } 

namespace observable {

// Visitor class for managing data transfers.
struct output_datum 
{

  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {}


   public:
    // serialization only
    output_datum() {}

    virtual ~output_datum() {}


   private:
    // No move
    output_datum(output_datum && source) = delete;

    // No copy
    output_datum(const output_datum & source) = delete;

    // No assignment
    output_datum & operator=(const output_datum & source) = delete;


   public:
    virtual void visit(output_series & sink) = 0;

    virtual void visit(output_text & sink) = 0;

    virtual void visit(output_rows & sink) = 0;


};

} // namespace observable
#endif
