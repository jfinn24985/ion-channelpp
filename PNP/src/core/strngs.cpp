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

#include "core/strngs.hpp"

// Manual includes
#include <algorithm>
#include <cctype>
// -
namespace core {

const std::string strngs::fsaccu()
{
  return std::string( "accum" );
}

// trial class option names {
//      uses  fsrtmv='ratmov'

const std::string strngs::fsadd()
{
  return std::string( "add" );
}

// statistic option names
const std::string strngs::fscgin()
{
  return std::string( "calgin" );
}

// specie type parameter names
const std::string strngs::fschex()
{
  return std::string( "chex" );
}

// input file sections

const std::string strngs::fschnl()
{
  return std::string( "channel" );
}

const std::string strngs::fschon()
{
  return std::string ("chonly");
}

const std::string strngs::fschpt()
{
  return std::string( "usepot" );
}

const std::string strngs::fsclac()
{
  return std::string( "calacc" );
}

const std::string strngs::fsclmb()
{
  return std::string( "calmob" );
}

const std::string strngs::fsconf()
{
  return std::string( "conf" );
}

const std::string strngs::fscrdf()
{
  return std::string( "calrdf" );
}

const std::string strngs::fsctrg()
{
  return std::string( "ctarg" );
}

const std::string strngs::fsd()
{
  return std::string( "d" );
}

const std::string strngs::fsdrg()
{
  return std::string( "drg" );
}

const std::string strngs::fsdrmi()
{
  return std::string( "drmaxin" );
}

const std::string strngs::fsdrmo()
{
  return std::string( "drmaxout" );
}

// patch class option names {

const std::string strngs::fsdxf()
{
  return std::string( "dxf" );
}

const std::string strngs::fsdxw()
{
  return std::string( "dxw" );
}

const std::string strngs::fsdzg()
{
  return std::string( "dzg" );
}

// generic option names
const std::string strngs::fsend()
{
  return std::string( "end" );
}

// subspecie parameter names
const std::string strngs::fsenth()
{
  return std::string( "enthalpy" );
}

const std::string strngs::fsentr()
{
  return std::string( "entropy" );
}

const std::string strngs::fsepsc()
{
  return std::string( "epsch" );
}

const std::string strngs::fsepsp()
{
  return std::string( "epspr" );
}

const std::string strngs::fsepsw()
{
  return std::string( "epsw" );
}

std::string strngs::fsexct()
{
  return std::string( "excited" ); 
}

const std::string strngs::fsflxd()
{
  return std::string( "flex" );
}

const std::string strngs::fsfree()
{
  return std::string( "free" );
}

const std::string strngs::fsfver()
{
  return std::string( "fileversion" );
}

const std::string strngs::fsgeom()
{
  return std::string( "geom" );
}

const std::string strngs::fsgrid()
{
  return std::string( "usegrid" );
}

const std::string strngs::fsgrl1()
{
  return std::string( "rl1" );
}

const std::string strngs::fsgrl4()
{
  return std::string( "rl4" );
}

const std::string strngs::fsgrl5()
{
  return std::string( "rl5" );
}

const std::string strngs::fsgrlc()
{
  return std::string( "rlcurv" );
}

const std::string strngs::fsgrlv()
{
  return std::string( "rlvest" );
}

std::string strngs::fsgrnd()
{
  return std::string( "ground" );
}

// geometry option names
const std::string strngs::fsgzl1()
{
  return std::string( "zl1" );
}

const std::string strngs::fsgzl4()
{
  return std::string( "zl4" );
}

const std::string strngs::fsgzlm()
{
  return std::string( "zlimit" );
}

const std::string strngs::fsgzoc()
{
  return std::string( "zocc" );
}

// include subfiles option name
const std::string strngs::fsincl()
{
  return std::string( "include" );
}

const std::string strngs::fsisav()
{
  return std::string( "isave" );
}

// salt parameter names
const std::string strngs::fsislt()
{
  return std::string( "cation" );
}

const std::string strngs::fsiwid()
{
  return std::string( "iwidom" );
}

const std::string strngs::fskmob()
{
  return std::string( "mobk" );
}

// (allowed specie type values)
const std::string strngs::fsmobl()
{
  return std::string( "mob" );
}

const std::string strngs::fsname()
{
  return std::string( "name" );
}

const std::string strngs::fsnavr()
{
  return std::string( "naver" );
}

const std::string strngs::fsnmcf()
{
  return std::string( "multiconf" );
}

const std::string strngs::fsn()
{
  return std::string( "n" );
}

const std::string strngs::fsnoch()
{
  return std::string( "nocharge" );
}

const std::string strngs::fsnsrt()
{
  return std::string( "oldreg" );
}

// simulation option names
const std::string strngs::fsnstp()
{
  return std::string( "nstep" );
}

const std::string strngs::fsnsub()
{
  return std::string( "nsub" );
}

const std::string strngs::fsntrg()
{
  return std::string( "ntarg" );
}

// unused fortran label
const std::string strngs::fsnblk()
{
  return std::string( "nbulk" );
}

const std::string strngs::fsptch()
{
  return std::string( "patch" );
}

const std::string strngs::fsregn()
{
  return std::string( "region" );
}

const std::string strngs::fsrtex()
{
  return std::string( "ratexc" );
}

const std::string strngs::fsrtgr()
{
  return std::string( "ratgr" );
}

const std::string strngs::fsrtid()
{
  return std::string( "ratind" );
}

const std::string strngs::fsrtjp()
{
  return std::string( "ratjmp" );
}

const std::string strngs::fsrtmv()
{
  return std::string( "ratmov" );
}

const std::string strngs::fsrtrg()
{
  return std::string( "ratreg" );
}

const std::string strngs::fsrtsl()
{
  return std::string( "ratslt" );
}

const std::string strngs::fsrtsp()
{
  return std::string( "ratspc" );
}

const std::string strngs::fsrtsw()
{
  return std::string( "ratswap" );
}

const std::string strngs::fssalt()
{
  return std::string( "salt" );
}

const std::string strngs::fsspec()
{
  return std::string( "specie" );
}

const std::string strngs::fssubs()
{
  return std::string( "subspecie" );
}

const std::string strngs::fstry()
{
  return std::string( "trial" );
}

const std::string strngs::fstsi()
{
  return std::string( "kelvin" );
}

const std::string strngs::fstype()
{
  return std::string( "type" );
}

const std::string strngs::fswidm()
{
  return std::string( "calwid" );
}

const std::string strngs::fsz()
{
  return std::string( "z" );
}

const std::string strngs::bulk_label()
{
  return std::string( "bulk" );
}

const std::string strngs::comment_begin()
{
  return std::string ("#");
}

const std::string strngs::comment_end()
{
  return std::string();
}

const std::string strngs::evaluator_label()
{
  return std::string ("evaluator");
}

// Formatting constants
const std::string strngs::horizontal_bar()
{
  return std::string( 70ul, '-' );
}

const std::string strngs::imc_label()
{
  return std::string( "super-looper" );
}

const std::string strngs::inner_label()
{
  return std::string( "inner" );
}

const std::string strngs::inputpattern_label()
{
  return std::string( "input" );
}

const std::string strngs::localizer_label()
{
  return std::string( "localize" );
}

const std::string strngs::outputdir_label()
{
  return std::string( "outputdir" );
}

const std::string strngs::rate_label()
{
  return std::string( "rate" );
}

const std::string strngs::sampler_label()
{
  return std::string( "sampler" );
}

const std::string strngs::simulator_label()
{
  return std::string( "simulator" );
}

// Check that argument is one "word"
// * valid words have no spaces anywhere in the string 
// * valid words have only alphabetic characters
bool strngs::is_one_word(std::string word)
{
  int (*isfn)(int) = std::isalpha; // assignment selects desired overload func.
  return word.size() == static_cast< std::size_t >(std::count_if(word.begin(), word.end(), isfn));

}

// Check that argument is a valid "name"
// * valid names have no spaces anywhere in the string 
// * valid names have only alphanumeric characters or '_' or '.'
// * in our case numbers, '.' and '_' can appear anywhere in the string
bool strngs::is_valid_name(std::string word)
{
  int (*isfn)(int) = std::isalnum; // assignment selects desired overload func.
  return word.size() == static_cast< std::size_t >(std::count_if(word.begin(), word.end(), isfn)
     + std::count(word.begin(), word.end(), '_') + std::count(word.begin(), word.end(), '.'));

}

// Check that argument is one "word"
// * valid words have no spaces anywhere in the string 
bool strngs::has_spaces(std::string word)
{
  int (*isfn)(int) = std::isspace; // assignment selects desired overload func.
  return 0 != std::count_if(word.begin(), word.end(), isfn);

}


} // namespace core
