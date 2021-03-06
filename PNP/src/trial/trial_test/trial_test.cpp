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

#define BOOST_TEST_MODULE trial_test
#include <boost/test/unit_test.hpp>

#include "trial/trial_test/trial_test.hpp"
#include "particle/change_set.hpp"
#include "particle/change_hash.hpp"
#include "trial/choice_manager.hpp"
#include "trial/choice_meta.hpp"
#include "trial/chooser.hpp"
#include "trial/std_choices.hpp"
#include "trial/trial_test/test_choice.hpp"
#include "core/input_parameter_memo.hpp"
#include "trial/choice.hpp"

// Manuals
#include "core/input_delegater.hpp"
#include "core/input_document.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
#include "geometry/cube_region.hpp"
#include "geometry/geometry_manager.hpp"
#include "geometry/periodic_cube_region.hpp"
#include "particle/ensemble.hpp"
#include "particle/particle_manager.hpp"
#include "particle/specie.hpp"
#include "utility/fuzzy_equals.hpp"
#include "utility/random.hpp"
//#include "utility/machine.hpp"
//#include "utility/mathutil.hpp"
//#include "utility/utility.hpp"

#include<boost/random.hpp>
// - 
// Mockup set of realistic species.
//
//  specie_count = 5
//  solute = "Na" (x2 particles) and "Cl" (x1 particle)
//  mobile = "CA" (x1 particle)
//  flexible = "CO" (x1 particle)
//  channel only = "OX" (x1 particle)
//  
boost::shared_ptr< particle::particle_manager > trial_test::mockup_particle_manager()
{
  boost::shared_ptr< particle::particle_manager > pman( new particle::particle_manager );
  {
    particle::specie spc1;
    spc1.set_label( "CA" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.11 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( 1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::MOBILE );
    spc1.append_position( geometry::coordinate( 2.0, 2.0, 0.0 ), geometry::centroid( 3.0, 2.0, 2.0, 0.0 ) );
    pman->add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "CO" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::FLEXIBLE );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 2.0 ), geometry::centroid( 3.0, 0.0, 0.0, 2.0 ) );
    pman->add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "OX" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::CHANNEL_ONLY );
    spc1.append_position( geometry::coordinate( 2.0, 0.0, 2.0 ) );
    pman->add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "Na" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( 1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::SOLUTE );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 2.0, 0.0, 0.0 ) );
    pman->add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "Cl" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.2 );
    spc1.set_rate( 0.2 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.3123 );
    spc1.set_type( particle::specie::SOLUTE );
    spc1.append_position( geometry::coordinate( 0.0, 2.0, 0.0 ) );
    pman->add_specie( spc1 );
  }
  pman->add_predefined_particles();
  return pman;

}

// Create geometry manager with PBC cube "cell" and 
// open cube "bulk" regions.
boost::shared_ptr< geometry::geometry_manager > trial_test::mockup_geometry_manager()
{
  boost::shared_ptr< geometry::base_region > regn( new geometry::periodic_cube_region( "cell", 5.0 ) );
  boost::shared_ptr< geometry::geometry_manager > gman( new geometry::geometry_manager( regn ) );
  {
    boost::shared_ptr< geometry::base_region > bulk( new geometry::cube_region( "bulk", 4.0, geometry::coordinate( 0.0, 0.0, 0.0 ), true ) );
    gman->add_region( bulk );
  }
  return gman;

}

std::vector< core::input_parameter_memo > trial_test::mockup_params()
{
  std::vector< core::input_parameter_memo > params;
  core::input_parameter_memo tmp;
  tmp.set_name( "end" );
  params.push_back( std::move( tmp ) );
  return std::move( params );
}

// Tests key, probabilty (get/set) and serialization. 
//
// * Does not test trial generation.
// 
void trial_test::test_base_choice_methods(boost::shared_ptr< trial::base_choice > choice)
{
  std::stringstream ss;
  double prob;
  particle::change_hash key;
  // serialization test
  {
    prob = choice->probability();
    key = choice->key();
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    oa << choice;
  }
  {
    boost::shared_ptr< trial::base_choice > copy;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    ia >> copy;
    BOOST_REQUIRE( utility::feq( copy->probability(), prob ) );
    BOOST_REQUIRE( copy->key() == key );
  
    // Try setting probability on copy
    prob += 1.0;
    BOOST_CHECK_NO_THROW( copy->set_probability( prob ) );
    BOOST_REQUIRE( utility::feq( copy->probability(), prob ) );
    BOOST_REQUIRE( not utility::feq( copy->probability(), choice->probability() ) );
  }
  

}

// Run choice.generate on a mockup ensemble. Uses
// Choice::permitted( spc ) and Choice::make_choice( idx, *params* ) 
// to create choice object. Params arg to make_choice is the
// params arg passed to this function.
//
//  ens.size = 6
//  specie_count = 5
//  solute = "Na" (x2 particles) and "Cl" (x1 particle)
//  mobile = "CA" (x1 particle)
//  flexible = "CO" (x1 particle)
//  channel only = "OX" (x1 particle)
//  system region = "cell" (PBC cube)
//  subregion = "bulk" (open cube)
//  all particles inside "bulk" (and "cell")
//  
template< class Choice > void trial_test::test_choice_generate(const std::vector< core::input_parameter_memo > & params)
{
  boost::shared_ptr< particle::particle_manager > pman( trial_test::mockup_particle_manager() );
  boost::shared_ptr< geometry::geometry_manager > gman( trial_test::mockup_geometry_manager() );
  BOOST_REQUIRE_EQUAL( pman->specie_count(), 5 );
  BOOST_REQUIRE_EQUAL( pman->get_ensemble().count(), 6 );
  {
    const auto& ens = pman->get_ensemble();
    for( std::size_t idx = 0; idx != ens.size(); ++idx )
    {
      if( ens.key( idx ) != particle::specie_key::nkey )
      {
        geometry::coordinate pos = ens.position( idx );
        BOOST_REQUIRE( gman->system_region().is_inside( pos, pman->get_specie( ens.key( idx ) ).radius() ) );
      }
    }
  }
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  
  bool usable = false;
  for( std::size_t idx = 0; idx != pman->specie_count(); ++idx )
  {
    if( Choice::permitted( pman->get_specie( idx ) ) )
    {
      usable = true;
      std::unique_ptr< trial::base_choice > chc( Choice::make_choice( idx, *gman, params ) );
      const double spc_radius = pman->get_specie( idx ).radius();
      std::size_t nonfails = 0;
      for( std::size_t attempts = 0; attempts != 1024; ++attempts )
      {
        std::unique_ptr< particle::change_set > tryit( chc->generate( *pman, *gman, rgnr ) );
        BOOST_CHECK( chc->key() == tryit->id() );
        BOOST_REQUIRE( tryit->fail() or ( tryit->size() > 0 ) );
        if( not tryit->fail() )
        {
          ++nonfails;
          // new position should be in system or fail should be set.
          const auto& atom = (*tryit)[ 0 ];
          BOOST_REQUIRE( gman->system_region().is_inside( atom.new_position, spc_radius ) );
        }
      }
      BOOST_WARN_GE( nonfails, 0 );
    }
  }
  BOOST_REQUIRE( usable );
  
  

}

// Run "Chooser" on a mockup ensemble. Uses
// base_chooser.generate_choices( *params*, *type*, *specielist*, *rate* ) 
// to create choice object. "*args*" to are directly the args passed to 
// this function.
//
// \param count : expected number of choices
// \param exp_rate : expected rate of each choice
//
//  ens.size = 6
//  specie_count = 5
//  solute = "Na" (x2 particles) and "Cl" (x1 particle)
//  mobile = "CA" (x1 particle)
//  flexible = "CO" (x1 particle)
//  channel only = "OX" (x1 particle)
//  system region = "cell" (PBC cube)
//  subregion = "bulk" (open cube)
//  all particles inside "bulk" (and "cell")
//  
template< class Chooser > void trial_test::test_make_chooser_methods(const std::vector< core::input_parameter_memo > & params, std::string type, const core::input_parameter_memo & specielist, double rate, std::size_t count, double exp_rate)
{
  boost::shared_ptr< particle::particle_manager > pman( trial_test::mockup_particle_manager() );
  boost::shared_ptr< geometry::geometry_manager > gman( trial_test::mockup_geometry_manager() );
  BOOST_REQUIRE_EQUAL( pman->specie_count(), 5 );
  BOOST_REQUIRE_EQUAL( pman->get_ensemble().count(), 6 );
  
  std::stringstream ss;
  {
    std::unique_ptr< trial::base_chooser > chsr = Chooser::make_chooser( params, type, specielist, rate );
    BOOST_CHECK_EQUAL( chsr->rate(), rate );
    BOOST_CHECK_EQUAL( chsr->type(), type );
    BOOST_CHECK( chsr->specie_list() == specielist );
    BOOST_CHECK( chsr->parameters() == params );
    boost::archive::text_oarchive oa( ss );
    // write class instance to archive
    oa << chsr;
  }
  {
    std::unique_ptr< trial::base_chooser > chsr;
    boost::archive::text_iarchive ia( ss );
    // read class instance from archive
    ia >> chsr;
    BOOST_CHECK_EQUAL( chsr->rate(), rate );
    BOOST_CHECK_EQUAL( chsr->type(), type );
    BOOST_CHECK( chsr->specie_list() == specielist );
    BOOST_CHECK( chsr->parameters() == params );
    boost::ptr_vector< trial::base_choice > choices;
    chsr->prepare_choices( pman->get_species(), *gman, choices );
    BOOST_REQUIRE_EQUAL( choices.size(), count );
    for( std::size_t idx = 0; idx != count; ++idx )
    {
      BOOST_CHECK_CLOSE( choices[ idx ].probability(), exp_rate, 0.00001 );
    }
  }
  

}

// Use test_choice class to test base_choice operations.
BOOST_AUTO_TEST_CASE( base_choice_test )
{
  {
    // Test for virtual noncopy pattern
    BOOST_CHECK( not std::is_default_constructible< test_choice >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< test_choice >::type {} );
    BOOST_CHECK( not std::is_move_constructible< test_choice >::type {} );
    BOOST_CHECK( not( std::is_assignable< test_choice, test_choice >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< test_choice >::type {} );
  }
  
  {
  // Public ctor 1
    const double prob = 0.1;
    particle::change_hash key1( 1, 2, 3, 4 );
    boost::shared_ptr< trial::base_choice > var1( new test_choice( key1 ) );
    BOOST_CHECK_EQUAL( var1->key(), key1 );
    BOOST_CHECK_EQUAL( var1->probability(), 0 );
  
    BOOST_CHECK_NO_THROW( var1->set_probability( prob ) );
    BOOST_CHECK_EQUAL( var1->probability(), prob );
  
    trial_test::test_base_choice_methods( var1 );
  }
  
  {
  // Public ctor 2
    particle::change_hash key2( 1, 0, 0, 0 );
    boost::shared_ptr< trial::base_choice > var2( new test_choice( 1ul ) );
    BOOST_CHECK_EQUAL( var2->key(), key2 );
    BOOST_CHECK_EQUAL( var2->probability(), 0 );
  
  }
  
  

}

BOOST_AUTO_TEST_CASE( choice_manager_test )
{
  {
    // Test for noncopyable and nonvirtual pattern
    BOOST_CHECK( std::is_default_constructible< trial::choice_manager >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::choice_manager >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::choice_manager >::type {} );
    BOOST_CHECK( not( std::is_assignable< trial::choice_manager, trial::choice_manager >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< trial::choice_manager >::type {} );
  }
  std::stringstream store;
  {
    // Default ctor
    boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
    // choice access methods
    BOOST_CHECK( cman->empty() );
    BOOST_CHECK_EQUAL( cman->size(), 0 );
    BOOST_CHECK( cman->begin() == cman->end() );
    // chooser access methods
    BOOST_CHECK( cman->empty_chooser() );
    BOOST_CHECK_EQUAL( cman->size_chooser(), 0 );
    BOOST_CHECK( cman->begin_chooser() == cman->end_chooser() );
    bool has = false;
    BOOST_CHECK_NO_THROW( has = cman->has_chooser( "move" ) );
    BOOST_CHECK_EQUAL( has, false );
  
    {
      core::input_delegater dg( 1 );
      trial::choice_manager::build_input_delegater( cman, dg );
      BOOST_REQUIRE( dg.has_section( "trial" ) );
      // Valid input
      /////////////////////////////
      std::string canon_input( "trial\ntype \"move\"\nrate 0.5\nspecie Na Cl\nend\n\n" );
      core::input_preprocess reader;
      reader.add_buffer( "dummy", canon_input );
      BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
      BOOST_REQUIRE( not cman->empty_chooser() );
      BOOST_CHECK_EQUAL( cman->size_chooser(), 1 );
      BOOST_CHECK( cman->has_chooser( "move" ) );
    }
    boost::archive::text_oarchive oa( store );
    // write class instance to archive
    oa << cman;
  }
  {
    boost::shared_ptr< trial::choice_manager > cman2;
    boost::archive::text_iarchive ia( store );
    // get class instance from archive
    ia >> cman2;
  
    // choice access methods
    BOOST_CHECK( cman2->empty() );
    BOOST_CHECK_EQUAL( cman2->size(), 0 );
    BOOST_CHECK( cman2->begin() == cman2->end() );
    // chooser access methods
    BOOST_REQUIRE( not cman2->empty_chooser() );
    BOOST_CHECK( cman2->has_chooser( "move" ) );
    BOOST_CHECK_EQUAL( cman2->size_chooser(), 1 );
    BOOST_CHECK_EQUAL( cman2->begin_chooser()->type(), "move" );
    BOOST_CHECK_CLOSE( cman2->begin_chooser()->rate(), 0.5, 0.000001 );
    BOOST_REQUIRE( cman2->begin_chooser()->has_specie_list() );
    BOOST_CHECK_EQUAL( cman2->begin_chooser()->specie_list().value(), "Na Cl" );
    BOOST_CHECK_EQUAL( cman2->begin_chooser()->parameters().size(), 1 );
  
    boost::shared_ptr< particle::particle_manager > pman = trial_test::mockup_particle_manager();
    boost::shared_ptr< geometry::geometry_manager > gman = trial_test::mockup_geometry_manager();
    boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
    utility::random_distribution rgnr( generator );
    cman2->prepare( pman->get_species(), *gman, rgnr );
    //BOOST_REQUIRE_NO_THROW( cman2->prepare( pman->get_species(), *gman, rgnr ) );
  
    BOOST_CHECK( not cman2->empty() );
    BOOST_REQUIRE_EQUAL( cman2->size(), 5 );
    BOOST_CHECK( cman2->begin() != cman2->end() );
    std::bitset< 5 > all_spc;
    all_spc.reset(); // set all to none
    const particle::change_hash mover( 0, 1, 1, 0 );
    for (auto const& choice : *cman2 )
    {
      BOOST_CHECK_GE( choice.key().key(), 0 );
      BOOST_CHECK_LE( choice.key().key(), 4 );
      BOOST_CHECK( mover.match( choice.key() ) );
      BOOST_REQUIRE_NO_THROW( all_spc.set( choice.key().key() ) );
    }
    BOOST_REQUIRE( all_spc.all() );
  }
  

}

BOOST_AUTO_TEST_CASE( add_specie_choice_test )
{
  const double prob = 0.1;
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< trial::add_specie >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::add_specie >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::add_specie >::type {} );
    BOOST_CHECK( not( std::is_assignable< trial::add_specie, trial::add_specie >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< trial::add_specie >::type {} );
  }
  particle::change_hash key1( 0, 0, 1, 0 );
  boost::shared_ptr< trial::add_specie > var1( new trial::add_specie( 0ul ) );
  BOOST_CHECK_EQUAL( var1->key(), key1 );
  BOOST_CHECK_EQUAL( var1->probability(), 0 );
  
  BOOST_CHECK_NO_THROW( var1->set_probability( prob ) );
  BOOST_CHECK_EQUAL( var1->probability(), prob );
  
  trial_test::test_base_choice_methods( var1 );
  
  {
    trial_test::test_choice_generate< trial::add_specie >( trial_test::mockup_params() );
    trial_test::test_make_chooser_methods< trial::chooser_pair< trial::add_specie,trial::remove_specie  > >( trial_test::mockup_params(), "individual", {}, 0.5, 4, 0.125 );
  }
  

}

BOOST_AUTO_TEST_CASE( jump_choice_test )
{
  const double prob = 0.1;
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< trial::jump_choice >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::jump_choice >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::jump_choice >::type {} );
    BOOST_CHECK( not( std::is_assignable< trial::jump_choice, trial::jump_choice >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< trial::jump_choice >::type {} );
  }
  particle::change_hash key1( 0, 1, 1, 1 );
  boost::shared_ptr< trial::jump_choice > var1( new trial::jump_choice( 0ul ) );
  BOOST_CHECK_EQUAL( var1->key(), key1 );
  BOOST_CHECK_EQUAL( var1->probability(), 0 );
  
  BOOST_CHECK_NO_THROW( var1->set_probability( prob ) );
  BOOST_CHECK_EQUAL( var1->probability(), prob );
  
  trial_test::test_base_choice_methods( var1 );
  
  std::map< std::string, std::string > params;
  
  trial_test::test_choice_generate< trial::jump_choice >( trial_test::mockup_params() );
  
  trial_test::test_make_chooser_methods< trial::chooser< trial::jump_choice > >( trial_test::mockup_params(), "jump", {}, 0.5, 2, 0.25 );

}

BOOST_AUTO_TEST_CASE( jump_in_choice_test )
{
  const double prob = 0.1;
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< trial::jump_in >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::jump_in >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::jump_in >::type {} );
    BOOST_CHECK( not( std::is_assignable< trial::jump_in, trial::jump_in >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< trial::jump_in >::type {} );
  }
  particle::change_hash key1( 0, 1, 1, 2 );
  boost::shared_ptr< trial::jump_in > var1( new trial::jump_in( 0ul, 1 ) );
  BOOST_CHECK_EQUAL( var1->key(), key1 );
  BOOST_CHECK_EQUAL( var1->probability(), 0 );
  BOOST_CHECK_EQUAL( var1->region_index(), 1 );
  
  BOOST_CHECK_NO_THROW( var1->set_probability( prob ) );
  BOOST_CHECK_EQUAL( var1->probability(), prob );
  
  trial_test::test_base_choice_methods( var1 );
  
  {
    std::vector< core::input_parameter_memo > params{ std::move( trial_test::mockup_params() ) };
    core::input_parameter_memo regn;
    regn.set_name( core::strngs::fsregn() );
    regn.set_value( "bulk" );
    params.insert( params.begin(), regn );
    trial_test::test_choice_generate< trial::jump_in >( params );
  
    trial_test::test_make_chooser_methods< trial::chooser< trial::jump_in > >( params, "jump-in", {}, 0.5, 2, 0.25 );
  }

}

BOOST_AUTO_TEST_CASE( jump_out_choice_test )
{
  const double prob = 0.1;
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< trial::jump_out >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::jump_out >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::jump_out >::type {} );
    BOOST_CHECK( not( std::is_assignable< trial::jump_out, trial::jump_out >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< trial::jump_out >::type {} );
  }
  particle::change_hash key1( 0, 1, 1, 3 );
  boost::shared_ptr< trial::jump_out > var1( new trial::jump_out( 0ul, 1ul ) );
  BOOST_CHECK_EQUAL( var1->key(), key1 );
  BOOST_CHECK_EQUAL( var1->probability(), 0 );
  BOOST_CHECK_EQUAL( var1->region_index(), 1 );
  
  BOOST_CHECK_NO_THROW( var1->set_probability( prob ) );
  BOOST_CHECK_EQUAL( var1->probability(), prob );
  
  trial_test::test_base_choice_methods( var1 );
  
  {
    std::vector< core::input_parameter_memo > params{ std::move( trial_test::mockup_params() ) };
    core::input_parameter_memo regn;
    regn.set_name( core::strngs::fsregn() );
    regn.set_value( "bulk" );
    params.insert( params.begin(), regn );
    trial_test::test_choice_generate< trial::jump_out >( params );
  
    trial_test::test_make_chooser_methods< trial::chooser< trial::jump_out > >( params, "jump-out", {}, 0.5, 2, 0.25 );
  }

}

BOOST_AUTO_TEST_CASE( jump_around_choice_test )
{
  const double prob = 0.1;
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< trial::jump_around >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::jump_around >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::jump_around >::type {} );
    BOOST_CHECK( not( std::is_assignable< trial::jump_around, trial::jump_around >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< trial::jump_around >::type {} );
  }
  particle::change_hash key1( 0, 1, 1, 4 );
  boost::shared_ptr< trial::jump_around > var1( new trial::jump_around( 0ul, 1ul ) );
  BOOST_CHECK_EQUAL( var1->key(), key1 );
  BOOST_CHECK_EQUAL( var1->probability(), 0 );
  BOOST_CHECK_EQUAL( var1->region_index(), 1 );
  
  BOOST_CHECK_NO_THROW( var1->set_probability( prob ) );
  BOOST_CHECK_EQUAL( var1->probability(), prob );
  
  trial_test::test_base_choice_methods( var1 );
  
  {
    std::vector< core::input_parameter_memo > params{ std::move( trial_test::mockup_params() ) };
    core::input_parameter_memo regn;
    regn.set_name( core::strngs::fsregn() );
    regn.set_value( "bulk" );
    params.insert( params.begin(), regn );
    trial_test::test_choice_generate< trial::jump_around >( params );
  
    trial_test::test_make_chooser_methods< trial::chooser< trial::jump_around > >( params, "jump-around", {}, 0.5, 1, 0.5 );
  }

}

BOOST_AUTO_TEST_CASE( move_choice_test )
{
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< trial::move_choice >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::move_choice >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::move_choice >::type {} );
    BOOST_CHECK( not( std::is_assignable< trial::move_choice, trial::move_choice >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< trial::move_choice >::type {} );
  }
  
  const double prob = 0.1;
  const double delta = 10.0;
  particle::change_hash key1( 0, 1, 1, 0 );
  boost::shared_ptr< trial::move_choice > var1( new trial::move_choice( 0ul ) );
  BOOST_CHECK_EQUAL( var1->key(), key1 );
  BOOST_CHECK_EQUAL( var1->probability(), 0 );
  
  BOOST_CHECK_NO_THROW( var1->set_probability( prob ) );
  BOOST_CHECK_EQUAL( var1->probability(), prob );
  
  BOOST_CHECK_EQUAL( var1->max_displacement(), trial::move_choice::default_displacement() );
  BOOST_CHECK_NO_THROW( var1->set_max_displacement( delta ) );
  BOOST_CHECK_EQUAL( var1->max_displacement(), delta );
  
  trial_test::test_base_choice_methods( var1 );
  
  {
    std::vector< core::input_parameter_memo > params{ std::move( trial_test::mockup_params() ) };
    core::input_parameter_memo regn;
    regn.set_name( "delta" );
    regn.set_value( "0.5" );
    params.insert( params.begin(), regn );
    trial_test::test_choice_generate< trial::move_choice >( params );
  
    trial_test::test_make_chooser_methods< trial::chooser< trial::move_choice > >( params, "move", {}, 0.5, 5, 0.1 );
  }

}

BOOST_AUTO_TEST_CASE( remove_specie_choice_test )
{
  const double prob = 0.1;
  {
    // Test is virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< trial::remove_specie >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::remove_specie >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::remove_specie >::type {} );
    BOOST_CHECK( not( std::is_assignable< trial::remove_specie, trial::remove_specie >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< trial::remove_specie >::type {} );
  }
  particle::change_hash key1( 0, 1, 0, 0 );
  boost::shared_ptr< trial::remove_specie > var1( new trial::remove_specie( 0ul ) );
  BOOST_CHECK_EQUAL( var1->key(), key1 );
  BOOST_CHECK_EQUAL( var1->probability(), 0 );
  
  BOOST_CHECK_NO_THROW( var1->set_probability( prob ) );
  BOOST_CHECK_EQUAL( var1->probability(), prob );
  
  trial_test::test_base_choice_methods( var1 );
  
  {
    trial_test::test_choice_generate< trial::remove_specie >( trial_test::mockup_params() );
  }

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( choice_meta_lifetime_test )
{
  {
    // Test for virtual object pattern
    BOOST_CHECK( not std::is_default_constructible< trial::choice_meta >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< trial::choice_meta >::type {} );
    BOOST_CHECK( not std::is_move_constructible< trial::choice_meta >::type {} );
    BOOST_CHECK( not ( std::is_assignable< trial::choice_meta, trial::choice_meta >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< trial::choice_meta >::type {} );
  }

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( choice_meta_move_choice_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  {
    core::input_help helper;
    dg.get_documentation( helper );
    std::stringstream omsg;
    helper.write( omsg );
    const std::string msg( omsg.str() );
    BOOST_CHECK_LE( msg.find( "rate" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "type" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "specie" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "delta" ), msg.size() );
  }
  // Valid input
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE( not cman->empty_chooser() );
  BOOST_CHECK( cman->has_chooser( "move" ) );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->type(), "move" );
  BOOST_CHECK_CLOSE( cman->begin_chooser()->rate(), 0.5, 0.000001 );
  BOOST_REQUIRE( cman->begin_chooser()->has_specie_list() );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->specie_list().value(), "Na Cl" );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().size(), 1ul );
  
  // Echo input.
  core::input_document inp( 1 );
  BOOST_CHECK_NO_THROW( cman->write_document( inp ) );
  BOOST_REQUIRE_EQUAL( inp.size(), 1 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].label(), "trial" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].size(), 3 );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "specie" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "type" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "rate" ) );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "specie" ), "Na Cl" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "type" ), "move" );
  BOOST_REQUIRE_CLOSE( boost::lexical_cast< double >( inp[ 0 ].get_entry( "rate" ) ), 0.5, 0.00001 );
  //inp.write( std::cout );

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( choice_meta_jump_choice_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump" ) );
  
    dg.add_input_delegate( meta );
  }
  {
    core::input_help helper;
    dg.get_documentation( helper );
    std::stringstream omsg;
    helper.write( omsg );
    const std::string msg( omsg.str() );
    BOOST_CHECK_LE( msg.find( "rate" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "type" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "specie" ), msg.size() );
  }
  // Valid input
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump\"\nrate 0.45\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE( not cman->empty_chooser() );
  BOOST_CHECK( cman->has_chooser( "jump" ) );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->type(), "jump" );
  BOOST_CHECK_CLOSE( cman->begin_chooser()->rate(), 0.45, 0.000001 );
  BOOST_REQUIRE( cman->begin_chooser()->has_specie_list() );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->specie_list().value(), "Na Cl" );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().size(), 1ul );
  
  // Echo input.
  core::input_document inp( 1 );
  BOOST_CHECK_NO_THROW( cman->write_document( inp ) );
  BOOST_REQUIRE_EQUAL( inp.size(), 1 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].label(), "trial" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].size(), 3 );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "specie" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "type" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "rate" ) );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "specie" ), "Na Cl" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "type" ), "jump" );
  BOOST_REQUIRE_CLOSE( boost::lexical_cast< double >( inp[ 0 ].get_entry( "rate" ) ), 0.45, 0.00001 );

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( choice_meta_jump_around_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_around::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-around" ) );
  
    dg.add_input_delegate( meta );
  }
  {
    core::input_help helper;
    dg.get_documentation( helper );
    std::stringstream omsg;
    helper.write( omsg );
    const std::string msg( omsg.str() );
    BOOST_CHECK_LE( msg.find( "rate" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "type" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "specie" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "region" ), msg.size() );
  }
  // Valid input
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-around\"\nregion \"channel\"\nrate 0.51\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE( not cman->empty_chooser() );
  BOOST_CHECK( cman->has_chooser( "jump-around" ) );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->type(), "jump-around" );
  BOOST_CHECK_CLOSE( cman->begin_chooser()->rate(), 0.51, 0.000001 );
  BOOST_REQUIRE( cman->begin_chooser()->has_specie_list() );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->specie_list().value(), "Na Cl" );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().size(), 2 );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().at( 0 ).name(), "region" );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().at( 0 ).value(), "\"channel\"" );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().at( 1 ).name(), "end" );
  
  // Echo input.
  core::input_document inp( 1 );
  BOOST_CHECK_NO_THROW( cman->write_document( inp ) );
  BOOST_REQUIRE_EQUAL( inp.size(), 1 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].label(), "trial" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].size(), 4 );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "specie" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "type" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "rate" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "region" ) );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "specie" ), "Na Cl" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "type" ), "jump-around" );
  BOOST_REQUIRE_CLOSE( boost::lexical_cast< double >( inp[ 0 ].get_entry( "rate" ) ), 0.51, 0.00001 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "region" ), "\"channel\"" );

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( choice_meta_jump_in_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_in::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-in" ) );
  
    dg.add_input_delegate( meta );
  }
  {
    core::input_help helper;
    dg.get_documentation( helper );
    std::stringstream omsg;
    helper.write( omsg );
    const std::string msg( omsg.str() );
    BOOST_CHECK_LE( msg.find( "rate" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "type" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "specie" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "region" ), msg.size() );
  }
  // Valid input
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-in\"\nregion \"bulk\"\nrate 0.55\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE( not cman->empty_chooser() );
  BOOST_CHECK( cman->has_chooser( "jump-in" ) );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->type(), "jump-in" );
  BOOST_CHECK_CLOSE( cman->begin_chooser()->rate(), 0.55, 0.000001 );
  BOOST_REQUIRE( cman->begin_chooser()->has_specie_list() );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->specie_list().value(), "Na Cl" );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().size(), 2 );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().at( 0 ).value(), "\"bulk\"" );
  
  // Echo input.
  core::input_document inp( 1 );
  BOOST_CHECK_NO_THROW( cman->write_document( inp ) );
  BOOST_REQUIRE_EQUAL( inp.size(), 1 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].label(), "trial" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].size(), 4 );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "specie" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "type" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "rate" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "region" ) );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "specie" ), "Na Cl" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "type" ), "jump-in" );
  BOOST_REQUIRE_CLOSE( boost::lexical_cast< double >( inp[ 0 ].get_entry( "rate" ) ), 0.55, 0.00001 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "region" ), "\"bulk\"" );

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( choice_meta_jump_out_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_out::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-out" ) );
  
    dg.add_input_delegate( meta );
  }
  {
    core::input_help helper;
    dg.get_documentation( helper );
    std::stringstream omsg;
    helper.write( omsg );
    const std::string msg( omsg.str() );
    BOOST_CHECK_LE( msg.find( "rate" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "type" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "specie" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "region" ), msg.size() );
  }
  // Valid input
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-out\"\nregion centre\nrate 0.35\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE( not cman->empty_chooser() );
  BOOST_CHECK( cman->has_chooser( "jump-out" ) );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->type(), "jump-out" );
  BOOST_CHECK_CLOSE( cman->begin_chooser()->rate(), 0.35, 0.000001 );
  BOOST_REQUIRE( cman->begin_chooser()->has_specie_list() );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->specie_list().value(), "Na Cl" );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().size(), 2 );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().at( 0 ).value(), "centre" );
  
  // Echo input.
  core::input_document inp( 1 );
  BOOST_CHECK_NO_THROW( cman->write_document( inp ) );
  BOOST_REQUIRE_EQUAL( inp.size(), 1 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].label(), "trial" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].size(), 4 );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "specie" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "type" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "rate" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "region" ) );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "specie" ), "Na Cl" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "type" ), "jump-out" );
  BOOST_REQUIRE_CLOSE( boost::lexical_cast< double >( inp[ 0 ].get_entry( "rate" ) ), 0.35, 0.00001 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "region" ), "centre" );

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( choice_meta_add_remove_choice_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::add_specie::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "individual" ) );
  
    dg.add_input_delegate( meta );
  }
  {
    core::input_help helper;
    dg.get_documentation( helper );
    std::stringstream omsg;
    helper.write( omsg );
    const std::string msg( omsg.str() );
    BOOST_CHECK_LE( msg.find( "rate" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "type" ), msg.size() );
    BOOST_CHECK_LE( msg.find( "specie" ), msg.size() );
  }
  // Valid input
  /////////////////////////////
  std::string canon_input( "trial\ntype \"individual\"\nrate 0.53\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE( not cman->empty_chooser() );
  BOOST_CHECK( cman->has_chooser( "individual" ) );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->type(), "individual" );
  BOOST_CHECK_CLOSE( cman->begin_chooser()->rate(), 0.53, 0.000001 );
  BOOST_REQUIRE( cman->begin_chooser()->has_specie_list() );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->specie_list().value(), "Na Cl" );
  BOOST_CHECK_EQUAL( cman->begin_chooser()->parameters().size(), 1 );
  
  // Echo input.
  core::input_document inp( 1 );
  BOOST_CHECK_NO_THROW( cman->write_document( inp ) );
  BOOST_REQUIRE_EQUAL( inp.size(), 1 );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].label(), "trial" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].size(), 3 );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "specie" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "type" ) );
  BOOST_REQUIRE( inp[ 0 ].has_entry( "rate" ) );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "specie" ), "Na Cl" );
  BOOST_REQUIRE_EQUAL( inp[ 0 ].get_entry( "type" ), "individual" );
  BOOST_REQUIRE_CLOSE( boost::lexical_cast< double >( inp[ 0 ].get_entry( "rate" ) ), 0.53, 0.00001 );

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_no_type_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\n#type \"move\"\nrate 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Trial definition section \"trial\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 5.\n** Add the following parameters to the section: type **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_type_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype #\"move\"\nrate 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Trial subtype  parameter \"type\" in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>type #\"move\"<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_type_bad_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump\"\nrate 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Trial subtype  parameter \"type\" with value (\"jump\") in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>type \"jump\"<<\n** A value from the list (move) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_duplicate_type_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\nspecie Na Cl\n\ntype \"move\"\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"type\" appears more than once in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>type \"move\"<<\n** Parameter \"type\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_no_rate_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\n#rate 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Trial definition section \"trial\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 5.\n** Add the following parameters to the section: rate **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_rate_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate #0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Trial likelihood parameter \"rate\" in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>rate #0.5<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_rate_negative_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate -0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Trial likelihood parameter \"rate\" with value (-0.5) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>rate -0.5<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_rate_zero_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.0\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Trial likelihood parameter \"rate\" with value (0.0) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>rate 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_rate_nonnumber_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate one\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Trial likelihood parameter \"rate\" with value (one) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>rate one<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_duplicate_rate_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\nspecie Na Cl\n\nrate 0.4\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"rate\" appears more than once in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>rate 0.4<<\n** Parameter \"rate\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_specielist_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\nspecie # Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Specie inclusion list parameter \"specie\" in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>specie # Na Cl<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_duplicate_specielist_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\nspecie Na Cl\n\nspecie \"move\"\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"specie\" appears more than once in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>specie \"move\"<<\n** Parameter \"specie\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_move_delta_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\ndelta #0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 5 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Move trial parameter \"delta\" in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>delta #0.5<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_move_delta_negative_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\ndelta -0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Move trial parameter \"delta\" with value (-0.5) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>delta -0.5<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_move_delta_zero_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\ndelta 0.0\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 5 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Move trial parameter \"delta\" with value (0.0) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>delta 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_move_delta_nonnumber_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.1\ndelta one\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 5 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Move trial parameter \"delta\" with value (one) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>delta one<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_move_duplicate_delta_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\ndelta 0.5\nspecie Na Cl\n\ndelta 0.4\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"delta\" appears more than once in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>delta 0.4<<\n** Parameter \"delta\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_move_unknown_parameter_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::move_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "move" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"move\"\nrate 0.5\nDELTA 0.5\nspecie Na Cl\n\ndelta 0.4\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"DELTA\" in section \"trial\" subtype \"move\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n** Parameter \"DELTA\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_unknown_parameter_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_choice::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump\"\nrate 0.5\ndelta 0.5\nspecie Na Cl\n\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"delta\" in section \"trial\" subtype \"jump\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n** Parameter \"delta\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_in_no_region_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_in::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-in" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-in\"\nrate 0.5\n#region bulk\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 2 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-in subtype trial section \"trial\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: region **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_in_region_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_in::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-in" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-in\"\nrate 0.5\nregion #bulk\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 2 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-in target region parameter \"region\" in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>region #bulk<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_in_region_bad_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_in::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-in" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-in\"\nrate 0.1\nregion one\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 5 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-in target region parameter \"region\" with value (one) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>region one<<\n** A value from the list (cell,bulk) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_in_duplicate_region_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_in::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-in" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-in\"\nrate 0.5\nregion bulk\nspecie Na Cl\n\nregion channel\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"region\" appears more than once in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>region channel<<\n** Parameter \"region\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_in_unknown_parameter_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_in::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-in" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-in\"\nrate 0.5\nDELTA 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"DELTA\" in section \"trial\" subtype \"jump-in\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n** Parameter \"DELTA\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_out_no_region_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_out::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-out" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-out\"\nrate 0.5\n#region bulk\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 2 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-out subtype trial section \"trial\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: region **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_out_region_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_out::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-out" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-out\"\nrate 0.5\nregion #bulk\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 2 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-out origin region parameter \"region\" in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>region #bulk<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_out_region_bad_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_out::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-out" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-out\"\nrate 0.1\nregion one\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 5 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-out origin region parameter \"region\" with value (one) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>region one<<\n** A value from the list (cell,bulk) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"cman->prepare( ... )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_out_duplicate_region_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_out::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-out" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-out\"\nrate 0.5\nregion bulk\nspecie Na Cl\n\nregion channel\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"region\" appears more than once in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>region channel<<\n** Parameter \"region\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_out_unknown_parameter_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_out::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-out" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-out\"\nrate 0.5\nDELTA 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"DELTA\" in section \"trial\" subtype \"jump-out\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n** Parameter \"DELTA\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_around_no_region_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_around::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-around" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-around\"\nrate 0.5\n#region bulk\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 2 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-around subtype trial section \"trial\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: region **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_around_region_no_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_around::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-around" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-around\"\nrate 0.5\nregion #bulk\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 2 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-around target region parameter \"region\" in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>region #bulk<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_around_region_bad_value_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_around::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-around" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-around\"\nrate 0.1\nregion one\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  auto pman = trial_test::mockup_particle_manager();
  auto gman = trial_test::mockup_geometry_manager();
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rgnr( generator );
  try
  {
    // attributes from parameters are only tested when choice object
    // is created (e.g. in choice_manager::prepare)
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    cman->prepare( pman->get_species(), *gman, rgnr );  
    // If no exception then check for number of choices created.
    BOOST_CHECK_EQUAL( cman->size(), 1 );
    BOOST_ERROR( "expected \"cman->prepare( ... )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Jump-around target region parameter \"region\" with value (one) in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>region one<<\n** A value from the list (cell,bulk) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"cman->prepare( ... )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_around_duplicate_region_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_around::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-around" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-around\"\nrate 0.5\nregion bulk\nspecie Na Cl\n\nregion channel\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Parameter \"region\" appears more than once in section \"trial\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>region channel<<\n** Parameter \"region\" can only appear once per section. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_jump_around_unknown_parameter_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::jump_around::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "jump-around" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"jump-around\"\nrate 0.5\nDELTA 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"DELTA\" in section \"trial\" subtype \"jump-around\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n** Parameter \"DELTA\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}

// Test combination of choice_meta and move_chocie.
BOOST_AUTO_TEST_CASE( trial_input_add_specie_unknown_parameter_test )
{
  core::input_delegater dg( 1 );
  boost::shared_ptr< trial::choice_manager > cman( new trial::choice_manager );
  {
    boost::shared_ptr< trial::choice_meta > meta( new trial::choice_meta( cman ) );
  
    trial::add_specie::add_definition( *meta );
  
    BOOST_REQUIRE( meta->has_trial_type( "individual" ) );
  
    dg.add_input_delegate( meta );
  }
  // Type is hidden by comment character
  /////////////////////////////
  std::string canon_input( "trial\ntype \"individual\"\nrate 0.5\ndelta 0.5\nspecie Na Cl\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  
  try
  {
    dg.read_input( reader );
    BOOST_ERROR( "expected \"dg.read_input( reader )\" exception not thrown" );
  }
  catch( core::input_error const& err )
  {
    const std::string msg( err.what() );
    //std::cout << msg << "\n";
    BOOST_CHECK( msg.find( "Unknown parameter \"delta\" in section \"trial\" subtype \"individual\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n** Parameter \"delta\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }
  
  

}



#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT_IMPLEMENT(test_choice);