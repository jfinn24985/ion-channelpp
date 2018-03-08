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

#include "core/help_entry.hpp"
#include "core/fixed_width_output_filter.hpp"

// manual includes
#include "utility/utility.hpp"
// --
#include <boost/iostreams/filtering_stream.hpp>
// --
namespace core {

// Write help description to destination stream using the given filter.
void help_entry::write(fixed_width_output_filter & filt, std::ostream & dest) const
{
  namespace io = boost::iostreams;
  {
    io::filtering_ostream os;
    os.push( filt );
    os.push( dest );
    os << this->title_ << "\n";
  }
  {
    core::indent_guard grd( filt );
    io::filtering_ostream out;
    out.push( filt );
    out.push( dest );
    if( not ( this->type_desc_.empty() and this->range_.empty() and this->default_value_.empty() ) )
    {
      out << "[";
      if( not this->type_desc_.empty() )
      {
        out << this->type_desc_;
        if( not ( this->range_.empty() and this->default_value_.empty() ) )
        {
          out << ": ";
        }
      }
      if( not this->range_.empty() )
      {
        out << "range(" << this->range_ << ")";
        if( not this->default_value_.empty() )
        {
          out << ", ";
        }
      }
      if( not this->default_value_.empty() )
      {
        out << "default{" << this->default_value_ << "}";
      }
      out << "]\n";
    }
    if( not this->description_.empty() )
    {
      out << this->description_ << "\n";
    }
  }
  

}


} // namespace core
