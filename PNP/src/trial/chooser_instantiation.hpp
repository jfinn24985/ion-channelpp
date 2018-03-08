#ifndef IONCH_TRIAL_CHOOSER_INSTANTIATION_HPP
#define IONCH_TRIAL_CHOOSER_INSTANTIATION_HPP

#include <boost/serialization/export.hpp>
#include "trial/chooser.hpp"
#include "trial/chooser_pair.hpp"
#include "trial/std_choices.hpp"

BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::move_choice >);
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::jump_choice >);
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::jump_in >);
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::jump_out >);
BOOST_CLASS_EXPORT_KEY(trial::chooser< trial::jump_around >);
BOOST_CLASS_EXPORT_KEY2(trial::chooser_pair< trial::add_specie BOOST_PP_COMMA() trial::remove_specie >, "trial::chooser_pair< trial::add_specie, trial::remove_specie >");
#endif
