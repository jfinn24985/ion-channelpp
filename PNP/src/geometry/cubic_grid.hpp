#ifndef IONCH_GEOMETRY_CUBIC_GRID_HPP
#define IONCH_GEOMETRY_CUBIC_GRID_HPP

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

#include "geometry/grid.hpp"
#include <boost/serialization/vector.hpp>
#include <cstddef>
#include "geometry/coordinate.hpp"
#include "utility/archive.hpp"

#include <iostream>
#include <boost/serialization/shared_ptr.hpp>

namespace utility { class random_distribution; } 

namespace geometry {

//Generate a grid of points within a cube. The grid starts half
//the 'spacing' distance from the //lower// cube edges and no
//less than half the 'spacing' distance from the //upper// cube
//edges. Grid points are retrieved sequentially.

class cubic_grid : public grid_generator
 {
   private:
    //The random sequence of indices
    std::vector<std::size_t> sequence_;

    //The origin of the grid's cube.
    coordinate origin_;

    //The number of points in each dimension
    std::size_t steps_;

    // inter-grid spacing
    double delta_;


   protected:
    // Initialise the cube grids with the given lengths, inter-node 
    // spacing and nodes per dimension.
    //
    // This leaves a minimum of 'spacing' distance between any grid
    // point and the cube edge. There will be an even number of grid
    // points in each direction.
    //
    // \param space : the inter-node spacing
    // \param per_dim : The number of nodes in each dimension.
    //
    // * For cube with width/length = space * per_dim
    
    cubic_grid(const coordinate & origin, double space, std::size_t per_dim, utility::random_distribution & rgen);


   private:
    // Default ctor (for serialize only)
    cubic_grid() = default;
    


   public:
    virtual ~cubic_grid();


  friend class boost::serialization::access;
   private:
    //Write the grid parameters to an archive
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object< geometry::grid_generator >(*this);
        ar & sequence_;
        ar & origin_;
        ar & steps_;
        ar & delta_;
      }


   public:
#if ( defined(DEBUG) and DEBUG>0 )
    //Dump grid data to stream.
void dump(std::ostream & os);
#endif
    // The number of remaining points in the grid.
    bool empty() const override
    {
      return this->sequence_.empty();
    }

    // Get the next location from the grid with
    // indication of iteration ending.
    //
    // \return size > 0
    bool next(coordinate & pnt) override;

    // The number of remaining points in the grid.
    std::size_t size() const override
    {
      return this->sequence_.size();
    }

    //  The inter-gridpoint distance.
    virtual double spacing() const override
    {
      return this->delta_;
    }


   private:
    // Generate a regular grid of nodes that fits in a cube of the given
    // width. The requested grid must have the given minimum inter-node
    // spacing and have a minimum of spacing/2 between the nodes and the
    // cube walls.
    //
    // \pre spacing < cube_width
    
    static boost::shared_ptr< cubic_grid > make_grid(double cube_width, double spacing, utility::random_distribution & rgenr);

    // Generate a regular grid of nodes that fits in a cube of the given
    // width. The requested grid must have the given minimum inter-node
    // spacing and have a minimum of spacing/2 between the nodes and the
    // cube walls.
    //
    // \pre spacing < cube_width
    
    static boost::shared_ptr< cubic_grid > make_grid(double cube_width, utility::random_distribution & rgenr, std::size_t count);


   public:
    // Generate a regular grid of nodes that fits in a cube of the given
    // width. The requested grid must have the given minimum inter-node
    // spacing and have a minimum of spacing/2 between the nodes and the
    // cube walls.
    //
    // \pre spacing < cube_width
    
    static boost::shared_ptr< cubic_grid > make_grid(const coordinate & origin, double cube_width, double spacing, utility::random_distribution & rgenr);

    // Generate a regular grid of nodes that fits in a cube of the given
    // width. The requested grid must have the given minimum inter-node
    // spacing and have a minimum of spacing/2 between the nodes and the
    // cube walls.
    //
    // \pre spacing < cube_width
    
    static boost::shared_ptr< cubic_grid > make_grid(const coordinate & origin, double cube_width, utility::random_distribution & rgenr, std::size_t count);


};

} // namespace geometry

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_KEY(geometry::cubic_grid);
#endif
