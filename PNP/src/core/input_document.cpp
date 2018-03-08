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

#include "core/input_document.hpp"

// Manual includes
#include "core/strngs.hpp"
//
namespace core {

//  Write the content in input file format to the given stream.
void input_section::write(std::ostream & os) const
{
     os << this->label_ << "\n";
     for (auto const& nvpair : this->content_)
     {
        os << nvpair.first << " " << nvpair.second << "\n";
     }
     os << core::strngs::fsend() << "\n\n";

}

// Create and add an new section with the given label and return
// the index to it.

std::size_t input_document::add_section(std::string label)
{
  this->content_.push_back( input_section( label ) );
  return this->content_.size() - 1;

}

// Remove a section with the given index.
//
// This make all indices higher than idx invalid.
void input_document::remove_section(std::size_t idx)
{
  this->content_.erase(this->content_.begin() + idx);

}

//  Write the content in input file format to the given stream.
void input_document::write(std::ostream & os)
{
  os << core::strngs::fsfver() << " " << this->version_ << "\n\n";
  for (auto const& sec : this->content_)
  {
     sec.write( os );
  }

}


} // namespace core
