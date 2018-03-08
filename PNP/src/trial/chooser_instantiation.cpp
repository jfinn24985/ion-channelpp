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

#include "trial/chooser_instantiation.hpp"


BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::move_choice >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::jump_choice >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::jump_in >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::jump_out >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser< trial::jump_around >);
BOOST_CLASS_EXPORT_IMPLEMENT(trial::chooser_pair< trial::add_specie BOOST_PP_COMMA() trial::remove_specie >);