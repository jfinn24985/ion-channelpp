#ifndef IONCH_GEOMETRY_DIGITIZER_3D_HPP
#define IONCH_GEOMETRY_DIGITIZER_3D_HPP

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

#include "geometry/coordinate.hpp"
#include "utility/digitizer.hpp"
#include <cstddef>
#include <array>

namespace geometry {

// Convert a coordinate inside a bounding box into a linear
// index suitable for making a histogram. Convert a linear
// index into a subcube of the bounding box ("corners").
//
// The digitizer operated in a fixed size bounding box.
//
// Bounding box is
// ( x_axis.mininum, y_axis.minimum, z_axis.minimum ) to 
//  ( x_axis.maxinum, y_axis.maximum, z_axis.maximum )
class digitizer_3d
 {
   public:
    // The data type we convert into an integer
    typedef coordinate data_type;


   private:
    // X-axis digitizer
    utility::digitizer xaxis_;

    // Y-axis digitizer
    utility::digitizer yaxis_;

    // Z-axis digitiser
    utility::digitizer zaxis_;

    // Sampling cube/digitiser width
    double spacing_;

    // Number of bins in X direction.
    //
    // \invariant yskip = xaxis.size
    std::size_t yskip_;

    // Number of bins in XY plane
    //
    // \invariant zskip = xaxis.size * yaxis.size
    std::size_t zskip_;

    // Total number of bins
    //
    // \invariant size = x_axis.size * y_axis.size * z_axis.size
    std::size_t size_;


   public:
    // Create digitizer.
    //
    // NOTE: spacing <= 0.0 is undefined.
    digitizer_3d(const coordinate & small, const coordinate & big, double spacing);

    // For serialization.
    digitizer_3d();

    ~digitizer_3d() = default;

    digitizer_3d(digitizer_3d && source)
    : xaxis_( std::move( source.xaxis_ ) )
    , yaxis_( std::move( source.yaxis_ ) )
    , zaxis_( std::move( source.zaxis_ ) )
    , spacing_( std::move( source.spacing_ ) )
    , yskip_( std::move( source.yskip_ ) )
    , zskip_( std::move( source.zskip_ ) )
    , size_( std::move( source.size_ ) )
    {}

    digitizer_3d(const digitizer_3d & source)
    : xaxis_( source.xaxis_ )
    , yaxis_( source.yaxis_ )
    , zaxis_( source.zaxis_ )
    , spacing_( source.spacing_ )
    , yskip_( source.yskip_ )
    , zskip_( source.zskip_ )
    , size_( source.size_ )
    {}

    void swap(digitizer_3d & source)
    {
      std::swap( xaxis_, source.xaxis_ );
      std::swap( yaxis_, source.yaxis_ );
      std::swap( zaxis_, source.zaxis_ );
      std::swap( spacing_, source.spacing_ );
      std::swap( yskip_, source.yskip_ );
      std::swap( zskip_, source.zskip_ );
      std::swap( size_, source.size_ );
    }
    

    digitizer_3d & operator=(digitizer_3d source)
    {
      this->swap( source );
      return *this;
    }


  friend class boost::serialization::access;    //Read/write object to an archive.
    template< class Archive > void serialize(Archive & ar, std::size_t a_ver)
    {
      ar & xaxis_; ar & yaxis_; ar & zaxis_; ar & spacing_; ar & yskip_; ar & zskip_; ar & size_;
    }

    // The end of a particular bin
    inline coordinate bin_maximum(std::size_t bin) const
    {
      std::size_t xi, yi, zi;
      this->bin_to_index( bin, xi, yi, zi );
      return coordinate( this->xaxis_.bin_maximum( xi ),  this->yaxis_.bin_maximum( yi ),  this->zaxis_.bin_maximum( zi ) );
    }

    // The end of a particular bin
    inline coordinate bin_midpoint(std::size_t bin) const
    {
      std::size_t xi, yi, zi;
      this->bin_to_index( bin, xi, yi, zi );
      return coordinate( this->xaxis_.bin_midpoint( xi ),  this->yaxis_.bin_midpoint( yi ),  this->zaxis_.bin_midpoint( zi ) );
    }

    // The end of a particular bin
    inline coordinate bin_minimum(std::size_t bin) const
    {
      std::size_t xi, yi, zi;
      this->bin_to_index( bin, xi, yi, zi );
      return coordinate( this->xaxis_.bin_minimum( xi ),  this->yaxis_.bin_minimum( yi ),  this->zaxis_.bin_minimum( zi ) );
    }

    // The end of a particular bin
    inline coordinate bin_width() const
    {
      return coordinate( this->xaxis_.bin_width(),  this->yaxis_.bin_width(),  this->zaxis_.bin_width() );
    }

    // Convert a coordinate inside the bounding box into a
    // linear index.
    //
    // \pre xaxis.in_range(pos.x) and yaxis.in_range(pos.y) and zaxis.in_range(pos.z)
    // \post result < size
    std::size_t convert(const coordinate & pos) const;

    // The end of a particular bin
    inline bool equivalent(digitizer_3d other) const
    {
      return this->xaxis_.equivalent( other.xaxis_ ) and this->yaxis_.equivalent( other.yaxis_ ) and this->zaxis_.equivalent( other.zaxis_ );
    }

    // The end of a particular bin
    inline bool in_range(coordinate val) const
    {
      return this->xaxis_.in_range( val.x ) and this->yaxis_.in_range( val.y ) and this->zaxis_.in_range( val.z );
    }

    // The end of a particular bin
    inline coordinate maximum() const
    {
      return coordinate( this->xaxis_.maximum(),  this->yaxis_.maximum(),  this->zaxis_.maximum() );
    }

    // The end of a particular bin
    inline coordinate minimum() const
    {
      return coordinate( this->xaxis_.minimum(),  this->yaxis_.minimum(),  this->zaxis_.minimum() );
    }

    friend inline bool operator==(const digitizer_3d & lhs, const digitizer_3d & rhs)
    {
      return lhs.equivalent( rhs );
    }

    friend inline bool operator!=(const digitizer_3d & lhs, const digitizer_3d & rhs)
    {
      return not lhs.equivalent( rhs );
    }

    // The number of bins
    std::size_t size() const
    {
      return size_;
    }

    // The end of a particular bin
    inline coordinate width() const
    {
      return coordinate( this->xaxis_.width(),  this->yaxis_.width(),  this->zaxis_.width() );
    }


   private:
    // Convert 3d bin index into x, y and z axis bin indices
    void bin_to_index(std::size_t bin, std::size_t & xidx, std::size_t & yidx, std::size_t & zidx) const;


   public:
    // Calculate corners of the sampling cube at given index.
    //
    // \pre idx < size
    // \post all points inside bounding box defined by ( x_axis.mininum, y_axis.minimum, z_axis.minimum ) to 
    //  ( x_axis.maxinum, y_axis.maximum, z_axis.maximum )
    void corners(std::size_t idx, std::array< coordinate, 8 > & points);

    const utility::digitizer& x_axis() const
    {
      return xaxis_;
    }

    const utility::digitizer& y_axis() const
    {
      return yaxis_;
    }

    const utility::digitizer& z_axis() const
    {
      return zaxis_;
    }


};

} // namespace geometry
#endif
