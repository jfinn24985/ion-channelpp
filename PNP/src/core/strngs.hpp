#ifndef IONCH_CORE_STRNGS_HPP
#define IONCH_CORE_STRNGS_HPP

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

#include <string>

namespace core {

//  INPUT FILE STRINGS
//
//  String constants that are used in the input file.
//
//  The string constants are all accessed using a function interface.
class strngs
 {
   public:
    static const std::string fsaccu();

    // trial class option names
    //      uses  fsrtmv='ratmov'
    static const std::string fsadd();

    // statistic option names
    static const std::string fscgin();

    // specie type parameter names
    static const std::string fschex();

    //  -------------------
    //  INPUT FILE STRINGS
    //  -------------------
    // input file sections
    static const std::string fschnl();

    static const std::string fschon();

    static const std::string fschpt();

    static const std::string fsclac();

    static const std::string fsclmb();

    static const std::string fsconf();

    static const std::string fscrdf();

    static const std::string fsctrg();

    static const std::string fsd();

    static const std::string fsdrg();

    static const std::string fsdrmi();

    static const std::string fsdrmo();

    // patch class option names
    static const std::string fsdxf();

    static const std::string fsdxw();

    static const std::string fsdzg();

    // generic option names
    static const std::string fsend();

    // subspecie parameter names
    static const std::string fsenth();

    static const std::string fsentr();

    static const std::string fsepsc();

    static const std::string fsepsp();

    static const std::string fsepsw();

    static std::string fsexct();

    static const std::string fsflxd();

    static const std::string fsfree();

    static const std::string fsfver();

    static const std::string fsgeom();

    static const std::string fsgrid();

    static const std::string fsgrl1();

    static const std::string fsgrl4();

    static const std::string fsgrl5();

    static const std::string fsgrlc();

    static const std::string fsgrlv();

    static std::string fsgrnd();

    // geometry option names
    static const std::string fsgzl1();

    static const std::string fsgzl4();

    static const std::string fsgzlm();

    static const std::string fsgzoc();

    // include subfiles option name
    static const std::string fsincl();

    static const std::string fsisav();

    // salt parameter names
    static const std::string fsislt();

    static const std::string fsiwid();

    static const std::string fskmob();

    // (allowed specie type values)
    static const std::string fsmobl();

    static const std::string fsname();

    static const std::string fsnavr();

    static const std::string fsnmcf();

    static const std::string fsn();

    static const std::string fsnoch();

    static const std::string fsnsrt();

    // simulation option names
    static const std::string fsnstp();

    static const std::string fsnsub();

    static const std::string fsntrg();

    // unused fortran label
    static const std::string fsnblk();

    static const std::string fsptch();

    static const std::string fsregn();

    static const std::string fsrtex();

    static const std::string fsrtgr();

    static const std::string fsrtid();

    static const std::string fsrtjp();

    static const std::string fsrtmv();

    static const std::string fsrtrg();

    static const std::string fsrtsl();

    static const std::string fsrtsp();

    static const std::string fsrtsw();

    static const std::string fssalt();

    static const std::string fsspec();

    static const std::string fssubs();

    static const std::string fstry();

    static const std::string fstsi();

    static const std::string fstype();

    static const std::string fswidm();

    static const std::string fsz();

    static const std::string bulk_label();

    static const std::string comment_begin();

    static const std::string comment_end();

    static const std::string evaluator_label();

    // Formatting constants
    static const std::string horizontal_bar();

    static const std::string imc_label();

    static const std::string inner_label();

    static const std::string inputpattern_label();

    static const std::string localizer_label();

    static const std::string outputdir_label();

    static const std::string rate_label();

    static const std::string sampler_label();

    static const std::string simulator_label();

    // Check that argument is one "word"
    // * valid words have no spaces anywhere in the string 
    // * valid words have only alphabetic characters
    static bool is_one_word(std::string word);

    // Check that argument is a valid "name"
    // * valid names have no spaces anywhere in the string 
    // * valid names have only alphanumeric characters or '_' or '.'
    // * in our case numbers, '.' and '_' can appear anywhere in the string
    static bool is_valid_name(std::string word);

    // Check that argument is one "word"
    // * valid words have no spaces anywhere in the string 
    static bool has_spaces(std::string word);


};

} // namespace core
#endif
