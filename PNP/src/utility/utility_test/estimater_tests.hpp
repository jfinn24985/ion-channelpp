#ifndef IONCH__ESTIMATER_TESTS_HPP
#define IONCH__ESTIMATER_TESTS_HPP

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

#include "utility/fuzzy_equals.hpp"
#include <boost/serialization/vector.hpp>

namespace utility { class basic_histogram; } 
namespace utility { class digitizer; } 
namespace utility { class estimater; } 
namespace utility { class estimate_array; } 
namespace utility { template<class Digitizer> class fixed_size_histogram; } 
namespace utility { class histogram; } 
struct pattern_test;
namespace utility { class binary_estimate; } 
namespace utility { class basic_mean; } 
namespace utility { class estimate_2d; } 
namespace utility { class estimate_3d; } 

//Unit tests for the statistics related classes in 'estimater.hpp'
//
//Classes:
//  - estimater
//  - base_mean_range
//  - estimate
//  - estimate_array
struct estimater_test_fw 
{
    //data set
    std::vector<double> var;

    //data set
    std::vector<double> data4;

    //data set
    std::vector<double> data5;

    //data set
    std::vector<double> data6;

    //data set
    std::vector<double> data_index;

    //geometric mean
    double mean;

    //Sample variance
    double svar;

    //population variance
    double variance;

    static estimater_test_fw exmplr;


   private:
    estimater_test_fw();

};
#endif
