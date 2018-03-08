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

#define BOOST_TEST_MODULE geom_distribution_test
#include <boost/test/unit_test.hpp>

#include "geometry/geometry_test/geom_distribution_test.hpp"
#include "geometry/coordinate_set.hpp"
#include "geometry/cube_region.hpp"
#include "geometry/cylinder_region.hpp"
#include "utility/fuzzy_equals.hpp"
#include "geometry/geometry_manager.hpp"
#include "geometry/membrane_cell_inserter.hpp"
#include "geometry/membrane_cylinder_region.hpp"
#include "geometry/open_cylinder_region.hpp"
#include "geometry/open_split_cylinder_region.hpp"
#include "geometry/periodic_cube_region.hpp"
#include "geometry/tubular_grid.hpp"
#include "geometry/base_region.hpp"

// Manuals
#include "core/constants.hpp"
#include "core/strngs.hpp"
#include "geometry/digitizer_3d.hpp"
#include "utility/estimater.hpp"
#include "utility/histogram.hpp"
#include "utility/random.hpp"
//#include "utility/digitizer.hpp"
//#include "utility/utility.hpp"
// - 
//-
// Check if a region calculates the correct volume for
// itself both for the full volume and for the volume
// accessible to a particle of the given radius.
void geom_distribution_test::calculate_volume_test(const geometry::base_region & reg, double full_volume, double radius, double reduced_volume)
{
  double full;
  double access;
  
  BOOST_REQUIRE_NO_THROW( full = reg.volume( 0.0 ) );
  BOOST_REQUIRE_NO_THROW( access = reg.volume( radius ) );
  BOOST_CHECK_EQUAL( full, full_volume );
  BOOST_CHECK_EQUAL( access, reduced_volume );

}

// Test distribution of new positions for a cubic region
// is equivolume.
//
// \require fits(okradius)
void geom_distribution_test::cubic_region_new_position_test(const geometry::base_region & regn, double okradius, std::string title)
{
  // region = regn
  // radius = okradius
  BOOST_REQUIRE( regn.fits( okradius ) );
  
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rand( generator );
  for( std::size_t rad_index = 0; rad_index != 11; ++rad_index )
  {
    const double radius
    {
      okradius * rad_index / 10.0
    }; 
  
    geometry::coordinate big, small;
    std::cerr << "[" << title << "] Doing radius " << radius << "\n";
    regn.extent( small, big, radius );
  
    // want about 1E6 elements.
    const double spacing
    {
      ( big.x - small.x ) / 100.0
    };
  
    // generate discretizer
    geometry::digitizer_3d digitizer( small, big, spacing );
  
    BOOST_REQUIRE_GE( digitizer.size(), 800000 );
    BOOST_REQUIRE_LE( digitizer.size(), 1200000 );
  
    std::vector< std::size_t > histogram( digitizer.size(), 0ul );
  
    const std::size_t loopsize { digitizer.size() };
    const std::size_t histsize { histogram.size() };
    // Generate 100 times more positions as historgram slots
    for( int outer = 0; outer < 100; ++outer )
    {
      for( std::size_t step = 0; step != loopsize; ++step )
      {
        geometry::coordinate pos;
        pos = regn.new_position( rand, radius );
        BOOST_REQUIRE( regn.is_inside( pos, radius ) );
        ++histogram[ digitizer.convert( pos ) ];
      }
    }
    // Assume all points are evenly distributed and average is
    // 100 and stddev is ~ 10
    utility::estimater sample;
    std::size_t inside = 0;
    {
      // Use corners to check if cube at index is in the cell
      for( std::size_t idx = 0; idx != loopsize; ++idx )
      {
        bool is_inside { true };
        std::array< geometry::coordinate, 8 > pts;
        digitizer.corners( idx, pts );
        for ( std::size_t jdx = 0; jdx != 8; ++jdx )
        {
          if (not regn.is_inside( pts[ jdx ], radius ) )
          {
            is_inside = false;
          }
        }
        geometry::coordinate pos( pts[0].x + spacing/2, pts[0].y + spacing/2, pts[0].z + spacing/2 );
        BOOST_REQUIRE_EQUAL( idx, digitizer.convert( pos ) );
        if( is_inside )
        {
          ++inside;
          sample.append( histogram[ idx ] );
        }
      }
    }
    std::cerr << "\% included cubes " << ((double)inside/loopsize) << "\n";
    std::cerr << "Mean " << sample.mean() << "\n";
    std::cerr << "Variance " << sample.variance() << "\n";
    BOOST_CHECK_LE( sample.mean(), 110.0 );
    BOOST_CHECK_GE( sample.mean(), 90.0 );
    BOOST_CHECK_LE( sample.variance(), 110.0 );
    BOOST_CHECK_GE( sample.variance(), 90.0 );
    if( sample.variance() > 110.0 )
    {
      utility::histogram check( 0.0, 160.0, 1.0, true );
      check.begin_sample();
      for (auto count : histogram )
      {
        check.sample_datum( count );
      } 
      check.end_sample();
      std::cout << "# " << title << "\n";
      std::cout << "# radius " << radius << "\n";
  
      for( auto iter = check.begin(); iter != check.end(); ++ iter )
      {
        std::cout << iter.minimum() << " " << iter.mean() << " " << iter.variance() << "\n";
      }
    }
    const double dev{ std::sqrt( sample.variance() ) };
    const double lower2{ sample.mean() - 2 * dev };
    const double upper2{ sample.mean() + 2 * dev };
    const double lower3{ sample.mean() - 3 * dev };
    const double upper3{ sample.mean() + 3 * dev };
    std::size_t twoout = 0;
    std::size_t threeout = 0;
    const double lower20{ sample.mean() - 20.0 };
    const double upper20{ sample.mean() + 20.0 };
    const double lower30{ sample.mean() - 30.0 };
    const double upper30{ sample.mean() + 30.0 };
    std::size_t twentyout = 0;
    std::size_t thirtyout = 0;
    for (auto count : histogram )
    {
      const double val = count;
      if (val > upper20 or lower20 > val)
      {
        ++twentyout;
        if (val > upper30 or lower30 > val)
        {
          ++thirtyout;
        }
      }
      if (val > upper2 or lower2 > val)
      {
        ++twoout;
        if (val > upper3 or lower3 > val)
        {
          ++threeout;
        }
      }
    }
    std::cout << "2 dev : " << (double(twoout)/double(histsize)) << "\n";
    std::cout << "3 dev : " << (double(threeout)/double(histsize)) << "\n";
    std::cout << "20  : " << (double(twentyout)/double(histsize)) << "\n";
    std::cout << "30  : " << (double(thirtyout)/double(histsize)) << "\n";
   
    if (okradius == 0.0)
    {
      break;
    }
  }
  std::cout << "Distribution test complete.\n";
  

}

// Test distribution of new positions for a cylindrical region
// is equivolume.
//
// \param regn : region to test
// \param offset : Any split and offset of the cylinder from the z origin
// \param okradius : a valid sphere radius for the cylinder
//
// \require fits(okradius)
void geom_distribution_test::cylinder_region_new_position_test(const geometry::base_region & regn, double okradius, std::string title)
{
  // region = regn
  // radius = okradius
  BOOST_REQUIRE( regn.fits( okradius ) );
  
  
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rand( generator );
  for( std::size_t rad_index = 0; rad_index != 11; ++rad_index )
  {
    const double radius { okradius * rad_index / 10.0 };
  
    geometry::coordinate big, small;
    std::cerr << "[" << title << "] Doing radius " << radius << "\n";
    regn.extent( small, big, radius );
    const double cyl_radius { big.x };
    const double cyl_hlength { big.y };
  
    // want about 1E6 elements.
    const double spacing
    {
      ( 2 * ( big.x - small.x ) + ( big.z - small.z ) ) / 300.0
    };
  
    // generate discretizer
    geometry::digitizer_3d digitizer( small, big, spacing );
  
    BOOST_REQUIRE_GE( digitizer.size(), 700000 );
    BOOST_REQUIRE_LE( digitizer.size(), 1200000 );
  
    std::vector< std::size_t > histogram( digitizer.size(), 0ul );
  
    const std::size_t loopsize { digitizer.size() };
    // Generate 100 times more positions as historgram slots
    for( int outer = 0; outer < 100; ++outer )
    {
      for( std::size_t step = 0; step != loopsize; ++step )
      {
        geometry::coordinate pos;
        pos = regn.new_position( rand, radius );
        ++histogram[ digitizer.convert( pos ) ];
      }
    }
    const double bound_volume { ( big.x - small.x ) * ( big.y - small.y ) * ( big.z - small.z ) };
    const double approx_mean { bound_volume * 100.0 / regn.volume( radius ) };
    utility::estimater sample;
    std::size_t inside = 0;
    {
      // Use cylinder gridder to get points inside cylinder
      std::array< geometry::coordinate, 8 > pts;
      geometry::coordinate pos;
      for( std::size_t idx = 0; idx != loopsize; ++idx )
      {
        bool is_inside { true };
        digitizer.corners( idx, pts );
        for ( std::size_t jdx = 0; jdx != 8; ++jdx )
        {
          if (not regn.is_inside( pts[ jdx ], radius ) )
          {
            is_inside = false;
          }
        }
        if( is_inside )
        {
          ++inside;
          sample.append( histogram[ idx ] );
        }
      }
    }
    std::cerr << "\% included cubes " << ((double)inside/loopsize) << "\n";
    std::cerr << "Mean " << sample.mean() << "\n";
    std::cerr << "Variance " << sample.variance() << "\n";
    BOOST_CHECK_LE( sample.mean(), approx_mean + 10.0 );
    BOOST_CHECK_GE( sample.mean(), approx_mean - 10.0 );
    BOOST_CHECK_LE( sample.variance(), approx_mean + 10.0 );
    BOOST_CHECK_GE( sample.variance(), approx_mean - 10.0 );
    if( sample.variance() > approx_mean + 10.0 )
    {
      utility::histogram check( 0.0, approx_mean + 60.0, 1.0, true );
      check.begin_sample();
      for (auto count : histogram )
      {
        check.sample_datum( count );
      } 
      check.end_sample();
      std::cout << "# " << title << "\n";
      std::cout << "# radius " << radius << "\n";
  
      for( auto iter = check.begin(); iter != check.end(); ++ iter )
      {
        std::cout << iter.minimum() << " " << iter.mean() << " " << iter.variance() << "\n";
      }
    }
    const double dev { std::sqrt( sample.variance() ) };
    const double lower2 { sample.mean() - 2 * dev };
    const double upper2 { sample.mean() + 2 * dev };
    const double lower3 { sample.mean() - 3 * dev };
    const double upper3 { sample.mean() + 3 * dev };
    std::size_t twoout = 0;
    std::size_t threeout = 0;
    const double lower20 { sample.mean() - 20.0 };
    const double upper20 { sample.mean() + 20.0 };
    const double lower30 { sample.mean() - 30.0 };
    const double upper30 { sample.mean() + 30.0 };
    std::size_t twentyout = 0;
    std::size_t thirtyout = 0;
    {
      // Use cylinder gridder to get points inside cylinder
      geometry::tubular_grid cursor( 2 * cyl_hlength, cyl_radius, spacing );
      geometry::coordinate pos;
      std::vector< std::size_t > once( loopsize, 0ul );
      while( cursor.next( pos ) )
      {
        const std::size_t idx
        {
          digitizer.convert( pos )
        };
        ++once[ idx ];
        if( once[ idx ] == 1 )
        {
          const double val = histogram[ idx ];
          if( val > upper20 or lower20 > val )
          {
            ++twentyout;
            if( val > upper30 or lower30 > val )
            {
              ++thirtyout;
            }
          }
          if( val > upper2 or lower2 > val )
          {
            ++twoout;
            if( val > upper3 or lower3 > val )
            {
              ++threeout;
            }
          }
        }
      }
    }
    std::cout << "2 dev : " << ( double( twoout )/double( loopsize ) ) << "\n";
    std::cout << "3 dev : " << ( double( threeout )/double( loopsize ) ) << "\n";
    std::cout << "20  : " << ( double( twentyout )/double( loopsize ) ) << "\n";
    std::cout << "30  : " << ( double( thirtyout )/double( loopsize ) ) << "\n";
  
    if( okradius == 0.0 )
    {
      break;
    }
  }
  std::cout << "Distribution test complete.\n";
  
  

}

// Test distribution of new positions for a cylindrical region
// is equivolume.
//
// \param regn : region to test
// \param offset : Any split and offset of the cylinder from the z origin
// \param okradius : a valid sphere radius for the cylinder
//
// \require fits(okradius)
void geom_distribution_test::general_new_position_test(const geometry::base_region & regn, double okradius, std::string title)
{
  // region = regn
  // radius = okradius
  BOOST_REQUIRE( regn.fits( okradius ) );
  
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rand( generator );
  for( std::size_t rad_index = 0; rad_index != 11; ++rad_index )
  {
    const double radius { okradius * rad_index / 10.0 };
  
    geometry::coordinate big, small;
    std::cerr << "[" << title << "] Doing radius " << radius << "\n";
    regn.extent( small, big, radius );
  
    // want about 1E6 elements.
    const double spacing
    {
      ( ( big.x - small.x ) + ( big.y - small.y ) + ( big.z - small.z ) ) / 300.0
    };
  
    // generate discretizer
    geometry::digitizer_3d digitizer( small, big, spacing );
  
    BOOST_REQUIRE_GE( digitizer.size(),  700000 );
    BOOST_REQUIRE_LE( digitizer.size(), 1250000 );
  
    BOOST_REQUIRE_LE( digitizer.x_axis().minimum(), small.x );
    if( not utility::feq( digitizer.x_axis().maximum(), big.x ) )
    {
      BOOST_REQUIRE_GE( digitizer.x_axis().maximum(), big.x );
    }
    BOOST_REQUIRE_LE( digitizer.y_axis().minimum(), small.y );
    if( not utility::feq( digitizer.y_axis().maximum(), big.y ) )
    {
      BOOST_REQUIRE_GE( digitizer.y_axis().maximum(), big.y );
    }
    BOOST_REQUIRE_LE( digitizer.z_axis().minimum(), small.z );
    if( not utility::feq( digitizer.z_axis().maximum(), big.z ) )
    {
      BOOST_REQUIRE_GE( digitizer.z_axis().maximum(), big.z );
    }
  
    std::vector< std::size_t > histogram( digitizer.size(), 0ul );
  
    const std::size_t loopsize { digitizer.size() };
    const std::size_t histsize { histogram.size() };
    // Generate 100 times more positions as historgram slots
    for( int outer = 0; outer < 100; ++outer )
    {
      for( std::size_t step = 0; step != loopsize; ++step )
      {
        geometry::coordinate pos;
        pos = regn.new_position( rand, radius );
        BOOST_REQUIRE( regn.is_inside( pos, radius ) );
        ++histogram[ digitizer.convert( pos ) ];
      }
    }
    const double bound_volume { ( big.x - small.x ) * ( big.y - small.y ) * ( big.z - small.z ) };
    const double approx_mean { bound_volume * 100.0 / regn.volume( radius ) };
    const double approx_delta { std::sqrt( approx_mean )  };
    utility::estimater sample;
    std::vector< bool > usable( histsize, false );
    std::size_t inside = 0;
    {
      // Use corners to check if cube at index is in the cell
      for( std::size_t idx = 0; idx != loopsize; ++idx )
      {
        bool is_inside { true };
        std::array< geometry::coordinate, 8 > pts;
        digitizer.corners( idx, pts );
        for ( std::size_t jdx = 0; jdx != 8; ++jdx )
        {
          if (not regn.is_inside( pts[ jdx ], radius ) )
          {
            is_inside = false;
          }
        }
        geometry::coordinate pos( pts[0].x + spacing/2, pts[0].y + spacing/2, pts[0].z + spacing/2 );
        BOOST_REQUIRE_EQUAL( idx, digitizer.convert( pos ) );
        usable[ idx ] = is_inside;
        if( is_inside )
        {
          ++inside;
          sample.append( histogram[ idx ] );
        }
      }
    }
    std::cerr << "\% included cubes " << ((double)inside/loopsize) << "\n";
    std::cerr << "Mean " << sample.mean() << "\n";
    std::cerr << "Variance " << sample.variance() << "\n";
    BOOST_CHECK_LE( sample.mean(), approx_mean + approx_delta );
    BOOST_CHECK_GE( sample.mean(), approx_mean - approx_delta );
    BOOST_CHECK_LE( sample.variance(), approx_mean + approx_delta );
    BOOST_CHECK_GE( sample.variance(), approx_mean - approx_delta );
    const double dev { std::sqrt( sample.variance() ) };
    const double lower2 { sample.mean() - 2 * dev };
    const double upper2 { sample.mean() + 2 * dev };
    const double lower3 { sample.mean() - 3 * dev };
    const double upper3 { sample.mean() + 3 * dev };
    std::size_t twoout = 0;
    std::size_t threeout = 0;
    const double sqrtmean { std::sqrt( sample.mean() ) };
    const double lower20 { sample.mean() - (2 * sqrtmean) };
    const double upper20 { sample.mean() + (2 * sqrtmean) };
    const double lower30 { sample.mean() - (3 * sqrtmean) };
    const double upper30 { sample.mean() + (3 * sqrtmean) };
    std::size_t twentyout = 0;
    std::size_t thirtyout = 0;
    {
      for( std::size_t idx = 0; idx != histsize; ++idx )
      {
        if( usable[ idx ] )
        {
          const double val
          {
            static_cast< double >( histogram[ idx ] )
          };
          if( val > upper20 or lower20 > val )
          {
            ++twentyout;
            if( val > upper30 or lower30 > val )
            {
              ++thirtyout;
            }
          }
          if( val > upper2 or lower2 > val )
          {
            ++twoout;
            if( val > upper3 or lower3 > val )
            {
              ++threeout;
            }
          }
        }
      }
    }
    std::cerr << "2 dev : " << ( double( twoout )/double( histsize ) ) << "\n";
    std::cerr << "3 dev : " << ( double( threeout )/double( histsize ) ) << "\n";
    std::cerr << "20  : " << ( double( twentyout )/double( histsize ) ) << "\n";
    std::cerr << "30  : " << ( double( thirtyout )/double( histsize ) ) << "\n";
  
    if( okradius == 0.0 )
    {
      break;
    }
  }
  std::cerr << "Distribution test complete.\n";
  
  

}

BOOST_AUTO_TEST_CASE( periodic_cube_region_test )
{
  
  // periodic cube main ctor
  geometry::periodic_cube_region var1( "bulk", 10.0 );
  BOOST_CHECK_EQUAL( var1.length(), 10.0 );
  BOOST_CHECK_EQUAL( var1.label(), "bulk" );
  BOOST_CHECK_EQUAL( var1.volume( 1.0 ), 1000.0 );
  
  geom_distribution_test::cubic_region_new_position_test( var1, 0.0, "Periodic Cube" );
  

}

BOOST_AUTO_TEST_CASE( cube_region_test )
{
  geometry::coordinate origin( 1.0, -2.0, 3.0 );
  
  // Cubic grid main ctor (use closed cube or radius is not considered)
  geometry::cube_region var1( "bulk", 10.0, origin, false );
  BOOST_CHECK_EQUAL( var1.length(), 10.0 );
  BOOST_CHECK_EQUAL( var1.label(), "bulk" );
  BOOST_CHECK( not var1.is_open() );
  BOOST_CHECK_EQUAL( var1.origin(), origin );
  
  geom_distribution_test::cubic_region_new_position_test( var1, 4.0, "Cubic Region (1,-2, 3)" );
  

}

BOOST_AUTO_TEST_CASE( cylinder_region_test )
{
  // Cylinder main ctor 
  geometry::cylinder_region var1( "bulk", 5.0, 10.0 );
  BOOST_CHECK_EQUAL( var1.half_length(), 10.0 );
  BOOST_CHECK_EQUAL( var1.radius(), 5.0 );
  BOOST_CHECK_EQUAL( var1.label(), "bulk" );
  
  geom_distribution_test::cylinder_region_new_position_test( var1, 2.0, "Cylinder" );
  

}

BOOST_AUTO_TEST_CASE( generic_region_test )
{
  // Cylinder main ctor 
  geometry::cylinder_region var1( "bulk", 5.0, 10.0 );
  BOOST_CHECK_EQUAL( var1.half_length(), 10.0 );
  BOOST_CHECK_EQUAL( var1.radius(), 5.0 );
  BOOST_CHECK_EQUAL( var1.label(), "bulk" );
  
  geom_distribution_test::general_new_position_test( var1, 2.0, "Cylinder 2" );
  

}

BOOST_AUTO_TEST_CASE( membrane_cell_region_test )
{
  // Cylinder main ctor 
  geometry::membrane_cylinder_region var1( "bulk", 50.0, 30.0, 20.0, 5.0, 15.0 );
  BOOST_CHECK_EQUAL( var1.half_length(), 30.0 );
  BOOST_CHECK_EQUAL( var1.radius(), 50.0 );
  BOOST_CHECK_EQUAL( var1.channel_half_length(), 20.0 );
  BOOST_CHECK_EQUAL( var1.channel_radius(), 5.0 );
  BOOST_CHECK_EQUAL( var1.arc_radius(), 15.0 );
  BOOST_CHECK_EQUAL( var1.label(), "bulk" );
  
  geom_distribution_test::general_new_position_test( var1, 2.0, "Membrane Cell" );
  

}

// Assumes equiarea distribution in xy and use standard generation method
BOOST_AUTO_TEST_CASE( membrane_cell_z_test )
{
  // Cylinder main ctor 
  geometry::membrane_cylinder_region var1( "bulk", 50.0, 30.0, 20.0, 5.0, 15.0 );
  BOOST_CHECK_EQUAL( var1.half_length(), 30.0 );
  BOOST_CHECK_EQUAL( var1.radius(), 50.0 );
  BOOST_CHECK_EQUAL( var1.channel_half_length(), 20.0 );
  BOOST_CHECK_EQUAL( var1.channel_radius(), 5.0 );
  BOOST_CHECK_EQUAL( var1.arc_radius(), 15.0 );
  BOOST_CHECK_EQUAL( var1.label(), "bulk" );
  
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rand( generator );
  
  for( std::size_t rad_iter = 0; rad_iter < 10; ++rad_iter )
  {
    double radius{ 0.0 };
    do
    {
      radius = var1.channel_radius() / (rad_iter + 2) ;
    }
    while( radius <= 0.0 or var1.channel_radius() <= radius );
    // histogram length
    const double length{ var1.channel_half_length() + var1.half_length() - radius };
    utility::histogram histo( 0.0, length, 1000ul, false );
    std::cout << "# RADIUS : " << radius << "\n";
    std::cout << "# CHANNEL_RADIUS : " <<  var1.channel_radius() << "\n";
    std::cout << "# VESTIBULE START : " <<  (var1.channel_half_length() - var1.arc_radius()) << "\n";
    std::cout << "# VESTIBULE END : " <<  (var1.channel_half_length() + radius) << "\n";
    std::cout << "# CELL END : " <<  (var1.channel_half_length() + var1.half_length() - radius) << "\n";
   
    for( std::size_t iter = 0; iter < 10000; ++iter )
    {
      histo.begin_sample();
      for( std::size_t jter = 0; jter < 1000; ++jter )
      {
        const geometry::coordinate pos{ var1.new_position( rand, radius ) };
        histo.sample_datum( std::abs( pos.z ) );
      }
      histo.end_sample();
    }
    
    for( auto iter = histo.begin(); iter != histo.end(); ++iter )
    {
      const double vol{ var1.sub_volume( iter.minimum(), iter.maximum(), radius ) };
      std::cout << iter.midpoint() << " " << iter.mean() << " " << iter.variance() << " " << (iter.mean()/vol) << " " << (iter.variance()/vol) << "\n";
    }
    std::cout << "\n\n";  
  }

}

// Assumes equiarea distribution in xy and use alternative generation method
BOOST_AUTO_TEST_CASE( membrane_cell_alt_z_test )
{
  // Cylinder main ctor 
  geometry::membrane_cylinder_region var1( "bulk", 50.0, 30.0, 20.0, 5.0, 15.0 );
  BOOST_CHECK_EQUAL( var1.half_length(), 30.0 );
  BOOST_CHECK_EQUAL( var1.radius(), 50.0 );
  BOOST_CHECK_EQUAL( var1.channel_half_length(), 20.0 );
  BOOST_CHECK_EQUAL( var1.channel_radius(), 5.0 );
  BOOST_CHECK_EQUAL( var1.arc_radius(), 15.0 );
  BOOST_CHECK_EQUAL( var1.label(), "bulk" );
  
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rand( generator );
  
  for( std::size_t rad_iter = 0; rad_iter < 10; ++rad_iter )
  {
    double radius{ 0.0 };
    do
    {
      radius = var1.channel_radius() / (rad_iter + 2) ;
    }
    while( radius <= 0.0 or var1.channel_radius() <= radius );
    // histogram length
    const double length{ var1.channel_half_length() + var1.half_length() - radius };
    utility::histogram histo( 0.0, length, 1000ul, false );
    std::cout << "# RADIUS : " << radius << "\n";
    std::cout << "# CHANNEL_RADIUS : " <<  var1.channel_radius() << "\n";
    std::cout << "# VESTIBULE START : " <<  (var1.channel_half_length() - var1.arc_radius()) << "\n";
    std::cout << "# VESTIBULE END : " <<  (var1.channel_half_length() + radius) << "\n";
    std::cout << "# CELL END : " <<  (var1.channel_half_length() + var1.half_length() - radius) << "\n";
   
    for( std::size_t iter = 0; iter < 10000; ++iter )
    {
      histo.begin_sample();
      for( std::size_t jter = 0; jter < 1000; ++jter )
      {
        const geometry::coordinate pos{ var1.alt_new_position( rand, radius ) };
        histo.sample_datum( std::abs( pos.z ) );
      }
      histo.end_sample();
    }
    
    for( auto iter = histo.begin(); iter != histo.end(); ++iter )
    {
      const double vol{ var1.sub_volume( iter.minimum(), iter.maximum(), radius ) };
      std::cout << iter.midpoint() << " " << iter.mean() << " " << iter.variance() << " " << (iter.mean()/vol) << " " << (iter.variance()/vol) << "\n";
    }
    std::cout << "\n\n";  
  }

}

// Assumes equiarea distribution in xy
BOOST_AUTO_TEST_CASE( membrane_inserter_z_test )
{
  // Cylinder main ctor 
  geometry::membrane_cylinder_region var1( "bulk", 50.0, 30.0, 20.0, 5.0, 15.0 );
  BOOST_CHECK_EQUAL( var1.half_length(), 30.0 );
  BOOST_CHECK_EQUAL( var1.radius(), 50.0 );
  BOOST_CHECK_EQUAL( var1.channel_half_length(), 20.0 );
  BOOST_CHECK_EQUAL( var1.channel_radius(), 5.0 );
  BOOST_CHECK_EQUAL( var1.arc_radius(), 15.0 );
  BOOST_CHECK_EQUAL( var1.label(), "bulk" );
  
  boost::shared_ptr< boost::mt19937> generator( new boost::mt19937 );
  utility::random_distribution rand( generator );
  try
  {
  for( std::size_t rad_iter = 0; rad_iter < 10; ++rad_iter )
  {
    double radius{ 0.0 };
    do
    {
      radius = var1.channel_radius() / (rad_iter + 2);
    }
    while( radius <= 0.0 or var1.channel_radius() <= radius );
    geometry::membrane_cell_inserter var2( radius, var1.radius(), var1.half_length(), var1.channel_radius(), var1.channel_half_length(), var1.arc_radius() );
    // histogram length
    
    const double length{ var1.channel_half_length() + var1.half_length() - radius };
    utility::histogram histo( 0.0, length, 1000ul, false );
    std::cout << "# RADIUS : " << radius << "\n";
    std::cout << "# CHANNEL_RADIUS : " <<  var1.channel_radius() << "\n";
    std::cout << "# VESTIBULE START : " <<  (var1.channel_half_length() - var1.arc_radius()) << "\n";
    std::cout << "# VESTIBULE END : " <<  (var1.channel_half_length() + radius) << "\n";
    std::cout << "# CELL END : " <<  (var1.channel_half_length() + var1.half_length() - radius) << "\n";
    for( std::size_t iter = 0; iter < 10000; ++iter )
    {
      histo.begin_sample();
      for( std::size_t jter = 0; jter < 1000; ++jter )
      {
        const geometry::coordinate pos{ var2.generate_position( rand ) };
        histo.sample_datum( std::abs( pos.z ) );
      }
      histo.end_sample();
    }
    
    for( auto iter = histo.begin(); iter != histo.end(); ++iter )
    {
      const double vol{ var1.sub_volume( iter.minimum(), iter.maximum(), radius ) };
      std::cout << iter.midpoint() << " " << iter.mean() << " " << iter.variance() << " " << (iter.mean()/vol) << " " << (iter.variance()/vol) << "\n";
    }
    std::cout << "\n\n";
  }
  }
  catch( std::runtime_error const& err )
  {
    std::cout << err.what() << "\n";
    BOOST_ERROR( "Unexpected exception." );
  }

}

