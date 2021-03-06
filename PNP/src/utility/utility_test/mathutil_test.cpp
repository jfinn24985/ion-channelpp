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

#define BOOST_TEST_MODULE utility_test
#include <boost/test/unit_test.hpp>

#include "utility/utility_test/mathutil_test.hpp"
#include "utility/fuzzy_equals.hpp"

// Manuals
#include "core/strngs.hpp"
#include "core/input_help.hpp"
#include "utility/archive.hpp"
#include "utility/multi_array.hpp"
#include "utility/random.hpp"
#include "utility/machine.hpp"
#include "utility/mathutil.hpp"
#include "utility/utility.hpp"
#ifndef DBL_EPSILON
#include <cfloat>
#endif
// - 
//Test random number generator conformance
//
//Test that the system gives numbers in the expected ranges.
BOOST_AUTO_TEST_CASE( random_distribution )
{
  boost::shared_ptr< boost::mt19937 > gentor(new boost::mt19937);
  
  {
  // Test that generator is shared not copied
    utility::random_distribution a(gentor), b(gentor);
    std::size_t differs = 0;
    for (int idx = 0; idx != 100; ++idx)
    {
      if (utility::feq(a.uniform(1.0), b.uniform(1.0)))
      {
        ++differs;
      }
    }
    BOOST_CHECK_MESSAGE(100 > differs, " Number of equal values ("<<differs<<") equals 100");
  }
  
  {
    utility::random_distribution gen(gentor);
  
    // operator(size_t u1) boundary condition check
    BOOST_CHECK_EQUAL(0ul, gen.randint(std::size_t(0)));
  
    for (std::size_t idx = 0; idx != 1000; ++idx)
    {
      {
        // Test operator() give number in [0,1)
        const double val1 (gen.uniform(0.0, 1.0));
        BOOST_CHECK_MESSAGE (0.0 <= val1, 0.0<<" > "<<val1);
        BOOST_CHECK_MESSAGE (1.0 > val1, 1.0<<" <= "<<val1);
      }
      const double dvalue1 (double (idx + 1)/2.1);
      {
        // Test operator(double d1) gives number in [0,d1)
        const double val1 (gen.uniform(0.0, dvalue1));
        BOOST_CHECK_MESSAGE (0.0 <= val1, 0.0<<" > "<<val1);
        BOOST_CHECK_MESSAGE (dvalue1 > val1, dvalue1<<" <= "<<val1);
      }
      const double dvalue2 (-double(idx+1)/2.3);
      {
        // Test operator(double d1, double d2) gives number in [d1,d2)
        const double val1 (gen.uniform(dvalue2,dvalue1));
        BOOST_CHECK_MESSAGE (dvalue2 <= val1, dvalue2<<" > "<<val1);
        BOOST_CHECK_MESSAGE (dvalue1 > val1, dvalue1<<" <= "<<val1);
        const double val2 (gen.uniform(dvalue1,dvalue2));
        BOOST_CHECK_MESSAGE (dvalue1 >= val2, dvalue1<<" < "<<val2);
        BOOST_CHECK_MESSAGE (dvalue2 < val2, dvalue2<<" >= "<<val2);
      }
      const double dvalue3 (-double(idx+1)/4.3);
      const double dvalue4 (double(idx+1)/4.3);
      {
        // Test operator(double d1, double d2, double d3, double d4) gives number
        // in [d1,d2) outside or (d3, d4]
        const double val1 (gen.split_uniform(dvalue2,dvalue1,dvalue3,dvalue4));
        BOOST_CHECK_MESSAGE (dvalue2 <= val1, dvalue2<<" > "<<val1);
        BOOST_CHECK_MESSAGE (dvalue1 > val1, dvalue1<<" <= "<<val1);
        BOOST_CHECK_MESSAGE (dvalue3 > val1 or dvalue4 <= val1
              ,dvalue3<<" <= "<<val1<<" < "<<dvalue4);
      }
      {
        // Test operator(size_t u1) gives number in [0,u1)
        const std::size_t val1 (gen.randint(idx+2ul));
        BOOST_CHECK_MESSAGE (val1 <= idx+2ul, (idx+2ul)<<" == "<<val1);
      }
    }
  }

}

//Test random number generator
BOOST_AUTO_TEST_CASE( fuzzy_equal )
{
  BOOST_CHECK(    utility::feq(M_PI, 3.141592653589793));
  BOOST_CHECK(    utility::feq(M_PI, 3.14159265358979));
  BOOST_CHECK(    utility::feq(M_PI, 3.1415926535898));
  BOOST_CHECK(not utility::feq(M_PI, 3.141592653590));
  BOOST_CHECK(not utility::feq(M_PI, 3.14159265359));
  

}

//Test of search functions.
BOOST_AUTO_TEST_CASE( find_n_tests )
{
  
  // -------------------------------------------------------------
  // find_n : Return position in vspec_key(1:a_max) of ith occurance of
  // a_value
  //
  // Returns a_max if no element found
  const std::vector< int > spec_key = { 1, 1, 2, 1, 3, 1, 0, 0, 0, 1, 0, 3 };
  std::vector<int> vspec_key (spec_key);
  
  {
    auto result = utility::mathutil::find_n(vspec_key.begin(), vspec_key.end(), 0, 1);
    if (result !=  vspec_key.end())
    {
      BOOST_CHECK_MESSAGE(*result == 0, "First '0' is "<<*result<<" not 0");
      auto index = result - vspec_key.begin();
      BOOST_CHECK_MESSAGE(index == 6, "First '0' at ["<<index<<"], expected 6");
    }
    else
    {
      BOOST_MESSAGE("First '0' not found, expected 6");
    }
  }
  {
    auto result = utility::mathutil::find_n(vspec_key.begin(), vspec_key.end(), 1, 4);
    if (result != vspec_key.end())
    {
      BOOST_CHECK_MESSAGE(*result == 1, "Fourth '1' is "<<*result<<" not 1");
      auto index = result - vspec_key.begin();
      BOOST_CHECK_MESSAGE(index == 5, "Fourth '1' at ["<<index<<"], expected 5");
    }
    else
    {
      BOOST_MESSAGE("Fourth '1' not found, expected 5");
    }
  }
  {
    auto result = utility::mathutil::find_n(&spec_key[0], &spec_key[spec_key.size()], 2, 1);
    if (result != &spec_key[spec_key.size()])
    {
      BOOST_CHECK_MESSAGE(*result == 2, "First '2' is "<<*result<<" not 2");
      auto index = result - &spec_key[0];
      BOOST_CHECK_MESSAGE(index == 2, "First '2' at ["<<index<<"], expected 2");
    }
    else
    {
      BOOST_MESSAGE("First '2' not found, expected 2");
    }
  }
  {
    auto result = utility::mathutil::find_n(vspec_key.begin(), vspec_key.end(), 3, 2);
    if (result != vspec_key.end())
    {
      BOOST_CHECK_MESSAGE(*result == 3, "Second '3' is "<<*result<<" not 3");
      auto index = result - vspec_key.begin();
      BOOST_CHECK_MESSAGE(index, "Second '3' at ["<<index<<"], expected 11");
    }
    else
    {
      BOOST_MESSAGE("Second '3' not found, expected 11");
    }
  }
  {
    auto result = utility::mathutil::find_n(vspec_key.begin(), vspec_key.end(), 3, 3);
    if (result != vspec_key.end())
    {
      BOOST_CHECK_MESSAGE(*result == 3, "Third '3' is "<<*result<<" not 3");
      auto index = result - vspec_key.begin();
      BOOST_CHECK_MESSAGE(index, "Third '3' at ["<<index<<"], expected none.");
    }
  }
  {
    auto result = utility::mathutil::find_n(vspec_key.begin(), vspec_key.end(), 4, 1);
    if (result != vspec_key.end())
    {
      BOOST_CHECK_MESSAGE(*result == 4, "First '4' is "<<*result<<" not 4");
      auto index = result - vspec_key.begin();
      BOOST_CHECK_MESSAGE(index, "First '4' at ["<<index<<"], expected none.");
    }
  }
  // find_n_of (begin, end, arr, value, ith)
  std::vector< std::size_t > arr1(spec_key.size(), 0ul);
  for (std::size_t i = 0; i != arr1.size(); ++i) arr1[i] = i;
  // should be { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  
  {
    auto result = utility::mathutil::find_n_of(&arr1[0], &arr1[arr1.size()], spec_key, 0, 1);
    if (result != &arr1[arr1.size()])
    {
      BOOST_CHECK_MESSAGE(*result == 6, "First '0' is at "<<*result<<", expected 6");
      BOOST_CHECK_MESSAGE(spec_key[*result] == 0, "First '0' was "<<spec_key[*result]);
    }
    else
    {
      BOOST_MESSAGE("First '0' not found, expected 6");
    }
  }
  {
    auto result = utility::mathutil::find_n_of(arr1.begin(), arr1.end(), spec_key, 1, 4);
    if (result != arr1.end())
    {
      BOOST_CHECK_MESSAGE(*result == 5, "Fourth '1' is at "<<*result<<", expected 5");
      BOOST_CHECK_MESSAGE(spec_key[*result] == 1, "Fourth '1' was "<<spec_key[*result]);
    }
    else
    {
      BOOST_MESSAGE("Fourth '1' not found, expected 5");
    }
  }
  {
    auto result = utility::mathutil::find_n_of(&arr1[0], &arr1[arr1.size()], spec_key, 2, 1);
    if (result != &arr1[arr1.size()])
    {
    BOOST_CHECK_MESSAGE(*result == 2, "First '2' is at "<<*result<<", expected 2");
      BOOST_CHECK_MESSAGE(spec_key[*result] == 2, "First '2' was "<<spec_key[*result]);
    }
    else
    {
      BOOST_MESSAGE("First '2' not found, expected 2");
    }
  }
  {
    auto result = utility::mathutil::find_n_of(&arr1[0], &arr1[arr1.size()], spec_key, 3, 2);
    if (result != &arr1[arr1.size()])
    {
      BOOST_CHECK_MESSAGE(*result == 11, "Second '3' is at "<<*result<<", expected 11");
      BOOST_CHECK_MESSAGE(spec_key[*result] == 3, "Second '3' was "<<spec_key[*result]);
    }
    else
    {
      BOOST_MESSAGE("Second '3' not found, expected 11");
    }
  }
  {
    auto result = utility::mathutil::find_n_of(&arr1[0], &arr1[arr1.size()], spec_key, 3, 3);
    if (result != &arr1[arr1.size()])
    {
      BOOST_MESSAGE("Third '3' is at "<<*result<<", expected none");
      BOOST_CHECK_MESSAGE(spec_key[*result] == 3, "Third '3' was "<<spec_key[*result]);
    }
  }
  {
    auto result = utility::mathutil::find_n_of(arr1.begin(), arr1.end(), spec_key, 4, 1);
    if (result != arr1.end())
    {
      BOOST_MESSAGE("First '4' is at "<<*result<<", expected none");
      BOOST_CHECK_MESSAGE(spec_key[*result] == 4, "First '4' was "<<spec_key[*result]);
    }
  }
  {
    auto result = utility::mathutil::find_n_of(&arr1[0], &arr1[arr1.size()], spec_key, 1, 0);
    if (result != &arr1[arr1.size()])
    {
      BOOST_MESSAGE("Zeroth '1' is "<<*result<<", expected none");
      BOOST_CHECK_MESSAGE(spec_key[*result] == 0, "Zeroth '0' was "<<spec_key[*result]);
    }
  }
  
  
  

}

//Test nextX methods for finding nearest multiples
BOOST_AUTO_TEST_CASE( next_x )
{
  BOOST_CHECK_EQUAL(utility::mathutil::next2(0),0);
  BOOST_CHECK_EQUAL(utility::mathutil::next64(0),0);
  for (int i=1; i < 1025; ++i)
  {
    // Test next2 should give next higher power of 2
    const int i2 (utility::mathutil::next2(i));
    BOOST_CHECK_LE(i, i2);
    BOOST_CHECK_GT(i, i2/2);
    // Test only one bit is set. {REF: Peter Wegner CACM 3 (1960), 322.}
    BOOST_CHECK_EQUAL(i2 & (i2 - 1), 0);
  
    // next64 should give next multiple of 64
    const int i3 (utility::mathutil::next64(i));
    BOOST_CHECK_LE(i, i3);
    BOOST_CHECK_GT(i, i3-64);
    BOOST_CHECK_EQUAL(i3 & (64 - 1), 0);
    BOOST_CHECK_EQUAL(i3 % 64, 0);
  }

}

BOOST_AUTO_TEST_CASE( range_checks )
{
  for (int i=0; i < 17; ++i)
  {
    BOOST_CHECK(utility::mathutil::in_closed_range(0, 17, i));
  }
  BOOST_CHECK(not utility::mathutil::in_open_range(0, 17, 0));
  BOOST_CHECK(not utility::mathutil::in_open_range(0, 17, 17));
  
  BOOST_CHECK(utility::mathutil::in_half_up_range(0, 17, 0));
  BOOST_CHECK(not utility::mathutil::in_half_up_range(0, 17, 17));
  
  BOOST_CHECK(not utility::mathutil::in_half_down_range(0, 17, 0));
  BOOST_CHECK(utility::mathutil::in_half_down_range(0, 17, 17));
  
  for (int i=0; i < 1024; ++i)
  {
    const double val (double(i)/1024.0);
    BOOST_CHECK(utility::mathutil::in_closed_range(0.0, 1.0, val));
  }
  BOOST_CHECK(not utility::mathutil::in_open_range(0.0, 1.0, 0.0));
  BOOST_CHECK(not utility::mathutil::in_open_range(0.0, 1.0, 1.0));
  
  BOOST_CHECK(utility::mathutil::in_half_up_range(0.0, 1.0, 0.0));
  BOOST_CHECK(not utility::mathutil::in_half_up_range(0.0, 1.0, 1.0));
  
  BOOST_CHECK(not utility::mathutil::in_half_down_range(0.0, 1.0, 0.0));
  BOOST_CHECK(utility::mathutil::in_half_down_range(0.0, 1.0, 1.0));
  

}

BOOST_AUTO_TEST_CASE( multi_array_serialize )
{
  boost::multi_array< double, 2 > mar1;
  boost::multi_array< double, 2 > mar2;
  boost::shared_ptr< boost::mt19937 > rgen (new boost::mt19937);
  utility::random_distribution ranf(rgen);
  
  for (std::size_t sz1 = 8; sz1 != 128; sz1 += 8)
  {
    for (std::size_t sz2 = 8; sz2 != 128; sz2 += 8)
    {
      mar1.resize(boost::extents[sz1][sz2]);
      for (std::size_t idx = 0; idx != sz1; ++idx)
      {
        for (std::size_t jdx = 0; jdx != sz2; ++jdx)
        {
          mar1[idx][jdx] = ranf.uniform(1.0);
        }
      }
      BOOST_CHECK(mar1 != mar2);
      std::stringstream store;
      {
        boost::archive::text_oarchive oa(store);
        oa << mar1;
      }
      {
        boost::archive::text_iarchive ia(store);
        ia >> mar2;
      }
      BOOST_CHECK(mar1 == mar2);
    }
  }
  

}

// // Assert method behind UTILITY_... macros 
//
// void do_assert(const char * a_test, std::string a_msg, const char
//  * a_fn, const char * a_filename, int a_linenum, void * const*
//  a_backtrace, int a_backsz);
BOOST_AUTO_TEST_CASE( test_do_assert )
{
  try
  {
    utility::do_assert("Testing method","in test do_assert",__func__,__FILE__, __LINE__, 0, 0);
    BOOST_FAIL("Expected 'do_assert' method to always throw an exception.");
  }
  catch(const std::runtime_error &err)
  {
    const std::string msg(err.what());
    BOOST_CHECK_NE(msg.find("Testing method"), std::string::npos);
  }

}

//Test the UTILITY_XXX design-by-contract macros
BOOST_AUTO_TEST_CASE( test_DBC )
{
  try
  {
    UTILITY_ALWAYS(false, "Test of 'UTILITY_ALWAYS' DbC macro");
    BOOST_FAIL("No error message thrown with UTILITY_ALWAYS macro");
  }
  catch(const std::runtime_error &err)
  {
    BOOST_CHECK_NE(std::string(err.what()).find("Test of 'UTILITY_ALWAYS' DbC macro"), std::string::npos);
  }

}

//Test the floating point environment
BOOST_AUTO_TEST_CASE( test_fp_env )
{
  struct helper
  {
     static double cutoff_()
     {
        return 100 * std::numeric_limits< double >::epsilon();
     }
    /*
      Newton/Raphson
      f(a) = x - a^2 = 0
      f'(a) = -2a
  
      a_1 = a_0 - f'(a_0)/f(a_0)
      a_1 = a_0 + (x - a^2)/2a
    */
    static void square_root_nr (double x)
    {
      const double x_ = x;
      //const double lib_ = sqrt (x);
      int i_ = 0;
      volatile double b = x_ / 2.;
      for (double a = b, aa
           ; fabs (b * b - x_) > cutoff_()
           ; aa = a*a, a += (x_ - aa) / (2. * a), ++i_)
      {
        b = a;
        // printf("%16.13lf, %16.13lf\n", lib_, a);
      }
      // printf ("%s [%8d] cutoff [%16.13g]\n", "Number of convergence steps", i_, cutoff_());
    }
    /*
      Use Halley's method to calculate square root using
  
     f(a) = x - a*a = 0
     f'(a) = -2*a
     f"(a) = -2
  
     a_1 = a_0 - {2 x f(a) x f'(a) } / { 2 x f'(a)^2 - f(a) y f"(a) }
  
     a_1 = a_0 + {2 * a * (X - a^2)} / { 3 * a^2 + X }
    */
    static void square_root (double x)
    {
      const double x_ = x;
      //const double lib_ = sqrt (x);
      int i_ = 0;
      volatile double b = x_ / 2.;
      for (double a = b, aa
           ; fabs (b * b - x_) > cutoff_ ()
           ; aa = a*a, a += 2 * a * (x_ - aa) / (3*aa + x_), ++i_)
      {
        b = a;
        // printf("%16.13lf, %16.13lf\n", lib_, a);
      }
      // printf ("%s [%8d] cutoff [%16.13g]\n", "Number of convergence steps", i_, cutoff_());
    }
  };
  
  const double sq_ = M_PI;
  
  utility::fp_env &env(utility::fp_env::env_);
  
  BOOST_REQUIRE_MESSAGE(env.ensure (), "Floating point initialisation failed");
  
  helper::square_root_nr (sq_);
  BOOST_CHECK_NE(0, env.Inexact & env.except());
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  env.set_round (env.Downward);
  BOOST_CHECK_EQUAL(env.Downward, env.round ());
  
  helper::square_root (sq_);
  BOOST_CHECK_NE(0, env.Inexact & env.except());
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  env.set_round (utility::fp_env::ToNearest);
  BOOST_CHECK_EQUAL(env.ToNearest, env.round ());
  
  helper::square_root (sq_);
  BOOST_CHECK_NE(0, (env.Inexact & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  env.set_round (env.TowardZero);
  BOOST_CHECK_EQUAL(env.TowardZero, env.round ());
  
  helper::square_root (sq_);
  BOOST_CHECK_NE(0, (env.Inexact & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  env.set_round (env.Upward);
  BOOST_CHECK_EQUAL(env.Upward, env.round ());
  
  helper::square_root (sq_);
  BOOST_CHECK_NE(0, (env.Inexact & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  env.set_round (env.ToNearest);
  BOOST_CHECK_EQUAL(env.ToNearest, env.round ());
  
  std::numeric_limits< double > dlim;
  
  BOOST_CHECKPOINT("(inexact) Compute (DBL_MAX - DBL_MIN)");
  BOOST_CHECK_EQUAL(dlim.max(), dlim.max() - dlim.min() );
  BOOST_CHECK_NE(0, (env.Inexact & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  BOOST_CHECKPOINT("Compute pow(1,-inf) which should not give an error");
  BOOST_CHECK_EQUAL(1.0, pow(1.0, -dlim.infinity()));
  BOOST_CHECK_EQUAL(0, env.except());
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  BOOST_CHECKPOINT("Compute pow(-inf,-3) which should not give an error");
  BOOST_CHECK_EQUAL(-0.0, pow(-dlim.infinity(),-3.0));
  BOOST_CHECK_EQUAL(0, env.except());
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  // printf("Compute 5%0 (int)               [%16d]\n", 5 % 0);
  // env.report(std::cout);
  // env.reset ();
  // env.report(std::cout);

}

//Test the floating point environment
BOOST_AUTO_TEST_CASE( test_fp_divbyzero )
{
  utility::fp_env &env(utility::fp_env::env_);
  
  utility::fp_env::fp_nonstop_scope scoper;
  
  BOOST_REQUIRE_MESSAGE(env.ensure (), "Floating point initialisation failed");
  
  BOOST_TEST_CHECKPOINT("(div-by-zero) Div by zero (epsilon/0)");
  BOOST_CHECK(std::isinf (DBL_EPSILON/0.0));
  BOOST_CHECK_NE(0, (env.DivByZero & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  

}

//Test the floating point environment
BOOST_AUTO_TEST_CASE( test_fp_invalid )
{
  utility::fp_env &env(utility::fp_env::env_);
  
  utility::fp_env::fp_nonstop_scope scoper;
  
  BOOST_REQUIRE_MESSAGE(env.ensure (), "Floating point initialisation failed");
  
  BOOST_CHECKPOINT("(invalid) Square root of negative number sqrt(-3)");
  // (nan) printf("Square root of negative number [%16.13g]\n", sqrt(-3.0));
  BOOST_CHECK(std::isnan (sqrt(-3.0)));
  BOOST_CHECK_NE(0, (env.Invalid & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  

}

//Test the floating point environment
BOOST_AUTO_TEST_CASE( test_fp_invalid_2 )
{
  utility::fp_env &env(utility::fp_env::env_);
  
  utility::fp_env::fp_nonstop_scope scoper;
  
  BOOST_REQUIRE_MESSAGE(env.ensure (), "Floating point initialisation failed");
  
  BOOST_CHECKPOINT("(invalid) Compute 0/-0 ");
  BOOST_CHECK(std::isnan (0.0 / -0.0));
  BOOST_CHECK_NE(0, (env.Invalid & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  

}

//Test the floating point environment
BOOST_AUTO_TEST_CASE( test_fp_overflow )
{
  struct helper
  {
    static const double cutoff_()
    {
      return (100 * DBL_EPSILON);
    }
  };
  
  utility::fp_env &env(utility::fp_env::env_);
  
  utility::fp_env::fp_nonstop_scope scoper;
  
  BOOST_REQUIRE_MESSAGE(env.ensure (), "Floating point initialisation failed");
  
  BOOST_CHECKPOINT("(overflow) Compute (1/100*epsilon)^42 ");
  BOOST_CHECK(std::isinf (pow(1.0/helper::cutoff_(), 42.0)));
  BOOST_CHECK_NE(0, (env.Overflow & env.except()));
  BOOST_CHECK_NE(0, (env.Inexact & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
    
  // Could throw inexact or underflow.
  std::numeric_limits< double > dlim;
  BOOST_CHECKPOINT("(inexact) Compute (DBL_MAX)^4");
  BOOST_CHECK_EQUAL(dlim.infinity(), pow(dlim.max(), 4.0));
  BOOST_CHECK_NE(0, (env.Overflow & env.except()));
  BOOST_CHECK_NE(0, (env.Inexact & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
   

}

//Test the floating point environment
BOOST_AUTO_TEST_CASE( test_fp_underflow )
{
  utility::fp_env &env(utility::fp_env::env_);
  
  utility::fp_env::fp_nonstop_scope scoper;
  
  std::numeric_limits< double > dlim;
  
  BOOST_CHECKPOINT("(underflow) Compute (DBL_MIN/3.0)");
  double under_flow_value = dlim.min()/3.0;
  BOOST_WARN_EQUAL( 0.0, under_flow_value );
  BOOST_CHECK_NE(0, (env.Underflow & env.except()));
  BOOST_CHECK_NE(0, (env.Inexact & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
  
  // Should throw inexact or underflow.
  BOOST_CHECKPOINT("(inexact) Compute (DBL_MIN)^4");
  BOOST_CHECK_EQUAL(0.0, pow(dlim.min(), 4.0));
  BOOST_CHECK_NE(0, (env.Underflow & env.except()));
  BOOST_CHECK_NE(0, (env.Inexact & env.except()));
  env.reset ();
  BOOST_CHECK_EQUAL(0, env.except());
   

}

