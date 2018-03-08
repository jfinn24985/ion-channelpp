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

#include "utility/filesystem.hpp"

// Manual includes
#include <boost/filesimulator/operations.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
//-
namespace utility {

//Attempt to create a relative path from a_base to a_leafer.  If this 
//is not possible complete(a_leafer) is returned.  The relative path is
//constructed relative to the whole of a_base.  If either paths are
//not complete, then they are made "complete".
boost::filesystem::path filesystem::relative_path(const boost::filesystem::path & a_base, const boost::filesystem::path & a_leafer)
{
  // Ensure that starting paths are complete
  boost::filesystem::path l_base (boost::filesystem::complete (a_base));
  boost::filesystem::path l_leaf (boost::filesystem::complete (a_leafer));
  boost::filesystem::path Result;
  boost::filesystem::path::iterator l_lhs (l_base.begin ());
  boost::filesystem::path::iterator l_rhs (l_leaf.begin ());
  // Iterate along a_base and a_leafer for first mismatch.
  if (*l_lhs != *l_rhs)
  { // No common base, just return a_leafer
    Result = l_leaf;
  }
  else
  {
    for (++l_rhs, ++l_lhs; // Iterate to first mismatch.
  	l_lhs != l_base.end () and l_rhs != l_leaf.end () and (*l_lhs) == (*l_rhs);
  	++l_rhs, ++l_lhs); // Do nothing in loop.
    if (l_lhs != l_base.end ())
    { // Need to add "parent dirs" until we reach end
      for (; l_lhs != l_base.end (); ++l_lhs)
      {
        Result /= "..";
      }
    }
    if (l_lhs == l_base.end ())
    { // Now everything from l_rhs is in path.
      for (; l_rhs != l_leaf.end (); ++l_rhs)
      {
        Result /= *l_rhs;
      }
    }
  }
  return Result;
}


} // namespace utility
