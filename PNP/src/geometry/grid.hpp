#ifndef IONCH_GEOMETRY_GRID_HPP
#define IONCH_GEOMETRY_GRID_HPP

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

#include <boost/noncopyable.hpp>

#include <iostream>
#include <cstddef>
#include "utility/archive.hpp"


// manual
#include "utility/random.hpp"
// -
namespace geometry { class coordinate; } 

namespace geometry {

class grid_generator : public boost::noncopyable
 {
   public:
#if ( defined(DEBUG) and DEBUG>0 )
    //Dump gridder data to stream for debugging.
virtual void dump(std::ostream & os) {}
#endif
    // Test if any grid points remain.
    virtual bool empty() const = 0;

    //  Get the next coordinate in grid. Return false when there are
    //  no coordinates left in the grid.
    //
    //  if true then pnt is a valid unique point in the grid
    //  if false then pnt is not set
    virtual bool next(coordinate & pnt) = 0;

    // The number of remaining grid points defined by the generator
    virtual std::size_t size() const = 0;

    //  The inter-gridpoint distance.
    virtual double spacing() const = 0;

    virtual ~grid_generator();


  friend class boost::serialization::access;
   private:
    //Write the grid parameters to an archive
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {}


};

} // namespace geometry
#endif
