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

#define BOOST_TEST_MODULE evaluator_test
#include <boost/test/unit_test.hpp>

#include "evaluator/evaluator_test/evaluator_test.hpp"
#include "particle/change_set.hpp"
#include "evaluator/coulomb.hpp"
#include "evaluator/evaluator_meta.hpp"
#include "evaluator/evaluator_manager.hpp"
#include "evaluator/hard_sphere_overlap.hpp"
#include "evaluator/icc_fortran.hpp"
#include "evaluator/icc_matrix.hpp"
#include "evaluator/icc_surface_grid.hpp"
#include "evaluator/induced_charge.hpp"
#include "evaluator/integrator.hpp"
#include "evaluator/lennard_jones.hpp"
#include "evaluator/localizer.hpp"
#include "evaluator/object_overlap.hpp"
#include "evaluator/evaluator_test/evaluator_fixture.hpp"
#include "core/input_parameter_memo.hpp"

// Manuals
#include "core/input_delegater.hpp"
#include "core/input_document.hpp"
#include "core/input_error.hpp"
#include "core/input_help.hpp"
#include "core/input_preprocess.hpp"
#include "core/strngs.hpp"
#include "geometry/geometry_manager.hpp"
#include "geometry/membrane_cylinder_region.hpp"
#include "geometry/periodic_cube_region.hpp"
#include "particle/ensemble.hpp"
#include "particle/particle_manager.hpp"
#include "particle/specie.hpp"
#include "particle/specie_meta.hpp"
// - 
//#include <XXarray>
#include <bitset>
#include <fstream>
#include <type_traits>
//-
namespace evaluator {

std::vector< core::input_parameter_memo > evaluator_test::mockup_params()
{
  std::vector< core::input_parameter_memo > params;
  core::input_parameter_memo tmp;
  tmp.set_name( "end" );
  params.push_back( std::move( tmp ) );
  return std::move( params );
}

BOOST_AUTO_TEST_CASE( evaluator_manager_lifetime )
{
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::evaluator_manager >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::evaluator_manager >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::evaluator_manager >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::evaluator_manager, evaluator::evaluator_manager >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< evaluator::evaluator_manager >::type {} );
  }
  BOOST_CHECK_EQUAL( 80.0, evaluator::evaluator_manager::standard_aqueous_permittivity() );
  BOOST_CHECK_EQUAL( 298.15, evaluator::evaluator_manager::standard_room_temperature() );
  {
    evaluator::evaluator_manager mgr;
    BOOST_CHECK_EQUAL( mgr.size(), 0 );
    BOOST_CHECK_EQUAL( mgr.permittivity(), evaluator::evaluator_manager::standard_aqueous_permittivity() );
    BOOST_CHECK_EQUAL( mgr.temperature(), evaluator::evaluator_manager::standard_room_temperature() );
    BOOST_CHECK( mgr.empty() );
  }

}

//Directly tested
// serialize
// description
// empty
// get_evaluators
// permittivity
// size
// temperature
// write_document
// compute_potential
// build_input_delegater
//
//Tested in trial simulation
// on_conclude_trial
// prepare

BOOST_AUTO_TEST_CASE( evaluator_manager_methods_test )
{
  // methods to test
  //  * serialize
  //  * description
  //  * empty
  //  * get_evaluators
  //  on_conclude_trial
  //  * permittivity
  //  prepare
  //  * size
  //  * temperature
  //  * write_document
  //  compute_potential
  //  * build_input_delegater
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  // Constructor test method
  const double tmpt { 300.5 };
  const double perm { 42.5 };
  std::stringstream store;
  {
    // Before adding any evaluators changes
    BOOST_CHECK_EQUAL( mngr->size(), 0ul );
    BOOST_CHECK( mngr->empty() );
    BOOST_CHECK( mngr->get_evaluators().empty() );
    BOOST_CHECK_EQUAL( mngr->get_evaluators().size(), 0ul );
  
    // default permittivity and temperature
    BOOST_CHECK_EQUAL( mngr->permittivity(), evaluator::evaluator_manager::standard_aqueous_permittivity() );
    BOOST_CHECK_EQUAL( mngr->temperature(), evaluator::evaluator_manager::standard_room_temperature() );
    mngr->permittivity( perm );
    BOOST_CHECK_EQUAL( mngr->permittivity(), perm );
    mngr->temperature( tmpt );
    BOOST_CHECK_EQUAL( mngr->temperature(), tmpt );
  
    core::input_delegater dg( 1 );
    evaluator::evaluator_manager::build_input_delegater( mngr, dg );
    BOOST_REQUIRE( dg.has_section( "evaluator" ) );
    {
      core::input_help hlpr;
      dg.get_documentation( hlpr );
      {
        std::stringstream ss;
        hlpr.write( ss );
        const std::string hstr( ss.str() );
        BOOST_CHECK( hstr.find( "coulomb" ) < hstr.size() );
        BOOST_CHECK( hstr.find( "lennard-jones" ) < hstr.size() );
        BOOST_CHECK( hstr.find( "localize" ) < hstr.size() );
        //std::cout << hstr;
      }
    }
    {
      std::string canon_input( "evaluator\ntype \"coulomb\"\nend\n\n" );
  
      core::input_preprocess reader;
      reader.add_buffer( "dummy", canon_input );
      BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
      BOOST_REQUIRE_EQUAL( mngr->size(), 1ul );
      BOOST_CHECK_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "coulomb" );
    }
    {
      std::stringstream ss;
      mngr->description( ss );
      const std::string dstr( ss.str() );
      BOOST_CHECK( dstr.find( "coulomb" ) < dstr.size() );
      BOOST_CHECK( dstr.find( "300.5" ) < dstr.size() );
      BOOST_CHECK( dstr.find( "42.5" ) < dstr.size() );
      //std::cout << dstr;
    }
    boost::archive::text_oarchive oa( store );
    oa << mngr;
  }
  // Serialization test method.
  {
    boost::shared_ptr< evaluator::evaluator_manager > mngr;
  
    boost::archive::text_iarchive ia( store );
    ia >> mngr;
  
    BOOST_CHECK_EQUAL( mngr->size(), 1 );
    BOOST_CHECK( not mngr->empty() );
    BOOST_CHECK_EQUAL( mngr->permittivity(), perm );
    BOOST_CHECK_EQUAL( mngr->temperature(), tmpt );
    BOOST_CHECK_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "coulomb" );
  
    std::string canon( "evaluator\ntype coulomb\nend\n" );
  
    core::input_document wrtr( 1ul );
    mngr->write_document( wrtr );
    std::stringstream idoc;
    wrtr.write( idoc );
    const std::string sdoc( idoc.str() );
    BOOST_CHECK_LE( sdoc.find( canon ), sdoc.size() );
  }

}

BOOST_AUTO_TEST_CASE( evaluator_definition_lifetime )
{
  {
    //  Constructor tests
    BOOST_CHECK( not std::is_default_constructible< evaluator::evaluator_definition >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::evaluator_definition >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::evaluator_definition >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::evaluator_definition, evaluator::evaluator_definition >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::evaluator_definition >::type {} );
  }
  const std::string defn_label( "evaluator" );
  const std::string defn_desc( "blah blech blab." );
  {
    evaluator::evaluator_definition defn( defn_label, defn_desc, [](const std::vector< core::input_parameter_memo >&)->std::unique_ptr< evaluator::base_evaluator>{ return std::unique_ptr< evaluator::base_evaluator>{}; });
    BOOST_CHECK_EQUAL( defn.label(), defn_label );
    BOOST_CHECK_EQUAL( defn.description(), defn_desc );
    BOOST_CHECK_EQUAL( defn.size(), 0 );
    BOOST_CHECK( defn.empty() );
  }
  

}

BOOST_AUTO_TEST_CASE( evaluator_meta_lifetime_test )
{
  {
    //  Constructor tests
    BOOST_CHECK( not std::is_default_constructible< evaluator::evaluator_meta >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::evaluator_meta >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::evaluator_meta >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::evaluator_meta, evaluator::evaluator_meta >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::evaluator_meta >::type {} );
  }
  const std::string meta_label( "evaluator" );
  {
    boost::shared_ptr< evaluator::evaluator_manager > eman( new evaluator::evaluator_manager{} );
    evaluator::evaluator_meta meta( eman );
    BOOST_CHECK_EQUAL( meta.required(), true );
    BOOST_CHECK_EQUAL( meta.multiple(), true );
    BOOST_CHECK_EQUAL( meta.section_label(), core::strngs::evaluator_label() );
    BOOST_CHECK_EQUAL( meta.size(), 0 );
    BOOST_CHECK( meta.empty() );
  
    meta.add_definition( std::unique_ptr< evaluator::evaluator_definition >( new evaluator::evaluator_definition( "coloumb", "blah blab blech.", [](const std::vector< core::input_parameter_memo >&)->std::unique_ptr< evaluator::base_evaluator>{ return std::unique_ptr< evaluator::base_evaluator>{}; }) ) );
    
    BOOST_CHECK_EQUAL( meta.size(), 1 );
    BOOST_CHECK( not meta.empty() );
    BOOST_CHECK( meta.has_definition( "coloumb" ) );
  
  }
  

}

BOOST_AUTO_TEST_CASE( input_unknown_parameter_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::coulomb::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype coulomb\nmobk 4.5\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Unknown parameter \"mobk\" in section \"evaluator\" subtype \"coulomb\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n** Parameter \"mobk\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_no_type_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::coulomb::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Energy function section \"evaluator\" is missing some required parameters at input file \"" + boost::filesystem::current_path().native() + "/dummy\" in section ending at line 2.\n** Add the following parameters to the section: type **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_type_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::coulomb::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype #localizer\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Evaluator selection parameter \"type\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>type #localizer<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_type_bad_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::coulomb::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype localizer\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Evaluator selection parameter \"type\" with value (localizer) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 2.\n   >>type localizer<<\n** A value from the list (coulomb) was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( coulomb_lifetime_tests )
{
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::coulomb >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::coulomb >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::coulomb >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::coulomb, evaluator::coulomb >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::coulomb >::type {} );
  }
  const std::string defn_label( "coulomb" );
  {
    evaluator::coulomb eval;
    BOOST_CHECK_EQUAL( eval.type_label(), defn_label );
    BOOST_CHECK_EQUAL( eval.factor(), 0.0 );
  }
  

}

BOOST_AUTO_TEST_CASE( coulomb_static_method_tests )
{
  {
    // Static method tests.
    BOOST_CHECK_CLOSE( evaluator::coulomb::invariant_factor(), 167109.7903783501, 0.0000001 );
  }
  const std::string defn_label( "coulomb" );
  {
    BOOST_CHECK_EQUAL( evaluator::coulomb::type_label_(), defn_label );
  
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
  
    BOOST_REQUIRE_NO_THROW( eval_ptr = evaluator::coulomb::make_evaluator( evaluator_test::mockup_params() ) );
    BOOST_REQUIRE( eval_ptr );
    evaluator::coulomb *clmb_ptr = dynamic_cast< evaluator::coulomb* >( eval_ptr.get() );
    BOOST_REQUIRE( clmb_ptr != nullptr );
    BOOST_CHECK_EQUAL( eval_ptr->type_label(), defn_label );
    BOOST_CHECK_EQUAL( clmb_ptr->factor(), 0.0 );
  
    boost::shared_ptr< evaluator::evaluator_manager > eman;
    evaluator::evaluator_meta m( eman );
    BOOST_REQUIRE_NO_THROW( evaluator::coulomb::add_definition( m ) );
    BOOST_CHECK( m.has_definition( defn_label ) );
  }
  

}

// Calculate total potential with two particles of +/-1 valency 1 Angstrom
// apart. 
BOOST_AUTO_TEST_CASE( coulomb_evaluator_r1_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  evaluator::coulomb eval;
  BOOST_CHECK_EQUAL( eval.factor(), 0.0 );
  
  eval.prepare( pman, gman, eman );
  BOOST_CHECK_CLOSE( eval.factor(), evaluator::coulomb::invariant_factor()/( eman.permittivity() * eman.temperature() ), 0.000001 );
  
  double energy = 0.0;
  BOOST_REQUIRE_NO_THROW( energy = eval.compute_total_potential( pman, gman ) );
  BOOST_CHECK_CLOSE( -energy, eval.factor(), 0.000001 );

}

// Calculate total potential with two particles of +/-1 valency 2 Angstrom
// apart. 
BOOST_AUTO_TEST_CASE( coulomb_evaluator_r2_test )
{
  // Build basic system.
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
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  evaluator::coulomb eval;
  BOOST_CHECK_EQUAL( eval.factor(), 0.0 );
  
  eval.prepare( pman, gman, eman );
  BOOST_CHECK_CLOSE( eval.factor(), evaluator::coulomb::invariant_factor()/( eman.permittivity() * eman.temperature() ), 0.000001 );
  
  double energy = 0.0;
  BOOST_REQUIRE_NO_THROW( energy = eval.compute_total_potential( pman, gman ) );
  BOOST_CHECK_CLOSE( -energy, eval.factor()/2.0, 0.000001 );
  
  // calculate move from current position to position in r1 test
  // (1 angstrom distance)
  
  // find Chloride particle
  const std::size_t lispec = pman.get_specie_key( "Cl" );
  const std::size_t lidx = pman.get_ensemble().nth_specie_index( lispec, 0 );
  {
    // Move particle 1 angstrom distance
    const geometry::coordinate pstart( pman.get_ensemble().position( lidx ) );
    const geometry::coordinate pend( 0.0, 1.0, 0.0 );
    particle::change_atom atm( lispec );
    atm.index = lidx;
    atm.do_old = true;
    atm.old_position = pstart;
    gman.calculate_distances( atm.old_position, pman.get_ensemble().get_coordinates(), atm.old_rij, 0, pman.get_ensemble().size() );
    atm.do_old = true;
    atm.new_position = pend;
    gman.calculate_distances( atm.new_position, pman.get_ensemble().get_coordinates(), atm.new_rij, 0, pman.get_ensemble().size() );
  
    particle::change_set move;
    move.add_atom( atm );
  
    const double distance( std::sqrt( gman.calculate_distance_squared( pstart, pend ) ) );
  
    BOOST_CHECK_CLOSE_FRACTION( distance, 1.0, 0.0000000001 );
    // r1 - r2 total energies
    const double delta_energy( -eval.factor()/2.0 );
  
    // Calculate energy
    eval.compute_potential( pman, gman, move );
    BOOST_REQUIRE( not move.fail() );
    BOOST_CHECK_CLOSE_FRACTION( move.energy(), delta_energy, 0.0000000001 );
  }
  
  
  

}

// Calculate total potential with four particles of +/-1 valency
// arranged in a 1 Angstrom square. 
BOOST_AUTO_TEST_CASE( coulomb_evaluator_d4r1_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 4 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  evaluator::coulomb eval;
  BOOST_CHECK_EQUAL( eval.factor(), 0.0 );
  
  eval.prepare( pman, gman, eman );
  BOOST_CHECK_CLOSE( eval.factor(), evaluator::coulomb::invariant_factor()/( eman.permittivity() * eman.temperature() ), 0.000001 );
  
  double energy = 0.0;
  BOOST_REQUIRE_NO_THROW( energy = eval.compute_total_potential( pman, gman ) );
  BOOST_CHECK_CLOSE( -energy, (4.0 - std::sqrt(2.0)) * eval.factor(), 0.000001 );

}

// Calculate total potential with eight particles of +/-1 valency
// arranged in a  1 Angstrom cube. 
//
//  ** includes serialization test **
BOOST_AUTO_TEST_CASE( coulomb_evaluator_d8r1_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  std::stringstream store;
  double expected_value = 0.0;
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr( new evaluator::coulomb{} );
    evaluator::coulomb* llp = dynamic_cast< evaluator::coulomb* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->factor(), 0.0 );
  
    eval_ptr->prepare( pman, gman, eman );
    BOOST_CHECK_CLOSE( llp->factor(), evaluator::coulomb::invariant_factor()/( eman.permittivity() * eman.temperature() ), 0.000001 );
  
    expected_value = -( 12.0 - ( 6 * std::sqrt( 2.0 ) ) + ( 4 / std::sqrt( 3 ) ) ) * llp->factor();
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  
    boost::archive::text_oarchive oa( store );
    oa << eval_ptr;
  }
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    boost::archive::text_iarchive ia( store );
    ia >> eval_ptr;
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  }
  

}

BOOST_AUTO_TEST_CASE( evaluator_meta_coulomb_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::coulomb::add_definition( *m );
  
    core::input_help hlpr;
    m->publish_help( hlpr );
    {
      std::stringstream ss;
      hlpr.write( ss );
      BOOST_CHECK( ss.str().find( "coulomb" ) < ss.str().size() );
    }
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"coulomb\"\nend\n\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE_EQUAL( mngr->size(), 1ul );
  BOOST_CHECK_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "coulomb" );
  
  std::string canon( "evaluator\ntype coulomb\nend\n" );
  
  core::input_document wrtr( 1ul );
  mngr->write_document( wrtr );
  std::stringstream idoc;
  wrtr.write( idoc );
  const std::string sdoc( idoc.str() );
  BOOST_CHECK_LE( sdoc.find( canon ), sdoc.size() );

}

BOOST_AUTO_TEST_CASE( hard_sphere_overlap_lifetime_tests )
{
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::hard_sphere_overlap >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::hard_sphere_overlap >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::hard_sphere_overlap >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::hard_sphere_overlap, evaluator::hard_sphere_overlap >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::hard_sphere_overlap >::type {} );
  }
  const std::string defn_label( "hard-sphere" );
  {
    evaluator::hard_sphere_overlap eval;
    BOOST_CHECK_EQUAL( eval.type_label(), defn_label );
  }

}

BOOST_AUTO_TEST_CASE( hard_sphere_overlap_static_method_tests )
{
  const std::string defn_label( "hard-sphere" );
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    BOOST_REQUIRE_NO_THROW( eval_ptr = evaluator::hard_sphere_overlap::make_evaluator( evaluator_test::mockup_params() ) );
    BOOST_REQUIRE( eval_ptr );
    evaluator::hard_sphere_overlap *clmb_ptr = dynamic_cast< evaluator::hard_sphere_overlap* >( eval_ptr.get() );
    BOOST_REQUIRE( clmb_ptr != nullptr );
    BOOST_CHECK_EQUAL( eval_ptr->type_label(), defn_label );
  
    boost::shared_ptr< evaluator::evaluator_manager > eman;
    evaluator::evaluator_meta m( eman );
    BOOST_REQUIRE_NO_THROW( evaluator::hard_sphere_overlap::add_definition( m ) );
    BOOST_REQUIRE( m.has_definition( defn_label ) );
  }
  

}

// Check overlap with single particle add/move on ensemble with eight particles 
// of +/-1 valency arranged in a  1 Angstrom cube.
//
//  ** includes serialization test **
BOOST_AUTO_TEST_CASE( hard_sphere_overlap_evaluator_d8r1_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  std::stringstream store;
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr( evaluator::hard_sphere_overlap::make_evaluator( evaluator_test::mockup_params() ) );
    evaluator::hard_sphere_overlap* llp = dynamic_cast< evaluator::hard_sphere_overlap* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
  
    eval_ptr->prepare( pman, gman, eman );
    {
      double energy = 0.0;
      BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
      BOOST_CHECK_CLOSE( energy, 0.0, 0.000001 );
    }
    {
      // attempt add with overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = pman.get_ensemble().size();
      atom.do_old = false;
      atom.do_new = true;
      atom.new_position = { 0.05, 0.05, 0.05 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( trial.fail() );
    }
    {
      // attempt add without overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = pman.get_ensemble().size();
      atom.do_old = false;
      atom.do_new = true;
      atom.new_position = { 0.5, 0.5, 0.5 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move without overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 0.5, 0.5, 0.5 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move with overlap to our old position
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 0.05, 0.05, 0.05 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move with overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 1.05, 1.05, 1.05 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( trial.fail() );
    }
  
    boost::archive::text_oarchive oa( store );
    oa << eval_ptr;
  }
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    boost::archive::text_iarchive ia( store );
    ia >> eval_ptr;
    {
      // attempt add with overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = pman.get_ensemble().size();
      atom.do_old = false;
      atom.do_new = true;
      atom.new_position = { 0.05, 0.05, 0.05 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( trial.fail() );
    }
    {
      // attempt add without overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = pman.get_ensemble().size();
      atom.do_old = false;
      atom.do_new = true;
      atom.new_position = { 0.5, 0.5, 0.5 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move without overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 0.5, 0.5, 0.5 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move with overlap to our old position
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 0.05, 0.05, 0.05 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move with overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 1.05, 1.05, 1.05 };
      gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( trial.fail() );
    }
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, 0.0, 0.000001 );
  }
  
  

}

// Check overlap with multiple particle add/move on ensemble with eight particles 
// of +/-1 valency arranged in a  1 Angstrom cube.
BOOST_AUTO_TEST_CASE( hard_sphere_overlap_evaluator_multi_d8r1_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr( evaluator::hard_sphere_overlap::make_evaluator( evaluator_test::mockup_params() ) );
    evaluator::hard_sphere_overlap* llp = dynamic_cast< evaluator::hard_sphere_overlap* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
  
    eval_ptr->prepare( pman, gman, eman );
    {
      double energy = 0.0;
      BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
      BOOST_CHECK_CLOSE( energy, 0.0, 0.000001 );
    }
    {
      // attempt add with overlap (first particle)
      particle::change_set trial;
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = pman.get_ensemble().size();
        atom.do_old = false;
        atom.do_new = true;
        atom.new_position = { 0.05, 0.05, 0.05 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      {
        particle::change_atom atom;
        atom.key = 1;
        atom.index = pman.get_ensemble().size() + 1;
        atom.do_old = false;
        atom.do_new = true;
        atom.new_position = { 0.5, 0.5, 0.5 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
       // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( trial.fail() );
    }
    {
      // attempt add without overlap
      particle::change_set trial;
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = pman.get_ensemble().size();
        atom.do_old = false;
        atom.do_new = true;
        atom.new_position = { 1.5, 1.5, 1.5 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      {
        particle::change_atom atom;
        atom.key = 1;
        atom.index = pman.get_ensemble().size() + 1;
        atom.do_old = false;
        atom.do_new = true;
        atom.new_position = { 0.5, 0.5, 0.5 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt add with internal overlap
      particle::change_set trial;
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = pman.get_ensemble().size();
        atom.do_old = false;
        atom.do_new = true;
        atom.new_position = { 0.5, 0.5, 0.55 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      {
        particle::change_atom atom;
        atom.key = 1;
        atom.index = pman.get_ensemble().size() + 1;
        atom.do_old = false;
        atom.do_new = true;
        atom.new_position = { 0.5, 0.5, 0.5 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( trial.fail() );
    }
    {
      // attempt move without overlap
      particle::change_set trial;
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = 0;
        atom.do_old = true;
        atom.new_position = pman.get_ensemble().position( atom.index );
        atom.do_new = true;
        atom.new_position = { 0.5, 0.5, 0.5 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = 1;
        atom.do_old = true;
        atom.new_position = pman.get_ensemble().position( atom.index );
        atom.do_new = true;
        atom.new_position = { 1.5, 1.5, 1.5 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move with overlap to our old positions
      particle::change_set trial;
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = 0;
        atom.do_old = true;
        atom.new_position = pman.get_ensemble().position( atom.index );
        atom.do_new = true;
        atom.new_position = { 1.05, 1.05, 0.05 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = 1;
        atom.do_old = true;
        atom.new_position = pman.get_ensemble().position( atom.index );
        atom.do_new = true;
        atom.new_position = { 0.05, 0.05, 0.05 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move with overlap (second particle)
      particle::change_set trial;
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = 0;
        atom.do_old = true;
        atom.new_position = pman.get_ensemble().position( atom.index );
        atom.do_new = true;
        atom.new_position = { 1.05, 1.05, 0.05 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = 1;
        atom.do_old = true;
        atom.new_position = pman.get_ensemble().position( atom.index );
        atom.do_new = true;
        atom.new_position = { 1.05, 1.05, 1.05 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( trial.fail() );
    }
    {
      // attempt move with internal overlap
      particle::change_set trial;
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = 0;
        atom.do_old = true;
        atom.new_position = pman.get_ensemble().position( atom.index );
        atom.do_new = true;
        atom.new_position = { 1.05, 1.0, 0.05 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      {
        particle::change_atom atom;
        atom.key = 0;
        atom.index = 1;
        atom.do_old = true;
        atom.new_position = pman.get_ensemble().position( atom.index );
        atom.do_new = true;
        atom.new_position = { 1.05, 1.05, 0.0 };
        gman.calculate_distances_sq( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij2, 0, pman.get_ensemble().size() );
        trial.add_atom( atom );
      }
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( trial.fail() );
    }
  }

}

BOOST_AUTO_TEST_CASE( evaluator_meta_hard_sphere_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::hard_sphere_overlap::add_definition( *m );
  
    core::input_help hlpr;
    m->publish_help( hlpr );
    {
      std::stringstream ss;
      hlpr.write( ss );
      BOOST_CHECK( ss.str().find( "hard-sphere" ) < ss.str().size() );
    }
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"hard-sphere\"\nend\n\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE_EQUAL( mngr->size(), 1ul );
  BOOST_CHECK_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "hard-sphere" );
  
  std::string canon( "evaluator\ntype hard-sphere\nend\n" );
  
  core::input_document wrtr( 1ul );
  mngr->write_document( wrtr );
  std::stringstream idoc;
  wrtr.write( idoc );
  const std::string sdoc( idoc.str() );
  BOOST_CHECK_LE( sdoc.find( canon ), sdoc.size() );

}

BOOST_AUTO_TEST_CASE( object_overlap_lifetime_tests )
{
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::object_overlap >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::object_overlap >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::object_overlap >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::object_overlap, evaluator::object_overlap >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::object_overlap >::type {} );
  }
  const std::string defn_label( "object-overlap" );
  {
    evaluator::object_overlap eval;
    BOOST_CHECK_EQUAL( eval.type_label(), defn_label );
  }

}

BOOST_AUTO_TEST_CASE( object_overlap_static_method_tests )
{
  const std::string defn_label( "object-overlap" );
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    BOOST_REQUIRE_NO_THROW( eval_ptr = evaluator::object_overlap::make_evaluator( evaluator_test::mockup_params() ) );
    BOOST_REQUIRE( eval_ptr );
    evaluator::object_overlap *clmb_ptr = dynamic_cast< evaluator::object_overlap* >( eval_ptr.get() );
    BOOST_REQUIRE( clmb_ptr != nullptr );
    BOOST_CHECK_EQUAL( eval_ptr->type_label(), defn_label );
  
    boost::shared_ptr< evaluator::evaluator_manager > eman;
    evaluator::evaluator_meta m( eman );
    BOOST_REQUIRE_NO_THROW( evaluator::object_overlap::add_definition( m ) );
    BOOST_REQUIRE( m.has_definition( defn_label ) );
  }
  

}

// Check object overlap with single particle add/move on ensemble with eight particles 
// of +/-1 valency arranged in a  1 Angstrom cube. Include serialization test.
BOOST_AUTO_TEST_CASE( object_overlap_evaluator_serialization_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  std::stringstream store;
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr( evaluator::object_overlap::make_evaluator( evaluator_test::mockup_params() ) );
    evaluator::object_overlap* llp = dynamic_cast< evaluator::object_overlap* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
  
    eval_ptr->prepare( pman, gman, eman );
    {
      double energy = 0.0;
      BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
      BOOST_CHECK_CLOSE( energy, 0.0, 0.000001 );
    }
    {
      // attempt add without overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = pman.get_ensemble().size();
      atom.do_old = false;
      atom.do_new = true;
      atom.new_position = { 0.5, 0.5, 0.5 };
      gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move without overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 0.5, 0.5, 0.5 };
      gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
  
    boost::archive::text_oarchive oa( store );
    oa << eval_ptr;
  }
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    boost::archive::text_iarchive ia( store );
    ia >> eval_ptr;
    {
      // attempt add without overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = pman.get_ensemble().size();
      atom.do_old = false;
      atom.do_new = true;
      atom.new_position = { 0.5, 0.5, 0.5 };
      gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move without overlap
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 0.5, 0.5, 0.5 };
      gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    {
      // attempt move with overlap to our old position
      particle::change_set trial;
      particle::change_atom atom;
      atom.key = 0;
      atom.index = 0;
      atom.do_old = true;
      atom.new_position = pman.get_ensemble().position( atom.index );
      atom.do_new = true;
      atom.new_position = { 0.05, 0.05, 0.05 };
      gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
      trial.add_atom( atom );
      // std::cout << trial << "\n";
      BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
      BOOST_CHECK( not trial.fail() );
    }
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, 0.0, 0.000001 );
  }
  
  

}

// Check overlap with particle add/move in membrane containing cell.
BOOST_AUTO_TEST_CASE( object_overlap_evaluator_cell_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  // System is membrane cell
  //        +> rB  +> r>
  //  +-------\      /------+  ^
  //  |        \____/       |  |
  //  |                     |  |  ^
  //  +---------------------+  +  +
  //  +-----------+ lA         Ra Rb
  //            +-+ lB
  //
  //  lA = 20 ,  lB = 3
  //  Rb = 2  ,  Rb = 7
  //  rA = 2  ,  rB = 2
  
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::membrane_cylinder_region( "cell", 7.0, 10.0, 3.0, 2.0, 2.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  // Check all existing points are valid
  for( std::size_t idx = 0; idx != pman.get_ensemble().size(); ++idx )
  {
    BOOST_REQUIRE( sys_rgn->is_inside( pman.get_ensemble().position( idx ), pman.get_specie( pman.get_ensemble().key( idx ) ).radius() ) );
  }
  
  evaluator::evaluator_manager eman;
  
  std::unique_ptr< evaluator::base_evaluator > eval_ptr( evaluator::object_overlap::make_evaluator( evaluator_test::mockup_params() ) );
  evaluator::object_overlap* llp = dynamic_cast< evaluator::object_overlap* >( eval_ptr.get() );
  BOOST_REQUIRE( nullptr != llp );
  
  eval_ptr->prepare( pman, gman, eman );
  {
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, 0.0, 0.000001 );
  }
  {
    // attempt add with overlap
    particle::change_set trial;
    particle::change_atom atom;
    atom.key = 0;
    atom.index = pman.get_ensemble().size();
    atom.do_old = false;
    atom.do_new = true;
    atom.new_position = { 1.5, 1.5, 0.05 };
    gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
    trial.add_atom( atom );
    // std::cout << trial << "\n";
    BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
    BOOST_CHECK( trial.fail() );
  }
  {
    // attempt add without overlap
    particle::change_set trial;
    particle::change_atom atom;
    atom.key = 0;
    atom.index = pman.get_ensemble().size();
    atom.do_old = false;
    atom.do_new = true;
    atom.new_position = { 0.5, 0.5, 0.5 };
    gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
    trial.add_atom( atom );
    // std::cout << trial << "\n";
    BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
    BOOST_CHECK( not trial.fail() );
  }
  {
    // attempt move without overlap
    particle::change_set trial;
    particle::change_atom atom;
    atom.key = 0;
    atom.index = 0;
    atom.do_old = true;
    atom.new_position = pman.get_ensemble().position( atom.index );
    atom.do_new = true;
    atom.new_position = { 0.5, 0.5, 0.5 };
    gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
    trial.add_atom( atom );
    // std::cout << trial << "\n";
    BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
    BOOST_CHECK( not trial.fail() );
  }
  {
    // attempt move with overlap to our old position
    particle::change_set trial;
    particle::change_atom atom;
    atom.key = 0;
    atom.index = 0;
    atom.do_old = true;
    atom.new_position = pman.get_ensemble().position( atom.index );
    atom.do_new = true;
    atom.new_position = { 0.05, 0.05, 0.05 };
    gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
    trial.add_atom( atom );
    // std::cout << trial << "\n";
    BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
    BOOST_CHECK( not trial.fail() );
  }
  {
    // attempt move with overlap
    particle::change_set trial;
    particle::change_atom atom;
    atom.key = 0;
    atom.index = 0;
    atom.do_old = true;
    atom.new_position = pman.get_ensemble().position( atom.index );
    atom.do_new = true;
    atom.new_position = { 1.5, 1.5, 1.05 };
    gman.calculate_distances( atom.new_position, pman.get_ensemble().get_coordinates(), atom.new_rij, 0, pman.get_ensemble().size() );
    trial.add_atom( atom );
    // std::cout << trial << "\n";
    BOOST_REQUIRE_NO_THROW( eval_ptr->compute_potential( pman, gman, trial ) );
    BOOST_CHECK( trial.fail() );
  }
  

}

BOOST_AUTO_TEST_CASE( evaluator_meta_object_overlap_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::object_overlap::add_definition( *m );
  
    core::input_help hlpr;
    m->publish_help( hlpr );
    {
      std::stringstream ss;
      hlpr.write( ss );
      BOOST_CHECK( ss.str().find( "object-overlap" ) < ss.str().size() );
    }
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"object-overlap\"\nend\n\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE_EQUAL( mngr->size(), 1ul );
  BOOST_CHECK_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "object-overlap" );
  
  std::string canon( "evaluator\ntype object-overlap\nend\n" );
  
  core::input_document wrtr( 1ul );
  mngr->write_document( wrtr );
  std::stringstream idoc;
  wrtr.write( idoc );
  const std::string sdoc( idoc.str() );
  BOOST_CHECK_LE( sdoc.find( canon ), sdoc.size() );

}

BOOST_AUTO_TEST_CASE( localizer_lifetime_tests )
{
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::localizer >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::localizer >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::localizer >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::localizer, evaluator::localizer >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::localizer >::type {} );
  }
  const std::string defn_label( core::strngs::localizer_label() );
  {
    evaluator::localizer eval;
    BOOST_CHECK_EQUAL( eval.type_label(), defn_label );
    BOOST_CHECK_EQUAL( eval.spring_factor(), evaluator::localizer::default_spring_factor() );
  }
  

}

BOOST_AUTO_TEST_CASE( localizer_static_method_tests )
{
  {
    // Static method tests.
    BOOST_CHECK_CLOSE( evaluator::localizer::default_spring_factor(), 4.5, 0.0000001 );
  }
  const std::string defn_label( core::strngs::localizer_label() );
  {
    BOOST_CHECK_EQUAL( evaluator::localizer::type_label_(), defn_label );
  
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    BOOST_REQUIRE_NO_THROW( eval_ptr = evaluator::localizer::make_evaluator( evaluator_test::mockup_params() ) );
    BOOST_REQUIRE( eval_ptr );
    evaluator::localizer *clmb_ptr = dynamic_cast< evaluator::localizer* >( eval_ptr.get() );
    BOOST_REQUIRE( clmb_ptr != nullptr );
    BOOST_CHECK_EQUAL( eval_ptr->type_label(), defn_label );
    BOOST_CHECK_EQUAL( clmb_ptr->spring_factor(), 4.5 );
  
    boost::shared_ptr< evaluator::evaluator_manager > eman;
    evaluator::evaluator_meta m( eman );
    BOOST_REQUIRE_NO_THROW( evaluator::localizer::add_definition( m ) );
    BOOST_REQUIRE( m.has_definition( defn_label ) );
    BOOST_CHECK( m.get_definition( defn_label ).has_definition( core::strngs::fskmob() ) );
  }
  

}

BOOST_AUTO_TEST_CASE( localizer_methods_test )
{
  std::stringstream store;
  const double spring_factor( 2.0 );
  {
    // static method test
    BOOST_CHECK_EQUAL( core::strngs::localizer_label(), evaluator::localizer::type_label_() );
  }
  // Constructor test methods
  {
    // default
    evaluator::localizer ll;
    BOOST_CHECK_EQUAL( ll.spring_factor(), evaluator::localizer::default_spring_factor() );
    ll.spring_factor( spring_factor );
    BOOST_CHECK_EQUAL( ll.spring_factor(), spring_factor );
  
    // write_document test
    core::input_document wr( 1 );
    ll.write_document( wr );
    BOOST_CHECK_EQUAL( wr.size(), 1 );
    BOOST_CHECK_EQUAL( wr[ 0 ].label(), core::strngs::evaluator_label() );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fstype() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fstype() ), core::strngs::localizer_label() );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fskmob() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fskmob() ), std::to_string( spring_factor ) );
  
    // serialize
    boost::archive::text_oarchive oa( store );
    oa << ll;
  }
  {
    evaluator::localizer ll;
    BOOST_CHECK_EQUAL( ll.spring_factor(),  evaluator::localizer::default_spring_factor() );
    // deserialize
    boost::archive::text_iarchive ia( store );
    ia >> ll;
    BOOST_CHECK_EQUAL( ll.spring_factor(), spring_factor );
  
    // write description
    std::stringstream desc;
    ll.description( desc );
    // 0 = evaluator, 1 = type, 2 = kmob (+ check kmob value)
    std::bitset< 3 > flags;
    flags.set();
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
      if( line.find( core::strngs::evaluator_label() ) != std::string::npos )
      {
        flags.set( 0, true );
        if( line.find( core::strngs::localizer_label() ) != std::string::npos )
        {
          flags.set( 1, true );
        }
      }
      if( line.find( core::strngs::fskmob() ) != std::string::npos )
      {
        flags.set( 2, true );
        if( line.find( core::strngs::localizer_label() ) != std::string::npos )
        {
          const std::size_t split_pos
          {
            line.find( ':' )
          };
          BOOST_REQUIRE( std::string::npos != split_pos );
          std::stringstream ss( line.substr( split_pos ) );
          double value;
          ss >> value;
          BOOST_CHECK_CLOSE( value, spring_factor, 0.000001 );
        }
      }
    }
    BOOST_CHECK_MESSAGE( flags[ 0 ], core::strngs::evaluator_label() << " not found." );
    BOOST_CHECK_MESSAGE( flags[ 1 ], core::strngs::localizer_label() << " not found." );
    BOOST_CHECK_MESSAGE( flags[ 2 ], core::strngs::fskmob() << " not found." );
  
  }
  {
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fskmob() );
    params.back().set_value( "1.0" );
    params.push_back( evaluator_test::mockup_params().back() );
  
    // One arg list with "mobk"
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    BOOST_CHECK_NO_THROW( eval_ptr = evaluator::localizer::make_evaluator( params ) );
  
    evaluator::localizer* llp = dynamic_cast< evaluator::localizer* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->spring_factor(), 1.0 );
  
    // write_document test
    core::input_document wr( 1 );
    eval_ptr->write_document( wr );
    BOOST_CHECK_EQUAL( wr.size(), 1 );
    BOOST_CHECK_EQUAL( wr[ 0 ].label(), core::strngs::evaluator_label() );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fstype() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fstype() ), core::strngs::localizer_label() );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fskmob() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fskmob() ), std::to_string( 1.0 ) );
  
    // serialize
    boost::archive::text_oarchive oa( store );
    oa << eval_ptr;
  }
  {
    std::unique_ptr< evaluator::base_evaluator > ll;
    // deserialize
    boost::archive::text_iarchive ia( store );
    ia >> ll;
    evaluator::localizer* llp = dynamic_cast< evaluator::localizer* >( ll.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->spring_factor(), 1.0 );
  }
  

}

BOOST_AUTO_TEST_CASE( localizer_evaluator_test )
{
  // Build basic system.
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
    pman.add_specie( spc1 );
  }
  {
    particle::specie spc1;
    spc1.set_label( "OX" );
    spc1.set_concentration( 1.0 );
    spc1.set_radius( 0.2 );
    spc1.set_rate( 0.1 );
    spc1.set_valency( -1.0 );
    spc1.set_excess_potential( 0.3123 );
    spc1.set_type( particle::specie::MOBILE );
    // Original position is on localization point, therefore potential is zero
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ), geometry::centroid( 3.0, 1.0, 1.0, 1.0 ) );
    pman.add_specie( spc1 );
  }
  
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 3 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  {
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fskmob() );
    params.back().set_value( "1.0" );
    params.push_back( evaluator_test::mockup_params().back() );
  
    std::unique_ptr< base_evaluator > eval_ptr;
    BOOST_REQUIRE_NO_THROW( eval_ptr = evaluator::localizer::make_evaluator( params ) );
  
    double energy = 0.0;
  
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE_FRACTION( energy, 0.0, 0.0000000001 );
  
    // find localized particle
    const std::size_t lispec = pman.get_specie_key( "OX" );
    const std::size_t lidx = pman.get_ensemble().nth_specie_index( lispec, 0 );
    {
      // Move particle 1 angstrom distance
      const geometry::coordinate pstart( pman.get_ensemble().position( lidx ) );
      const geometry::coordinate pend( pstart.x + 0.6, pstart.y, pstart.z - 0.8 );
      particle::change_atom atm( lispec );
      atm.index = lidx;
      atm.do_old = true;
      atm.do_new = true;
      atm.old_position = pstart;
      atm.new_position = pend;
      particle::change_set move;
      move.add_atom( atm );
  
      const double distance( std::sqrt( gman.calculate_distance_squared( pstart, pend ) ) );
  
      BOOST_CHECK_CLOSE_FRACTION( distance, 1.0, 0.0000000001 );
      // U = 1/2 k x^2 : spring factor == 1/2 k
      const double delta_energy( 1.0/9.0 );
  
      // Calculate energy
      eval_ptr->compute_potential( pman, gman, move );
      BOOST_REQUIRE( not move.fail() );
      BOOST_CHECK_CLOSE_FRACTION( move.energy(), delta_energy, 0.0000000001 );
    }
    {
      // Move particle 1 angstrom distance
      const geometry::coordinate pstart( pman.get_ensemble().position( lidx ) );
      const geometry::coordinate pend( pstart.x + 0.3, pstart.y, pstart.z - 0.4 );
      particle::change_atom atm( lispec );
      atm.index = lidx;
      atm.do_old = true;
      atm.do_new = true;
      atm.old_position = pstart;
      atm.new_position = pend;
      particle::change_set move;
      move.add_atom( atm );
  
      const double distance( std::sqrt( gman.calculate_distance_squared( pstart, pend ) ) );
  
      BOOST_CHECK_CLOSE_FRACTION( distance, 0.5, 0.0000000001 );
      // U = 1/2 k x^2 : spring factor == 1/2 k
      const double delta_energy( 0.25/9.0 );
  
      // Calculate energy
      eval_ptr->compute_potential( pman, gman, move );
      BOOST_REQUIRE( not move.fail() );
      BOOST_CHECK_CLOSE_FRACTION( move.energy(), delta_energy, 0.0000000001 );
    }
    {
      // Move particle 1 angstrom distance
      const geometry::coordinate pstart( pman.get_ensemble().position( lidx ) );
      const geometry::coordinate pend( pstart.x + 1.9, pstart.y, pstart.z - 2.4 );
      particle::change_atom atm( lispec );
      atm.index = lidx;
      atm.do_old = true;
      atm.do_new = true;
      atm.old_position = pstart;
      atm.new_position = pend;
      particle::change_set move;
      move.add_atom( atm );
  
      const double distance( std::sqrt( gman.calculate_distance_squared( pstart, pend ) ) );
  
      BOOST_CHECK_CLOSE_FRACTION( distance, 3.061, 0.001 );
  
      // Calculate energy
      eval_ptr->compute_potential( pman, gman, move );
      BOOST_REQUIRE( move.fail() );
    }
  }

}

BOOST_AUTO_TEST_CASE( evaluator_meta_localizer_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::localizer::add_definition( *m );
  
    core::input_help hlpr;
    m->publish_help( hlpr );
    {
      std::stringstream ss;
      hlpr.write( ss );
      BOOST_CHECK( ss.str().find( "localize" ) < ss.str().size() );
      BOOST_CHECK( ss.str().find( "mobk" ) < ss.str().size() );
    }
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"localize\"\nend\n\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE_EQUAL( mngr->size(), 1ul );
  BOOST_CHECK_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "localize" );
  
  std::string canon( "evaluator\nmobk 4.500000\ntype localize\nend\n" );
  
  core::input_document wrtr( 1ul );
  mngr->write_document( wrtr );
  std::stringstream idoc;
  wrtr.write( idoc );
  const std::string sdoc( idoc.str() );
  BOOST_CHECK_LE( sdoc.find( canon ), sdoc.size() );
  

}

BOOST_AUTO_TEST_CASE( input_localizer_no_mobk_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::localizer::add_definition( *m );
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"localize\"\nmobk #one\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Harmonic potential factor parameter \"mobk\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>mobk #one<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_localizer_bad_mobk_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::localizer::add_definition( *m );
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"localize\"\nmobk one\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Harmonic potential factor parameter \"mobk\" with value (one) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>mobk one<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( lennard_jones_lifetime_tests )
{
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::lennard_jones >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::lennard_jones >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::lennard_jones >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::lennard_jones, evaluator::lennard_jones >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::lennard_jones >::type {} );
  }
  const std::string defn_label( "lennard-jones" );
  {
    evaluator::lennard_jones eval;
    BOOST_CHECK_EQUAL( eval.type_label(), defn_label );
  }
  

}

BOOST_AUTO_TEST_CASE( lennard_jones_static_method_tests )
{
    // Static method tests.
  const std::string defn_label( "lennard-jones" );
  {
    BOOST_CHECK_EQUAL( evaluator::lennard_jones::type_label_(), defn_label );
  
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
  
    BOOST_REQUIRE_NO_THROW( eval_ptr = evaluator::lennard_jones::make_evaluator( evaluator_test::mockup_params() ) );
    BOOST_REQUIRE( eval_ptr );
    evaluator::lennard_jones *lj_ptr = dynamic_cast< evaluator::lennard_jones* >( eval_ptr.get() );
    BOOST_REQUIRE( lj_ptr != nullptr );
    BOOST_CHECK_EQUAL( eval_ptr->type_label(), defn_label );
  
    boost::shared_ptr< evaluator::evaluator_manager > eman;
    evaluator::evaluator_meta m( eman );
    BOOST_REQUIRE_NO_THROW( evaluator::lennard_jones::add_definition( m ) );
    BOOST_REQUIRE( m.has_definition( defn_label ) );
    BOOST_CHECK( particle::specie_meta::has_keyword( "epsilon" ) );
    BOOST_CHECK( particle::specie_meta::has_keyword( "sigma" ) );
  }
  

}

// Attempt to build evaluator with bad params
BOOST_AUTO_TEST_CASE( lennard_jones_species_no_epsilon_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "0.24" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "0.48" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "0.48" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    BOOST_REQUIRE_NO_THROW( eval_ptr = evaluator::lennard_jones::make_evaluator( evaluator_test::mockup_params() ) );
    try
    {
      eval_ptr->prepare( pman, gman, eman );
      BOOST_ERROR( "expected \"evaluator::lennard_jones::make_evaluator\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Lennard-Jones parameter(s) in section \"specie\" is missing some required parameters.\n** Add the following parameters to the section: epsilon **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"evaluator::lennard_jones::make_evaluator\" was not expected type: " ) + err.what() );
    }
  }

}

// Attempt to build evaluator with bad params
BOOST_AUTO_TEST_CASE( lennard_jones_species_no_sigma_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "0.24" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "0.48" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "0.48" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    BOOST_REQUIRE_NO_THROW( eval_ptr = evaluator::lennard_jones::make_evaluator( evaluator_test::mockup_params() ) );
    try
    {
      eval_ptr->prepare( pman, gman, eman );
      BOOST_ERROR( "expected \"evaluator::lennard_jones::make_evaluator\" exception not thrown" );
    }
    catch( core::input_error const& err )
    {
      const std::string msg( err.what() );
      //std::cout << msg << "\n";
      BOOST_CHECK( msg.find( "Lennard-Jones parameter(s) in section \"specie\" is missing some required parameters.\n** Add the following parameters to the section: sigma **" ) < msg.size() );
    }
    catch( std::exception const& err )
    {
      BOOST_ERROR( std::string( "exception thrown by \"evaluator::lennard_jones::make_evaluator\" was not expected type: " ) + err.what() );
    }
  }

}

// Calculate total potential with eight particles of +/-1 valency
// arranged in a  1 Angstrom cube.  (eps=sig=1.0)
//
//  ** includes serialization test **
BOOST_AUTO_TEST_CASE( lennard_jones_evaluator_d8r1_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "1.0" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "1.0" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "1.0" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "1.0" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  std::stringstream store;
  double expected_value = 0.0;
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr( evaluator::lennard_jones::make_evaluator( evaluator_test::mockup_params() ) );
    evaluator::lennard_jones* llp = dynamic_cast< evaluator::lennard_jones* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
  
    eval_ptr->prepare( pman, gman, eman );
  
    expected_value = 12.0 * ( 1/64.0 - 1/8.0 ) + 4 * ( 1/729.0 - 1/27.0 );
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  
    boost::archive::text_oarchive oa( store );
    oa << eval_ptr;
  }
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    boost::archive::text_iarchive ia( store );
    ia >> eval_ptr;
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  }
  

}

// Calculate total potential with eight particles of +/-1 valency
// arranged in a  1 Angstrom cube. (eps=1, sig!=1)
//
//  ** includes serialization test **
BOOST_AUTO_TEST_CASE( lennard_jones_evaluator_d8r1_v2_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "1.0" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "2.0" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "1.0" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "3.0" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  std::stringstream store;
  double expected_value = 0.0;
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr( evaluator::lennard_jones::make_evaluator( evaluator_test::mockup_params() ) );
    evaluator::lennard_jones* llp = dynamic_cast< evaluator::lennard_jones* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
  
    eval_ptr->prepare( pman, gman, eman );
  
    const double r1_nc6 = std::pow( 2.5, 6 );
    const double r1_nc = r1_nc6 * ( r1_nc6 - 1.0 );
    const double r2_nn6 = 8.0;
    const double r2_nn = r2_nn6 * ( r2_nn6 - 1.0 );
    const double r2_cc6 = 729.0 / 8.0;
    const double r2_cc = r2_cc6 * ( r2_cc6 - 1.0 );
    const double r3_cn6 = std::pow( 2.5, 6 ) / 27.0;
    const double r3_cn = r3_cn6 * ( r3_cn6 - 1.0 );
  
    expected_value = 6 * ( 2 * r1_nc + r2_nn + r2_cc ) + 4 * r3_cn;
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  
    boost::archive::text_oarchive oa( store );
    oa << eval_ptr;
  }
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    boost::archive::text_iarchive ia( store );
    ia >> eval_ptr;
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  }
  

}

// Calculate total potential with eight particles of +/-1 valency
// arranged in a  1 Angstrom cube.  (eps!=1 , sig=1.0)
//
//  ** includes serialization test **
BOOST_AUTO_TEST_CASE( lennard_jones_evaluator_d8r1_v3_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "0.5" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "1.0" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "0.3" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "1.0" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  std::stringstream store;
  double expected_value = 0.0;
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr( evaluator::lennard_jones::make_evaluator( evaluator_test::mockup_params() ) );
    evaluator::lennard_jones* llp = dynamic_cast< evaluator::lennard_jones* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
  
    eval_ptr->prepare( pman, gman, eman );
  
    const double eps_nc = std::sqrt( 0.5 * 0.3 );
  
    expected_value = 6.0 * ( 1/64.0 - 1/8.0 ) * ( 0.5 + 0.3 ) + 4 * eps_nc * ( 1/729.0 - 1/27.0 );
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  
    boost::archive::text_oarchive oa( store );
    oa << eval_ptr;
  }
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    boost::archive::text_iarchive ia( store );
    ia >> eval_ptr;
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  }
  

}

// Calculate total potential with eight particles of +/-1 valency
// arranged in a  1 Angstrom cube. (eps!=1, sig!=1)
//
//  ** includes serialization test **
BOOST_AUTO_TEST_CASE( lennard_jones_evaluator_d8r1_v4_test )
{
  // Build basic system.
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
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "0.5" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "2.0" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
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
    spc1.append_position( geometry::coordinate( 0.0, 1.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 0.0, 0.0 ) );
    spc1.append_position( geometry::coordinate( 0.0, 0.0, 1.0 ) );
    spc1.append_position( geometry::coordinate( 1.0, 1.0, 1.0 ) );
    {
      core::input_parameter_memo eps;
      eps.set_name( "epsilon" );
      eps.set_value( "0.3" );
      spc1.set_parameter( eps );
    }
    {
      core::input_parameter_memo sig;
      sig.set_name( "sigma" );
      sig.set_value( "3.0" );
      spc1.set_parameter( sig );
    }
    {
      core::input_parameter_memo end;
      end.set_name( "end" );
      spc1.set_parameter( end );
    }
    pman.add_specie( spc1 );
  }
  pman.add_predefined_particles();
  BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
  BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 8 );
  
  boost::shared_ptr< geometry::base_region > sys_rgn( new geometry::periodic_cube_region( "cell", 10.0 ) );
  geometry::geometry_manager gman( sys_rgn );
  
  evaluator::evaluator_manager eman;
  
  std::stringstream store;
  double expected_value = 0.0;
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr( evaluator::lennard_jones::make_evaluator( evaluator_test::mockup_params() ) );
    evaluator::lennard_jones* llp = dynamic_cast< evaluator::lennard_jones* >( eval_ptr.get() );
    BOOST_REQUIRE( nullptr != llp );
  
    eval_ptr->prepare( pman, gman, eman );
  
    const double eps_nc = std::sqrt( 0.5 * 0.3 );
    const double r1_nc6 = std::pow( 2.5, 6 );
    const double r1_nc = r1_nc6 * ( r1_nc6 - 1.0 );
    const double r2_nn6 = 8.0;
    const double r2_nn = r2_nn6 * ( r2_nn6 - 1.0 );
    const double r2_cc6 = 729.0 / 8.0;
    const double r2_cc = r2_cc6 * ( r2_cc6 - 1.0 );
    const double r3_cn6 = std::pow( 2.5, 6 ) / 27.0;
    const double r3_cn = r3_cn6 * ( r3_cn6 - 1.0 );
  
    expected_value = 12 * eps_nc * r1_nc + 6 * ( 0.5 * r2_nn + 0.3 * r2_cc ) + 4 * eps_nc * ( r3_cn );
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  
    boost::archive::text_oarchive oa( store );
    oa << eval_ptr;
  }
  {
    std::unique_ptr< evaluator::base_evaluator > eval_ptr;
    boost::archive::text_iarchive ia( store );
    ia >> eval_ptr;
  
    double energy = 0.0;
    BOOST_REQUIRE_NO_THROW( energy = eval_ptr->compute_total_potential( pman, gman ) );
    BOOST_CHECK_CLOSE( energy, expected_value, 0.000001 );
  }
  

}

BOOST_AUTO_TEST_CASE( evaluator_meta_lennard_jones_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::lennard_jones::add_definition( *m );
  
    core::input_help hlpr;
    m->publish_help( hlpr );
    {
      std::stringstream ss;
      hlpr.write( ss );
      BOOST_CHECK( ss.str().find( "lennard-jones" ) < ss.str().size() );
    }
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"lennard-jones\"\nend\n\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  BOOST_REQUIRE_NO_THROW( dg.read_input( reader ) );
  
  BOOST_REQUIRE_EQUAL( mngr->size(), 1ul );
  BOOST_CHECK_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "lennard-jones" );
  
  std::string canon( "evaluator\ntype lennard-jones\nend\n" );
  
  core::input_document wrtr( 1ul );
  mngr->write_document( wrtr );
  std::stringstream idoc;
  wrtr.write( idoc );
  const std::string sdoc( idoc.str() );
  // std::cout << sdoc << "\n";
  BOOST_CHECK_LE( sdoc.find( canon ), sdoc.size() );
  

}

BOOST_AUTO_TEST_CASE( input_lennard_jones_unknown_parameter_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::lennard_jones::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"lennard-jones\"\nmobk \"Na\" 2.0 Cl 3.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Unknown parameter \"mobk\" in section \"evaluator\" subtype \"lennard-jones\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n** Parameter \"mobk\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( arc_integrator_lifetime_test )
{
  {
    //  Constructor tests virtual with default ctor.
    BOOST_CHECK( std::is_default_constructible< evaluator::arc_integrator >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::arc_integrator >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::arc_integrator >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::arc_integrator, evaluator::arc_integrator >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::arc_integrator >::type {} );
  }
  {
    evaluator::arc_integrator acc;
    BOOST_CHECK_EQUAL( acc.index(), 0 );
    BOOST_CHECK_EQUAL( acc.nsub_other(), 0 );
    BOOST_CHECK_EQUAL( acc.nsub_self(), 0 );
    BOOST_CHECK_EQUAL( acc.za0(), 0.0 );
    BOOST_CHECK_EQUAL( acc.ra0(), 0.0 );
    BOOST_CHECK_EQUAL( acc.radius(), 0.0 );
    BOOST_CHECK_EQUAL( acc.theta1(), 0.0 );
    BOOST_CHECK_EQUAL( acc.theta2(), 0.0 );
    BOOST_CHECK_EQUAL( acc.phi1(), 0.0 );
    BOOST_CHECK_EQUAL( acc.phi2(), 0.0 );
  }
  
  const double za0( 5.0 );
  const double ra0( 4.0 );
  const double radius( 2.0 );
  const double theta1( 1.0 );
  const double theta2( 1.1 );
  const double phi1( 4.3 );
  const double phi2( 4.4 );
  const std::size_t nsub( 10 );
  std::size_t ndx( 57 );
  std::stringstream store;
  {
    // constructor
    std::unique_ptr< evaluator::integrator > intr_ptr( new evaluator::arc_integrator( nsub, ndx, za0, ra0, radius, theta1, theta2, phi1, phi2 ) );
    evaluator::arc_integrator const* acc_ptr = dynamic_cast< evaluator::arc_integrator* >( intr_ptr.get() );
    BOOST_REQUIRE( nullptr != acc_ptr );
    BOOST_CHECK_EQUAL( acc_ptr->index(), ndx );
    BOOST_CHECK_EQUAL( acc_ptr->nsub_other(), nsub );
    BOOST_CHECK_EQUAL( acc_ptr->nsub_self(), 6 * nsub );
    BOOST_CHECK_EQUAL( acc_ptr->za0(), za0 );
    BOOST_CHECK_EQUAL( acc_ptr->ra0(), ra0 );
    BOOST_CHECK_EQUAL( acc_ptr->radius(), radius );
    BOOST_CHECK_EQUAL( acc_ptr->theta1(), theta1 );
    BOOST_CHECK_EQUAL( acc_ptr->theta2(), theta2 );
    BOOST_CHECK_EQUAL( acc_ptr->phi1(), phi1 );
    BOOST_CHECK_EQUAL( acc_ptr->phi2(), phi2 );
  
    boost::archive::text_oarchive oa( store );
    oa << intr_ptr;
  }
  {
    std::unique_ptr< evaluator::integrator > intr_ptr;
  
    boost::archive::text_iarchive ia( store );
    ia >> intr_ptr;
  
    evaluator::arc_integrator const* acc_ptr = dynamic_cast< evaluator::arc_integrator* >( intr_ptr.get() );
    BOOST_REQUIRE( nullptr != acc_ptr );
    BOOST_CHECK_EQUAL( acc_ptr->index(), ndx );
    BOOST_CHECK_EQUAL( acc_ptr->nsub_other(), nsub );
    BOOST_CHECK_EQUAL( acc_ptr->nsub_self(), 6 * nsub );
    BOOST_CHECK_EQUAL( acc_ptr->za0(), za0 );
    BOOST_CHECK_EQUAL( acc_ptr->ra0(), ra0 );
    BOOST_CHECK_EQUAL( acc_ptr->radius(), radius );
    BOOST_CHECK_EQUAL( acc_ptr->theta1(), theta1 );
    BOOST_CHECK_EQUAL( acc_ptr->theta2(), theta2 );
    BOOST_CHECK_EQUAL( acc_ptr->phi1(), phi1 );
    BOOST_CHECK_EQUAL( acc_ptr->phi2(), phi2 );
  }

}

BOOST_AUTO_TEST_CASE( cylinder_integrator_lifetime_test )
{
  {
    //  Constructor tests virtual with default ctor.
    BOOST_CHECK( std::is_default_constructible< evaluator::arc_integrator >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::arc_integrator >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::arc_integrator >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::arc_integrator, evaluator::arc_integrator >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::arc_integrator >::type {} );
  }
  
  {
    evaluator::cylinder_integrator cyl;
    BOOST_CHECK_EQUAL( cyl.index(), 0 );
    BOOST_CHECK_EQUAL( cyl.nsub_other(), 0 );
    BOOST_CHECK_EQUAL( cyl.nsub_self(), 0 );
    BOOST_CHECK_EQUAL( cyl.radius(), 0.0 );
    BOOST_CHECK_EQUAL( cyl.zmin(), 0.0 );
    BOOST_CHECK_EQUAL( cyl.zmax(), 0.0 );
    BOOST_CHECK_EQUAL( cyl.phimin(), 0.0 );
    BOOST_CHECK_EQUAL( cyl.phimax(), 0.0 );
  }
  const double radius( 2.0 );
  const double zmin( 1.0 );
  const double zmax( 1.1 );
  const double phi1( 4.3 );
  const double phi2( 4.4 );
  const std::size_t nsub( 10 );
  std::size_t ndx( 57 );
  std::stringstream store;
  {
    // constructor
    std::unique_ptr< evaluator::integrator > intr_ptr( new evaluator::cylinder_integrator( nsub, ndx, radius, zmin, zmax, phi1, phi2 ) );
    evaluator::cylinder_integrator const* cyl_ptr = dynamic_cast< evaluator::cylinder_integrator* >( intr_ptr.get() );
    BOOST_REQUIRE( nullptr != cyl_ptr );
    BOOST_CHECK_EQUAL( cyl_ptr->index(), ndx );
    BOOST_CHECK_EQUAL( cyl_ptr->nsub_other(), nsub );
    BOOST_CHECK_EQUAL( cyl_ptr->nsub_self(), 6 * nsub );
    BOOST_CHECK_EQUAL( cyl_ptr->radius(), radius );
    BOOST_CHECK_EQUAL( cyl_ptr->zmin(), zmin );
    BOOST_CHECK_EQUAL( cyl_ptr->zmax(), zmax );
    BOOST_CHECK_EQUAL( cyl_ptr->phimin(), phi1 );
    BOOST_CHECK_EQUAL( cyl_ptr->phimax(), phi2 );
  
    boost::archive::text_oarchive oa( store );
    oa << intr_ptr;
  }
  {
    std::unique_ptr< evaluator::integrator > intr_ptr;
  
    boost::archive::text_iarchive ia( store );
    ia >> intr_ptr;
  
    evaluator::cylinder_integrator const* cyl_ptr = dynamic_cast< evaluator::cylinder_integrator* >( intr_ptr.get() );
    BOOST_REQUIRE( nullptr != cyl_ptr );
  
    BOOST_CHECK_EQUAL( cyl_ptr->index(), ndx );
    BOOST_CHECK_EQUAL( cyl_ptr->nsub_other(), nsub );
    BOOST_CHECK_EQUAL( cyl_ptr->nsub_self(), 6 * nsub );
    BOOST_CHECK_EQUAL( cyl_ptr->radius(), radius );
    BOOST_CHECK_EQUAL( cyl_ptr->zmin(), zmin );
    BOOST_CHECK_EQUAL( cyl_ptr->zmax(), zmax );
    BOOST_CHECK_EQUAL( cyl_ptr->phimin(), phi1 );
    BOOST_CHECK_EQUAL( cyl_ptr->phimax(), phi2 );
  
  }

}

BOOST_AUTO_TEST_CASE( wall_integrator_lifetime_test )
{
  {
    //  Constructor tests virtual with default ctor.
    BOOST_CHECK( std::is_default_constructible< evaluator::wall_integrator >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::wall_integrator >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::wall_integrator >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::wall_integrator, evaluator::wall_integrator >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::wall_integrator >::type {} );
  }
  
  {
    evaluator::wall_integrator wall;
    BOOST_CHECK_EQUAL( wall.index(), 0 );
    BOOST_CHECK_EQUAL( wall.nsub_other(), 0 );
    BOOST_CHECK_EQUAL( wall.nsub_self(), 0 );
    BOOST_CHECK_EQUAL( wall.z(), 0.0 );
    BOOST_CHECK_EQUAL( wall.ra0(), 0.0 );
    BOOST_CHECK_EQUAL( wall.ra1(), 0.0 );
    BOOST_CHECK_EQUAL( wall.radius(), 0.0 );
    BOOST_CHECK_EQUAL( wall.phi1(), 0.0 );
    BOOST_CHECK_EQUAL( wall.phi2(), 0.0 );
  }
  
  const double z( 4.0 );
  const double ra0( 1.0 );
  const double ra1( 1.1 );
  const double radius( 2.0 );
  const double phi1( 4.3 );
  const double phi2( 4.4 );
  const std::size_t nsub( 10 );
  std::size_t ndx( 57 );
  std::stringstream store;
  {
    // constructor
    evaluator::wall_integrator wall( nsub, ndx, z, ra0, ra1, radius, phi1, phi2 );
    std::unique_ptr< evaluator::integrator > intr_ptr( new evaluator::wall_integrator( nsub, ndx, z, ra0, ra1, radius, phi1, phi2 ) );
    evaluator::wall_integrator const* wall_ptr = dynamic_cast< evaluator::wall_integrator* >( intr_ptr.get() );
    BOOST_REQUIRE( nullptr != wall_ptr );
    BOOST_CHECK_EQUAL( wall_ptr->index(), ndx );
    BOOST_CHECK_EQUAL( wall_ptr->nsub_other(), nsub );
    BOOST_CHECK_EQUAL( wall_ptr->nsub_self(), 6 * nsub );
    BOOST_CHECK_EQUAL( wall_ptr->z(), z );
    BOOST_CHECK_EQUAL( wall_ptr->ra0(), ra0 );
    BOOST_CHECK_EQUAL( wall_ptr->ra1(), ra1 );
    BOOST_CHECK_EQUAL( wall_ptr->radius(), radius );
    BOOST_CHECK_EQUAL( wall_ptr->phi1(), phi1 );
    BOOST_CHECK_EQUAL( wall_ptr->phi2(), phi2 );
  
    boost::archive::text_oarchive oa( store );
    oa << intr_ptr;
  }
  {
    std::unique_ptr< evaluator::integrator > intr_ptr;
    boost::archive::text_iarchive ia( store );
    ia >> intr_ptr;
  
    evaluator::wall_integrator const* wall_ptr = dynamic_cast< evaluator::wall_integrator* >( intr_ptr.get() );
    BOOST_REQUIRE( nullptr != wall_ptr );
  
    BOOST_CHECK_EQUAL( wall_ptr->index(), ndx );
    BOOST_CHECK_EQUAL( wall_ptr->nsub_other(), nsub );
    BOOST_CHECK_EQUAL( wall_ptr->nsub_self(), 6 * nsub );
    BOOST_CHECK_EQUAL( wall_ptr->z(), z );
    BOOST_CHECK_EQUAL( wall_ptr->ra0(), ra0 );
    BOOST_CHECK_EQUAL( wall_ptr->ra1(), ra1 );
    BOOST_CHECK_EQUAL( wall_ptr->radius(), radius );
    BOOST_CHECK_EQUAL( wall_ptr->phi1(), phi1 );
    BOOST_CHECK_EQUAL( wall_ptr->phi2(), phi2 );
  }

}

BOOST_AUTO_TEST_CASE( patch_matrix_fixture_test )
{
  const double temperature( 300.0 );
  const double epsw( 80.0 );
  const double epspr( 10.0 );
  const double zl1( 5.3 );
  const double rl1( 8.8 );
  const double rl4( 30.0 );
  const double rlvest( 10.0 );
  const double rlmemb( 5.0 );
  const double dxw( 4.0 );
  const double dxf( 4.0 );
  std::size_t nsub( 10 );
  std::size_t expect( 620 );
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::patch_matrix_fixture >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::patch_matrix_fixture >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::patch_matrix_fixture >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::patch_matrix_fixture, evaluator::patch_matrix_fixture >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< evaluator::patch_matrix_fixture >::type {} );
  }
  evaluator::patch_matrix_fixture var {};
  BOOST_CHECK_EQUAL( var.epsw(), epsw );
  BOOST_CHECK_EQUAL( var.epspr(), epspr );
  BOOST_CHECK_EQUAL( var.dxf(), dxf );
  BOOST_CHECK_EQUAL( var.dxw(), dxw );
  BOOST_CHECK_EQUAL( var.nsub(), nsub );
  BOOST_CHECK_EQUAL( var.expected_grid_size(), expect );
  BOOST_CHECK_EQUAL( var.zl1(), zl1 );
  BOOST_CHECK_EQUAL( var.zl2(), zl1 + rlvest );
  BOOST_CHECK_EQUAL( var.zl3(), zl1 + rlvest - rlmemb );
  BOOST_CHECK_EQUAL( var.rl1(), rl1 );
  BOOST_CHECK_EQUAL( var.rl2(), rl1 + rlvest );
  BOOST_CHECK_EQUAL( var.rl3(), rl4 - rlmemb );
  BOOST_CHECK_EQUAL( var.rl4(), rl4 );
  BOOST_CHECK_EQUAL( var.rlvest(), rlvest );
  BOOST_CHECK_EQUAL( var.rlmemb(), rlmemb );
  BOOST_CHECK_EQUAL( var.temperature(), temperature );

}

BOOST_AUTO_TEST_CASE( icc_surface_grid_test )
{
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::icc_surface_grid >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::icc_surface_grid >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::icc_surface_grid >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::icc_surface_grid, evaluator::icc_surface_grid >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< evaluator::icc_surface_grid >::type {} );
  }
  const double dxf( 1.1 );
  const double dxw( 1.1 );
  const std::size_t nsub( 6 );
  evaluator::patch_matrix_fixture var;
  std::stringstream store;
  {
     // constructor
     evaluator::icc_surface_grid grid;
     BOOST_CHECK_EQUAL( grid.size(), 0 );
     BOOST_CHECK_EQUAL( grid.empty(), true );
     BOOST_CHECK_EQUAL( grid.have_integrators(), false );
     BOOST_CHECK_EQUAL( grid.get_dxf(), 1.6 );
     BOOST_CHECK_EQUAL( grid.get_dxw(), 1.6 );
     BOOST_CHECK_EQUAL( grid.get_nsub0(), 10 );
  
     // setters
     grid.set_dxf( dxf );
     BOOST_CHECK_EQUAL( grid.get_dxf(), dxf );
     grid.set_dxw( dxw );
     BOOST_CHECK_EQUAL( grid.get_dxw(), dxw );
     grid.set_nsub0( nsub );
     BOOST_CHECK_EQUAL( grid.get_nsub0(), nsub );
  
     var.define_grid( grid );
     // Should set dxf, dxw and nsub as
     // well as define patches.
     BOOST_CHECK_EQUAL( grid.get_dxf(), var.dxf() );
     BOOST_CHECK_EQUAL( grid.get_dxw(), var.dxw() );
     BOOST_CHECK_EQUAL( grid.get_nsub0(), var.nsub() );
  
     BOOST_CHECK_NE( grid.size(), 0 );
     BOOST_CHECK_EQUAL( grid.size(), var.expected_grid_size() );
     BOOST_CHECK_EQUAL( grid.empty(), false );
     BOOST_CHECK_EQUAL( grid.have_integrators(), true );
     {
        std::ofstream os( "patch.txt" );
        grid.write_grid( os );
     }
     boost::archive::text_oarchive oa( store );
     oa << grid;
  }
  {
     evaluator::icc_surface_grid grid;
  
     boost::archive::text_iarchive ia( store );
     ia >> grid;
  
     BOOST_CHECK_EQUAL( grid.get_dxf(), var.dxf() );
     BOOST_CHECK_EQUAL( grid.get_dxw(), var.dxw() );
     BOOST_CHECK_EQUAL( grid.get_nsub0(), var.nsub() );
     BOOST_CHECK_EQUAL( grid.size(), var.expected_grid_size() );
     BOOST_CHECK_EQUAL( grid.empty(), false );
     BOOST_CHECK_EQUAL( grid.have_integrators(), true );
  }

}

BOOST_AUTO_TEST_CASE( icc_matrix_test )
{
  evaluator::patch_matrix_fixture fix;
  std::stringstream store;
  std::stringstream iostore;
  evaluator::icc_matrix::array_type amx;
  std::vector< std::size_t > indx;
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::icc_matrix >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::icc_matrix >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::icc_matrix >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::icc_matrix, evaluator::icc_matrix >::type {} ) );
    BOOST_CHECK( not std::has_virtual_destructor< evaluator::icc_matrix >::type {} );
  }
  {
     evaluator::icc_matrix var;
     BOOST_CHECK_EQUAL( var.size(), 0 );
     BOOST_CHECK_EQUAL( var.empty(), true );
  
     BOOST_CHECK_THROW( var.write_a_matrix( store ), std::runtime_error );
  
     evaluator::icc_surface_grid gridder;
  
     fix.define_grid( gridder );
     
     var.compute_amx( gridder );
     {
        std::ofstream os( "amx1.txt" );
        var.write_a_matrix( os );
     }
     BOOST_CHECK_EQUAL( var.size(), gridder.size() );
     BOOST_CHECK_EQUAL( var.size(), fix.expected_grid_size() );
     var.lu_decompose_amx();
     {
        std::ofstream os( "amx.txt" );
        var.write_a_matrix( os );
     }
  
     boost::archive::text_oarchive oa( iostore );
     oa << var;
  
     amx.resize( boost::extents[ var.size() ][ var.size() ] );
     indx.resize( var.size() );
     for (std::size_t i = 0; i != var.size(); ++i)
     {
        std::array< std::size_t, 2 > idx;
        idx[ 0 ] = i;
        indx[ i ] = var.pivot( i );
        for (std::size_t j = 0; j != var.size(); ++j)
        {
           idx[ 1 ] = j;
           amx( idx ) = var.a( i, j );
        }
     }
  }
  {
     evaluator::icc_matrix var;
  
     BOOST_CHECK_EQUAL( var.size(), 0 );
     BOOST_CHECK_EQUAL( var.empty(), true );
  
     boost::archive::text_iarchive ia( iostore );
     ia >> var;
     BOOST_CHECK_EQUAL( var.size(), fix.expected_grid_size() );
     BOOST_CHECK_EQUAL( var.empty(), false );
     for (std::size_t i = 0; i != var.size(); ++i)
     {
        std::array< std::size_t, 2 > idx;
        idx[ 0 ] = i;
        BOOST_CHECK_EQUAL( indx[ i ], var.pivot( i ) );
        for (std::size_t j = 0; j != var.size(); ++j)
        {
           idx[ 1 ] = j;
           BOOST_CHECK_EQUAL( amx( idx ), var.a( i, j ) );
        }
     }
  }

}

BOOST_AUTO_TEST_CASE( induced_charge_test )
{
  evaluator::patch_matrix_fixture fix;
  //std::map< std::string, std::string > params;
  std::stringstream store;
  std::stringstream log;
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::induced_charge >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::induced_charge >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::induced_charge >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::induced_charge, evaluator::icc_matrix >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::induced_charge >::type {} );
  }
  {
    // static method test
    BOOST_CHECK_EQUAL( core::strngs::fsptch(), evaluator::induced_charge::type_label_() );
  }
  // Constructor test method
  {
    // default
    evaluator::induced_charge icceval;
  
    BOOST_CHECK_EQUAL( icceval.size(), 0 );
    BOOST_CHECK_EQUAL( icceval.empty(), true );
    BOOST_CHECK_EQUAL( icceval.type_label(), core::strngs::fsptch() );
    BOOST_CHECK( icceval.get_a_matrix_filename().empty()  );
    BOOST_CHECK( icceval.get_patch_filename().empty() );
    BOOST_CHECK_EQUAL( icceval.get_dxf(), 1.6 );
    BOOST_CHECK_EQUAL( icceval.get_dxw(), 1.6 );
    BOOST_CHECK_EQUAL( icceval.get_membrane_arc_radius(), 0.0 );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), 0.0 );
    BOOST_CHECK_EQUAL( icceval.get_protein_permittivity(), 10.0 );
    BOOST_CHECK_EQUAL( icceval.get_protein_radius(), 0.0 );
  
    BOOST_CHECK_NO_THROW( icceval.set_a_matrix_filename( "amx-test.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_a_matrix_filename(), "amx-test.txt" );
  
    BOOST_CHECK_NO_THROW( icceval.set_patch_filename( "patch-test.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_patch_filename(), "patch-test.txt" );
  
    BOOST_CHECK_NO_THROW( icceval.set_dxf( 1.25 ) );
    BOOST_CHECK_EQUAL( icceval.get_dxf(), 1.25 );
  
    BOOST_CHECK_NO_THROW( icceval.set_dxw( 3.5 ) );
    BOOST_CHECK_EQUAL( icceval.get_dxw(), 3.5 );
  
    BOOST_CHECK_NO_THROW( icceval.set_nsub0( 11 ) );
    BOOST_CHECK_EQUAL( icceval.get_nsub0(), 11 );
  
    BOOST_CHECK_NO_THROW( icceval.set_membrane_arc_radius( 12.25 ) );
    BOOST_CHECK_EQUAL( icceval.get_membrane_arc_radius(), 12.25 );
  
    BOOST_CHECK_NO_THROW( icceval.set_permittivity( 80.5 ) );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), 80.5 );
  
    BOOST_CHECK_NO_THROW( icceval.set_protein_permittivity( 8.125 ) );
    BOOST_CHECK_EQUAL( icceval.get_protein_permittivity(), 8.125 );
  
    BOOST_CHECK_NO_THROW( icceval.set_protein_radius( 32.5 ) );
    BOOST_CHECK_EQUAL( icceval.get_protein_radius(), 32.5 );
  
    BOOST_CHECK_EQUAL( icceval.size(), 0 );
    BOOST_CHECK_EQUAL( icceval.empty(), true );
  
    boost::archive::text_oarchive oa( store );
    oa << icceval;
  }
  // Serialization test method.
  {
    evaluator::induced_charge icceval;
  
    boost::archive::text_iarchive ia( store );
    ia >> icceval;
  
    BOOST_CHECK_EQUAL( icceval.size(), 0 );
    BOOST_CHECK_EQUAL( icceval.empty(), true );
    BOOST_CHECK_EQUAL( icceval.get_a_matrix_filename(), "amx-test.txt" );
    BOOST_CHECK_EQUAL( icceval.get_patch_filename(), "patch-test.txt" );
    BOOST_CHECK_EQUAL( icceval.get_dxf(), 1.25 );
    BOOST_CHECK_EQUAL( icceval.get_dxw(), 3.5 );
    BOOST_CHECK_EQUAL( icceval.get_nsub0(), 11 );
    BOOST_CHECK_EQUAL( icceval.get_membrane_arc_radius(), 12.25 );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), 80.5 );
    BOOST_CHECK_EQUAL( icceval.get_protein_permittivity(), 8.125 );
    BOOST_CHECK_EQUAL( icceval.get_protein_radius(), 32.5 );
  }
  // Manual build test
  {
    // default
    evaluator::induced_charge icceval;
    evaluator::patch_matrix_fixture simfix;
  
    // Set parameters as per 'fix'.
    BOOST_CHECK_NO_THROW( icceval.set_dxf( fix.dxf() ) );
    BOOST_CHECK_NO_THROW( icceval.set_dxw( fix.dxw() ) );
    BOOST_CHECK_NO_THROW( icceval.set_nsub0( fix.nsub() ) );
    BOOST_CHECK_NO_THROW( icceval.set_alfa( fix.temperature() ) );
    BOOST_CHECK_NO_THROW( icceval.set_permittivity( fix.epsw() ) );
    BOOST_CHECK_EQUAL( icceval.get_alfa(), fix.alfa() );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), fix.epsw() );
    BOOST_CHECK_NO_THROW( icceval.set_membrane_arc_radius( fix.rlmemb() ) );
    BOOST_CHECK_NO_THROW( icceval.set_protein_permittivity( fix.epspr() ) );
    BOOST_CHECK_NO_THROW( icceval.set_protein_radius( fix.rl4() ) );
    BOOST_CHECK_NO_THROW( icceval.set_a_matrix_filename( "amx-ctest.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_a_matrix_filename(), "amx-ctest.txt" );
  
    BOOST_CHECK_NO_THROW( icceval.set_patch_filename( "patch-ctest.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_patch_filename(), "patch-ctest.txt" );
  
    // Build amx from data from 'fix'. We need to
    // do this as simfix is not derived from channel_system.
    BOOST_CHECK_NO_THROW( icceval.create_amx( fix.zl1(), fix.rl1(), fix.rlvest() ) );
    BOOST_CHECK_NO_THROW( icceval.write_data() );
    std::cout << "C-area " << icceval.surface_area() << "\n";
  
    // Check patches are on surface.
    for( std::size_t idx = 0ul; idx != icceval.size(); ++idx )
    {
      geometry::coordinate p,u;
      icceval.get_patch_coordinate( idx, p, u );
      BOOST_CHECK( fix.on_surface( p.x, p.y, p.z ) );
    }
  
    // We assume this simple size test means that the surface grid and the A
    // matrix are correctly built because independent tests of the surface grid
    // and A matrix classes produce the correct results.
    BOOST_CHECK_EQUAL( icceval.size(), fix.expected_grid_size() );
  }
  {
    // Construction as if from an input file with no parameters
    boost::shared_ptr< evaluator::base_evaluator > eval;
  
    // Empty parameter list allowed
    BOOST_REQUIRE_NO_THROW( eval = evaluator::induced_charge::make_evaluator( evaluator_test::mockup_params() ) );
  
    evaluator::induced_charge const* llp = dynamic_cast< evaluator::induced_charge const* >( eval.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->get_dxf(), 1.6);
  
    // write_document test
    core::input_document wr( 1 );
    eval->write_document( wr );
    BOOST_CHECK_EQUAL( wr.size(), 1  );
    BOOST_CHECK_EQUAL( wr[ 0 ].label(), core::strngs::evaluator_label() );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fstype() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fstype() ), core::strngs::fsptch() );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fsdxw() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fsdxw() ), std::to_string( 1.6 ) );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fsdxf() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fsdxf() ), std::to_string( 1.6 ) );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fsnsub() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fsnsub() ), std::to_string( 10 ) );
  }
  {
    // Construction as if from an input file with valid parameters
    boost::shared_ptr< evaluator::base_evaluator > eval;
  
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxf() );
    params.back().set_value( "4.0" );
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxw() );
    params.back().set_value( "6.0" );
    params.push_back( {} );
    params.back().set_name( core::strngs::fsnsub() );
    params.back().set_value( "50" );
    params.push_back( evaluator_test::mockup_params().back() );
  
    // Empty parameter list allowed
    BOOST_REQUIRE_NO_THROW( eval = evaluator::induced_charge::make_evaluator( params ) );
  
    evaluator::induced_charge const* llp = dynamic_cast< evaluator::induced_charge const* >( eval.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->get_dxf(), 4.0 );
    BOOST_CHECK_EQUAL( llp->get_dxw(), 6.0 );
    BOOST_CHECK_EQUAL( llp->get_nsub0(), 50 );
  }
  {
    // Construction as if from an input file with valid parameters
    boost::shared_ptr< evaluator::base_evaluator > eval;
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxf() );
    params.back().set_value( "4.0" );
    params.push_back( evaluator_test::mockup_params().back() );
    BOOST_REQUIRE_NO_THROW( eval = evaluator::induced_charge::make_evaluator( params ) );
  
    evaluator::induced_charge const* llp = dynamic_cast< evaluator::induced_charge const* >( eval.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->get_dxf(), 4.0 );
  }
  {
    // Construction as if from an input file with invalid parameters : invalid value
    boost::shared_ptr< evaluator::base_evaluator > eval;
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxf() );
    params.back().set_value( "four" );
    params.push_back( evaluator_test::mockup_params().back() );
    BOOST_REQUIRE_THROW( eval = evaluator::induced_charge::make_evaluator( params ), core::input_error );
  }
  {
    // Construction as if from an input file with invalid parameters : invalid name
    boost::shared_ptr< evaluator::base_evaluator > eval;
   
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxf() );
    params.back().set_value( "-4" );
    params.push_back( evaluator_test::mockup_params().back() );
   
    BOOST_REQUIRE_THROW( eval = evaluator::induced_charge::make_evaluator( params ), core::input_error );
  }
  {
    // Construction as if from an input file with invalid parameters we would
    // like to be valid
    boost::shared_ptr< evaluator::base_evaluator > eval;
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( evaluator::induced_charge::amx_file_label() );
    params.back().set_value( "amx.txt" );
    params.push_back( {} );
    params.back().set_name( evaluator::induced_charge::patch_file_label() );
    params.push_back( evaluator_test::mockup_params().back() );
    BOOST_REQUIRE_THROW( eval = evaluator::induced_charge::make_evaluator( params ), core::input_error );
  }

}

// Test conversion of all input file parameters from valid input.
BOOST_AUTO_TEST_CASE( input_induced_charge_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf 0.5\ndxw 0.25\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\npatch_file patch.dat\namx_file \"amx.dat\"\nend\n\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
  
    BOOST_REQUIRE_EQUAL( mngr->size(), 1ul );
    BOOST_REQUIRE_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "patch" );
    const evaluator::induced_charge* icc = dynamic_cast< const evaluator::induced_charge* >( &mngr->get_evaluators()[ 0 ] );
    BOOST_REQUIRE( icc != nullptr ); 
    BOOST_CHECK_EQUAL( icc->get_dxf(), 0.5 );
    BOOST_CHECK_EQUAL( icc->get_dxw(), 0.25 );
    BOOST_CHECK_EQUAL( icc->get_nsub0(), 20 );
    BOOST_CHECK_EQUAL( icc->get_protein_permittivity(), 3.0 );
    BOOST_CHECK_EQUAL( icc->get_protein_radius(), 30.0 );
    BOOST_CHECK_EQUAL( icc->get_membrane_arc_radius(), 10.0 );
    BOOST_CHECK_EQUAL( icc->get_patch_filename(), "patch.dat" );
    BOOST_CHECK_EQUAL( icc->get_a_matrix_filename(), "amx.dat" );
    
  }catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_unknown_parameter_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf 0.5\ndxw 0.3\nmobk \"Na\"\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Unknown parameter \"mobk\" in section \"evaluator\" subtype \"patch\" detected at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n** Parameter \"mobk\" is not expected in this section and type. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_amx_file_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf 0.5\ndxw 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\namx_file \nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Patch file path parameter \"amx_file\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 9.\n   >>amx_file <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_amx_file_empty_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf 0.5\ndxw 0.3\namx_file \"\"\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Patch file path parameter \"amx_file\" with value (\"\") in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>amx_file \"\"<<\n** Expected a non-empty value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_patch_file_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf 0.5\ndxw 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\npatch_file \nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Patch file path parameter \"patch_file\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 9.\n   >>patch_file <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_patch_file_empty_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf 0.5\ndxw 0.3\npatch_file \"\"\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Patch file path parameter \"patch_file\" with value (\"\") in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>patch_file \"\"<<\n** Expected a non-empty value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxf_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf\ndxw 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Angular integration factor parameter \"dxf\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxf<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxf_empty_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf \"\"\ndxw 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Angular integration factor parameter \"dxf\" with value (\"\") in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxf \"\"<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxf_bad_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf three\ndxw 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Angular integration factor parameter \"dxf\" with value (three) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxf three<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxf_zero_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf 0.0\ndxw 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Angular integration factor parameter \"dxf\" with value (0.0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxf 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxf_negative_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxf -0.5\ndxw 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Angular integration factor parameter \"dxf\" with value (-0.5) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxf -0.5<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxw_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Axial integration factor parameter \"dxw\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxw<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxw_empty_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw \"\"\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Axial integration factor parameter \"dxw\" with value (\"\") in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxw \"\"<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxw_bad_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw three\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Axial integration factor parameter \"dxw\" with value (three) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxw three<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxw_zero_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.0\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Axial integration factor parameter \"dxw\" with value (0.0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxw 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_dxw_negative_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw -0.5\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Axial integration factor parameter \"dxw\" with value (-0.5) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 3.\n   >>dxw -0.5<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_epspr_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein relative permittivity parameter \"epspr\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>epspr<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_epspr_empty_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr \"\"\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein relative permittivity parameter \"epspr\" with value (\"\") in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>epspr \"\"<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_epspr_bad_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr three\nrl4 30.0\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein relative permittivity parameter \"epspr\" with value (three) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>epspr three<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_epspr_zero_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 0.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein relative permittivity parameter \"epspr\" with value (0.0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>epspr 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_epspr_negative_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.5\ndxf 0.3\nnsub 20\nepspr -3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein relative permittivity parameter \"epspr\" with value (-3.0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 6.\n   >>epspr -3.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rl4_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 \nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer radius parameter \"rl4\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>rl4 <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rl4_empty_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 \"\"\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer radius parameter \"rl4\" with value (\"\") in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>rl4 \"\"<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rl4_bad_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 ten\nrlcurv 10.0\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer radius parameter \"rl4\" with value (ten) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>rl4 ten<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rl4_zero_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 0.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer radius parameter \"rl4\" with value (0.0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>rl4 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rl4_negative_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.5\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 -30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer radius parameter \"rl4\" with value (-30.0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 7.\n   >>rl4 -30.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rlcurv_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv \n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer arc radius parameter \"rlcurv\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>rlcurv <<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rlcurv_empty_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv \"\"\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer arc radius parameter \"rlcurv\" with value (\"\") in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>rlcurv \"\"<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rlcurv_bad_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv ten\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer arc radius parameter \"rlcurv\" with value (ten) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>rlcurv ten<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rlcurv_zero_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 0.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer arc radius parameter \"rlcurv\" with value (0.0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>rlcurv 0.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_rlcurv_negative_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.5\ndxf 0.3\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv -10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Protein outer arc radius parameter \"rlcurv\" with value (-10.0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 8.\n   >>rlcurv -10.0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_nsub_no_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Subpatch integration factor parameter \"nsub\" in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>nsub<<\n** Expected a value. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_nsub_empty_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub \"\"\nepspr 3.0\nrl4 30.0\nrlcurv 12\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Subpatch integration factor parameter \"nsub\" with value (\"\") in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>nsub \"\"<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_nsub_bad_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub three\nepspr 3.0\nrl4 30.0\nrlcurv 10\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Subpatch integration factor parameter \"nsub\" with value (three) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>nsub three<<\n** A numeric value was expected. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_nsub_zero_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.3\ndxf 0.3\nnsub 0\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Subpatch integration factor parameter \"nsub\" with value (0) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>nsub 0<<\n** Input value must be greater than zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( input_induced_charge_nsub_negative_value_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::induced_charge::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"patch\"\ndxw 0.5\ndxf 0.3\nnsub -20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\n\nend\n\n" );
  
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
    BOOST_CHECK( msg.find( "Subpatch integration factor parameter \"nsub\" with value (-20) in section \"evaluator\" at input file \"" + boost::filesystem::current_path().native() + "/dummy\" line 5.\n   >>nsub -20<<\n** Input value must be greater than or equal to zero. **" ) < msg.size() );
  }
  catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "exception thrown by \"dg.read_input( reader )\" was not expected type: " ) + err.what() );
  }

}

BOOST_AUTO_TEST_CASE( legendre_test )
{
  //XX// In order to test the ICC code an analytically tractable problem
  //XX// is a single charge near a sphere.
  //XX//
  //XX// CHARGE INSIDE
  //XX//
  //XX//std::stringstream store;
  //XX{
  //XX   // default
  //XX   platform::simulator_fixture simfix;
  //XX   std::map< std::string, std::string > params;
  //XX   const double inside_eps{ 10.0 };
  //XX   const double outside_eps{ 80.0 };
  //XX   const double temperature{ 300.0 };
  //XX   const double ion_charge{ 1.0 };
  //XX   const double dxf{ 1.6 };
  //XX   const double dxw{ 1.6 };
  //XX   const std::size_t nsub{ 20 };
  //XX   const double sphere_radius{ 10.0 };
  //XX
  //XX   // Empty parameter list allowed
  //XX   BOOST_REQUIRE_NO_THROW( evaluator::induced_charge::make_evaluator( params, *simfix.sim ) );
  //XX
  //XX   simfix.sim->set_solvent_permittivity( outside_eps );
  //XX
  //XX   geometry::coordinate external_coord{ 0.0, 0.0, 0.0 };
  //XX   // Add specie and particle to simulation
  //XX   {
  //XX      particle::specie spc;
  //XX      spc.set_label( "Na" );
  //XX      spc.set_valency( ion_charge );
  //XX      spc.set_radius( 1.0 );
  //XX      spc.set_rate( 1.0 );
  //XX      spc.set_excess_potential( 0.0 );
  //XX      spc.set_concentration( 0.055 );
  //XX      spc.set_type( particle::specie::SOLUTE );
  //XX      spc.append_position( external_coord );
  //XX      spc.set_count( 1 );
  //XX      simfix.sim->add_specie( spc );
  //XX   }
  //XX
  //XX   simfix.sim->generate_simulation( std::cout );
  //XX
  //XX   BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().size(), 1 );
  //XX   BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().x( 0 ), external_coord.x );
  //XX   BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().y( 0 ), external_coord.y );
  //XX   BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().z( 0 ), external_coord.z );
  //XX   BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().eps( 0 ), outside_eps );
  //XX   const_cast< particle::ensemble& >( simfix.sim->get_ensemble() ).set_eps( 0, inside_eps );
  //XX   BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().eps( 0 ), inside_eps );
  //XX
  //XX   std::unique_ptr< evaluator::icc_surface_grid > gridder( new evaluator::icc_surface_grid );
  //XX
  //XX   // Create a sphere.
  //XX   gridder->set_dxf( dxf );
  //XX   gridder->set_dxw( dxw );
  //XX   gridder->set_nsub0( nsub );
  //XX
  //XX   gridder->add_arc( 0.0, 0.0, sphere_radius, 0.0, core::constants::pi()/2, inside_eps, outside_eps, 10, 10, true, std::cout );
  //XX   gridder->add_arc( 0.0, 0.0, sphere_radius,  core::constants::pi()/2, core::constants::pi(), inside_eps, outside_eps, 10, 10, true, std::cout );
  //XX
  //XX   {
  //XX      std::ofstream circle( "circle.txt" );
  //XX      gridder->write_grid( circle );
  //XX   }
  //XX   const std::size_t gsize = gridder->size();
  //XX   std::cout << " Number of tiles : " << gsize << "\n";
  //XX
  //XX   evaluator::base_evaluator const* bep( &(*simfix.sim->get_evaluators().begin()) );
  //XX   evaluator::induced_charge const* llp = dynamic_cast< evaluator::induced_charge const* >( bep );
  //XX   BOOST_REQUIRE( llp != nullptr );
  //XX
  //XX   // Swap test grid into evaluator.
  //XX   const_cast< evaluator::induced_charge* >(llp)->create_amx( gridder, std::cout );
  //XX
  //XX   BOOST_REQUIRE_EQUAL( gsize, llp->size() );
  //XX
  //XX   const_cast< evaluator::induced_charge* >(llp)->set_alfa( temperature );
  //XX   const_cast< evaluator::induced_charge* >(llp)->set_permittivity( outside_eps );
  //XX
  //XX   // Calculate the initial induced charge
  //XX   const_cast< evaluator::induced_charge* >(llp)->compute_initial_c_h( simfix.sim->get_species(), simfix.sim->get_ensemble() );
  //XX
  //XX   {
  //XX      std::ofstream chrg( "charge_in.txt" );
  //XX      chrg << "# tiles : " << llp->size() << "\n";
  //XX      chrg << "# external charge : " << ion_charge << "\n";
  //XX      chrg << "# external location : " << external_coord << "\n";
  //XX      for( std::size_t ii = 0; ii != llp->size(); ++ii)
  //XX      {
  //XX         chrg << llp->surface_charge( ii ) <<  " " << llp->surface_area( ii ) <<  " "  << llp->field( ii ) <<  "\n";
  //XX      }
  //XX   }
  //XX
  //XX   {
  //XX      double area, chrg;
  //XX      llp->compute_total_surface_charge( chrg, area );
  //XX
  //XX      std::cout << "Total surface area = " << area << "\n";
  //XX      BOOST_CHECK_CLOSE_FRACTION( area, (4*core::constants::pi()*sphere_radius*sphere_radius), 0.0001 );
  //XX      std::cout << "Change in eps = " << (inside_eps - outside_eps) << "\n";
  //XX      std::cout << "Average eps = " << ((inside_eps + outside_eps)/2) << "\n";
  //XX      std::cout << "Ratio 1 delta_e' = " << (2*(inside_eps - outside_eps)/(inside_eps + outside_eps)) << "\n";
  //XX      std::cout << "Ratio 2 (e_i.e_o)/(e_i - e_o) = " << (inside_eps * outside_eps/(inside_eps - outside_eps)) << "\n";
  //XX      std::cout << "Total surface charge = " << chrg << "\n";
  //XX      chrg *= inside_eps * outside_eps/(inside_eps - outside_eps);
  //XX      std::cout << "Estimate of original charge from surface charge = " << chrg << "\n";
  //XX      BOOST_CHECK_CLOSE_FRACTION( chrg, ion_charge, 0.01 );
  //XX
  //XX      //llp->compute_inner_surface_charge( chrg, area );
  //XX      //BOOST_CHECK_CLOSE_FRACTION( chrg, ion_charge, 0.001 );
  //XX      //std::cout << "Inner surface charge = " << chrg << "\n";
  //XX   }
  //XX}
  //XX//XX  //
  //XX//XX  //  CHARGE OUTSIDE
  //XX//XX  //
  //XX//XX  {
  //XX//XX     // default
  //XX//XX     platform::simulator_fixture simfix;
  //XX//XX     std::map< std::string, std::string > params;
  //XX//XX     // Empty parameter list allowed
  //XX//XX     BOOST_REQUIRE_NO_THROW( evaluator::induced_charge::make_evaluator( params, *simfix.sim ) );
  //XX//XX
  //XX//XX     geometry::coordinate external_coord{ 0.0, 0.0, 15.0 };
  //XX//XX     // Add specie and particle to simulation
  //XX//XX     {
  //XX//XX        particle::specie spc;
  //XX//XX        spc.set_label( "Na" );
  //XX//XX        spc.set_valency( 1.0 );
  //XX//XX        spc.set_radius( 1.0 );
  //XX//XX        spc.set_rate( 1.0 );
  //XX//XX        spc.set_excess_potential( 0.0 );
  //XX//XX        spc.set_concentration( 0.055 );
  //XX//XX        spc.set_type( particle::specie::SOLUTE );
  //XX//XX        spc.append_position( external_coord );
  //XX//XX        spc.set_count( 1 );
  //XX//XX        simfix.sim->add_specie( spc );
  //XX//XX     }
  //XX//XX
  //XX//XX     simfix.sim->generate_simulation( std::cout );
  //XX//XX
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().size(), 1 );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().x( 0 ), external_coord.x );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().y( 0 ), external_coord.y );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().z( 0 ), external_coord.z );
  //XX//XX
  //XX//XX     std::unique_ptr< evaluator::icc_surface_grid > gridder( new evaluator::icc_surface_grid );
  //XX//XX
  //XX//XX     // Create a sphere.
  //XX//XX     const double epsw = 80.0;
  //XX//XX     const double temperature = 300.0;
  //XX//XX     const double epspr = 10.0;
  //XX//XX     const double deps =  2 * (epspr - epsw) / (epspr + epsw );
  //XX//XX     gridder->add_arc( 0.0, 0.0, 10.0, 0.0, core::constants::pi()/2, deps, 10, 10, true, std::cout );
  //XX//XX     gridder->add_arc( 0.0, 0.0, 10.0,  core::constants::pi()/2, core::constants::pi(), deps, 10, 10, true, std::cout );
  //XX//XX
  //XX//XX     const std::size_t gsize = gridder->size();
  //XX//XX
  //XX//XX     evaluator::base_evaluator const* bep( &(*simfix.sim->get_evaluators().begin()) );
  //XX//XX     evaluator::induced_charge const* llp = dynamic_cast< evaluator::induced_charge const* >( bep );
  //XX//XX     BOOST_REQUIRE( llp != nullptr );
  //XX//XX
  //XX//XX     // Swap test grid into evaluator.
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->create_amx( gridder, std::cout );
  //XX//XX
  //XX//XX     BOOST_REQUIRE_EQUAL( gsize, llp->size() );
  //XX//XX
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->set_alfa( temperature );
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->set_permittivity( epsw );
  //XX//XX
  //XX//XX     // Calculate the initial induced charge
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->compute_initial_c_h( simfix.sim->get_species(), simfix.sim->get_ensemble() );
  //XX//XX
  //XX//XX     {
  //XX//XX        std::ofstream chrg( "charge_out.txt" );
  //XX//XX        chrg << "# tiles : " << llp->size() << "\n";
  //XX//XX        chrg << "# external charge : +1.0\n";
  //XX//XX        chrg << "# external location : " << external_coord << "\n";
  //XX//XX        for( std::size_t ii = 0; ii != llp->size(); ++ii)
  //XX//XX        {
  //XX//XX           chrg << llp->surface_charge( ii ) <<  " " << llp->surface_area( ii ) <<  " " << llp->field( ii ) <<  "\n";
  //XX//XX        }
  //XX//XX     }
  //XX//XX
  //XX//XX     {
  //XX//XX        double area, chrg;
  //XX//XX        llp->compute_total_surface_charge( chrg, area );
  //XX//XX        std::cout << "Total surface charge = " << chrg << "\n";
  //XX//XX        std::cout << "Total surface area = " << area << "\n";
  //XX//XX        BOOST_CHECK_CLOSE_FRACTION( chrg, 0.0, 0.0001 );
  //XX//XX        BOOST_CHECK_CLOSE_FRACTION( area, 4*core::constants::pi()*100.0, 0.0001 );
  //XX//XX     }
  //XX//XX  }
  //XX//XX  //
  //XX//XX  //  CHARGE AT CENTRE
  //XX//XX  //
  //XX//XX  {
  //XX//XX     // default
  //XX//XX     platform::simulator_fixture simfix;
  //XX//XX     std::map< std::string, std::string > params;
  //XX//XX     // Empty parameter list allowed
  //XX//XX     BOOST_REQUIRE_NO_THROW( evaluator::induced_charge::make_evaluator( params, *simfix.sim ) );
  //XX//XX
  //XX//XX     geometry::coordinate external_coord{ 0.0, 0.0, 0.0 };
  //XX//XX     // Add specie and particle to simulation
  //XX//XX     {
  //XX//XX        particle::specie spc;
  //XX//XX        spc.set_label( "Na" );
  //XX//XX        spc.set_valency( 1.0 );
  //XX//XX        spc.set_radius( 1.0 );
  //XX//XX        spc.set_rate( 1.0 );
  //XX//XX        spc.set_excess_potential( 0.0 );
  //XX//XX        spc.set_concentration( 0.055 );
  //XX//XX        spc.set_type( particle::specie::SOLUTE );
  //XX//XX        spc.append_position( external_coord );
  //XX//XX        spc.set_count( 1 );
  //XX//XX        simfix.sim->add_specie( spc );
  //XX//XX     }
  //XX//XX
  //XX//XX     simfix.sim->generate_simulation( std::cout );
  //XX//XX
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().size(), 1 );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().x( 0 ), external_coord.x );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().y( 0 ), external_coord.y );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().z( 0 ), external_coord.z );
  //XX//XX
  //XX//XX     std::unique_ptr< evaluator::icc_surface_grid > gridder( new evaluator::icc_surface_grid );
  //XX//XX
  //XX//XX     gridder->set_dxf( 1.0 );
  //XX//XX     gridder->set_dxw( 1.0 );
  //XX//XX     // Create a sphere.
  //XX//XX     const double epsw = 80.0;
  //XX//XX     const double temperature = 300.0;
  //XX//XX     const double epspr = 10.0;
  //XX//XX     const double deps =  2 * (epspr - epsw) / (epspr + epsw );
  //XX//XX     gridder->add_arc( 0.0, 0.0, 10.0, 0.0, core::constants::pi()/2, deps, 10, 10, true, std::cout );
  //XX//XX     gridder->add_arc( 0.0, 0.0, 10.0,  core::constants::pi()/2, core::constants::pi(), deps, 10, 10, true, std::cout );
  //XX//XX
  //XX//XX     const std::size_t gsize = gridder->size();
  //XX//XX
  //XX//XX     evaluator::base_evaluator const* bep( &(*simfix.sim->get_evaluators().begin()) );
  //XX//XX     evaluator::induced_charge const* llp = dynamic_cast< evaluator::induced_charge const* >( bep );
  //XX//XX     BOOST_REQUIRE( llp != nullptr );
  //XX//XX
  //XX//XX     // Swap test grid into evaluator.
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->create_amx( gridder, std::cout );
  //XX//XX
  //XX//XX     BOOST_REQUIRE_EQUAL( gsize, llp->size() );
  //XX//XX
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->set_alfa( temperature );
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->set_permittivity( epsw );
  //XX//XX
  //XX//XX     // Calculate the initial induced charge
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->compute_initial_c_h( simfix.sim->get_species(), simfix.sim->get_ensemble() );
  //XX//XX
  //XX//XX     {
  //XX//XX        std::ofstream chrg( "charge_ctr.txt" );
  //XX//XX        chrg << "# tiles : " << llp->size() << "\n";
  //XX//XX        chrg << "# external charge : +1.0\n";
  //XX//XX        chrg << "# external location : " << external_coord << "\n";
  //XX//XX        for( std::size_t ii = 0; ii != llp->size(); ++ii)
  //XX//XX        {
  //XX//XX           chrg << llp->surface_charge( ii ) <<  " " << llp->surface_area( ii ) <<  " " << llp->field( ii ) <<  "\n";
  //XX//XX        }
  //XX//XX     }
  //XX//XX
  //XX//XX     {
  //XX//XX        double area, chrg;
  //XX//XX        llp->compute_total_surface_charge( chrg, area );
  //XX//XX        std::cout << "Total surface charge = " << chrg << "\n";
  //XX//XX        std::cout << "Total surface area = " << area << "\n";
  //XX//XX        BOOST_CHECK_CLOSE_FRACTION( chrg, 0.0, 0.0001 );
  //XX//XX        BOOST_CHECK_CLOSE_FRACTION( area, 4*core::constants::pi()*100.0, 0.0001 );
  //XX//XX     }
  //XX//XX  }
  //XX//XX  //
  //XX//XX  //  CHARGE AWAY FROM WALL
  //XX//XX  //
  //XX//XX  {
  //XX//XX     // default
  //XX//XX     platform::simulator_fixture simfix;
  //XX//XX     std::map< std::string, std::string > params;
  //XX//XX     // Empty parameter list allowed
  //XX//XX     BOOST_REQUIRE_NO_THROW( evaluator::induced_charge::make_evaluator( params, *simfix.sim ) );
  //XX//XX
  //XX//XX     geometry::coordinate external_coord{ 0.0, 0.0, 12.0 };
  //XX//XX     // Add specie and particle to simulation
  //XX//XX     {
  //XX//XX        particle::specie spc;
  //XX//XX        spc.set_label( "Na" );
  //XX//XX        spc.set_valency( 1.0 );
  //XX//XX        spc.set_radius( 1.0 );
  //XX//XX        spc.set_rate( 1.0 );
  //XX//XX        spc.set_excess_potential( 0.0 );
  //XX//XX        spc.set_concentration( 0.055 );
  //XX//XX        spc.set_type( particle::specie::SOLUTE );
  //XX//XX        spc.append_position( external_coord );
  //XX//XX        spc.set_count( 1 );
  //XX//XX        simfix.sim->add_specie( spc );
  //XX//XX     }
  //XX//XX
  //XX//XX     simfix.sim->generate_simulation( std::cout );
  //XX//XX
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().size(), 1 );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().x( 0 ), external_coord.x );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().y( 0 ), external_coord.y );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().z( 0 ), external_coord.z );
  //XX//XX
  //XX//XX     std::unique_ptr< evaluator::icc_surface_grid > gridder( new evaluator::icc_surface_grid );
  //XX//XX
  //XX//XX     // Create a sphere.
  //XX//XX     const double epsw = 80.0;
  //XX//XX     const double temperature = 300.0;
  //XX//XX     const double epspr = 10.0;
  //XX//XX     const double deps =  2 * (epspr - epsw) / (epspr + epsw );
  //XX//XX     gridder->add_wall( 0.0, 0.0, 30.0, deps, 10, 10, true, std::cout );
  //XX//XX
  //XX//XX     {
  //XX//XX        std::ofstream gridos( "wall.txt" );
  //XX//XX        gridder->write_grid( gridos );
  //XX//XX     }
  //XX//XX
  //XX//XX     const std::size_t gsize = gridder->size();
  //XX//XX
  //XX//XX     evaluator::base_evaluator const* bep( &(*simfix.sim->get_evaluators().begin()) );
  //XX//XX     evaluator::induced_charge const* llp = dynamic_cast< evaluator::induced_charge const* >( bep );
  //XX//XX     BOOST_REQUIRE( llp != nullptr );
  //XX//XX
  //XX//XX     // Swap test grid into evaluator.
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->create_amx( gridder, std::cout );
  //XX//XX
  //XX//XX     BOOST_REQUIRE_EQUAL( gsize, llp->size() );
  //XX//XX
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->set_alfa( temperature );
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->set_permittivity( epsw );
  //XX//XX
  //XX//XX     // Calculate the initial induced charge
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->compute_initial_c_h( simfix.sim->get_species(), simfix.sim->get_ensemble() );
  //XX//XX
  //XX//XX     {
  //XX//XX        std::ofstream chrg( "charge_wall.txt" );
  //XX//XX        chrg << "# tiles : " << llp->size() << "\n";
  //XX//XX        chrg << "# external charge : +1.0\n";
  //XX//XX        chrg << "# external location : " << external_coord << "\n";
  //XX//XX        for( std::size_t ii = 0; ii != llp->size(); ++ii)
  //XX//XX        {
  //XX//XX           chrg << llp->surface_charge( ii ) <<  " " << llp->surface_area( ii ) <<  " " << llp->field( ii ) <<  "\n";
  //XX//XX        }
  //XX//XX     }
  //XX//XX
  //XX//XX     {
  //XX//XX        double area, chrg;
  //XX//XX        llp->compute_total_surface_charge( chrg, area );
  //XX//XX        std::cout << "Total surface charge = " << chrg << "\n";
  //XX//XX        std::cout << "Total surface area = " << area << "\n";
  //XX//XX        BOOST_CHECK_CLOSE_FRACTION( chrg, 0.0, 0.0001 );
  //XX//XX        BOOST_CHECK_CLOSE_FRACTION( area, core::constants::pi()*900.0, 0.0001 );
  //XX//XX     }
  //XX//XX  }
  //XX//XX  //
  //XX//XX  //  CHARGE AT CENTRE OF CYLINDER
  //XX//XX  //
  //XX//XX  {
  //XX//XX     // default
  //XX//XX     platform::simulator_fixture simfix;
  //XX//XX     std::map< std::string, std::string > params;
  //XX//XX     // Empty parameter list allowed
  //XX//XX     BOOST_REQUIRE_NO_THROW( evaluator::induced_charge::make_evaluator( params, *simfix.sim ) );
  //XX//XX
  //XX//XX     geometry::coordinate external_coord{ 0.0, 0.0, 0.0 };
  //XX//XX     // Add specie and particle to simulation
  //XX//XX     {
  //XX//XX        particle::specie spc;
  //XX//XX        spc.set_label( "Na" );
  //XX//XX        spc.set_valency( 1.0 );
  //XX//XX        spc.set_radius( 1.0 );
  //XX//XX        spc.set_rate( 1.0 );
  //XX//XX        spc.set_excess_potential( 0.0 );
  //XX//XX        spc.set_concentration( 0.055 );
  //XX//XX        spc.set_type( particle::specie::SOLUTE );
  //XX//XX        spc.append_position( external_coord );
  //XX//XX        spc.set_count( 1 );
  //XX//XX        simfix.sim->add_specie( spc );
  //XX//XX     }
  //XX//XX
  //XX//XX     simfix.sim->generate_simulation( std::cout );
  //XX//XX
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().size(), 1 );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().x( 0 ), external_coord.x );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().y( 0 ), external_coord.y );
  //XX//XX     BOOST_REQUIRE_EQUAL( simfix.sim->get_ensemble().z( 0 ), external_coord.z );
  //XX//XX
  //XX//XX     std::unique_ptr< evaluator::icc_surface_grid > gridder( new evaluator::icc_surface_grid );
  //XX//XX
  //XX//XX     // Create a sphere.
  //XX//XX     const double epsw = 80.0;
  //XX//XX     const double temperature = 300.0;
  //XX//XX     const double epspr = 10.0;
  //XX//XX     const double deps =  2 * (epspr - epsw) / (epspr + epsw );
  //XX//XX     gridder->add_line( -20.0, 20.0, 10.0, deps, 10, 10, true, std::cout );
  //XX//XX
  //XX//XX     {
  //XX//XX        std::ofstream gridos( "cylinder.txt" );
  //XX//XX        gridder->write_grid( gridos );
  //XX//XX     }
  //XX//XX
  //XX//XX     const std::size_t gsize = gridder->size();
  //XX//XX
  //XX//XX     evaluator::base_evaluator const* bep( &(*simfix.sim->get_evaluators().begin()) );
  //XX//XX     evaluator::induced_charge const* llp = dynamic_cast< evaluator::induced_charge const* >( bep );
  //XX//XX     BOOST_REQUIRE( llp != nullptr );
  //XX//XX
  //XX//XX     // Swap test grid into evaluator.
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->create_amx( gridder, std::cout );
  //XX//XX
  //XX//XX     BOOST_REQUIRE_EQUAL( gsize, llp->size() );
  //XX//XX
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->set_alfa( temperature );
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->set_permittivity( epsw );
  //XX//XX
  //XX//XX     // Calculate the initial induced charge
  //XX//XX     const_cast< evaluator::induced_charge* >(llp)->compute_initial_c_h( simfix.sim->get_species(), simfix.sim->get_ensemble() );
  //XX//XX
  //XX//XX     {
  //XX//XX        std::ofstream chrg( "charge_cyl.txt" );
  //XX//XX        chrg << "# tiles : " << llp->size() << "\n";
  //XX//XX        chrg << "# external charge : +1.0\n";
  //XX//XX        chrg << "# external location : " << external_coord << "\n";
  //XX//XX        for( std::size_t ii = 0; ii != llp->size(); ++ii)
  //XX//XX        {
  //XX//XX           chrg << llp->surface_charge( ii ) <<  " " << llp->surface_area( ii ) <<  " " << llp->field( ii ) <<  "\n";
  //XX//XX        }
  //XX//XX     }
  //XX//XX
  //XX//XX     {
  //XX//XX        double area, chrg;
  //XX//XX        llp->compute_total_surface_charge( chrg, area );
  //XX//XX        std::cout << "Total surface charge = " << chrg << "\n";
  //XX//XX        std::cout << "Total surface area = " << area << "\n";
  //XX//XX        BOOST_CHECK_CLOSE_FRACTION( chrg, 0.0, 0.0001 );
  //XX//XX        BOOST_CHECK_CLOSE_FRACTION( area, 2*core::constants::pi()*40.0*10, 0.0001 );
  //XX//XX     }
  //XX//XX  }

}

BOOST_AUTO_TEST_CASE( induced_charge_f_test )
{
  evaluator::patch_matrix_fixture fix;
  // Manual build like fortran test
  {
    // default
    evaluator::induced_charge icceval;
    evaluator::patch_matrix_fixture simfix;
  
    // Set parameters as per 'fix'.
    BOOST_CHECK_NO_THROW( icceval.set_dxf( fix.dxf() ) );
    BOOST_CHECK_NO_THROW( icceval.set_dxw( fix.dxw() ) );
    BOOST_CHECK_NO_THROW( icceval.set_nsub0( fix.nsub() ) );
    BOOST_CHECK_NO_THROW( icceval.set_alfa( fix.temperature() ) );
    BOOST_CHECK_NO_THROW( icceval.set_permittivity( fix.epsw() ) );
    BOOST_CHECK_EQUAL( icceval.get_alfa(), fix.alfa() );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), fix.epsw() );
    BOOST_CHECK_NO_THROW( icceval.set_membrane_arc_radius( fix.rlmemb() ) );
    BOOST_CHECK_NO_THROW( icceval.set_protein_permittivity( fix.epspr() ) );
    BOOST_CHECK_NO_THROW( icceval.set_protein_radius( fix.rl4() ) );
    BOOST_CHECK_NO_THROW( icceval.set_a_matrix_filename( "amx-ctest.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_a_matrix_filename(), "amx-ctest.txt" );
  
    BOOST_CHECK_NO_THROW( icceval.set_patch_filename( "patch-ctest.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_patch_filename(), "patch-ctest.txt" );
  
    // Build amx from data from 'fix'. We need to
    // do this as simfix is not derived from channel_system.
    BOOST_CHECK_NO_THROW( icceval.create_amx_f( fix.zl1(), fix.rl1(), fix.rlvest() ) );
    BOOST_CHECK_NO_THROW( icceval.write_data() );
    std::cout << "C-area " << icceval.surface_area() << " >< " << fix.surface_area() << "\n";
    BOOST_CHECK_CLOSE( icceval.surface_area(), fix.surface_area(), 0.00001 );
  
    // Check patches are on surface.
    for( std::size_t idx = 0ul; idx != icceval.size(); ++idx )
    {
      geometry::coordinate p,u;
      icceval.get_patch_coordinate( idx, p, u );
      BOOST_CHECK( fix.on_surface( p.x, p.y, p.z ) );
      if( not fix.normal_test( p, u ) )
      {
        std::cout << idx << " P " << p << " U " << u << "\n";
      }
      //BOOST_CHECK( fix.normal_test( p, u ) );
    }
    {
      // Build basic system.
      particle::particle_manager pman;
      icceval.compute_initial_c_h( pman );
      // No charges in system.
      // ---------------------
      std::cout << " gaussbox 0 " << icceval.gaussbox() << "\n";
      // Two opposite charges inside channel.
      // ------------------------------------
      {
        particle::specie spc1;
        spc1.set_label( "Na" );
        spc1.set_concentration( 1.0 );
        spc1.set_radius( 0.12 );
        spc1.set_rate( 0.1 );
        spc1.set_valency( 1.0 );
        spc1.set_excess_potential( 0.123 );
        spc1.set_type( particle::specie::SOLUTE );
        spc1.append_position( geometry::coordinate( 0.0, 0.0, -3.0 ) );
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
        spc1.append_position( geometry::coordinate( 0.0, 0.0, 3.0 ) );
        pman.add_specie( spc1 );
      }
      pman.add_predefined_particles();
      BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
      BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 1 " << icceval.gaussbox() << "\n";
    }
    {
      // negative charge in channel, positive charge inside membrane.
      // ------------------------------------------------------------
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
        spc1.append_position( geometry::coordinate( 0.0, 13.0, -3.0 ) );
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
        spc1.append_position( geometry::coordinate( 0.0, 0.0, 3.0 ) );
        pman.add_specie( spc1 );
      }
      pman.add_predefined_particles();
      BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
      BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 2 " << icceval.gaussbox() << "\n";
    }
    {
      // positive and negative charge in membrane.
      // -----------------------------------------
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
        spc1.append_position( geometry::coordinate( 0.0, 13.0, -3.0 ) );
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
        spc1.append_position( geometry::coordinate( 0.0, -13.0, 3.0 ) );
        pman.add_specie( spc1 );
      }
      pman.add_predefined_particles();
      BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
      BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 3 " << icceval.gaussbox() << "\n";
    }
    {
      // double negative in channel, positive charge inside membrane.
      // ------------------------------------------------------------
      particle::particle_manager pman;
      {
        particle::specie spc1;
        spc1.set_label( "Na" );
        spc1.set_concentration( 1.0 );
        spc1.set_radius( 0.12 );
        spc1.set_rate( 0.1 );
        spc1.set_valency( 2.0 );
        spc1.set_excess_potential( 0.123 );
        spc1.set_type( particle::specie::SOLUTE );
        spc1.append_position( geometry::coordinate( 0.0, 13.0, -3.0 ) );
        pman.add_specie( spc1 );
      }
      {
        particle::specie spc1;
        spc1.set_label( "Cl" );
        spc1.set_concentration( 1.0 );
        spc1.set_radius( 0.2 );
        spc1.set_rate( 0.1 );
        spc1.set_valency( -2.0 );
        spc1.set_excess_potential( 0.3123 );
        spc1.set_type( particle::specie::SOLUTE );
        spc1.append_position( geometry::coordinate( 0.0, 0.0, 3.0 ) );
        pman.add_specie( spc1 );
      }
      pman.add_predefined_particles();
      BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
      BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 2x2 " << icceval.gaussbox() << "\n";
    }
    {
      // positive charge in channel, negative charge inside membrane.
      // ------------------------------------------------------------
      particle::particle_manager pman;
      {
        particle::specie spc1;
        spc1.set_label( "F" );
        spc1.set_concentration( 1.0 );
        spc1.set_radius( 0.12 );
        spc1.set_rate( 0.1 );
        spc1.set_valency( -1.0 );
        spc1.set_excess_potential( 0.123 );
        spc1.set_type( particle::specie::SOLUTE );
        spc1.append_position( geometry::coordinate( 0.0, 13.0, -3.0 ) );
        pman.add_specie( spc1 );
      }
      {
        particle::specie spc1;
        spc1.set_label( "K" );
        spc1.set_concentration( 1.0 );
        spc1.set_radius( 0.2 );
        spc1.set_rate( 0.1 );
        spc1.set_valency( 1.0 );
        spc1.set_excess_potential( 0.3123 );
        spc1.set_type( particle::specie::SOLUTE );
        spc1.append_position( geometry::coordinate( 0.0, 0.0, 3.0 ) );
        pman.add_specie( spc1 );
      }
      pman.add_predefined_particles();
      BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
      BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 5 " << icceval.gaussbox() << "\n";
    }
  
    // We assume this simple size test means that the surface grid
    // and the A matrix are correctly built because independent
    // tests of the surface grid and A matrix classes produce
    // the correct results.
    BOOST_CHECK_EQUAL( icceval.size(), fix.expected_grid_size() );
  }

}

BOOST_AUTO_TEST_CASE( icc_fortran_test )
{
  evaluator::patch_matrix_fixture fix;
  //std::map< std::string, std::string > params;
  std::stringstream store;
  std::stringstream log;
  {
    //  Constructor tests
    BOOST_CHECK( std::is_default_constructible< evaluator::icc_fortran >::type {} );
    BOOST_CHECK( not std::is_copy_constructible< evaluator::icc_fortran >::type {} );
    BOOST_CHECK( not std::is_move_constructible< evaluator::icc_fortran >::type {} );
    BOOST_CHECK( ( not std::is_assignable< evaluator::icc_fortran, evaluator::icc_matrix >::type {} ) );
    BOOST_CHECK( std::has_virtual_destructor< evaluator::icc_fortran >::type {} );
  }
  {
    // static method test
    BOOST_CHECK_EQUAL( "fpatch", evaluator::icc_fortran::type_label_() );
  }
  // Constructor test method
  {
    // default
    evaluator::icc_fortran icceval;
  
    BOOST_CHECK_EQUAL( icceval.size(), 0 );
    BOOST_CHECK_EQUAL( icceval.empty(), true );
    BOOST_CHECK_EQUAL( icceval.type_label(), "fpatch" );
    BOOST_CHECK( icceval.get_a_matrix_filename().empty()  );
    BOOST_CHECK( icceval.get_patch_filename().empty() );
    BOOST_CHECK_EQUAL( icceval.get_dxf(), 1.6 );
    BOOST_CHECK_EQUAL( icceval.get_dxw(), 1.6 );
    BOOST_CHECK_EQUAL( icceval.get_membrane_arc_radius(), 0.0 );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), 0.0 );
    BOOST_CHECK_EQUAL( icceval.get_protein_permittivity(), 10.0 );
    BOOST_CHECK_EQUAL( icceval.get_protein_radius(), 0.0 );
  
    BOOST_CHECK_NO_THROW( icceval.set_a_matrix_filename( "amx-test.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_a_matrix_filename(), "amx-test.txt" );
  
    BOOST_CHECK_NO_THROW( icceval.set_patch_filename( "patch-test.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_patch_filename(), "patch-test.txt" );
  
    BOOST_CHECK_NO_THROW( icceval.set_dxf( 1.25 ) );
    BOOST_CHECK_EQUAL( icceval.get_dxf(), 1.25 );
  
    BOOST_CHECK_NO_THROW( icceval.set_dxw( 3.5 ) );
    BOOST_CHECK_EQUAL( icceval.get_dxw(), 3.5 );
  
    BOOST_CHECK_NO_THROW( icceval.set_nsub0( 11 ) );
    BOOST_CHECK_EQUAL( icceval.get_nsub0(), 11 );
  
    BOOST_CHECK_NO_THROW( icceval.set_membrane_arc_radius( 12.25 ) );
    BOOST_CHECK_EQUAL( icceval.get_membrane_arc_radius(), 12.25 );
  
    BOOST_CHECK_NO_THROW( icceval.set_permittivity( 80.5 ) );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), 80.5 );
  
    BOOST_CHECK_NO_THROW( icceval.set_protein_permittivity( 8.125 ) );
    BOOST_CHECK_EQUAL( icceval.get_protein_permittivity(), 8.125 );
  
    BOOST_CHECK_NO_THROW( icceval.set_protein_radius( 32.5 ) );
    BOOST_CHECK_EQUAL( icceval.get_protein_radius(), 32.5 );
  
    BOOST_CHECK_EQUAL( icceval.size(), 0 );
    BOOST_CHECK_EQUAL( icceval.empty(), true );
  
    boost::archive::text_oarchive oa( store );
    oa << icceval;
  }
  // Serialization test method.
  {
    evaluator::icc_fortran icceval;
  
    boost::archive::text_iarchive ia( store );
    ia >> icceval;
  
    BOOST_CHECK_EQUAL( icceval.size(), 0 );
    BOOST_CHECK_EQUAL( icceval.empty(), true );
    BOOST_CHECK_EQUAL( icceval.get_a_matrix_filename(), "amx-test.txt" );
    BOOST_CHECK_EQUAL( icceval.get_patch_filename(), "patch-test.txt" );
    BOOST_CHECK_EQUAL( icceval.get_dxf(), 1.25 );
    BOOST_CHECK_EQUAL( icceval.get_dxw(), 3.5 );
    BOOST_CHECK_EQUAL( icceval.get_nsub0(), 11 );
    BOOST_CHECK_EQUAL( icceval.get_membrane_arc_radius(), 12.25 );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), 80.5 );
    BOOST_CHECK_EQUAL( icceval.get_protein_permittivity(), 8.125 );
    BOOST_CHECK_EQUAL( icceval.get_protein_radius(), 32.5 );
  }
  // Manual build test
  {
    // default
    evaluator::icc_fortran icceval;
    evaluator::patch_matrix_fixture simfix;
  
    // Set parameters as per 'fix'.
    BOOST_CHECK_NO_THROW( icceval.set_dxf( fix.dxf() ) );
    BOOST_CHECK_NO_THROW( icceval.set_dxw( fix.dxw() ) );
    BOOST_CHECK_NO_THROW( icceval.set_nsub0( fix.nsub() ) );
    BOOST_CHECK_NO_THROW( icceval.set_alfa( fix.temperature() ) );
    BOOST_CHECK_NO_THROW( icceval.set_permittivity( fix.epsw() ) );
    BOOST_CHECK_EQUAL( icceval.get_alfa(), fix.alfa() );
    BOOST_CHECK_EQUAL( icceval.get_permittivity(), fix.epsw() );
    BOOST_CHECK_NO_THROW( icceval.set_membrane_arc_radius( fix.rlmemb() ) );
    BOOST_CHECK_NO_THROW( icceval.set_protein_permittivity( fix.epspr() ) );
    BOOST_CHECK_NO_THROW( icceval.set_protein_radius( fix.rl4() ) );
    BOOST_CHECK_NO_THROW( icceval.set_a_matrix_filename( "amx-ftest.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_a_matrix_filename(), "amx-ftest.txt" );
  
    BOOST_CHECK_NO_THROW( icceval.set_patch_filename( "patch-ftest.txt" ) );
    BOOST_CHECK_EQUAL( icceval.get_patch_filename(), "patch-ftest.txt" );
  
    // Build amx from data from 'fix'. We need to
    // do this as simfix is not derived from channel_system.
    BOOST_CHECK_NO_THROW( icceval.create_amx( fix.zl1(), fix.rl1(), fix.rlvest() ) );
  
    // We assume this simple size test means that the surface grid and the A
    // matrix are correctly built because independent tests of the surface grid
    // and A matrix classes produce the correct results.
    BOOST_CHECK_EQUAL( icceval.size(), fix.expected_grid_size() );
    std::cout << "F-area " << icceval.surface_area() << " >< " << fix.surface_area() << "\n";
    BOOST_CHECK_CLOSE( icceval.surface_area(), fix.surface_area(), 0.00001 );
  
    // Check patches are on surface.
    for( std::size_t idx = 0ul; idx != icceval.size(); ++idx )
    {
      geometry::coordinate p,u;
      icceval.get_patch_coordinate( idx, p, u );
      BOOST_CHECK( fix.on_surface( p.x, p.y, p.z ) );
      BOOST_CHECK( fix.normal_test( p, u ) );
    }
    {
      // Build basic system.
      particle::particle_manager pman;
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 0 " << icceval.gaussbox() << "\n";
      {
        particle::specie spc1;
        spc1.set_label( "Na" );
        spc1.set_concentration( 1.0 );
        spc1.set_radius( 0.12 );
        spc1.set_rate( 0.1 );
        spc1.set_valency( 1.0 );
        spc1.set_excess_potential( 0.123 );
        spc1.set_type( particle::specie::SOLUTE );
        spc1.append_position( geometry::coordinate( 0.0, 0.0, -3.0 ) );
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
        spc1.append_position( geometry::coordinate( 0.0, 0.0, 3.0 ) );
        pman.add_specie( spc1 );
      }
      pman.add_predefined_particles();
      BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
      BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 1 " << icceval.gaussbox() << "\n";
    }
    {
      // positive charge in membrane.
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
        spc1.append_position( geometry::coordinate( 0.0, 13.0, -3.0 ) );
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
        spc1.append_position( geometry::coordinate( 0.0, 0.0, 3.0 ) );
        pman.add_specie( spc1 );
      }
      pman.add_predefined_particles();
      BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
      BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 2 " << icceval.gaussbox() << "\n";
    }
    {
      // positive and negative charge in membrane.
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
        spc1.append_position( geometry::coordinate( 0.0, 13.0, -3.0 ) );
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
        spc1.append_position( geometry::coordinate( 0.0, -13.0, 3.0 ) );
        pman.add_specie( spc1 );
      }
      pman.add_predefined_particles();
      BOOST_REQUIRE_EQUAL( pman.specie_count(), 2 );
      BOOST_REQUIRE_EQUAL( pman.get_ensemble().count(), 2 );
      icceval.compute_initial_c_h( pman );
      std::cout << " gaussbox 3 " << icceval.gaussbox() << "\n";
    }
  }
  {
    // Construction as if from an input file with no parameters
    boost::shared_ptr< evaluator::base_evaluator > eval;
    // Empty parameter list allowed
    BOOST_REQUIRE_NO_THROW( eval = evaluator::icc_fortran::make_evaluator( evaluator_test::mockup_params() ) );
  
    evaluator::icc_fortran const* llp = dynamic_cast< evaluator::icc_fortran const* >( eval.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->get_dxf(), 1.6);
  
    // write_document test
    core::input_document wr( 1 );
    eval->write_document( wr );
    BOOST_CHECK_EQUAL( wr.size(), 1  );
    BOOST_CHECK_EQUAL( wr[ 0 ].label(), core::strngs::evaluator_label() );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fstype() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fstype() ), "fpatch" );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fsdxw() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fsdxw() ), std::to_string( 1.6 ) );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fsdxf() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fsdxf() ), std::to_string( 1.6 ) );
    BOOST_CHECK( wr[ 0 ].has_entry( core::strngs::fsnsub() ) );
    BOOST_CHECK_EQUAL( wr[ 0 ].get_entry( core::strngs::fsnsub() ), std::to_string( 10 ) );
  }
  {
    // Construction as if from an input file with valid parameters
    boost::shared_ptr< evaluator::base_evaluator > eval;
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxf() );
    params.back().set_value( "4.0" );
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxw() );
    params.back().set_value( "6.0" );
    params.push_back( {} );
    params.back().set_name( core::strngs::fsnsub() );
    params.back().set_value( "50" );
    params.push_back( evaluator_test::mockup_params().back() );
  
   // Empty parameter list allowed
    BOOST_REQUIRE_NO_THROW( eval = evaluator::icc_fortran::make_evaluator( params ) );
  
    evaluator::icc_fortran const* llp = dynamic_cast< evaluator::icc_fortran const* >( eval.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->get_dxf(), 4.0 );
    BOOST_CHECK_EQUAL( llp->get_dxw(), 6.0 );
    BOOST_CHECK_EQUAL( llp->get_nsub0(), 50 );
  }
  {
    // Construction as if from an input file with valid parameters
    boost::shared_ptr< evaluator::base_evaluator > eval;
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxf() );
    params.back().set_value( "4.0" );
    params.push_back( evaluator_test::mockup_params().back() );
    BOOST_REQUIRE_NO_THROW( eval = evaluator::icc_fortran::make_evaluator( params ) );
  
    evaluator::icc_fortran const* llp = dynamic_cast< evaluator::icc_fortran const* >( eval.get() );
    BOOST_REQUIRE( nullptr != llp );
    BOOST_CHECK_EQUAL( llp->get_dxf(), 4.0 );
  }
  {
    // Construction as if from an input file with invalid parameters : invalid value
    boost::shared_ptr< evaluator::base_evaluator > eval;
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxf() );
    params.back().set_value( "four" );
    params.push_back( evaluator_test::mockup_params().back() );
    BOOST_REQUIRE_THROW( eval = evaluator::icc_fortran::make_evaluator( params ), core::input_error );
  }
  {
    // Construction as if from an input file with invalid parameters : invalid name
    boost::shared_ptr< evaluator::base_evaluator > eval;
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( core::strngs::fsdxf() );
    params.back().set_value( "-4" );
    params.push_back( evaluator_test::mockup_params().back() );
    BOOST_REQUIRE_THROW( eval = evaluator::icc_fortran::make_evaluator( params ), core::input_error );
  }
  {
    // Construction as if from an input file with invalid parameters we would
    // like to be valid
    boost::shared_ptr< evaluator::base_evaluator > eval;
    std::vector< core::input_parameter_memo > params;
    params.push_back( {} );
    params.back().set_name( evaluator::induced_charge::amx_file_label() );
    params.back().set_value( "amx.txt" );
    params.push_back( {} );
    params.back().set_name( evaluator::induced_charge::patch_file_label() );
    params.push_back( evaluator_test::mockup_params().back() );
    BOOST_REQUIRE_THROW( eval = evaluator::icc_fortran::make_evaluator( params ), core::input_error );
  }

}

// Test conversion of all input file parameters from valid input.
BOOST_AUTO_TEST_CASE( input_icc_fortran_test )
{
  // Test read input (specie meta)
  boost::shared_ptr< evaluator::evaluator_manager > mngr( new evaluator::evaluator_manager {} );
  core::input_delegater dg( 1 );
  {
    boost::shared_ptr< evaluator::evaluator_meta > m( new evaluator::evaluator_meta( mngr ) );
    evaluator::icc_fortran::add_definition( *m );
  
    dg.add_input_delegate( m );
  }
  
  std::string canon_input( "evaluator\ntype \"fpatch\"\ndxf 0.5\ndxw 0.25\nnsub 20\nepspr 3.0\nrl4 30.0\nrlcurv 10.0\npatch_file patch.dat\namx_file \"amx.dat\"\nend\n\n" );
  
  core::input_preprocess reader;
  reader.add_buffer( "dummy", canon_input );
  try
  {
    dg.read_input( reader );
  
    BOOST_REQUIRE_EQUAL( mngr->size(), 1ul );
    BOOST_REQUIRE_EQUAL( mngr->get_evaluators()[ 0 ].type_label(), "fpatch" );
    const evaluator::icc_fortran* icc = dynamic_cast< const evaluator::icc_fortran* >( &mngr->get_evaluators()[ 0 ] );
    BOOST_REQUIRE( icc != nullptr ); 
    BOOST_CHECK_EQUAL( icc->get_dxf(), 0.5 );
    BOOST_CHECK_EQUAL( icc->get_dxw(), 0.25 );
    BOOST_CHECK_EQUAL( icc->get_nsub0(), 20 );
    BOOST_CHECK_EQUAL( icc->get_protein_permittivity(), 3.0 );
    BOOST_CHECK_EQUAL( icc->get_protein_radius(), 30.0 );
    BOOST_CHECK_EQUAL( icc->get_membrane_arc_radius(), 10.0 );
    BOOST_CHECK_EQUAL( icc->get_patch_filename(), "patch.dat" );
    BOOST_CHECK_EQUAL( icc->get_a_matrix_filename(), "amx.dat" );
    
  }catch( std::exception const& err )
  {
    BOOST_ERROR( std::string( "Unexpected exception thrown by \"dg.read_input( reader )\": " ) + err.what() );
  }

}


} // namespace evaluator
