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


#ifndef DEBUG
#define DEBUG 0
#endif

#include <boost/version.hpp>

#include "utility/random.hpp"

namespace utility {

//Return a uniform random number between [min(a,b):max(a,b)]
double random_distribution::uniform(double a, double b) 
{
  if ( a == b ) return a;
  boost::random::uniform_real_distribution<> dist( std::min( a, b ), std::max( a, b ) );
  return dist( *this->generator_ );

}

//  Generate a random integer in interval [a,b] 
//
//\if a == b result = a
std::size_t random_distribution::randint(std::size_t a, std::size_t b) 
{
if (a == b) return a;
boost::random::uniform_int_distribution<> dist(std::min(a, b), std::max(a, b));
return dist( *this->generator_ );

}

std::string random_distribution::version()

{
  std::stringstream is;
  is << "Boost library version (compile) " <<  (BOOST_VERSION / 100000) << "." << ((BOOST_VERSION / 100) % 1000) << "." << (BOOST_VERSION % 100);
  return is.str ();

}


} // namespace utility
