#ifndef IONCH_OBSERVABLE_WIDOM_ROW_HPP
#define IONCH_OBSERVABLE_WIDOM_ROW_HPP

//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or (at
//your option) any later version.
//
//This program is distributed in the hope that it will be useful, but
//WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
//for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------

#include "observable/output_rows.hpp"
#include <boost/serialization/vector.hpp>
#include "observable/widom.hpp"
#include <cstddef>
#include <iostream>
#include "utility/unique_ptr.hpp"

namespace observable {

class widom_row : public output_row
 {
   private:
    std::vector< widom::widom_datum > data_;

    // Rank of data.
    std::size_t loopindex_;

    // serialization only
    widom_row()
    : output_row()
    , data_()
    , loopindex_()
    {}


   public:
    // Main
    widom_row(std::size_t loopcount)
    : output_row()
    , data_()
    , loopindex_( loopcount )
    {}
    

    virtual ~widom_row() {}


  friend class boost::serialization::access;
   private:
    template<class Archive>
      inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & boost::serialization::base_object< output_row >(*this);
      ar & this->data_;
      ar & this->loopindex_;
    }


   public:
    // Output a space separated list of the field labels.
    virtual void labels(std::ostream & os) const override;

    // Output a space separated list of the field units.
    virtual void units(std::ostream & os) const override;

    // Output a space separated list of the field entries.
    virtual void row(std::ostream & os) const override;

    // Merge the given row into this row.
    //
    // \pre size = source.size and loopindex = loopindex
    virtual void merge(std::unique_ptr< output_row > source) override;

    std::size_t size() const
    {
      return this->data_.size();
    }

    std::size_t loopindex() const
    {
      return this->loopindex_;
    }

    template< class Iter > void append(Iter begin, Iter end)
    {
      for( ; begin != end; ++begin ) this->data_.push_back( *begin );
    }


};

} // namespace observable

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY( observable::widom_row );
#endif
