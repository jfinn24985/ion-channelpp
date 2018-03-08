#ifndef IONCH_CORE_INPUT_PARAMETER_REFERENCE_HPP
#define IONCH_CORE_INPUT_PARAMETER_REFERENCE_HPP

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

#include "core/base_input_parameter.hpp"
#include <string>
#include <cstddef>

namespace core { class input_base_reader; } 

namespace core {

// Refer to reader for information on parameter
class input_parameter_reference : public base_input_parameter
 {
   private:
    // The reader of this parameter
    const input_base_reader * reader_;

    // No default
    input_parameter_reference() = delete;


   public:
    input_parameter_reference(const input_base_reader & reader)
    : reader_( &reader )
    {}

    virtual ~input_parameter_reference() {}


   private:
    // No move
    input_parameter_reference(input_parameter_reference && source) = delete;

    // No copy
    input_parameter_reference(const input_parameter_reference & source) = delete;

    // No assignment
    input_parameter_reference & operator=(const input_parameter_reference & source) = delete;


   public:
    // The parameter name
    virtual std::string name() const override
    {
      return this->reader_->name();
    }

    // The parameter value
    virtual std::string value() const override
    {
      return this->reader_->value();
    }

    // The line containing the name/value pair
    virtual std::string line() const override
    {
      return this->reader_->line();
    }

    // The name of the input file containing the parameter
    virtual std::string filename() const override
    {
      return this->reader_->current_filename();
    }

    // The line number in the input file that contains this parameter.
    virtual std::size_t line_number() const override
    {
      return this->reader_->current_line_number();
    }


};

} // namespace core
#endif
