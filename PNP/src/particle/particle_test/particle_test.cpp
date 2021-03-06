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

#define BOOST_TEST_MODULE particle_test
#include <boost/test/unit_test.hpp>

#include "particle/particle_test/particle_test.hpp"
#include "particle/change_set.hpp"
#include "geometry/digitizer_3d.hpp"
#include "particle/ensemble.hpp"
#include "utility/fuzzy_equals.hpp"
#include "core/input_delegater.hpp"
#include "particle/particle_manager.hpp"
#include "particle/specie_meta.hpp"

// Manuals
//#include "utility/multi_array.hpp"
//#include "utility/random.hpp"
//#include "utility/machine.hpp"
//#include "utility/mathutil.hpp"
//#include "utility/utility.hpp"
#include "core/input_help.hpp"
#include "core/input_document.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
#include "geometry/geometry_manager.hpp"
#include "geometry/periodic_cube_region.hpp"
#include "utility/random.hpp"
// - 
//#include <boost/random.hpp>
//-
BOOST_AUTO_TEST_CASE( change_atom_lifetime_test )
{
  const std::vector< double > nrij { 1.0, 1.3, 2.4 };
  const std::vector< double > orij { 1.1, 1.4, 2.0 };
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< particle::change_atom >::type{} );
    BOOST_CHECK( std::is_copy_constructible< particle::change_atom >::type{} );
    BOOST_CHECK( std::is_move_constructible< particle::change_atom >::type{} );
    BOOST_CHECK( (std::is_assignable< particle::change_atom, particle::change_atom >::type{}) );
    BOOST_CHECK( not std::has_virtual_destructor< particle::change_atom >::type{} );
  }
  {
    // Default ctor
    particle::change_atom var1;
    BOOST_CHECK_EQUAL(var1.new_position.x, 0.0);
    BOOST_CHECK_EQUAL(var1.new_position.y, 0.0);
    BOOST_CHECK_EQUAL(var1.new_position.z, 0.0);
    BOOST_CHECK_EQUAL(var1.old_position.x, 0.0);
    BOOST_CHECK_EQUAL(var1.old_position.y, 0.0);
    BOOST_CHECK_EQUAL(var1.old_position.z, 0.0);
    BOOST_CHECK_EQUAL(var1.energy_old, 0.0);
    BOOST_CHECK_EQUAL(var1.energy_new, 0.0);
    BOOST_CHECK_EQUAL(var1.index, 0);
    BOOST_CHECK_EQUAL(var1.key, static_cast< std::size_t >(particle::specie_key::nkey));
    BOOST_CHECK_EQUAL(var1.do_old, true);
    BOOST_CHECK_EQUAL(var1.do_new, true);
    BOOST_CHECK(var1.old_rij.empty());
    BOOST_CHECK(var1.new_rij.empty());
    BOOST_CHECK(var1.old_rij2.empty());
    BOOST_CHECK(var1.new_rij2.empty());
  
  }
  std::stringstream store;
  {
    // Ctor with arg
    particle::change_atom var1(2);
    BOOST_CHECK_EQUAL(var1.new_position.x, 0.0);
    BOOST_CHECK_EQUAL(var1.new_position.y, 0.0);
    BOOST_CHECK_EQUAL(var1.new_position.z, 0.0);
    BOOST_CHECK_EQUAL(var1.old_position.x, 0.0);
    BOOST_CHECK_EQUAL(var1.old_position.y, 0.0);
    BOOST_CHECK_EQUAL(var1.old_position.z, 0.0);
    BOOST_CHECK_EQUAL(var1.energy_old, 0.0);
    BOOST_CHECK_EQUAL(var1.energy_new, 0.0);
    BOOST_CHECK_EQUAL(var1.index, 0);
    BOOST_CHECK_EQUAL(var1.key, 2); // ONLY NON-DEFAULT VALUE
    BOOST_CHECK_EQUAL(var1.do_old, true);
    BOOST_CHECK_EQUAL(var1.do_new, true);
    BOOST_CHECK(var1.old_rij.empty());
    BOOST_CHECK(var1.new_rij.empty());
    BOOST_CHECK(var1.old_rij2.empty());
    BOOST_CHECK(var1.new_rij2.empty());
  
    // Set atom "start" values
    var1.do_old = true;
    var1.do_new = true;
    var1.index = 10;
    var1.old_position.x = 1.0;
    var1.old_position.y = 10.0;
    var1.old_position.z = 2.0;
    var1.new_position.x = 0.9;
    var1.new_position.y = 9.8;
    var1.new_position.z = 2.15;
    // Set "calculated" values
    var1.energy_old = 0.001;
    var1.energy_new = 0.02;
    var1.old_rij = orij;
    var1.new_rij = nrij;
    var1.old_rij2 = orij;
    var1.new_rij2 = nrij;
    for( auto &r : var1.old_rij2 ) r = r * r;
    for( auto &r : var1.new_rij2 ) r = r * r;
    // Use var1 to make copies
    {
      // Copy ctor.
      particle::change_atom var2(var1);
      BOOST_CHECK(var2.equivalent(var1));
    }
    {
      // Assignment operator
      particle::change_atom var2;
      // Check differs
      BOOST_CHECK(not var2.equivalent(var1));
      // check equivalent
      var2 = var1;
      BOOST_CHECK(var2.equivalent(var1));
    }
    {
      particle::change_atom var2;
      particle::change_atom var3(var1);
  
      // Check differs  
      BOOST_CHECK(not var2.equivalent(var3));
  
      var2.swap (var3);
      // Check still differs
      BOOST_CHECK(not var2.equivalent(var3));
    }
    boost::archive::text_oarchive oa(store);
    // write class instance to archive
    oa << var1;
  }
  {
    particle::change_atom var1;
  
    boost::archive::text_iarchive ia(store);
    // get class instance from archive
    ia >> var1;
  
    BOOST_CHECK_EQUAL( var1.do_old, true );
    BOOST_CHECK_EQUAL( var1.do_new, true );
    BOOST_CHECK_EQUAL( var1.index, 10 );
    BOOST_CHECK_EQUAL( var1.old_position.x, 1.0 );
    BOOST_CHECK_EQUAL( var1.old_position.y, 10.0 );
    BOOST_CHECK_EQUAL( var1.old_position.z, 2.0 );
    BOOST_CHECK_EQUAL( var1.new_position.x, 0.9 );
    BOOST_CHECK_EQUAL( var1.new_position.y, 9.8 );
    BOOST_CHECK_EQUAL( var1.new_position.z, 2.15 );
    BOOST_CHECK_EQUAL( var1.key, 2 );
    BOOST_CHECK_EQUAL( var1.energy_old, 0.001 );
    BOOST_CHECK_EQUAL( var1.energy_new, 0.02 );
    BOOST_CHECK_EQUAL_COLLECTIONS( var1.old_rij.begin(), var1.old_rij.end()
          , orij.begin(), orij.end() );
    BOOST_CHECK_EQUAL_COLLECTIONS( var1.new_rij.begin(), var1.new_rij.end()
          , nrij.begin(), nrij.end() );
    std::vector< double > nrij2 { nrij };
    std::vector< double > orij2 { orij };
    for( auto &r : nrij2 ) r = r * r;
    for( auto &r : orij2 ) r = r * r;
    BOOST_CHECK_EQUAL_COLLECTIONS( var1.old_rij2.begin(), var1.old_rij2.end()
          , orij2.begin(), orij2.end() );
    BOOST_CHECK_EQUAL_COLLECTIONS( var1.new_rij2.begin(), var1.new_rij2.end()
          , nrij2.begin(), nrij2.end() );
  }
  {
    // Ctor with arg
    particle::change_atom var1;
    // Set atom rij2 "start" values
    var1.old_rij2 = orij;
    var1.new_rij2 = nrij;
    for( auto &r : var1.old_rij2 ) r = r * r;
    for( auto &r : var1.new_rij2 ) r = r * r;
    BOOST_CHECK(var1.old_rij.empty());
    BOOST_CHECK(var1.new_rij.empty());
    BOOST_CHECK(not var1.old_rij2.empty());
    BOOST_CHECK(not var1.new_rij2.empty());
    // should fill old_rij and new_rij
    var1.ensure_rij();
    BOOST_CHECK(not var1.old_rij.empty());
    BOOST_CHECK(not var1.new_rij.empty());
    BOOST_CHECK_EQUAL( var1.old_rij.size(), var1.old_rij2.size() );
    BOOST_CHECK_EQUAL( var1.new_rij.size(), var1.new_rij2.size() );
    BOOST_CHECK_EQUAL_COLLECTIONS( var1.old_rij.begin(), var1.old_rij.end()
          , orij.begin(), orij.end() );
    BOOST_CHECK_EQUAL_COLLECTIONS( var1.new_rij.begin(), var1.new_rij.end()
          , nrij.begin(), nrij.end() );
    var1.old_rij.clear();
    var1.old_rij2.clear();
    BOOST_CHECK(var1.old_rij.empty());
    BOOST_CHECK(not var1.new_rij.empty());
    BOOST_CHECK(var1.old_rij2.empty());
    BOOST_CHECK(not var1.new_rij2.empty());
    // should make no change
    var1.ensure_rij();
    BOOST_CHECK(var1.old_rij.empty());
    BOOST_CHECK(not var1.new_rij.empty());
    BOOST_CHECK(var1.old_rij2.empty());
    BOOST_CHECK(not var1.new_rij2.empty());
    var1.new_rij.clear();
    BOOST_CHECK(var1.new_rij.empty());
    // should fill new_rij
    var1.ensure_rij();
    BOOST_CHECK(not var1.new_rij.empty());
    BOOST_CHECK_EQUAL( var1.new_rij.size(), var1.new_rij2.size() );
  }
  
  

}

BOOST_AUTO_TEST_CASE( change_atom_comparison_test )
{
  const std::vector< double > nrij { 1.0, 1.3, 2.4 };
  const std::vector< double > orij { 1.1, 1.4, 2.0 };
  
  // Ctor with arg
  particle::change_atom var1( 2 );
  // Set atom "start" values
  var1.do_old = true;
  var1.do_new = true;
  var1.index = 10;
  var1.old_position.x = 1.0;
  var1.old_position.y = 10.0;
  var1.old_position.z = 2.0;
  var1.new_position.x = 0.9;
  var1.new_position.y = 9.8;
  var1.new_position.z = 2.15;
  // Set "calculated" values
  var1.energy_old = 0.001;
  var1.energy_new = 0.02;
  var1.old_rij = orij;
  var1.new_rij = nrij;
  var1.old_rij2 = orij;
  var1.new_rij2 = nrij;
  for( auto &r : var1.old_rij2 ) r = r * r;
  for( auto &r : var1.new_rij2 ) r = r * r;
  // Use var1 to make copies
  {
    // Copy ctor.
    particle::change_atom var2( var1 );
    BOOST_CHECK_EQUAL( var2.new_position.x, var1.new_position.x );
    BOOST_CHECK_EQUAL( var2.new_position.y, var1.new_position.y );
    BOOST_CHECK_EQUAL( var2.new_position.z, var1.new_position.z );
    BOOST_CHECK_EQUAL( var2.old_position.x, var1.old_position.x );
    BOOST_CHECK_EQUAL( var2.old_position.y, var1.old_position.y );
    BOOST_CHECK_EQUAL( var2.old_position.z, var1.old_position.z );
    BOOST_CHECK_EQUAL( var2.index, var1.index );
    BOOST_CHECK_EQUAL( var2.key, var1.key );
    BOOST_CHECK_EQUAL( var2.do_old, var1.do_old );
    BOOST_CHECK_EQUAL( var2.do_new, var1.do_new );
    BOOST_CHECK_EQUAL( var2.energy_old, var1.energy_old );
    BOOST_CHECK_EQUAL( var2.energy_new, var1.energy_new );
    BOOST_CHECK_EQUAL_COLLECTIONS( var2.old_rij.begin(), var2.old_rij.end()
                                   , orij.begin(), orij.end() );
    BOOST_CHECK_EQUAL_COLLECTIONS( var2.new_rij.begin(), var2.new_rij.end()
                                   , nrij.begin(), nrij.end() );
    std::vector< double > nrij2 { nrij };
    std::vector< double > orij2 { orij };
    for( auto &r : nrij2 ) r = r * r;
    for( auto &r : orij2 ) r = r * r;
    BOOST_CHECK_EQUAL_COLLECTIONS( var2.old_rij2.begin(), var2.old_rij2.end()
                                   , orij2.begin(), orij2.end() );
    BOOST_CHECK_EQUAL_COLLECTIONS( var2.new_rij2.begin(), var2.new_rij2.end()
                                   , nrij2.begin(), nrij2.end() );
     // Test equivalent is true when all data is the same
    BOOST_CHECK_EQUAL( var2, var1 );
  }
  {
    // TEST equivalent is sensitive to change in any one parameter
    BOOST_CHECK( var1.equivalent( var1 ) );
    {
      particle::change_atom var2( var1 );
      var2.new_position.x += 0.1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.new_position.y += 0.1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.new_position.z += 0.1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.old_position.x += 0.1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.old_position.y += 0.1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.old_position.z += 0.1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.energy_old += 0.1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.energy_new += 0.1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.do_old = not var2.do_old;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.do_new = not var2.do_new;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.key += 1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.index += 1;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.new_rij = orij;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.old_rij = nrij;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.new_rij2 = orij;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
    {
      particle::change_atom var2( var1 );
      var2.old_rij2 = nrij;
      BOOST_CHECK( not var2.equivalent( var1 ) );
    }
  
  }
  
  

}

BOOST_AUTO_TEST_CASE( change_hash_test )
{
  {
    // Test for canonical pattern
    BOOST_CHECK( std::is_default_constructible< particle::change_hash >::type {} );
    BOOST_CHECK( std::is_copy_constructible< particle::change_hash >::type {} );
    BOOST_CHECK( std::is_move_constructible< particle::change_hash >::type {} );
    BOOST_CHECK( ( std::is_assignable< particle::change_hash, particle::change_hash >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< particle::change_hash >::type {} );
  }
  std::stringstream store;
  {
    // Default ctor
    particle::change_hash var1;
    BOOST_CHECK_EQUAL( var1.key(), 0 );
    BOOST_CHECK_EQUAL( var1.start(), 0 );
    BOOST_CHECK_EQUAL( var1.finish(), 0 );
    BOOST_CHECK_EQUAL( var1.subtype(), 0 );
  
    particle::change_hash var2;
    BOOST_CHECK( var1.equivalent( var2 ) );
    BOOST_CHECK( var1 == var2 );
    BOOST_CHECK( var1 >= var2 );
    BOOST_CHECK( var1 <= var2 );
    BOOST_CHECK( not (var1 != var2) );
    BOOST_CHECK( not (var1.less_than( var2 ) ) );
    BOOST_CHECK( not (var1 < var2) );
  
    var1.key( 1 );
    var1.start( 2 );
    var1.finish( 3 );
    var1.subtype( 4 );
  
    BOOST_CHECK( not var1.equivalent( var2 ) );
  
    particle::change_hash var3( 1, 2, 3, 4 );
    BOOST_CHECK( var1.key() == var3.key() );
    BOOST_CHECK( var1.start() == var3.start() );
    BOOST_CHECK( var1.finish() == var3.finish() );
    BOOST_CHECK( var1.subtype() == var3.subtype() );
  
    // copy ctor
    particle::change_hash var4( var1 );
    BOOST_CHECK( var1 == var4 );
  
    // move ctor
    particle::change_hash var5( std::move( var4 ) );
    BOOST_CHECK( var1 == var5 );
  
    var5.subtype( 5 );
    BOOST_CHECK( var1 != var5 );
    // op=
    var5 = var3;
    BOOST_CHECK( var1 == var5 );
  
    boost::archive::text_oarchive oa(store);
    // write class instance to archive
    oa << var1;
  }
  {
    particle::change_hash var1;
    particle::change_hash var2;
  
    BOOST_CHECK( var1 == var2 );
  
    boost::archive::text_iarchive ia(store);
    // get class instance from archive
    ia >> var1;
  
    BOOST_CHECK( var1 != var2 );
    BOOST_CHECK_EQUAL( var1.key(), 1 );
    BOOST_CHECK_EQUAL( var1.start(), 2 );
    BOOST_CHECK_EQUAL( var1.finish(), 3 );
    BOOST_CHECK_EQUAL( var1.subtype(), 4 );
  }
  {
    // Check comparison
    particle::change_hash var1( 1, 2, 3, 4 );
    particle::change_hash var2( 1, 2, 3, 5 );
    BOOST_CHECK( var1.key() == var2.key() );
    BOOST_CHECK( var1.start() == var2.start() );
    BOOST_CHECK( var1.finish() == var2.finish() );
    BOOST_CHECK( var1.subtype() < var2.subtype() );
  
    BOOST_CHECK( var1 != var2 );
    BOOST_CHECK( var1 < var2 );
    BOOST_CHECK( var1 <= var2 );
    BOOST_CHECK( var2 > var1 );
    BOOST_CHECK( var2 >= var1 );
    BOOST_CHECK( not var1.match( var2 ) );
  
    var2 = var1;
    var2.finish( var2.finish() + 1 );
  
    BOOST_CHECK( var1 != var2 );
    BOOST_CHECK( var1 < var2 );
    BOOST_CHECK( var1 <= var2 );
    BOOST_CHECK( var2 > var1 );
    BOOST_CHECK( var2 >= var1 );
    BOOST_CHECK( not var1.match( var2 ) );
  
    var2 = var1;
    var2.start( var2.start() + 1 );
  
    BOOST_CHECK( var1 != var2 );
    BOOST_CHECK( var1 < var2 );
    BOOST_CHECK( var1 <= var2 );
    BOOST_CHECK( var2 > var1 );
    BOOST_CHECK( var2 >= var1 );
    BOOST_CHECK( not var1.match( var2 ) );
  
    var2 = var1;
    var2.key( var2.key() + 1 );
  
    BOOST_CHECK( var1 != var2 );
    BOOST_CHECK( var1 < var2 );
    BOOST_CHECK( var1 <= var2 );
    BOOST_CHECK( var2 > var1 );
    BOOST_CHECK( var2 >= var1 );
    BOOST_CHECK( var1.match( var2 ) );
  
  }

}

// Test ctor, copy, assign(?)
BOOST_AUTO_TEST_CASE( change_set_lifetime_test )
{
  auto set_atom = []( particle::change_atom &a, bool b1, bool b2, std::size_t idx, geometry::coordinate p1, geometry::coordinate p2 )->void
  {
    a.do_old = b1;
    a.do_new = b2;
    a.index = idx;
    a.old_position = p1;
    a.new_position = p2;
  };
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< particle::change_set >::type {} );
    BOOST_CHECK( std::is_copy_constructible< particle::change_set >::type {} );
    BOOST_CHECK( std::is_move_constructible< particle::change_set >::type {} );
    BOOST_CHECK( ( std::is_assignable< particle::change_set, particle::change_set >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< particle::change_set >::type {} );
  }
  {
    // Default ctor
    particle::change_set var1;
    BOOST_CHECK_EQUAL( var1.accept(), false );
    BOOST_CHECK( var1.begin() == var1.end() );
    BOOST_CHECK_EQUAL( var1.energy(), 0.0 );
    BOOST_CHECK_EQUAL( var1.exponential_factor(), 0.0 );
    BOOST_CHECK_EQUAL( var1.fail(), false );
    BOOST_CHECK_EQUAL( var1.metropolis_factor(), 1.0 );
    BOOST_CHECK_EQUAL( var1.probability_factor(), 1.0 );
    BOOST_CHECK_EQUAL( var1.size(), 0ul );
  }
  {
    std::stringstream store;
    {
      particle::change_set var1;
      // Make non-default 
      var1.update_exponential_factor( 0.05 );
      var1.update_probability_factor( 0.5 );
      particle::change_atom s1( 1 );
      set_atom( s1, true, true, 1, geometry::coordinate( -0.1, 0.1, 0.1 ), geometry::coordinate() );
      // Check add
      var1.add_atom( s1 );
  
      boost::archive::text_oarchive oa( store );
      // write class instance to archive
      oa << var1;
  
      // Make a copy
      {
        particle::change_set var2( var1 );
        BOOST_CHECK_EQUAL( var2.energy(), var1.energy() );
        BOOST_CHECK_EQUAL( var2.exponential_factor(), var1.exponential_factor() );
        BOOST_CHECK_EQUAL( var2.probability_factor(), var1.probability_factor() );
        BOOST_CHECK_EQUAL( var2.size(), var1.size() );
  
        BOOST_CHECK_EQUAL( var2[0].new_position.x, var1[0].new_position.x );
        BOOST_CHECK_EQUAL( var2[0].new_position.y, var1[0].new_position.y );
        BOOST_CHECK_EQUAL( var2[0].new_position.z, var1[0].new_position.z );
        BOOST_CHECK_EQUAL( var2[0].old_position.x, var1[0].old_position.x );
        BOOST_CHECK_EQUAL( var2[0].old_position.y, var1[0].old_position.y );
        BOOST_CHECK_EQUAL( var2[0].old_position.z, var1[0].old_position.z );
        BOOST_CHECK_EQUAL( var2[0].key, var1[0].key );
        BOOST_CHECK_EQUAL( var2[0].index, var1[0].index );
      }
      // Make a move
      {
        particle::change_set var3( var1 );
        particle::change_set var2( std::move( var3 ) );
        BOOST_CHECK_EQUAL( var2.energy(), var1.energy() );
        BOOST_CHECK_EQUAL( var2.exponential_factor(), var1.exponential_factor() );
        BOOST_CHECK_EQUAL( var2.probability_factor(), var1.probability_factor() );
        BOOST_CHECK_EQUAL( var2.size(), var1.size() );
  
        BOOST_CHECK_EQUAL( var2[0].new_position.x, var1[0].new_position.x );
        BOOST_CHECK_EQUAL( var2[0].new_position.y, var1[0].new_position.y );
        BOOST_CHECK_EQUAL( var2[0].new_position.z, var1[0].new_position.z );
        BOOST_CHECK_EQUAL( var2[0].old_position.x, var1[0].old_position.x );
        BOOST_CHECK_EQUAL( var2[0].old_position.y, var1[0].old_position.y );
        BOOST_CHECK_EQUAL( var2[0].old_position.z, var1[0].old_position.z );
        BOOST_CHECK_EQUAL( var2[0].key, var1[0].key );
        BOOST_CHECK_EQUAL( var2[0].index, var1[0].index );
      }
      // Assign a copy
      {
        particle::change_set var2;
        // Use var2 to stop compiler eliding assignment to a copy.
        BOOST_CHECK_EQUAL( var2.accept(), false );
  
        var2 = var1;
        BOOST_CHECK_EQUAL( var2.energy(), var1.energy() );
        BOOST_CHECK_EQUAL( var2.exponential_factor(), var1.exponential_factor() );
        BOOST_CHECK_EQUAL( var2.probability_factor(), var1.probability_factor() );
        BOOST_CHECK_EQUAL( var2.size(), var1.size() );
  
        BOOST_CHECK_EQUAL( var2[0].new_position.x, var1[0].new_position.x );
        BOOST_CHECK_EQUAL( var2[0].new_position.y, var1[0].new_position.y );
        BOOST_CHECK_EQUAL( var2[0].new_position.z, var1[0].new_position.z );
        BOOST_CHECK_EQUAL( var2[0].old_position.x, var1[0].old_position.x );
        BOOST_CHECK_EQUAL( var2[0].old_position.y, var1[0].old_position.y );
        BOOST_CHECK_EQUAL( var2[0].old_position.z, var1[0].old_position.z );
        BOOST_CHECK_EQUAL( var2[0].key, var1[0].key );
        BOOST_CHECK_EQUAL( var2[0].index, var1[0].index );
      }
    }
    {
      // Test serialization
      particle::change_set var2;
      boost::archive::text_iarchive ia( store );
      // get class instance from archive
      ia >> var2;
  
      BOOST_CHECK_EQUAL( var2.energy(), 0.0 );
      BOOST_CHECK_EQUAL( var2.exponential_factor(), 0.05 );
      BOOST_CHECK_EQUAL( var2.probability_factor(), 0.5 );
      BOOST_CHECK_EQUAL( var2.size(), 1ul );
  
      BOOST_CHECK_EQUAL( var2[0].new_position.x, 0.0 );
      BOOST_CHECK_EQUAL( var2[0].new_position.y, 0.0 );
      BOOST_CHECK_EQUAL( var2[0].new_position.z, 0.0 );
      BOOST_CHECK_EQUAL( var2[0].old_position.x, -0.1 );
      BOOST_CHECK_EQUAL( var2[0].old_position.y, 0.1 );
      BOOST_CHECK_EQUAL( var2[0].old_position.z, 0.1 );
      BOOST_CHECK_EQUAL( var2[0].key, 1 );
      BOOST_CHECK_EQUAL( var2[0].index, 1 );
    }
  }
  

}

BOOST_AUTO_TEST_CASE( change_set_methods_test )
{
  auto set_atom = []( particle::change_atom &a, bool b1, bool b2, std::size_t idx, geometry::coordinate p1, geometry::coordinate p2 )->void
  {
    a.do_old = b1;
    a.do_new = b2;
    a.index = idx;
    a.old_position = p1;
    a.new_position = p2;
  };
  {
    // Test attempt to set a bad probability value
    // Valid only if VALUE > 0.0
    particle::change_set var2;
    BOOST_CHECK_EQUAL( var2.probability_factor(), 1.0 );
    {
      particle::change_set var2cp( var2 );
      const std::string fncall{ "var2cp.update_probability_factor( 0.0 )" };
      try
      {
        var2cp.update_probability_factor( 0.0 );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Bad update value, factor should be greater than 0.0" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    {
      particle::change_set var2cp( var2 );
      const std::string fncall{ "var2cp.update_probability_factor( -1.0 )" };
      try
      {
        var2cp.update_probability_factor( -1.0 );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Bad update value, factor should be greater than 0.0" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
  }
  {
    // Test add steps
    particle::change_set var1;
  
    particle::change_atom s1( 0 );
    set_atom( s1, true, true, 0, geometry::coordinate(), geometry::coordinate( 0.1, 0.0, 0.1 ) );
  
    // Add once have individual jump move
    var1.add_atom( s1 );
    // Variable that should be changed
    BOOST_CHECK( var1.begin() != var1.end() );
    BOOST_REQUIRE_EQUAL( var1.size(), 1ul );
    BOOST_CHECK( s1.equivalent( var1[0] ) );
    BOOST_CHECK( s1.equivalent( *var1.begin() ) );
  
    // Add again: now have salt jump move
    s1.old_position.x += 3.0;
    s1.new_position.x += 3.0;
    s1.key = 1;
    var1.add_atom( s1 );
    // Variable that should be changed
    BOOST_CHECK( var1.begin() != var1.end() );
    BOOST_CHECK_EQUAL( var1.size(), 2ul );
    BOOST_CHECK( s1.equivalent( var1[1] ) );
    BOOST_CHECK( s1.equivalent( *( var1.begin() + 1 ) ) );
  
    // Add third time: still have salt jump move
    s1.old_position.y += 3.0;
    s1.new_position.y += 3.0;
    var1.add_atom( s1 );
    // Variable that should be changed
    BOOST_CHECK( var1.begin() != var1.end() );
    BOOST_CHECK_EQUAL( var1.size(), 3ul );
    BOOST_CHECK( s1.equivalent( var1[2] ) );
    BOOST_CHECK( s1.equivalent( *( var1.begin() + 2 ) ) );
    // Variables that should be unchanged
    BOOST_CHECK_EQUAL( var1.accept(), false );
    BOOST_CHECK_EQUAL( var1.energy(), 0.0 );
    BOOST_CHECK_EQUAL( var1.exponential_factor(), 0.0 );
    BOOST_CHECK_EQUAL( var1.fail(), false );
    BOOST_CHECK_EQUAL( var1.metropolis_factor(), 1.0 );
    BOOST_CHECK_EQUAL( var1.probability_factor(), 1.0 );
  
    // test equivalent()
    particle::change_set var2( var1 );
    // Verify individually
    BOOST_CHECK_EQUAL( var1.accept(), var2.accept() );
    BOOST_CHECK_EQUAL( var1.exponential_factor(), var2.exponential_factor() );
    BOOST_CHECK_EQUAL( var1.fail(), var2.fail() );
    BOOST_CHECK_EQUAL( var1.probability_factor(), var2.probability_factor() );
    BOOST_CHECK_EQUAL_COLLECTIONS( var1.begin(), var1.end()
    , var2.begin(), var2.end() );
    // before using class method.
    BOOST_CHECK_EQUAL( var1, var2 );
  
  }
  // Test factor adjustments
  {
    particle::change_set var2;
    particle::change_atom a1;
    var2.add_atom( a1 );
    BOOST_CHECK_EQUAL( var2.exponential_factor(), 0.0 );
    BOOST_CHECK_EQUAL( var2.probability_factor(), 1.0 );
    {
      // Test metropolis factor
      //  energy==0 exp==0.0 prob==1 beta=1
      //  prob.e( -(beta*energy-exp ))
      //  1 . e -( 0 - 0 ) == 1
      const double metro( var2.metropolis_factor() );
      if( not utility::feq( metro, 1.0 ) )
      {
        BOOST_ERROR( "metropolis_factor ["<<metro<<"] not expected value 1.0" );
      }
    }
    {
      // Test update_energy and metropolis
      //  energy==0.1 expf==0.0 probf==1 beta=1
      //  probf.e( -(beta*energy-expf ))
      // 1 . e( -(0.1 - 0 )) == 0.90483741803596
      var2.begin()->energy_new = 0.1;
      BOOST_CHECK( utility::feq( var2.energy(), 0.1 ) );
  
      const double metro( var2.metropolis_factor() );
      if( not utility::feq( metro, 0.90483741803596 ) )
      {
        BOOST_ERROR( "metropolis_factor ["<<metro<<"] not expected value 0.90483741803596" );
      }
    }
    {
      // Test update_probability_factor and metropolis
      //  energy==0.1 expf==0.0 probf==0.1 beta=1
      //  probf.e( -(beta*energy-expf ))
      // 0.3 . e( -(0.1 - 0 )) == 0.3 * 0.90484 = 0.27145
      var2.update_probability_factor( 0.1 );
      BOOST_CHECK( utility::feq( var2.probability_factor(), 0.1 ) );
      var2.update_probability_factor( 3.0 );
      BOOST_CHECK( utility::feq( var2.probability_factor(), 0.3 ) );
  
      const double metro( var2.metropolis_factor() );
      if( not utility::feq( metro, 0.271451225411, 1 << 12 ) )
      {
        BOOST_ERROR( "metropolis_factor ["<<metro<<"] not expected value 0.271451225411" );
      }
    }
    {
      // Test update_exponential_factor
      //  energy==0.1 expf==0.1 probf==0.1 beta=1
      //  probf.e( -(beta*energy-expf )) ->0.1
      // 0.3 . e( -(0.1 - 0.1 )) == 0.3 * 1 = 0.3
      var2.update_exponential_factor( 0.1 );
      BOOST_CHECK_EQUAL( var2.exponential_factor(), 0.1 );
  
      const double metro( var2.metropolis_factor() );
      if( not utility::feq( metro, 0.3 ) )
      {
        BOOST_ERROR( "metropolis_factor ["<<metro<<"] not expected value 0.3" );
      }
    }
    {
      // Test update_exponential_factor
      //  energy==0.1 expf==0.1 probf==0.1 beta=1
      //  probf.e( -(beta*energy-expf )) ->0.1
      // 0.3 . e( -(1.2 - 0.1 ) == 0.3 * 0.33287 == 0.09986
      var2.begin()->energy_new += 1.1;
      BOOST_CHECK( utility::feq( var2.energy(), 1.2 ) );
  
      const double metro( var2.metropolis_factor() );
      BOOST_CHECK_NE( metro, 0.0 );
      if( not utility::feq( metro, 0.0998613251094, 1 << 12 ) )
      {
        BOOST_ERROR( "metropolis_factor ["<<metro<<"] not expected value 0.0998613251094" );
      }
    }
    {
      // Test update_energy with large energy
      //  energy==100.1 expf==0.1 probf==0.1 beta=1
      //  probf.e( -(beta*energy-expf )) ->0.0....
      // 0.3 . e( -(101.3 - 0.1 ) == 0.3 * 1.238e-44 == 3.3614e-45
  
      var2.begin()->energy_new += 100.1;
      BOOST_CHECK( utility::feq( var2.energy(), 101.3 ) );
  
      const double metro( var2.metropolis_factor() );
      BOOST_CHECK_NE( metro, 0.0 );
      if( not utility::feq( metro, 3.36139605555e-45, 1 << 13 ) )
      {
        BOOST_ERROR( "metropolis_factor ["<<metro<<"] not expected value 3.36139605555e-45" );
      }
    }
  }
  
  

}

BOOST_AUTO_TEST_CASE( ensemble_lifetime_test )
{
  // Test framework
  
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< particle::ensemble >::type {} );
    BOOST_CHECK( std::is_copy_constructible< particle::ensemble >::type {} );
    BOOST_CHECK( std::is_move_constructible< particle::ensemble >::type {} );
    BOOST_CHECK( ( std::is_assignable< particle::ensemble, particle::ensemble >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< particle::ensemble >::type {} );
  }
  {
    // default ctor.
    particle::ensemble ens;
    BOOST_CHECK_EQUAL( ens.count(), 0 );
    BOOST_CHECK_EQUAL( ens.size(), 0 );
    BOOST_CHECK_NO_THROW( ens.check_invariants() );
  
    // Add a particle so no longer default object
    {
      std::size_t ispec( 0 );
      const geometry::coordinate pos
      {
        1.62127193214, 9.29177995257, 5.25370757073
      };
      ens.append_position( ispec, pos );
      // size should increase by one, specie count should be altered.
      BOOST_CHECK_EQUAL( ens.count(), 1 );
      BOOST_CHECK_EQUAL( ens.size(), 1 );
      BOOST_CHECK_EQUAL( ens.specie_count( ispec ), 1 );
      // Invariant should pass.
      BOOST_CHECK_NO_THROW( ens.check_invariants() );
      // Check set values
      BOOST_CHECK_EQUAL( ens.key( 0 ), ispec );
      BOOST_CHECK_EQUAL( ens.x( 0 ), pos.x );
      BOOST_CHECK_EQUAL( ens.y( 0 ), pos.y );
      BOOST_CHECK_EQUAL( ens.z( 0 ), pos.z );
      BOOST_CHECK_EQUAL( ens.position( 0 ), pos );
    }
  
    // copy ctor
    {
      particle::ensemble ens1( ens );
      BOOST_CHECK( ens == ens1 );
    }
    // move ctor
    {
      particle::ensemble ens2( ens );
      particle::ensemble ens1( std::move( ens2 ) );
  
      BOOST_CHECK( ens == ens1 );
    }
  
    // assignment
    {
      particle::ensemble ens1;
      BOOST_CHECK_NO_THROW( ens.check_invariants() );
      ens1 = ens;
      BOOST_CHECK( ens == ens1 );
    }
  }
  

}

// TODO known_keys
BOOST_AUTO_TEST_CASE( ensemble_methods_test )
{
  auto set_atom = []( particle::change_atom &a, bool b1, bool b2, std::size_t idx, geometry::coordinate p1, geometry::coordinate p2 )->void
  {
    a.do_old = b1;
    a.do_new = b2;
    a.index = idx;
    a.old_position = p1;
    a.new_position = p2;
  };
  
  // for testing serialization
  std::stringstream store;
  {
    particle::ensemble ens;
    BOOST_CHECK_EQUAL( ens.count(), 0 );
    BOOST_CHECK_EQUAL( ens.size(), 0 );
    BOOST_CHECK_NO_THROW( ens.check_invariants() );
  
    // Add a Na particle
    std::size_t ispec( 0 );
    {
      const geometry::coordinate pos { 1.62127193214, 9.29177995257, 5.25370757073
      };
      ens.append_position( ispec, pos );
      // size should increase by one, specie count should be altered.
      BOOST_CHECK_EQUAL( ens.count(), 1 );
      BOOST_CHECK_EQUAL( ens.size(), 1 );
      BOOST_CHECK_EQUAL( ens.specie_count( ispec ), 1 );
      BOOST_CHECK_NO_THROW( ens.check_invariants() );
      // Check set values
      BOOST_CHECK_EQUAL( ens.key( 0 ), ispec );
      BOOST_CHECK_EQUAL( ens.x( 0 ), pos.x );
      BOOST_CHECK_EQUAL( ens.y( 0 ), pos.y );
      BOOST_CHECK_EQUAL( ens.z( 0 ), pos.z );
      BOOST_CHECK_EQUAL( ens.position( 0 ), pos );
    }
  
    // Add some more particles
    ispec = 1;
    ens.append_position( 1, { 0.629669015988, 9.4781017778, 0.436567124972 } );
    ispec = 2;
    ens.append_position( 2, { 1.81235688249, 6.99878476378, 6.05346341778 } );
    ens.append_position( 2, { 9.86409500135, 8.03444072878, 6.96346891359 } );
    ens.append_position( 0, { 6.12488878987, 0.936444546808, 7.92035117147 } );
    ens.append_position( 2, { 4.02705270915, 4.9878484368, 9.33967304135 } );
    ens.append_position( 1, { 8.8153046596, 8.25296759285, 9.27222366556 } );
    ens.append_position( 2, { 3.01632820001, 3.43431356454, 0.880214873216 } );
    ispec = 0;
  
    BOOST_CHECK_EQUAL( ens.specie_count( 0 ), 2 );
    BOOST_CHECK_EQUAL( ens.specie_count( 1 ), 2 );
    BOOST_CHECK_EQUAL( ens.specie_count( 2 ), 4 );
  
    BOOST_CHECK_NO_THROW( ens.check_invariants() );
  
    std::size_t gidx;
    std::size_t lidx;
    BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 0, 0 ) );
    BOOST_CHECK_EQUAL( gidx, 0 );
    BOOST_CHECK_NO_THROW( lidx = ens.specie_index( gidx ) );
    BOOST_CHECK_EQUAL( lidx, 0 );
    BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 0, 1 ) );
    BOOST_CHECK_EQUAL( gidx, 4 );
    BOOST_CHECK_NO_THROW( lidx = ens.specie_index( gidx ) );
    BOOST_CHECK_EQUAL( lidx, 1 );
  
    BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 1, 0 ) );
    BOOST_CHECK_EQUAL( gidx, 1 );
    BOOST_CHECK_NO_THROW( lidx = ens.specie_index( gidx ) );
    BOOST_CHECK_EQUAL( lidx, 0 );
    BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 1, 1 ) );
    BOOST_CHECK_EQUAL( gidx, 6 );
    BOOST_CHECK_NO_THROW( lidx = ens.specie_index( gidx ) );
    BOOST_CHECK_EQUAL( lidx, 1 );
  
    BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 2, 0 ) );
    BOOST_CHECK_EQUAL( gidx, 2 );
    BOOST_CHECK_NO_THROW( lidx = ens.specie_index( gidx ) );
    BOOST_CHECK_EQUAL( lidx, 0 );
    BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 2, 1 ) );
    BOOST_CHECK_EQUAL( gidx, 3 );
    BOOST_CHECK_NO_THROW( lidx = ens.specie_index( gidx ) );
    BOOST_CHECK_EQUAL( lidx, 1 );
    BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 2, 2 ) );
    BOOST_CHECK_EQUAL( gidx, 5 );
    BOOST_CHECK_NO_THROW( lidx = ens.specie_index( gidx ) );
    BOOST_CHECK_EQUAL( lidx, 2 );
    BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 2, 3 ) );
    BOOST_CHECK_EQUAL( gidx, 7 );
    BOOST_CHECK_NO_THROW( lidx = ens.specie_index( gidx ) );
    BOOST_CHECK_EQUAL( lidx, 3 );
  
    {
      // nth_specie_index is constant so ens should remain in well defined
      // state. gidx is undefined.
      const std::string fncall { "gidx = ens.nth_specie_index( 0, 2 )" };
      try
      {
        ens.nth_specie_index( 0, 2 );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Fewer count of given key than requested" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    {
      // nth_specie_index is constant so ens should remain in well defined
      // state. gidx is undefined.
      const std::string fncall { "gidx = ens.nth_specie_index( 1, 2 )" };
      try
      {
        ens.nth_specie_index( 1, 2 );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Fewer count of given key than requested" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    {
      // nth_specie_index is constant so ens should remain in well defined
      // state. gidx is undefined.
      const std::string fncall { "gidx = ens.nth_specie_index( 2, 4 )" };
      try
      {
        ens.nth_specie_index( 2, 4 );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Fewer count of given key than requested" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    {
      // specie index is constant so ens should be well defined. lidx
      // undefined.
      const std::string fncall { "lidx = ens.specie_index( 8 )" };
      try
      {
        ens.specie_index( 8 );
        BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
      }
      catch( std::runtime_error const& err )
      {
        const std::string msg( err.what() );
        //std::cout << msg << "\n";
        BOOST_CHECK( msg.find( "Index out of range" ) < msg.size() );
      }
      catch( std::exception const& err )
      {
        BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
      }
    }
    // test nth_index
    // ---------------
    // NOTE: deletion list is empty so nth index will
    // always be identical to n
    BOOST_CHECK_EQUAL( ens.count(), ens.size() );
    for( std::size_t itry = 0; itry != ens.count(); ++itry )
    {
      const std::size_t i1 = ens.nth_index( itry );
      BOOST_CHECK_EQUAL( i1, itry );
    }
  
    // test commit
    // Create changesets to (1) add, (2) move and (3) delete a particle
    {
      // ADD
      particle::change_set trial;
      const geometry::coordinate pos
      {
        7.45423699019, 5.14523134204, 2.44775178272
      };
      particle::change_atom at( 0 );
      set_atom( at, false, true, ens.size(), {}, pos );
      trial.add_atom( at );
  
      // Check system before change
      BOOST_CHECK_EQUAL( ens.count(), 8 );
      BOOST_CHECK_EQUAL( ens.size(), 8 );
      BOOST_CHECK_EQUAL( ens.specie_count( 0 ), 2 );
  
      ens.commit( trial );
  
      // Check for changes.
      BOOST_CHECK_NO_THROW( ens.check_invariants() );
      BOOST_CHECK_EQUAL( ens.count(), 9 );
      BOOST_CHECK_EQUAL( ens.size(), 9 );
      BOOST_CHECK_EQUAL( ens.specie_count( 0 ), 3 );
      BOOST_CHECK_EQUAL( ens.position( 8 ), pos );
      BOOST_CHECK_EQUAL( ens.key( 8 ), 0 );
      BOOST_CHECK_EQUAL( ens.count(), ens.size() );
      BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 0, 2 ) );
      BOOST_CHECK_EQUAL( gidx, 8 );
    }
    {
      // MOVE
      particle::change_set trial;
      const std::size_t index
      {
        4
      };
      const geometry::coordinate pos
      {
        3.51095035114, 1.08643570153, 0.603658014653
      };
      particle::change_atom at( ens.key( index ) );
      set_atom( at, true, true, index, ens.position( index ), pos );
      trial.add_atom( at );
  
      // Check system before change
      BOOST_CHECK_EQUAL( ens.count(), 9 );
      BOOST_CHECK_EQUAL( ens.size(), 9 );
      BOOST_CHECK_EQUAL( ens.specie_count( 0 ), 3 );
      BOOST_CHECK_EQUAL( ens.key( index ), 0 );
  
      ens.commit( trial );
  
      // Check there are no changes.
      BOOST_CHECK_NO_THROW( ens.check_invariants() );
      BOOST_CHECK_EQUAL( ens.count(), 9 );
      BOOST_CHECK_EQUAL( ens.size(), 9 );
      BOOST_CHECK_EQUAL( ens.specie_count( 0 ), 3 );
      BOOST_CHECK_EQUAL( ens.key( index ), 0 );
      BOOST_CHECK_EQUAL( ens.position( index ), pos );
      BOOST_CHECK_EQUAL( ens.count(), ens.size() );
    }
    {
      // DELETE
      particle::change_set trial;
      const std::size_t index
      {
        4
      };
      particle::change_atom at( 0 );
      set_atom( at, true, false, index, ens.position( index ), {} );
      trial.add_atom( at );
  
      // Check system before change
      BOOST_CHECK_EQUAL( ens.count(), 9 );
      BOOST_CHECK_EQUAL( ens.size(), 9 );
      BOOST_CHECK_EQUAL( ens.specie_count( 0 ), 3 );
  
      ens.commit( trial );
  
      // Check for changes.
      BOOST_CHECK_NO_THROW( ens.check_invariants() );
      BOOST_CHECK_EQUAL( ens.count(), 8 );
      BOOST_CHECK_EQUAL( ens.size(), 9 );
      BOOST_CHECK_EQUAL( ens.specie_count( 0 ), 2 );
      BOOST_CHECK_EQUAL( ens.key( index ), particle::specie_key::nkey );
      BOOST_CHECK_NE( ens.count(), ens.size() );
      BOOST_CHECK_NO_THROW( gidx = ens.nth_specie_index( 0, 1 ) );
      BOOST_CHECK_EQUAL( gidx, 8 );
  
      // test nth_index
      // ---------------
      // NOTE: deletion list is not empty so nth index will
      // not always be identical to n
      BOOST_CHECK_NE( ens.count(), ens.size() );
      for( std::size_t itry = 0; itry != ens.count(); ++itry )
      {
        const std::size_t i1 = ens.nth_index( itry );
        if( itry >= index )
        {
          BOOST_CHECK_EQUAL( i1, itry + 1 );
        }
        else
        {
          BOOST_CHECK_EQUAL( i1, itry );
        }
  
      }
    }
    // conf::serialize
    // ---------------
    boost::archive::text_oarchive oa( store );
    // write class instance to archive
    oa << ens;
  }
  {
    particle::ensemble ens2;
    boost::archive::text_iarchive ia( store );
    // get class instance from archive
    ia >> ens2;
    BOOST_CHECK_EQUAL( ens2.count(), 8 );
    BOOST_CHECK_EQUAL( ens2.size(), 9 );
    BOOST_CHECK_EQUAL( ens2.specie_count( 0 ), 2 );
    BOOST_CHECK_EQUAL( ens2.specie_count( 1 ), 2 );
    BOOST_CHECK_EQUAL( ens2.specie_count( 2 ), 4 );
    BOOST_CHECK_NE( ens2.count(), ens2.size() );
  
    std::size_t gidx;
    BOOST_CHECK_NO_THROW( gidx = ens2.nth_specie_index( 0, 1 ) );
    BOOST_CHECK_EQUAL( gidx, 8 );
    BOOST_CHECK_NO_THROW( gidx = ens2.nth_specie_index( 0, 0 ) );
    BOOST_CHECK_EQUAL( gidx, 0 );
  
    BOOST_CHECK_NO_THROW( ens2.check_invariants() );
  }
  

}

BOOST_AUTO_TEST_CASE( particle_manager_lifetime_test )
{
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< particle::particle_manager >::type {} );
    BOOST_CHECK( std::is_copy_constructible< particle::particle_manager >::type {} );
    BOOST_CHECK( std::is_move_constructible< particle::particle_manager >::type {} );
    BOOST_CHECK( ( std::is_assignable< particle::particle_manager, particle::particle_manager >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< particle::particle_manager >::type {} );
  }
  {
    // default ctor.
    particle::particle_manager mgr;
    BOOST_CHECK_EQUAL( mgr.charge(), 0.0 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().count(), 0ul );
    BOOST_CHECK_EQUAL( mgr.specie_count(), 0ul );
    BOOST_CHECK_EQUAL( mgr.get_species().size(), 0ul );
    BOOST_CHECK_EQUAL( mgr.target_ionic_strength(), 0.0 );
    BOOST_CHECK_EQUAL( mgr.target_count(), 0ul );
    BOOST_CHECK_EQUAL( mgr.ionic_strength( 100.0 ), 0.0 );
  
    {
      const std::string canon( "----------------------------------------------------------------------\nParticle system details.\n------------------------\n     number of species : 0\n target particle count : 0\n target ionic strength : 0\n----------------------------------------------------------------------\nensemble\n   particle # :0\n         size :0" );
      std::stringstream ss;
      mgr.description( ss );
      const std::string dstr( ss.str() );
      // std::cout << "CANON\n" << canon << "\n";
      // std::cout << "OUTPUT\n" << dstr << "\n";
      BOOST_CHECK( dstr.find( canon ) < dstr.size() );
    }
    {
      const std::string canon( "fileversion 1\n\n" );
      core::input_document wrtr( 1ul );
      mgr.write_document( wrtr );
      std::stringstream idoc;
      wrtr.write( idoc );
      const std::string sdoc( idoc.str() );
      //std::cout << "[" << sdoc << "]\n";
      BOOST_CHECK( sdoc.find( canon ) < sdoc.size() );
    }
  //   try
  //   {
  //     double vol = mgr.target_volume();
  //     BOOST_CHECK_EQUAL( vol, 0.0 );
  //     BOOST_ERROR( "expected \"mgr.target_volume()\" exception not thrown" );
  //   }
  //   catch( core::input_error const& err )
  //   {
  //     const std::string msg( err.what() );
  //     // std::cout << msg << "\n";
  //     BOOST_CHECK( msg.find( "Cannot calculate target volume without a target concentration" ) < msg.size() );
  //   }
  //   catch( std::exception const& err )
  //   {
  //     BOOST_ERROR( std::string( "exception thrown by \"mgr.target_volume()\" was not expected type: " ) + err.what() );
  //   }
    // HAVE VOLUME ONLY -> ERROR
    {
      double conc, vol;
      std::size_t count;
      bool cset, vset, nset;
      try
      {
      std::tie( conc, cset, vol, vset, count, nset ) = mgr.compute_initial_size( 830270.0 );BOOST_ERROR( "expected \"mgr.compute_initial_size()\" exception not thrown" );
     }
     catch( core::input_error const& err )
     {
       const std::string msg( err.what() );
       //std::cout << msg << "\n";
       BOOST_CHECK( msg.find( "Unable to automatically determine initial per-specie particle counts from input in section \"specie\".\n** Add predefined particles (parameter \"n\") non-solute species. **" ) < msg.size() );
     }
     catch( std::exception const& err )
     {
       BOOST_ERROR( std::string( "exception thrown by \"mgr.compute_initial_size()\" was not expected type: " ) + err.what() );
     }
    }
    mgr.set_target_count( 100ul );
    BOOST_REQUIRE_EQUAL( mgr.target_count(), 100ul );
    // HAVE VOLUME AND COUNT -> set CONCENTRATION
    {
      double conc, vol;
      std::size_t count;
      bool cset, vset, nset;
      std::tie( conc, cset, vol, vset, count, nset ) = mgr.compute_initial_size( 830270.0 );
      BOOST_REQUIRE( cset );
      BOOST_REQUIRE_CLOSE( conc, 0.2, 0.0001 );
      BOOST_REQUIRE( not vset );
      BOOST_REQUIRE_EQUAL( vol, 830270.0 );
      BOOST_REQUIRE( not nset );
      BOOST_REQUIRE_EQUAL( count, 100ul );
    }
  }

}

BOOST_AUTO_TEST_CASE( particle_manager_methods_test )
{
  // Methods tested
  //  * serialize
  //  * add_specie
  //  * add_predefined_particles
  //  * build_input_delegater
  //  * charge
  //  * description
  //  * get_ensemble
  //  * get_specie
  //  * get_specie_key
  //  * get_species
  //  * has_specie
  //  * ionic_strength
  //  * set_target_count
  //  * specie_count
  //  * target_ionic_strength
  //  * target_count
  //  * write_document
  // Tested elsewhere
  //  commit
  //
  auto compare_str = [](std::string s2, std::string s1)
  {
    if( s1 != s2)
    {
      std::stringstream ss1(s1), ss2(s2);
      std::size_t line = 0;
      std::string st2;
      std::getline( ss2, st2 );
      while( ss1 and ss2 )
      {
        std::string st1;
        std::getline( ss1, st1 );
        ++line;
        if( st1.empty() ) continue;
        if( st1.find(st2) > st1.size() and st2.find(st1) > st2.size() )
        {
          std::cerr << "Difference at line [" << line << "]\n\"" << st1 << "\"\n\"" << st2 << "\"\n";
        }
        else
        {
          std::cerr << "Start match at line [" << line << "]\n\"" << st1 << "\"\n\"" << st2 << "\"\n";
          break;
        }
      }
      while( ss1 and ss2 )
      {
        std::string st1;
        std::getline( ss1, st1 );
        std::getline( ss2, st2 );
        ++line;
        if( st1 != st2 )
        {
           std::cerr << "Difference at line [" << line << "]\n\"" << st1 << "\"\n\"" << st2 << "\"\n";
        }
      }
    }
  };
  
  auto check_manager = []( particle::particle_manager const& mgr )->void
  {
    const geometry::coordinate pos0( 0.3, 0.1, 0.4 );
    const geometry::coordinate pos1( 2.0, 0.0, 0.0 );
    const geometry::coordinate pos2( 0.0, 2.0, 0.0 );
    const geometry::coordinate pos3( 0.0, 0.0, 2.0 );
    const geometry::centroid ctr0( 1.0, 0.2, 0.0, 3.4 );
    const geometry::coordinate pos4( 1.3, 1.1, 1.4 );
    BOOST_CHECK_EQUAL( mgr.specie_count(), 3 );
    BOOST_CHECK_EQUAL( mgr.get_species().size(), 3 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().count(), 5 );
    BOOST_CHECK_EQUAL( mgr.charge(), -1.0 );
    BOOST_CHECK_EQUAL( mgr.target_count(), 100ul );
    BOOST_CHECK( mgr.has_specie( "PC" ) );
    BOOST_CHECK( mgr.has_specie( "Na" ) );
    BOOST_CHECK( mgr.has_specie( "Cl" ) );
    BOOST_CHECK_EQUAL( mgr.get_specie_key( "PC" ), 0ul );
    BOOST_CHECK_EQUAL( mgr.get_specie_key( "Na" ), 1ul );
    BOOST_CHECK_EQUAL( mgr.get_specie_key( "Cl" ), 2ul );
    BOOST_CHECK_EQUAL( mgr.get_specie( 0 ).label(), "PC" );
    BOOST_CHECK_EQUAL( mgr.get_specie( 1 ).label(), "Na" );
    BOOST_CHECK_EQUAL( mgr.get_specie( 2 ).label(), "Cl" );
    BOOST_CHECK_EQUAL( mgr.get_specie( 0 ).valency(), 1.0 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 1 ).valency(), 1.0 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 2 ).valency(), -1.0 );
    BOOST_CHECK_EQUAL( mgr.get_species().at( 0 ).label(), "PC" );
    BOOST_CHECK_EQUAL( mgr.get_species().at( 1 ).label(), "Na" );
    BOOST_CHECK_EQUAL( mgr.get_species().at( 2 ).label(), "Cl" );
    BOOST_CHECK( mgr.get_specie( 0 ).is_mobile() );
    BOOST_CHECK( mgr.get_specie( 1 ).is_solute() );
    BOOST_CHECK( mgr.get_specie( 2 ).is_solute() );
    BOOST_CHECK_EQUAL( mgr.get_specie( 0 ).get_position_size(), 1 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 1 ).get_position_size(), 1 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 2 ).get_position_size(), 3 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 0 ).get_localization_size(), 1 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 1 ).get_localization_size(), 0 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 2 ).get_localization_size(), 0 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 0 ).get_position( 0 ), pos0 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 0 ).get_localization_data( 0 ), ctr0 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 1 ).get_position( 0 ), pos4 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 2 ).get_position( 0 ), pos1 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 2 ).get_position( 1 ), pos2 );
    BOOST_CHECK_EQUAL( mgr.get_specie( 2 ).get_position( 2 ), pos3 );
  
    BOOST_CHECK_EQUAL( mgr.get_ensemble().specie_count( 0 ), 1 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().specie_count( 1 ), 1 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().specie_count( 2 ), 3 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().position( 0 ), pos0 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().position( 1 ), pos4 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().position( 2 ), pos1 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().position( 3 ), pos2 );
    BOOST_CHECK_EQUAL( mgr.get_ensemble().position( 4 ), pos3 );
  };
  
  std::stringstream store;
  {
    boost::shared_ptr< particle::particle_manager > mgr( new particle::particle_manager );
    {
      // Use input file to add data to manager
      core::input_delegater dg( 1 );
      particle::particle_manager::build_input_delegater( mgr, dg );
      BOOST_REQUIRE( dg.has_section( "specie" ) );
      std::string canon_input( "specie\ntype mob\nname \"PC\"\nz 1.0\nd 1.1\nn 1\n0.3 0.1 0.4 1.0 0.2 0.0 3.4\nend\n\nspecie\ntype free\nname \"Na\"\nz 1.0\nd 1.0\nn 1\n1.3 1.1 1.4\nctarg 0.1\nend\n\n" );
      core::input_preprocess reader;
      reader.add_buffer( "dummy", canon_input );
      BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
      core::input_help hlpr;
      dg.get_documentation( hlpr );
      std::stringstream ss;
      hlpr.write( ss );
      const std::string hstr( ss.str() );
      //std::cout << hstr << "\n";
      BOOST_CHECK( hstr.find( "  specie\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    chex\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    d\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    n\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    name\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    ratexc\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    ratgr\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    ratmov\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    ratreg\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    ratspc\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    type\n" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "mob|flex|chonly|free" ) < hstr.size() );
      BOOST_CHECK( hstr.find( "\n    z\n" ) < hstr.size() );
    }
    BOOST_REQUIRE_EQUAL( mgr->specie_count(), 2 );
    BOOST_REQUIRE_EQUAL( mgr->get_species().size(), 2 );
    BOOST_REQUIRE( mgr->has_specie( "PC" ) );
    BOOST_REQUIRE( mgr->has_specie( "Na" ) );
    BOOST_REQUIRE_EQUAL( mgr->get_specie_key( "PC" ), 0ul );
    BOOST_REQUIRE_EQUAL( mgr->get_specie_key( "Na" ), 1ul );
    BOOST_REQUIRE_EQUAL( mgr->get_specie( 0 ).get_position_size(), 1 );
    BOOST_REQUIRE_EQUAL( mgr->get_specie( 0 ).get_localization_size(), 1 );
    BOOST_REQUIRE_EQUAL( mgr->get_specie( 1 ).get_position_size(), 1 );
    BOOST_REQUIRE_EQUAL( mgr->get_specie( 1 ).get_localization_size(), 0 );
    BOOST_REQUIRE_EQUAL( mgr->get_ensemble().count(), 0 );
    BOOST_REQUIRE_EQUAL( mgr->charge(), 0.0 );
  
    // Manually build and add specie
    {
      const geometry::coordinate pos1( 2.0, 0.0, 0.0 );
      const geometry::coordinate pos2( 0.0, 2.0, 0.0 );
      const geometry::coordinate pos3( 0.0, 0.0, 2.0 );
  
      particle::specie var;
      var.set_radius( 0.9 );
      var.set_rate( 0.5 );
      var.set_valency( -1.0 );
      var.set_concentration( 0.1 );
      var.set_label( "Cl" );
      var.append_position( pos1 );
      var.append_position( pos2 );
      var.append_position( pos3 );
      var.set_type( particle::specie::SOLUTE );
      mgr->add_specie( var );
    }
    BOOST_REQUIRE( mgr->has_specie( "Cl" ) );
    BOOST_REQUIRE_EQUAL( mgr->get_specie( 2 ).get_position_size(), 3 );
    BOOST_REQUIRE_EQUAL( mgr->get_specie( 2 ).get_localization_size(), 0 );
  
    BOOST_REQUIRE_EQUAL( mgr->get_ensemble().count(), 0 );
    // copy position from species to ensemble
    BOOST_REQUIRE_NO_THROW( mgr->add_predefined_particles() );
    BOOST_REQUIRE_EQUAL( mgr->get_ensemble().count(), 5 );
    BOOST_REQUIRE_EQUAL( mgr->charge(), -1.0 );
  
    BOOST_REQUIRE_EQUAL( mgr->target_count(), 0ul );
    BOOST_REQUIRE_EQUAL( mgr->target_ionic_strength(), 0.2 );
    // HAVE VOLUME AND IONIC_STRENGTH -> set COUNT
    {
      double conc, vol;
      std::size_t count;
      bool cset, vset, nset;
      std::tie( conc, cset, vol, vset, count, nset ) = mgr->compute_initial_size( 830270.0 );
      BOOST_REQUIRE( not cset );
      BOOST_REQUIRE_EQUAL( conc, 0.2 );
      BOOST_REQUIRE( not vset );
      BOOST_REQUIRE_EQUAL( vol, 830270.0 );
      BOOST_REQUIRE( nset );
      BOOST_REQUIRE_EQUAL( count, 100ul );
    }
    mgr->set_target_count( 100ul );
    BOOST_REQUIRE_EQUAL( mgr->target_count(), 100ul );
    // HAVE VOLUME, COUNT AND IONIC_STRENGTH -> set VOLUME
    {
      double conc, vol;
      std::size_t count;
      bool cset, vset, nset;
      std::tie( conc, cset, vol, vset, count, nset ) = mgr->compute_initial_size( 100.0 );
      BOOST_REQUIRE( not cset );
      BOOST_REQUIRE_EQUAL( conc, 0.2 );
      BOOST_REQUIRE( vset );
      BOOST_CHECK_CLOSE( vol, 830269.63836775627, 0.000000001 );
      BOOST_REQUIRE( not nset );
      BOOST_REQUIRE_EQUAL( count, 100ul );
    }
    BOOST_CHECK_CLOSE( mgr->ionic_strength( 50.0 ), 132.84314213884102, 0.0000000001 );
  
    check_manager( *mgr );
  
    // Test serialization
    boost::archive::text_oarchive oa( store );
    // write class instance to archive
    oa << mgr;
  }
  {
    // Test (de)serialization
    boost::shared_ptr< particle::particle_manager > mgr;
    boost::archive::text_iarchive ia( store );
    // get class instance from archive
    ia >> mgr;
    BOOST_REQUIRE( mgr );
    check_manager( *mgr );
  
    {
      std::string canon( "----------------------------------------------------------------------\nParticle system details.\n------------------------\n     number of species : 3\n target particle count : 100\n target ionic strength : 0.2\n----------------------------------------------------------------------\nspecie\n        label :PC\n       radius :0.55\n         rate :1\n      valency :1\n  excess c.p. :0\n   chem. pot. :0\n target conc. :0\n         type :mob\nspecie\n        label :Na\n       radius :0.5\n         rate :1\n      valency :1\n  excess c.p. :0\n   chem. pot. :-9.71748\n target conc. :0.1\n         type :free\nspecie\n        label :Cl\n       radius :0.9\n         rate :0.5\n      valency :-1\n  excess c.p. :0\n   chem. pot. :-9.71748\n target conc. :0.1\n         type :free\n----------------------------------------------------------------------\nensemble\n   particle # :5\n         size :5\n    0   0.30000   0.10000   0.40000  0\n    1   1.30000   1.10000   1.40000  1\n    2   2.00000   0.00000   0.00000  2\n    3   0.00000   2.00000   0.00000  2\n    4   0.00000   0.00000   2.00000  2\n" );
      std::stringstream ss;
      mgr->description( ss );
      const std::string dstr( ss.str() );
      // std::cout << "CANON\n" << canon << "\n";
      // std::cout << "OUTPUT\n" << dstr << "\n";
      BOOST_CHECK( dstr.find( canon ) < dstr.size() );
      if( dstr.find( canon ) > dstr.size() ) compare_str( canon, dstr );
    }
  
    {
      std::string canon( "specie\nchex 0\nctarg 0\nd 1.100000\nn 1\n0.3 0.1 0.4 1 0.2 0 3.4\nname \"PC\"\nrate 1.000000\ntype mob\nz 1.000000\nend\n\nspecie\nchex 0\nctarg 0.100000\nd 1.000000\nn 1\n1.3 1.1 1.4\nname \"Na\"\nrate 1.000000\ntype free\nz 1.000000\nend\n\nspecie\nchex 0\nctarg 0.100000\nd 1.800000\nn 3\n2 0 0\n0 2 0\n0 0 2\nname \"Cl\"\nrate 0.500000\ntype free\nz -1\nend" );
      core::input_document wrtr( 1ul );
      mgr->write_document( wrtr );
      std::stringstream idoc;
      wrtr.write( idoc );
      const std::string sdoc( idoc.str() );
      // std::cout << "CANON\n" << canon << "\n";
      // std::cout << "OUTPUT\n" << sdoc << "\n";
      BOOST_CHECK( sdoc.find( canon ) < sdoc.size() );
      if( sdoc.find( canon ) > sdoc.size() ) compare_str( canon, sdoc );
    }
  }
  

}

// Test manually building a system from specie objects
BOOST_AUTO_TEST_CASE( mockup_system_test )
{
  particle::particle_manager pman;
  {
    particle::specie spc1;
    spc1.set_label( "Na" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.1 );
    spc1.set_valency( 1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::SOLUTE );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 2.0, 0.0, 0.0 ) );
    pman.add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "Cl" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.2 );
    spc1.set_rate( 0.1 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.3123 );
    spc1.set_type( particle::specie::SOLUTE );
    spc1.append_position( geometry::coordinate( 0.0, 2.0, 0.0 ) );
    pman.add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "CA" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.11 );
    spc1.set_rate( 0.1 );
    spc1.set_valency( 1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::MOBILE );
    spc1.append_position( geometry::coordinate( 2.0, 2.0, 0.0 ), geometry::centroid( 3.0, 2.0, 2.0, 0.0 ) );
    pman.add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "CO" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.1 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::FLEXIBLE );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 2.0 ), geometry::centroid( 3.0, 0.0, 0.0, 2.0 ) );
    pman.add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "OX" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.12 );
    spc1.set_rate( 0.1 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.123 );
    spc1.set_type( particle::specie::CHANNEL_ONLY );
    spc1.append_position( geometry::coordinate( 2.0, 0.0, 2.0 ) );
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 5 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 6 );
  
  

}

// Test building a system from specie objects
BOOST_AUTO_TEST_CASE( particle_man_generate_simulation_test )
{
  std::stringstream store;
  {
    particle::particle_manager pman;
    {
      particle::specie spc1;
      spc1.set_label( "Na" );
      spc1.set_concentration( 1.0 );
      spc1.set_radius( 0.12 );
      spc1.set_rate( 0.1 );
      spc1.set_valency( 1.0 );
      spc1.set_excess_potential( 0.123 );
      spc1.set_type( particle::specie::SOLUTE );
      spc1.append_position( geometry::coordinate( 0.0, 0.0, 0.0 ) );
      spc1.append_position( geometry::coordinate( 2.0, 0.0, 0.0 ) );
      pman.add_specie( spc1 );
    }
    {
      particle::specie spc1;
      spc1.set_label( "Cl" );
      spc1.set_concentration( 1.0 );
      spc1.set_radius( 0.2 );
      spc1.set_rate( 0.1 );
      spc1.set_valency( -1.0 );
      spc1.set_excess_potential( 0.3123 );
      spc1.set_type( particle::specie::SOLUTE );
      spc1.append_position( geometry::coordinate( 0.0, 2.0, 0.0 ) );
      pman.add_specie( spc1 );
    }
    {
      particle::specie spc1;
      spc1.set_label( "CA" );
      spc1.set_concentration( 1.0 );
      spc1.set_radius( 0.11 );
      spc1.set_rate( 0.1 );
      spc1.set_valency( 1.0 );
      spc1.set_excess_potential( 0.123 );
      spc1.set_type( particle::specie::MOBILE );
      spc1.append_position( geometry::coordinate( 2.0, 2.0, 0.0 ), geometry::centroid( 3.0, 2.0, 2.0, 0.0 ) );
      pman.add_specie( spc1 );
    }
    {
      particle::specie spc1;
      spc1.set_label( "CO" );
      spc1.set_concentration( 1.0 );
      spc1.set_radius( 0.12 );
      spc1.set_rate( 0.1 );
      spc1.set_valency( -1.0 );
      spc1.set_excess_potential( 0.123 );
      spc1.set_type( particle::specie::FLEXIBLE );
      spc1.append_position( geometry::coordinate( 0.0, 0.0, 2.0 ), geometry::centroid( 3.0, 0.0, 0.0, 2.0 ) );
      pman.add_specie( spc1 );
    }
    {
      particle::specie spc1;
      spc1.set_label( "OX" );
      spc1.set_concentration( 1.0 );
      spc1.set_radius( 0.12 );
      spc1.set_rate( 0.1 );
      spc1.set_valency( -1.0 );
      spc1.set_excess_potential( 0.123 );
      spc1.set_type( particle::specie::CHANNEL_ONLY );
      spc1.append_position( geometry::coordinate( 2.0, 0.0, 2.0 ) );
      pman.add_specie( spc1 );
    }
  
    BOOST_REQUIRE_EQUAL( pman.target_count(), 0ul );
    // Test serialization
    boost::archive::text_oarchive oa( store );
    // write class instance to archive
    oa << pman;
  
    const std::string label( "bulk" );
    // Cubic grid main ctor
    boost::shared_ptr< geometry::periodic_cube_region > sys_reg( new geometry::periodic_cube_region( label, 100.0 ) );
    geometry::geometry_manager gman( sys_reg );
    boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
    utility::random_distribution rand( generator );
  
    {
      double conc, vol;
      std::size_t count;
      bool cset, vset, nset;
      std::tie( conc, cset, vol, vset, count, nset ) = pman.compute_initial_size( sys_reg->volume( 0.0 ) );
      // std::cout << "conc   : " << conc << "\n";
      // std::cout << "volume : " << vol << "\n";
      // std::cout << "count  : " << count << "\n";
      BOOST_REQUIRE( not cset );
      BOOST_REQUIRE_CLOSE( conc, 2.0, 0.0001 );
      BOOST_REQUIRE( not vset );
      BOOST_CHECK_CLOSE( vol, 1000000.0, 0.00001 );
      BOOST_REQUIRE( nset );
      BOOST_REQUIRE_EQUAL( count, 1204ul );
    }
    // Call generate simulation
    std::stringstream log;
    pman.generate_simulation( gman, rand, log );
  
    // count should be 1204 solute + 3 specials
    BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 1207ul );
  
  }
  {
    const std::string label( "bulk" );
    // Cubic grid main ctor
    boost::shared_ptr< geometry::periodic_cube_region > sys_reg( new geometry::periodic_cube_region( label, 100.0 ) );
    geometry::geometry_manager gman( sys_reg );
    boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
    utility::random_distribution rand( generator );
  
    // Test (de)serialization
    particle::particle_manager mgr;
    boost::archive::text_iarchive ia( store );
    // get class instance from archive
    ia >> mgr;
  
    BOOST_REQUIRE_EQUAL( mgr.target_count(), 0ul );
    mgr.set_target_count( 500 );
    {
      double conc, vol;
      std::size_t count;
      bool cset, vset, nset;
      std::tie( conc, cset, vol, vset, count, nset ) = mgr.compute_initial_size( sys_reg->volume( 0.0 ) );
      // std::cout << "conc   : " << conc << "\n";
      // std::cout << "volume : " << vol << "\n";
      // std::cout << "count  : " << count << "\n";
      BOOST_REQUIRE( not cset );
      BOOST_REQUIRE_CLOSE( conc, 2.0, 0.0001 );
      BOOST_REQUIRE( vset );
      BOOST_CHECK_CLOSE( vol, 415134.82, 0.00001 );
      BOOST_REQUIRE( not nset );
      BOOST_REQUIRE_EQUAL( count, 500ul );
    }
  
    // Call generate simulation
    std::stringstream log;
    mgr.generate_simulation( gman, rand, log );
  
    // count should be 500 solute + 3 specials
    BOOST_REQUIRE_EQUAL( mgr.get_ensemble().count(), 503ul );
  
    BOOST_CHECK_CLOSE( sys_reg->volume( 0.0 ), 415134.82, 0.00001 );
  
  }

}

BOOST_AUTO_TEST_CASE( specie_meta_test )
{
  {
    // Test is virtual + noncopyable object type
    BOOST_CHECK( not std::is_default_constructible< particle::specie_meta >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< particle::specie_meta >::type {} );
    BOOST_CHECK( not std::is_move_constructible< particle::specie_meta >::type {} );
    BOOST_CHECK( not( std::is_assignable< particle::specie_meta, particle::specie_meta >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< particle::specie_meta >::type {} );
  }
  // TEST management of keywords
  for( std::string kw :
       {
         core::strngs::fsname(), core::strngs::fstype(), core::strngs::fsrtsp(), core::strngs::fsd(), core::strngs::fsz(), core::strngs::fsctrg(), core::strngs::fschex(), core::strngs::fsn()
       } )
  {
    BOOST_CHECK( particle::specie_meta::is_standard_keyword( kw ) );
  }
  {
    // Matches keyword so should throw an error. !!Assume string exception
    // guarrantee.!!
    const std::string fncall { "particle::specie_meta::add_keyword( \"ratspc\" )" };
    try
    {
      particle::specie_meta::add_keyword( { "ratspc", "", "", "", "" } );
      BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Attempt to register keyword \"ratspc\" that is the same as a standard keyword" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
    }
  }
  
  if( not particle::specie_meta::has_keyword( "ratexc" ) )
  {
    particle::specie_meta::add_keyword( { "ratexc", "", "", "", "" } );
  }
  if( not particle::specie_meta::has_keyword( "ratgr" ) )
  {
    particle::specie_meta::add_keyword( {"ratgr", "", "", "", "" } );
  }
  if( not particle::specie_meta::has_keyword( "ratreg" ) )
  {
    particle::specie_meta::add_keyword( {"ratreg", "", "", "", "" } );
  }
  if( not particle::specie_meta::has_keyword( "ratmov" ) )
  {
    particle::specie_meta::add_keyword( {"ratmov", "", "", "", "" } );
  }
  
  // Test read input (specie meta)
  {
    boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
      dg.add_input_delegate( m );
    }
  
    std::string canon_input( "specie\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\ntype free\nend\n\n" );
  
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
    BOOST_CHECK_EQUAL( appl->specie_count(), 1ul );
    BOOST_CHECK( appl->has_specie( "Ca" ) );
    const particle::specie &var1( appl->get_specie( appl->get_specie_key( "Ca" ) ) );
    BOOST_CHECK( var1.is_valid() );
    BOOST_CHECK( var1.has_parameter( "ratreg" ) );
    BOOST_CHECK( var1.has_parameter( "ratmov" ) );
    BOOST_CHECK( var1.has_parameter( "ratexc" ) );
    BOOST_CHECK( var1.has_parameter( "ratgr" ) );
    BOOST_CHECK( var1.is_solute() );
    BOOST_CHECK_EQUAL( var1.label(), "Ca" );
    BOOST_CHECK_EQUAL( var1.rate(), 1.0 );
    BOOST_CHECK_EQUAL( var1.parameter( "ratmov" ).value(), "1.0" );
    BOOST_CHECK_EQUAL( var1.parameter( "ratexc" ).value(), "2.0" );
    BOOST_CHECK_EQUAL( var1.parameter( "ratgr" ).value(), "3.0" );
    BOOST_CHECK_EQUAL( var1.valency(), 2.0 );
    BOOST_CHECK_EQUAL( var1.radius(), 0.55 );
    BOOST_CHECK_EQUAL( var1.excess_potential(), 0.792 );
    BOOST_CHECK_EQUAL( var1.is_solute(), true );
    BOOST_CHECK_EQUAL( var1.parameter( "ratreg" ).value(), "0.3 0.3 0.3 0.1" );
  }
  {
    boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
      dg.add_input_delegate( m );
    }
  
    // Valid : sub-type known : free
    ////////////////////////////////
    std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    BOOST_CHECK( appl->get_specie( 0 ).is_solute() );
  }
  {
    boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
      dg.add_input_delegate( m );
    }
  
    // Valid : sub-type known : mob
    ///////////////////////////////
    std::string canon_input( "specie\ntype mob\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nn 1\n0.3 0.1 0.4 1.0 0.2 0.0 3.4\nend\n\n" );
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    BOOST_CHECK( appl->get_specie( 0 ).is_mobile() );
    BOOST_CHECK_EQUAL( appl->get_specie( 0 ).get_position_size(), 1 );
    BOOST_CHECK_EQUAL( appl->get_specie( 0 ).get_localization_size(), 1 );
    geometry::coordinate pos( 0.3, 0.1, 0.4 );
    geometry::centroid ctr( 1.0, 0.2, 0.0, 3.4 );
    BOOST_CHECK_EQUAL( appl->get_specie( 0 ).get_position( 0 ), pos );
    BOOST_CHECK_EQUAL( appl->get_specie( 0 ).get_localization_data( 0 ), ctr );
  }
  {
    boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
      dg.add_input_delegate( m );
    }
  
    // Valid : sub-type known : chonly
    //////////////////////////////////
    std::string canon_input( "specie\ntype chonly\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    BOOST_CHECK_EQUAL( appl->specie_count(), 1 );
    BOOST_CHECK( appl->get_specie( 0 ).is_channel_only() );
  }
  {
    boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
    core::input_delegater dg( 1 );
    {
      boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
      dg.add_input_delegate( m );
    }
  
    // Valid : sub-type known : flex
    ////////////////////////////////
    std::string canon_input( "specie\ntype flex\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
    core::input_preprocess reader;
    reader.add_buffer( "dummy", canon_input );
    BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
    BOOST_CHECK_EQUAL( appl->specie_count(), 1 );
    BOOST_CHECK( appl->get_specie( 0 ).is_flexible() );
  }

}

BOOST_AUTO_TEST_CASE( specie_lifetime_test )
{
  //Lambda to test specie validity
  auto specie_validation = []( particle::specie const& aspec )
  {
    if( not aspec.is_valid() )
    {
      BOOST_CHECK( not( aspec.is_localized() or aspec.is_flexible()
                        or aspec.is_solute() or aspec.is_channel_only()
                        or aspec.is_mobile() ) );
      BOOST_CHECK( aspec.label().empty() );
      BOOST_CHECK_EQUAL( aspec.sub_type(), particle::specie::INVALID );
    }
    else
    {
      BOOST_CHECK( aspec.is_flexible() xor aspec.is_channel_only()
                   xor aspec.is_mobile() xor aspec.is_solute() );
      BOOST_CHECK( not aspec.is_localized() or ( aspec.is_flexible() or aspec.is_mobile() ) );
      BOOST_CHECK_NE( aspec.sub_type(), particle::specie::INVALID );
      BOOST_CHECK( not aspec.label().empty() );
      BOOST_CHECK_EQUAL( aspec.label().size(), 2u );
    }
  };
  
  {
    // Test is canonical pattern
    BOOST_CHECK( std::is_default_constructible< particle::specie >::type {} );
    BOOST_CHECK( std::is_copy_constructible< particle::specie >::type {} );
    BOOST_CHECK( std::is_move_constructible< particle::specie >::type {} );
    BOOST_CHECK( ( std::is_assignable< particle::specie, particle::specie >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< particle::specie >::type {} );
  }
  // Test default ctor
  {
    particle::specie var;
    specie_validation( var );
    // Test default values
    BOOST_CHECK( not var.is_valid() );
    BOOST_CHECK_EQUAL( var.concentration(), 0.0 );
  
    BOOST_CHECK_EQUAL( var.excess_potential(), 0.0 );
    BOOST_CHECK_EQUAL( var.chemical_potential(), 0.0 );
    BOOST_CHECK_EQUAL( var.radius(), 0.0 );
    BOOST_CHECK_EQUAL( var.rate(), 1.0 );
    BOOST_CHECK_EQUAL( var.valency(), 0.0 );
    BOOST_CHECK( var.label().empty() );
    BOOST_CHECK_EQUAL( var.sub_type(), particle::specie::INVALID );
    BOOST_CHECK( not var.is_channel_only() );
    BOOST_CHECK( not var.is_flexible() );
    BOOST_CHECK( not var.is_localized() );
    BOOST_CHECK( not var.is_mobile() );
    BOOST_CHECK( not var.is_solute() );
    BOOST_CHECK( not var.is_valid() );
    BOOST_CHECK_EQUAL( var.get_position_size(), 0 );
  }
  {
    // store for serialize test.
    std::stringstream store;
    // Test values
    const double concentration( 0.1 );
    const double excess_potential( 0.1 );
    const double chem_pot( std::log( concentration/core::constants::to_SI() ) + excess_potential );
    const double radius( 0.9 );
    const double rate( 0.5 );
    const double valency( -1.0 );
    const std::string label( "Na" );
    const std::string k1( "key1" );
    const std::string v1( "value 1" );
    const std::string k2( "key 2" );
    const std::string v2( "value2" );
    const std::string k3( "key3" );
    const std::string v3( "value\n3" );
    const std::string k4( "key4" );
    const std::string v4( "value\t4" );
    const std::string k5( "key5" );
    const std::string v5( "value5" );
    const geometry::coordinate pos1( 2.0, 0.0, 0.0 );
    const geometry::coordinate pos2( 0.0, 2.0, 0.0 );
    const geometry::coordinate pos3( 0.0, 0.0, 2.0 );
    const geometry::coordinate pos4( -2.0, 0.0, 2.0 );
    const geometry::centroid ctr1( 2.4, 2.1, 0.0, 0.0 );
    const geometry::centroid ctr2( 2.4, 0.0, 2.1, 0.0 );
    const geometry::centroid ctr3( 2.4, 0.0, 0.0, 2.1 );
    auto test_specie = [&]( const particle::specie &spc )->void
    {
      BOOST_CHECK_EQUAL( spc.concentration(), concentration );
      BOOST_CHECK_EQUAL( spc.excess_potential(), excess_potential );
      BOOST_CHECK_EQUAL( spc.chemical_potential(), chem_pot );
      BOOST_CHECK_EQUAL( spc.radius(), radius );
      BOOST_CHECK_EQUAL( spc.rate(), rate );
      BOOST_CHECK_EQUAL( spc.valency(), valency );
      BOOST_CHECK_EQUAL( spc.get_position( 0 ), pos1 );
      BOOST_CHECK_EQUAL( spc.get_position( 1 ), pos2 );
      BOOST_CHECK_EQUAL( spc.get_position( 2 ), pos3 );
      BOOST_CHECK_EQUAL( spc.label(), label );
      BOOST_CHECK_EQUAL( spc.sub_type(), particle::specie::SOLUTE );
      BOOST_CHECK( not spc.is_channel_only() );
      BOOST_CHECK( not spc.is_flexible() );
      BOOST_CHECK( not spc.is_localized() );
      BOOST_CHECK( not spc.is_mobile() );
      BOOST_CHECK( spc.is_solute() );
      BOOST_CHECK( spc.is_valid() );
  
      BOOST_REQUIRE( spc.has_parameter( k1 ) );
      BOOST_CHECK_EQUAL( spc.parameter( k1 ).value(), v1 );
      BOOST_REQUIRE( spc.has_parameter( k2 ) );
      BOOST_CHECK_EQUAL( spc.parameter( k2 ).value(), v2 );
      BOOST_REQUIRE( spc.has_parameter( k3 ) );
      BOOST_CHECK_EQUAL( spc.parameter( k3 ).value(), v3 );
      BOOST_REQUIRE( spc.has_parameter( k4 ) );
      BOOST_CHECK_EQUAL( spc.parameter( k4 ).value(), v4 );
      BOOST_REQUIRE( spc.has_parameter( k5 ) );
      BOOST_CHECK_EQUAL( spc.parameter( k5 ).value(), v5 );
  
      BOOST_CHECK_EQUAL( spc.get_position( 0 ), pos1 );
      BOOST_CHECK_EQUAL( spc.get_position( 1 ), pos2 );
      BOOST_CHECK_EQUAL( spc.get_position( 2 ), pos3 );
    };
    {
      particle::specie var;
      BOOST_CHECK( not var.is_valid() );
  
      BOOST_CHECK_NO_THROW( var.set_concentration( concentration ) );
      BOOST_CHECK_NO_THROW( var.set_excess_potential( excess_potential ) );
      BOOST_CHECK_NO_THROW( var.set_radius( radius ) );
      BOOST_CHECK_NO_THROW( var.set_rate( rate ) );
      BOOST_CHECK_NO_THROW( var.set_valency( valency ) );
      BOOST_CHECK_NO_THROW( var.set_label( label ) );
      BOOST_CHECK_NO_THROW( var.set_type( particle::specie::SOLUTE ) );
      // DO these test after setting type to SOLUTE
      BOOST_CHECK_NO_THROW( var.append_position( pos1 ) );
      BOOST_CHECK_NO_THROW( var.append_position( pos2 ) );
      BOOST_CHECK_NO_THROW( var.append_position( pos3 ) );
      BOOST_CHECK_EQUAL( var.get_position( 0 ), pos1 );
      BOOST_CHECK_EQUAL( var.get_position( 1 ), pos2 );
      BOOST_CHECK_EQUAL( var.get_position( 2 ), pos3 );
      core::input_parameter_memo tmp;
      tmp.set_name( k1 ); tmp.set_value( v1 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      tmp.set_name( k2 ); tmp.set_value( v2 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      tmp.set_name( k3 ); tmp.set_value( v3 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      tmp.set_name( k4 ); tmp.set_value( v4 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      tmp.set_name( k5 ); tmp.set_value( v5 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      specie_validation( var );
      test_specie( var );
      // Test serialization
      boost::archive::text_oarchive oa( store );
      // write class instance to archive
      oa << var;
  
      // Test copy ctor
      {
        particle::specie var2( var );
        BOOST_CHECK( var == var2 );
        test_specie( var2 );
      }
  
      // Test move ctor
      {
        particle::specie var3( var );
        BOOST_CHECK( var == var3 );
        particle::specie var2( std::move( var3 ) );
        BOOST_CHECK( var == var2 );
        test_specie( var2 );
      }
  
      // Test assignment
      {
        particle::specie var2;
        BOOST_CHECK( not var2.is_valid() );
        var2 = var;
        BOOST_CHECK( var == var2 );
        test_specie( var2 );
      }
  
    }
    {
      // Test (de)serialization
      particle::specie var;
      boost::archive::text_iarchive ia( store );
      // get class instance from archive
      ia >> var;
  
      specie_validation( var );
      test_specie( var ); 
    }
  }
  
  
  

}

BOOST_AUTO_TEST_CASE( specie_static_methods_test )
{
  // Test static methods
  bool ifset { false };
  BOOST_CHECK_EQUAL( particle::specie::type_label( particle::specie::MOBILE ), core::strngs::fsmobl() );
  BOOST_CHECK_EQUAL( particle::specie::string_to_specie_type( core::strngs::fsmobl(), ifset ), particle::specie::MOBILE );
  BOOST_CHECK( ifset );
  ifset = false;
  BOOST_CHECK_EQUAL( particle::specie::type_label( particle::specie::FLEXIBLE ), core::strngs::fsflxd() );
  BOOST_CHECK_EQUAL( particle::specie::string_to_specie_type( core::strngs::fsflxd(), ifset ), particle::specie::FLEXIBLE );
  BOOST_CHECK( ifset );
  ifset = false;
  BOOST_CHECK_EQUAL( particle::specie::type_label( particle::specie::CHANNEL_ONLY ), core::strngs::fschon() );
  BOOST_CHECK_EQUAL( particle::specie::string_to_specie_type( core::strngs::fschon(), ifset ), particle::specie::CHANNEL_ONLY );
  BOOST_CHECK( ifset );
  ifset = false;
  BOOST_CHECK_EQUAL( particle::specie::type_label( particle::specie::SOLUTE ), core::strngs::fsfree() );
  BOOST_CHECK_EQUAL( particle::specie::string_to_specie_type( core::strngs::fsfree(), ifset ), particle::specie::SOLUTE );
  BOOST_CHECK( ifset );
  ifset = false;
  BOOST_CHECK_EQUAL( particle::specie::string_to_specie_type( "free", ifset ), particle::specie::SOLUTE );
  BOOST_CHECK( ifset );
  ifset = false;
  // Unrecognised string value, should return INVALID and ifset = false
  BOOST_CHECK_EQUAL( particle::specie::string_to_specie_type( core::strngs::fsspec(), ifset ), particle::specie::INVALID );
  BOOST_CHECK( not ifset );
  // Unrecognised string value, should return INVALID and ifset = false
  ifset = true;
  BOOST_CHECK_EQUAL( particle::specie::string_to_specie_type( core::strngs::fsspec(), ifset ), particle::specie::INVALID );
  BOOST_CHECK( not ifset );
  
  BOOST_CHECK_EQUAL( particle::specie::type_label( particle::specie::INVALID ), "invalid" );
  // type_label should act like const method and give a strong exception
  // guarrantee.
  {
    const std::string fncall{ "particle::specie::type_label( particle::specie::INVALID + 1 )" };
    try
    {
      particle::specie::type_label( particle::specie::INVALID + 1 );
      BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
    }
    catch( std::runtime_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Attempt to get label for invalid specie type" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
    }
  }

}

BOOST_AUTO_TEST_CASE( specie_methods_test )
{
  //Lambda to test specie validity
  auto specie_validation = []( particle::specie const& aspec )
  {
    if( not aspec.is_valid() )
    {
      BOOST_CHECK( not( aspec.is_localized() or aspec.is_flexible()
                        or aspec.is_solute() or aspec.is_channel_only()
                        or aspec.is_mobile() ) );
      BOOST_CHECK( aspec.label().empty() );
      BOOST_CHECK_EQUAL( aspec.sub_type(), particle::specie::INVALID );
    }
    else
    {
      BOOST_CHECK( aspec.is_flexible() xor aspec.is_channel_only()
                   xor aspec.is_mobile() xor aspec.is_solute() );
      BOOST_CHECK( not aspec.is_localized() or ( aspec.is_flexible() or aspec.is_mobile() ) );
      BOOST_CHECK_NE( aspec.sub_type(), particle::specie::INVALID );
      BOOST_CHECK( not aspec.label().empty() );
      BOOST_CHECK_EQUAL( aspec.label().size(), 2u );
    }
  };
  {
    // store for serialize test.
    std::stringstream store;
    // store for description test.
    std::stringstream desc;
    // Test values
    const double concentration( 0.1 );
    const double excess_potential( 0.1 );
    const double chem_pot( std::log( concentration/core::constants::to_SI() ) + excess_potential );
    const double radius( 0.9 );
    const double rate( 0.5 );
    const double valency( -1.0 );
    const std::size_t count( 3ul );
    const std::string label( "Na" );
    const std::string k1( "key1" );
    const std::string v1( "value 1" );
    const std::string k2( "key 2" );
    const std::string v2( "value2" );
    const std::string k3( "key3" );
    const std::string v3( "value\n3" );
    const std::string k4( "key4" );
    const std::string v4( "value\t4" );
    const std::string k5( "key5" );
    const std::string v5( "value5" );
    const geometry::coordinate pos1( 2.0, 0.0, 0.0 );
    const geometry::coordinate pos2( 0.0, 2.0, 0.0 );
    const geometry::coordinate pos3( 0.0, 0.0, 2.0 );
    const geometry::coordinate pos4( -2.0, 0.0, 2.0 );
    const geometry::centroid ctr1( 2.4, 2.1, 0.0, 0.0 );
    const geometry::centroid ctr2( 2.4, 0.0, 2.1, 0.0 );
    const geometry::centroid ctr3( 2.4, 0.0, 0.0, 2.1 );
    {
      particle::specie var;
      BOOST_CHECK( not var.is_valid() );
  
      // Test getters/setters
      BOOST_CHECK_EQUAL( var.concentration(), 0.0 );
      BOOST_CHECK_NO_THROW( var.set_concentration( concentration ) );
      BOOST_CHECK_EQUAL( var.concentration(), concentration );
  
      BOOST_CHECK_EQUAL( var.excess_potential(), 0.0 );
      BOOST_CHECK_NO_THROW( var.set_excess_potential( excess_potential ) );
      BOOST_CHECK_EQUAL( var.excess_potential(), excess_potential );
      BOOST_CHECK_EQUAL( var.chemical_potential(), chem_pot );
  
      BOOST_CHECK_EQUAL( var.radius(), 0.0 );
      BOOST_CHECK_NO_THROW( var.set_radius( radius ) );
      BOOST_CHECK_EQUAL( var.radius(), radius );
  
      BOOST_CHECK_EQUAL( var.rate(), 1.0 );
      BOOST_CHECK_NO_THROW( var.set_rate( rate ) );
      BOOST_CHECK_EQUAL( var.rate(), rate );
  
      BOOST_CHECK_EQUAL( var.valency(), 0.0 );
      BOOST_CHECK_NO_THROW( var.set_valency( valency ) );
      BOOST_CHECK_EQUAL( var.valency(), valency );
  
      BOOST_CHECK( var.label().empty() );
      // NOTE : set_label assume label is valid and will not throw
      // an error with an invalid label
      BOOST_CHECK_NO_THROW( var.set_label( "" ) ); // Empty string is invalid
      BOOST_CHECK_NO_THROW( var.set_label( "  " ) ); // All white space is invalid
      BOOST_CHECK_NO_THROW( var.set_label( "N" ) ); // Single character is invalid
      BOOST_CHECK_NO_THROW( var.set_label( "Nab" ) ); // More than two characters is invalid
      BOOST_CHECK_NO_THROW( var.set_label( label ) );
      BOOST_CHECK_EQUAL( var.label(), label );
  
      BOOST_CHECK_EQUAL( var.sub_type(), particle::specie::INVALID );
      BOOST_CHECK( not var.is_valid() );
  
      BOOST_CHECK_NO_THROW( var.set_type( particle::specie::MOBILE ) );
      BOOST_CHECK_EQUAL( var.sub_type(), particle::specie::MOBILE );
      BOOST_CHECK( not var.is_channel_only() );
      BOOST_CHECK( not var.is_flexible() );
      BOOST_CHECK( var.is_localized() );
      BOOST_CHECK( var.is_mobile() );
      BOOST_CHECK( not var.is_solute() );
      BOOST_CHECK( var.is_valid() );
  
      BOOST_CHECK_NO_THROW( var.set_type( particle::specie::FLEXIBLE ) );
      BOOST_CHECK_EQUAL( var.sub_type(), particle::specie::FLEXIBLE );
      BOOST_CHECK( not var.is_channel_only() );
      BOOST_CHECK( var.is_flexible() );
      BOOST_CHECK( var.is_localized() );
      BOOST_CHECK( not var.is_mobile() );
      BOOST_CHECK( not var.is_solute() );
      BOOST_CHECK( var.is_valid() );
  
      BOOST_CHECK_NO_THROW( var.set_type( particle::specie::CHANNEL_ONLY ) );
      BOOST_CHECK_EQUAL( var.sub_type(), particle::specie::CHANNEL_ONLY );
      BOOST_CHECK( var.is_channel_only() );
      BOOST_CHECK( not var.is_flexible() );
      BOOST_CHECK( not var.is_localized() );
      BOOST_CHECK( not var.is_mobile() );
      BOOST_CHECK( not var.is_solute() );
      BOOST_CHECK( var.is_valid() );
  
      BOOST_CHECK_NO_THROW( var.set_type( particle::specie::INVALID ) );
      BOOST_CHECK_EQUAL( var.sub_type(), particle::specie::INVALID );
      BOOST_CHECK( not var.is_channel_only() );
      BOOST_CHECK( not var.is_flexible() );
      BOOST_CHECK( not var.is_localized() );
      BOOST_CHECK( not var.is_mobile() );
      BOOST_CHECK( not var.is_solute() );
      BOOST_CHECK( not var.is_valid() );
  
      BOOST_CHECK_NO_THROW( var.set_type( particle::specie::SOLUTE ) );
      BOOST_CHECK_EQUAL( var.sub_type(), particle::specie::SOLUTE );
      BOOST_CHECK( not var.is_channel_only() );
      BOOST_CHECK( not var.is_flexible() );
      BOOST_CHECK( not var.is_localized() );
      BOOST_CHECK( not var.is_mobile() );
      BOOST_CHECK( var.is_solute() );
      BOOST_CHECK( var.is_valid() );
      // Test parameter handling
      {
      core::input_parameter_memo tmp;
  
      BOOST_CHECK( not var.has_parameter( k1 ) );
      tmp.set_name( k1 ); tmp.set_value( v1 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      BOOST_CHECK( var.has_parameter( k1 ) );
      BOOST_CHECK_EQUAL( var.parameter( k1 ).value(), v1 );
  
      BOOST_CHECK( not var.has_parameter( k2 ) );
      tmp.set_name( k2 ); tmp.set_value( v2 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      BOOST_CHECK( var.has_parameter( k2 ) );
      BOOST_CHECK_EQUAL( var.parameter( k2 ).value(), v2 );
  
      BOOST_CHECK( not var.has_parameter( k3 ) );
      tmp.set_name( k3 ); tmp.set_value( v3 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      BOOST_CHECK( var.has_parameter( k3 ) );
      BOOST_CHECK_EQUAL( var.parameter( k3 ).value(), v3 );
  
      BOOST_CHECK( not var.has_parameter( k4 ) );
      tmp.set_name( k4 ); tmp.set_value( v4 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      BOOST_CHECK( var.has_parameter( k4 ) );
      BOOST_CHECK_EQUAL( var.parameter( k4 ).value(), v4 );
  
      BOOST_CHECK( not var.has_parameter( k5 ) );
      tmp.set_name( k5 ); tmp.set_value( v5 );
      BOOST_CHECK_NO_THROW( var.set_parameter( tmp ) );
      BOOST_CHECK( var.has_parameter( k5 ) );
      BOOST_CHECK_EQUAL( var.parameter( k5 ).value(), v5 );
      }
      // Make copy of specie for later tests without
      // predefined particles for later tests
      //////////////////////////////////////////////
      particle::specie var4( var );
  
      // DO these test after setting type to SOLUTE
      BOOST_CHECK_NO_THROW( var.append_position( pos1 ) );
      BOOST_CHECK_NO_THROW( var.append_position( pos2 ) );
      BOOST_CHECK_NO_THROW( var.append_position( pos3 ) );
      BOOST_CHECK_EQUAL( var.get_position( 0 ), pos1 );
      BOOST_CHECK_EQUAL( var.get_position( 1 ), pos2 );
      BOOST_CHECK_EQUAL( var.get_position( 2 ), pos3 );
  
      specie_validation( var );
      var.description( desc );
  
      // overwrite existing value not allowed
      {
        particle::specie var2( var );
        BOOST_REQUIRE( var2.has_parameter( k1 ) );
        const std::string fncall { "var2.set_parameter( k1, v2 )" };
        try
        {
          core::input_parameter_memo tmp;
          tmp.set_name( k1 ); tmp.set_value( v2 );
          var2.set_parameter( tmp );
          BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
        }
        catch( std::runtime_error const& err )
        {
          const std::string msg( err.what() );
          //std::cout << msg << "\n";
          BOOST_CHECK( msg.find( "Can not change an existing parameter" ) < msg.size() );
        }
        catch( std::exception const& err )
        {
          BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
        }
        BOOST_REQUIRE( var2.has_parameter( k1 ) );
        BOOST_CHECK_EQUAL( var2.parameter( k1 ).value(), v1 );
      }
  
      // Attempts to add particles using "update_position" to MOBILE specie
      // fail
      {
        particle::specie var2( var );
        BOOST_CHECK_NO_THROW( var2.set_type( particle::specie::MOBILE ) );
        const std::string fncall { "var2.update_position( 3, pos4 )" };
        try
        {
          var2.update_position( 3, pos4 );
          BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
        }
        catch( std::runtime_error const& err )
        {
          const std::string msg( err.what() );
          //std::cout << msg << "\n";
          BOOST_CHECK( msg.find( "Only solute particles can be added or removed" ) < msg.size() );
        }
        catch( std::exception const& err )
        {
          BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
        }
  
      }
      // Attempts to add particles using "update_position" to FLEXIBLE specie
      // fail
      {
        particle::specie var2( var );
        BOOST_CHECK_NO_THROW( var2.set_type( particle::specie::FLEXIBLE ) );
        const std::string fncall { "var2.update_position( 3, pos4 )" };
        try
        {
          var2.update_position( 3, pos4 );
          BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
        }
        catch( std::runtime_error const& err )
        {
          const std::string msg( err.what() );
          //std::cout << msg << "\n";
          BOOST_CHECK( msg.find( "Only solute particles can be added or removed" ) < msg.size() );
        }
        catch( std::exception const& err )
        {
          BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
        }
  
      }
      // Attempts to add particles using "update_position" to CHANNEL_ONLY
      // specie fail
      {
        particle::specie var2( var );
        BOOST_CHECK_NO_THROW( var2.set_type( particle::specie::CHANNEL_ONLY ) );
        const std::string fncall { "var2.update_position( 3, pos4 )" };
        try
        {
          var2.update_position( 3, pos4 );
          BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
        }
        catch( std::runtime_error const& err )
        {
          const std::string msg( err.what() );
          //std::cout << msg << "\n";
          BOOST_CHECK( msg.find( "Only solute particles can be added or removed" ) < msg.size() );
        }
        catch( std::exception const& err )
        {
          BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
        }
  
      }
      // Attempts to add particles using "update_position" to SOLUTE
      // specie work
      {
        particle::specie var2( var );
        BOOST_CHECK_NO_THROW( var2.set_type( particle::specie::SOLUTE ) );
        BOOST_CHECK_NO_THROW( var2.update_position( 3, pos4 ) );
      }
      {
        // Test invalid append_position (NOT MOBILE or FLEXIBLE)
        particle::specie var2( var );
        BOOST_CHECK_NO_THROW( var2.set_type( particle::specie::SOLUTE ) );
        const std::string fncall { "var2.append_position( pos1, ctr1 )" };
        try
        {
          var2.append_position( pos1, ctr1 );
          BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
        }
        catch( std::runtime_error const& err )
        {
          const std::string msg( err.what() );
          //std::cout << msg << "\n";
          BOOST_CHECK( msg.find( "Only specie type MOBILE and FLEXIBLE require centre/localization information" ) < msg.size() );
        }
        catch( std::exception const& err )
        {
          BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
        }
  
      }
      {
        // Test invalid append_position to mobile specie without centroid
        particle::specie var2;
        BOOST_CHECK_NO_THROW( var2.set_type( particle::specie::MOBILE ) );
        const std::string fncall { "var2.append_position( pos2 )" };
        try
        {
          var2.append_position( pos2 );
          BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
        }
        catch( std::runtime_error const& err )
        {
          const std::string msg( err.what() );
          //std::cout << msg << "\n";
          BOOST_CHECK( msg.find( "Specie type MOBILE and FLEXIBLE require centre information" ) < msg.size() );
        }
        catch( std::exception const& err )
        {
          BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
        }
  
      }
      {
        // Test invalid append_position (already have locations without
        // local_data)
        particle::specie var2;
        BOOST_CHECK_NO_THROW( var2.append_position( pos2 ) );
        const std::string fncall { "var2.append_position( pos1, ctr1 )" };
        try
        {
          var2.append_position( pos1, ctr1 );
          BOOST_ERROR( "expected \"" + fncall + "\" exception not thrown" );
        }
        catch( std::runtime_error const& err )
        {
          const std::string msg( err.what() );
          //std::cout << msg << "\n";
          BOOST_CHECK( msg.find( "Error in location cache, different sizes for position and centre information" ) < msg.size() );
        }
        catch( std::exception const& err )
        {
          BOOST_ERROR( "exception thrown by \"" + fncall + "\" was not expected type: " + err.what() );
        }
      }
      {
        // Test write_document
        particle::specie var1( var4 );
        BOOST_CHECK_NO_THROW( var1.set_type( particle::specie::MOBILE ) );
        BOOST_CHECK_NO_THROW( var1.append_position( pos1, ctr1 ) );
        BOOST_CHECK_NO_THROW( var1.append_position( pos2, ctr2 ) );
        BOOST_CHECK_NO_THROW( var1.append_position( pos3, ctr3 ) );
  
        core::input_document wrtr( 1 );
        BOOST_CHECK( wrtr.empty() );
        var1.write_document( wrtr );
        BOOST_CHECK( not wrtr.empty() );
        BOOST_CHECK_EQUAL( wrtr.size(), 1 );
        auto const& isec = wrtr[ 0 ];
        // DEBUG isec.write( std::cout );
        BOOST_CHECK_EQUAL( isec.label(), core::strngs::fsspec() );
        // Check for all entries
        auto quote = []( std::string s )
        {
          return '"' + s + '"';
        };
  
        BOOST_REQUIRE( isec.has_entry( core::strngs::fstype() ) );
        BOOST_CHECK_EQUAL( isec.get_entry( core::strngs::fstype() ), particle::specie::type_label( particle::specie::MOBILE ) );
        BOOST_REQUIRE( isec.has_entry( core::strngs::fsctrg() ) );
        BOOST_CHECK_EQUAL( std::stod( isec.get_entry( core::strngs::fsctrg() ) ), concentration );
        BOOST_CHECK( isec.has_entry( core::strngs::fsname() ) );
        BOOST_CHECK_EQUAL( isec.get_entry( core::strngs::fsname() ), quote( label ) );
        BOOST_REQUIRE( isec.has_entry( core::strngs::fsd() ) );
        BOOST_CHECK_EQUAL( std::stod( isec.get_entry( core::strngs::fsd() ) )/2, radius );
        BOOST_REQUIRE( isec.has_entry( core::strngs::fsz() ) );
        BOOST_CHECK_EQUAL( std::stod( isec.get_entry( core::strngs::fsz() ) ), valency );
        BOOST_REQUIRE( isec.has_entry( core::strngs::rate_label() ) );
        BOOST_CHECK_EQUAL( std::stod( isec.get_entry( core::strngs::rate_label() ) ), rate );
        BOOST_REQUIRE( isec.has_entry( core::strngs::fschex() ) );
        BOOST_CHECK_EQUAL( std::stod( isec.get_entry( core::strngs::fschex() ) ), excess_potential );
        BOOST_REQUIRE( isec.has_entry( core::strngs::fstype() ) );
        BOOST_CHECK_EQUAL( isec.get_entry( core::strngs::fstype() ), core::strngs::fsmobl() );
        BOOST_REQUIRE( isec.has_entry( quote( k1 ) ) );
        BOOST_CHECK_EQUAL( isec.get_entry( quote( k1 ) ), quote( v1 ) );
        BOOST_REQUIRE( isec.has_entry( quote( k2 ) ) );
        BOOST_CHECK_EQUAL( isec.get_entry( quote( k2 ) ), quote( v2 ) );
        BOOST_REQUIRE( isec.has_entry( quote( k3 ) ) );
        BOOST_CHECK_EQUAL( isec.get_entry( quote( k3 ) ), quote( v3 ) );
        BOOST_REQUIRE( isec.has_entry( quote( k4 ) ) );
        BOOST_CHECK_EQUAL( isec.get_entry( quote( k4 ) ), quote( v4 ) );
        BOOST_REQUIRE( isec.has_entry( quote( k5 ) ) );
        BOOST_CHECK_EQUAL( isec.get_entry( quote( k5 ) ), quote( v5 ) );
        BOOST_REQUIRE( isec.has_entry( core::strngs::fsn() ) );
        const std::string value( isec.get_entry( core::strngs::fsn() ) );
        std::stringstream iss( value );
        std::size_t ncount;
        iss >> ncount;
        BOOST_CHECK_EQUAL( ncount, count );
        geometry::coordinate pos;
        geometry::centroid ctr;
        iss >> pos >> ctr;
        BOOST_CHECK_EQUAL( pos, pos1 );
        BOOST_CHECK_EQUAL( ctr, ctr1 );
        iss >> pos >> ctr;
        BOOST_CHECK_EQUAL( pos, pos2 );
        BOOST_CHECK_EQUAL( ctr, ctr2 );
        iss >> pos >> ctr;
        BOOST_CHECK_EQUAL( pos, pos3 );
        BOOST_CHECK_EQUAL( ctr, ctr3 );
      }
      // Parse description
      while( true )
      {
        std::string line;
        std::getline( desc, line );
        // DEBUG : std::cout << "LINE[" << line << "]\n";
        if( desc.eof() )
        {
          break;
        }
        if( line.empty() )
        {
          continue;
        }
        const std::size_t split_pos { line.find( ':' ) };
        if( std::string::npos != split_pos )
        {
          std::size_t beg( 0 ), end( 0 );
          for( end = split_pos - 1; std::isspace( line[end] ) and end != 0; --end )
          {}
          if( not std::isspace( line[end] ) ) ++end;
          BOOST_REQUIRE( end != 0 );
  
          for( beg = 0; std::isspace( line[beg] ) and beg != end; ++beg )
          {};
          BOOST_REQUIRE_NE( beg, end );
  
          const std::string name( line.substr( beg, end - beg ) );
  
          end = line.size();
          for( end = line.size() - 1; std::isspace( line[end] ) and end != split_pos + 1; --end )
          {}
          if( not std::isspace( line[end] ) ) ++end;
          for( beg = split_pos + 1; std::isspace( line[beg] ) and beg != end; ++beg )
          {};
          if( beg != split_pos + 1 ) --beg;
          BOOST_REQUIRE_LE( beg, end );
  
          const std::string value( line.substr( beg, end - beg ) );
          // DEBUG std::cout << "CHECK [" << name << "][" << value << "]\n";
          if( name == "label" )
          {
            BOOST_CHECK_EQUAL( value, label );
          }
          else if( name == "radius" )
          {
            BOOST_CHECK( utility::feq( std::stod( value ), radius ) );
          }
          else if( name == "rate" )
          {
            BOOST_CHECK( utility::feq( std::stod( value ), rate ) );
          }
          else if( name == "valency" )
          {
            BOOST_CHECK( utility::feq( std::stod( value ), valency ) );
          }
          else if( name == "excess c.p." )
          {
            BOOST_CHECK( utility::feq( std::stod( value ), excess_potential ) );
          }
          else if( name == "chem. pot." )
          {
            const double cp
            {
              std::stod( value )
            };
            BOOST_CHECK_LT( cp, chem_pot + 0.000005 );
            BOOST_CHECK_GT( cp, chem_pot - 0.000005 );
          }
          else if( name == "target conc." )
          {
            BOOST_CHECK( utility::feq( std::stod( value ), concentration ) );
          }
          else if( name == "type" )
          {
            BOOST_CHECK_EQUAL( value, particle::specie::type_label( particle::specie::SOLUTE ) );
          }
          else if( name == "radius" )
          {
            BOOST_CHECK( utility::feq( std::stod( value ), radius ) );
          }
          else if( name == k1 )
          {
            BOOST_CHECK_EQUAL( v1, value );
          }
          else if( name == k2 )
          {
            BOOST_CHECK_EQUAL( v2, value );
          }
          else if( name == k3 )
          {
            std::string value2;
            std::getline( desc, value2 );
            value2.insert( 0, "\n" );
            value2.insert( 0, value );
            BOOST_CHECK_EQUAL( v3, value2 );
          }
          else if( name == k4 )
          {
            BOOST_CHECK_EQUAL( v4, value );
          }
          else if( name == k5 )
          {
            BOOST_CHECK_EQUAL( v5, value );
          }
          else
          {
            std::cout << "Unchecked [" << name << "][" << value << "]\n";
          }
        }
      }
    }
  }
  
  

}

// Unknown subtype value
BOOST_AUTO_TEST_CASE( specie_input_bad_subtype_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "specie\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\ntype freeze\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie subtype parameter \"type\" with value (freeze) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 11.\n   >>type freeze<<\n** A value from the list (mob,flex,chonly,free) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

// Missing subtype value
BOOST_AUTO_TEST_CASE( specie_input_missing_subtype_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  // Invalid : missing value for sub-type 
  ///////////////////////////////////////
  std::string canon_input( "specie\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\ntype\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie subtype parameter \"type\" in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 11.\n   >>type<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_section_has_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : value after section keyword
  ////////////////////////////////////////
  std::string canon_input( "specie freeze\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Section start entry parameter \"specie\" with value (freeze) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 1.\n   >>specie freeze<<\n** Remove or comment text after section start keyword \"specie\". **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_PASS_bad_parameter_value_test_ )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : negative "ratreg" rate value which is an extended input value so it
  // is not checked.
  ////////////////////////////////////////////////////////////////////////////////
  std::string canon_input( "specie\ntype \"free\"  \nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 -0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  BOOST_CHECK_EQUAL( appl->specie_count(), 1 );
  BOOST_CHECK( appl->get_specie( 0 ).has_parameter( "ratreg" ) );
  BOOST_CHECK_EQUAL( appl->get_specie( 0 ).parameter( "ratreg" ).value(), "0.3 -0.3 0.3 0.1" );
  
  

}

BOOST_AUTO_TEST_CASE( specie_input_missing_name_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : missing specie name value
  //////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie label parameter \"name\" in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>name<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_bad_name_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : One letter specie name.
  ////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"C\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie label parameter \"name\" with value (\"C\") in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>name \"C\"<<\n** The specie label must have exactly two letters. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_negative_rate_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : negative "ratspc" value
  ////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc -1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie relative trial rate parameter \"ratspc\" with value (-1.0) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>ratspc -1.0<<\n** Input value must be greater than or equal to zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_bad_rate_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : non-numerical "ratspc" value
  /////////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc -FF.F\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie relative trial rate parameter \"ratspc\" with value (-FF.F) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>ratspc -FF.F<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_missing_rate_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : missing "ratspc" value
  ///////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie relative trial rate parameter \"ratspc\" in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>ratspc<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_malformed_rate_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : number with trailing characters for "ratspc" value
  ///////////////////////////////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 0.5G\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie relative trial rate parameter \"ratspc\" with value (0.5G) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 4.\n   >>ratspc 0.5G<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_malformed_valency_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : half numeric valency.
  //////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0C\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie valency parameter \"z\" with value (2.0C) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>z 2.0C<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_bad_valency_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : non-numerical valency.
  ///////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz two\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie valency parameter \"z\" with value (two) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>z two<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_missing_valency_value )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : missing valency value
  //////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie valency parameter \"z\" in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>z<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_negative_diameter_value )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : negative diameter.
  ///////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd -1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie diameter parameter \"d\" with value (-1.1) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 9.\n   >>d -1.1<<\n** Input value must be greater than or equal to zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_bad_diameter_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : non-numerical diameter.
  ////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd three\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie diameter parameter \"d\" with value (three) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 9.\n   >>d three<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_missing_diameter_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : missing diameter value
  ///////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie diameter parameter \"d\" in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 9.\n   >>d<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_bad_conc_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : "conc" has non-numeric parameter value.
  ////////////////////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nctarg twelve\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie target concentration parameter \"ctarg\" with value (twelve) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 10.\n   >>ctarg twelve<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_missing_conc_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : "conc" missing value.
  //////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nctarg\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie target concentration parameter \"ctarg\" in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 10.\n   >>ctarg<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_bad_chex_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : "chex" has non-numeric parameter value.
  ////////////////////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex twelve\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie excess chemical potential parameter \"chex\" with value (twelve) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 10.\n   >>chex twelve<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_missing_chex_value_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : "chex" has no value.
  /////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Specie excess chemical potential parameter \"chex\" in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 10.\n   >>chex<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_bad_localization_data_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Localization data has non-numeric parameter value.
  ///////////////////////////////////////////////////////////////
  std::string canon_input( "specie\ntype flex\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 1\n0.3 0.1 0.4 0.1 0.2 f.0 3.4\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Expecting a position definition parameter \"n\" with value (0.3 0.1 0.4 0.1 0.2 f.0 3.4) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n   >>0.3 0.1 0.4 0.1 0.2 f.0 3.4<<\n** Element f.0: A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_bad_position_data_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Position data has non-numeric parameter value.
  ///////////////////////////////////////////////////////////
  std::string canon_input( "specie\ntype flex\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 1\n0.3 D.1 0.4 0.1 0.2 0.0 3.4\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Expecting a position definition parameter \"n\" with value (0.3 D.1 0.4 0.1 0.2 0.0 3.4) in section \"specie\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n   >>0.3 D.1 0.4 0.1 0.2 0.0 3.4<<\n** Element E.1: A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_subtype_position_mismatch_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Localization diameter is negative.
  ///////////////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 1\n0.3 0.1 0.4 -0.1 0.2 0.0 3.4\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Position definition parameter \"n\" with value (0.3 0.1 0.4 -0.1 0.2 0.0 3.4) in section \"specie\" ending at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n** Specie type does not match position definition, only expecting x, y and z coordinates. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_negative_localization_radius_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Localization diameter is negative.
  ///////////////////////////////////////////////
  std::string canon_input( "specie\ntype flex\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 1\n0.3 0.1 0.4 -0.1 0.2 0.0 3.4\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Position definition parameter \"n\" with value (0.3 0.1 0.4 -0.1 0.2 0.0 3.4) in section \"specie\" ending at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n** The fourth value (localization radius: -0.100000 must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_zero_localization_radius_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Localization diameter is zero.
  ///////////////////////////////////////////
  std::string canon_input( "specie\ntype flex\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 1\n0.3 0.1 0.4 0.0 0.2 0.0 3.4\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Position definition parameter \"n\" with value (0.3 0.1 0.4 0.0 0.2 0.0 3.4) in section \"specie\" ending at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n** The fourth value (localization radius: 0.000000 must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_short_position_data_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Not enough location values
  ///////////////////////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 1\n0.3 0.1\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Position definition parameter \"n\" with value (0.3 0.1) in section \"specie\" ending at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n** A position definition should have 3, 4 or 7 values. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_short_localization_data_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Not enough localization values
  ///////////////////////////////////////////
  std::string canon_input( "specie\ntype flex\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 1\n0.3 0.1 -0.1 0.2 0.0 3.4\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Position definition parameter \"n\" with value (0.3 0.1 -0.1 0.2 0.0 3.4) in section \"specie\" ending at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n** A position definition should have 3, 4 or 7 values. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_short_position_lines_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Not enough entries
  ///////////////////////////////
  std::string canon_input( "specie\ntype free\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 2\n0.3 0.1 0.4\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Position definition parameter \"n\" with value (end) in section \"specie\" ending at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 13.\n** First element was not a number or not as many position definitions (1) as indicated (2). **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_mixed_type_localization_data_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  
  // Invalid : Not enough localization values
  ///////////////////////////////////////////
  std::string canon_input( "specie\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nz 2.0\nd 1.1\nchex 0.792\nn 2\n0.3 0.1 -0.1 1.0 0.2 0.0 3.4\n1.0 1.3 2.4\ntype flex\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Position definition parameter \"n\" with value (1.0 1.3 2.4) in section \"specie\" ending at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 12.\n** Specie type does not match position definition, expecting more than just x, y and z coordinates. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_missing_type_keyword_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "specie\nname \"Ca\"\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Particle type definition section \"specie\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 11.\n** Add the following parameters to the section: type **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_missing_name_keyword_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "specie\nratspc 1.0\nratmov 1.0\nratexc 2.0\nratgr 3.0\nratreg 0.3 0.3 0.3 0.1\nz 2.0\nd 1.1\nchex 0.792\ntype free\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Particle type definition section \"specie\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 11.\n** Add the following parameters to the section: name **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( specie_input_missing_valency_keyword_test )
{
  boost::shared_ptr< particle::particle_manager > appl( new particle::particle_manager );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< core::input_base_meta > m( new particle::specie_meta( appl ) );
    dg.add_input_delegate( m );
  }
  // Invalid : unknown sub-type
  /////////////////////////////
  std::string canon_input( "specie\nname \"Ca\"\nd 1.1\nchex 0.792\ntype free\nend\n\n" );
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
    BOOST_CHECK( msg.find( "Particle type definition section \"specie\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 6.\n** Add the following parameters to the section: z **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

