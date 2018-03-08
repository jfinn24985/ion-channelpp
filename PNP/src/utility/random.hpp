#ifndef IONCH_UTILITY_RANDOM_H
#define IONCH_UTILITY_RANDOM_H

//Uniform distribution decorator for the Mersenne Twister random number generator.
//
//Authors: Justin Finnerty
//
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

#include <boost/serialization/shared_ptr.hpp>
#include <boost/random.hpp>

#include <cstddef>
#include "utility/archive.hpp"

#include <boost/ptr_container/serialize_ptr_vector.hpp>

//-
#include "utility/config.hpp"
#include <boost/serialization/split_free.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <string>
#include <sstream>
//
namespace utility {

//Decorator class that provides uniformly distributed numbers based
//on the Mersenne Twister with the 19937 parameters. The simplest use
//if with operator() which generates a uniform sequence of psuedo
//random numbers between [0,1).
//
// (Unit test suite "utility_random_suite")
//
//

class random_distribution
 {
   private:
    //The internal data structure of the generator
    boost::shared_ptr<boost::mt19937> generator_;


   public:
    //Return a uniform random number between [min(a,b):max(a,b)]
    double uniform(double a = 1, double b = 0);

    // Random number in an interval that is split in two.
    // Generate a random number that is between outlft and outrht but
    // not between inlft and inrht
    //
    // To make the intervals symmetric we do for intervals (z1,z2) and (z3,z4)
    //  left_interval (z2 - z1)
    //  width = (left_interval + z4 - z3)
    //  result = width * rnd()
    //  if result >= (left_interval)
    //    return x3 + result - left_interval
    //  else
    //    return z2 - result
    //
    // For rnd() [) this gives (] [) * MT output interval
    //           [] this gives (] []
    //           () this gives () [)
    //           (] this gives () []
    //
    // fortran name geom::rndit4 interface geom::rndint
    inline double split_uniform(double outlft, double outrht, double inlft, double inrht);

    //  Generate a random integer in interval [a,b] 
    //
    //\if a == b result = a
    std::size_t randint(std::size_t a, std::size_t b = 0);

    random_distribution & operator=(random_distribution source)
    {
      this->swap (source);
      return *this;
    }

    //(default constructor should be used only for serialize operations)
    random_distribution()
    : generator_()
    {}

    random_distribution(boost::shared_ptr< boost::mt19937 > a_gen)
    : generator_(a_gen)
    {}

    random_distribution(const random_distribution & source)
    : generator_(source.generator_)
    {}

    ~random_distribution() {}


  friend class boost::serialization::access;
   private:
    template<class Archive> inline void serialize(Archive & ar, const unsigned int version)
    {
      ar & generator_;
    }


   public:
    void swap(random_distribution & source)
    {
      std::swap (this->generator_, source.generator_);
    }

    static std::string version();

    //Randomize the contents of the container in-place.
    template< typename ContentType > inline void shuffle(boost::ptr_vector< ContentType >& alist);

    //Randomize the contents of the container in-place.
    template< typename Container > inline void shuffle(Container & alist)
    {
      std::random_shuffle( alist.begin(), alist.end(), boost::bind(&random_distribution::randint, this, 0, _1) );
    }

    //Randomize the contents of the container in-place.
    template< typename IterType > inline void shuffle(IterType abegin, IterType aend)
    {
      std::random_shuffle( abegin, aend, boost::bind(&random_distribution::randint, this, 0, _1) );
    }


};
// Random number in an interval that is split in two.
// Generate a random number that is between outlft and outrht but
// not between inlft and inrht
//
// To make the intervals symmetric we do for intervals (z1,z2) and (z3,z4)
//  left_interval (z2 - z1)
//  width = (left_interval + z4 - z3)
//  result = width * rnd()
//  if result >= (left_interval)
//    return x3 + result - left_interval
//  else
//    return z2 - result
//
// For rnd() [) this gives (] [) * MT output interval
//           [] this gives (] []
//           () this gives () [)
//           (] this gives () []
//
// fortran name geom::rndit4 interface geom::rndint
inline double random_distribution::split_uniform(double outlft, double outrht, double inlft, double inrht) 
{
  // outlft, inlft, inright, outright
  std::vector< double > bounds = { outlft, inlft, inrht, outrht };
  std::sort(bounds.begin(), bounds.end());
  const double skip ( bounds[2] - bounds[1] );
  const double Result = this->uniform(bounds[0], bounds[3] - skip);
  return (Result > bounds[1] ? Result + skip : Result);

}

//Randomize the contents of the container in-place.
template< typename ContentType > inline void random_distribution::shuffle(boost::ptr_vector< ContentType >& alist) 
{
  typedef typename boost::ptr_vector< ContentType >::difference_type size_type;
  for (size_type n = alist.size() - 1; n > 0; --n)
  {
    size_type off(this->randint(0, n));
    if (off != n)
    {
      // replace element at 'off' with element from 'n'
      // (This should be well defined as off < n)
      auto ptr = alist.replace(alist.begin() + off, alist.release(alist.begin() + n).release());
      // insert element from 'off' at nth position.
      alist.insert(alist.begin() + n, ptr.release());
    }
  }

}


} // namespace utility

namespace boost {
namespace serialization {
template<class Archive>
void save(Archive & ar, const boost::mt19937 & t, unsigned int version)
{
  std::stringstream os;
  os << t;
  std::string c (os.str ());
  ar << c;
}
template<class Archive>
void load(Archive & ar, boost::mt19937 & t, unsigned int version)
{
  std::string c;
  ar >> c;
  std::stringstream is (c);
  is >> t;
}

template<class Archive>
inline void serialize(
    Archive & ar,
    boost::mt19937 & t,
    const unsigned int file_version
){
    split_free(ar, t, file_version); 
}
}
}


#endif
