#ifndef IONCH_CORE_INPUT_DELEGATER_HPP
#define IONCH_CORE_INPUT_DELEGATER_HPP

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

#include <boost/noncopyable.hpp>

#include <boost/serialization/map.hpp>
#include <string>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <cstddef>

namespace core { class input_base_reader; } 
namespace core { class strngs; } 
namespace core { class input_base_meta; } 
namespace core { class input_help; } 

namespace core {

// Manage link between input file section names and section readers
//
// Provide a map between the name of an input file section and a
// handler function for the section.  This class works with the
// input_reader class to provide a well managed interface to
// reading the randomly ordered content of an IONCH input file.
//
// This class does not handle any post-processing that may be
// necessary after the input file has been read.  For example,
// initialising objects in a specific dependency order.
class input_delegater : public boost::noncopyable
 {
   private:
    //Map of section names to objects responsible for processing input file data.
    std::map< std::string, boost::shared_ptr< input_base_meta > > input_delegate_map_;

    // List of sections that appeared in the input file
    std::vector< boost::shared_ptr< input_base_meta > > section_list_;

    std::size_t file_version_;

    std::size_t max_version_;


   public:
    input_delegater(std::size_t maxver);


   private:
    //For serialization
    input_delegater() = default;


   public:
    ~input_delegater() = default;

    bool empty() const
    {
        return this->input_delegate_map_.empty ();
    }

    void add_input_delegate(boost::shared_ptr< input_base_meta >  delegate);

    //Has a delegate that can interpret section {name}
    bool has_section(std::string name) const
    {
        return this->input_delegate_map_.end() != this->input_delegate_map_.find(name);
    }

    void get_documentation(input_help & helper);

    std::size_t max_version() const
    {
        return this->max_version_;
    }

    std::size_t size() const
    {
        return this->input_delegate_map_.size ();
    }

    void read_input(input_base_reader & reader);

    std::size_t read_version() const
    {
        return this->file_version_;
    }


};

} // namespace core
#endif
