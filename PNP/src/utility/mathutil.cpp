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

#include "utility/mathutil.hpp"


namespace utility {

//Calculate if line segment x1,y1 : x2,y2 and circle x0,y0,radius
//intersect.
//
//TODO: No test method
bool mathutil::linesegment_circle_intersect(double x1, double y1, double x2, double y2, double x0, double y0, double radius) 
{
  // First check if either point is in circle
  const double x2_0_1 = (x1-x0)*(x1-x0);
  const double y2_0_1 = (y1-y0)*(y1-y0);
  const double r2 = radius * radius;
  if (r2 > (x2_0_1 + y2_0_1))
  {
    return false;
  }
  const double x2_0_2 = (x2-x0)*(x2-x0);
  const double y2_0_2 = (y2-y0)*(y2-y0);
  if (r2 > (x2_0_2 + y2_0_2))
  {
    return false;
  }
  // http://local.wasp.uwa.edu.au/~pbourke/geometry/sphereline/
  // Need following between 0 and 1 then for intersect in segment
  //
  // (x0 - x1)(x2 - x1) + (y0 - y1)(y2 - y1)
  // ---------------------------------------
  // (x2 - x1)(x2 - x1) + (y2 - y1)(y2 - y1) { == a needed below }
  const double a = ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
  
  const double u = ((x0 - x1)*(x2 - x1) + (y0 - y1)*(y2 - y1)) / a;
  if (0. > u)
  {
    return true;
  }
  if (1. < u)
  {
    return true;
  }
  
  // Now we need to check for intersection
  // b * b - 4 * a * c <= 0
  // b*b <= 4 * a * c
  // (b/2)^2 <= a * c
  // where
  // a = (x2 - x1)2 + (y2 - y1)2
  // b = 2[ (x2 - x1) (x1 - x0) + (y2 - y1) (y1 - y0) ]
  // c = x0^2 + y0^2 + x1^2 + y1^2 - 2[x0*x1 + y0*y1] - r2
  //
  const double half_b = ((x2 - x1)*(x1 - x0) + (y2 - y1)*(y1 - y0));
  const double c = x0*x0 + y0*y0 + x1*x1 + y1*y1 - 2*(x0*x1 + y0*y1) - radius*radius;
  return half_b * half_b <=  a * c;
  

}


} // namespace utility
