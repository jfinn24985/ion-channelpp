#ifndef IONCH_GEOMETRY_COORDINATE_SET_HPP
#define IONCH_GEOMETRY_COORDINATE_SET_HPP


#include "utility/archive.hpp"

#include <boost/serialization/vector.hpp>
#include <cstddef>

namespace geometry {

// A set of ( x, y, z ) coordinates.  This does not conform to
// any of the STL definitions of a container.
class coordinate_set
 {
   private:
    //The X coordinates of the particles
    std::vector<double> x_;

    //The Y coordinates of the particles
    std::vector<double> y_;

    //The Z coordinates of the particles
    std::vector<double> z_;


   public:
    coordinate_set()
    : x_()
    , y_()
    , z_()
    {}

    coordinate_set(const coordinate_set & source)
    : x_( source.x_ )
    , y_( source.y_ )
    , z_( source.z_ )
    {}

    coordinate_set(coordinate_set && source)
    : x_( std::move( source.x_ ) )
    , y_( std::move( source.y_ ) )
    , z_( std::move( source.z_ ) )
    {}
    

    // Swap contents of two objects
    void swap(coordinate_set & source)
    {
       std::swap( this->x_, source.x_ );
       std::swap( this->y_, source.y_ );
       std::swap( this->z_, source.z_ );
    }

    // Remove all coordinates
    void clear()
    {
       this->x_.clear();
       this->y_.clear();
       this->z_.clear();
    }
    

    // Is the set empty?
    std::size_t empty() const
    {
       return this->x_.empty();
    }

    // Test for equivalence
    bool equivalent(const coordinate_set & other) const
    {
      return size() == other.size() and x_ == other.x_ and y_ == other.y_ and z_ == other.z_;
    }
    

    friend bool operator==(const coordinate_set & lhs, const coordinate_set & rhs)
    {
      return lhs.equivalent( rhs );
    }

    // Assign to this set
    coordinate_set & operator =(coordinate_set source)
    {
       this->swap( source );
       return *this;
    }

    // Change size of vector
    void resize(std::size_t npart)
    {
       this->x_.resize(npart, 0.0);
       this->y_.resize(npart, 0.0);
       this->z_.resize(npart, 0.0);
    }



  friend class boost::serialization::access;
   private:
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
       ar & x_;
       ar & y_;
       ar & z_;
    }


   public:
    //  Set x position
    void set_x(std::size_t idx, double val)
    {
       this->x_[ idx ] = val;
    }

    //  Set y position
    inline void set_y(std::size_t idx, double val)
    {
       this->y_[ idx ] = val;
    }

    //  Set z position
    void set_z(std::size_t idx, double val)
    {
        this->z_[ idx ] = val;
    }

    // Number of coordinates
    std::size_t size() const
    {
       return this->x_.size();
    }

    // Get an x coordinate
    double x(std::size_t index) const
    {
       return this->x_[index];
    }

    // Get an y coordinate
    double y(std::size_t index) const
    {
       return this->y_[index];
    }

    // Get an z coordinate
    double z(std::size_t index) const
    {
       return this->z_[index];
    }


};

} // namespace geometry
#endif
