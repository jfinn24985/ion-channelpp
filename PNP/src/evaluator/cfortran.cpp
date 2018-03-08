#define sunFortran
#include "evaluator/cfortran.hpp"

#include "utility/fuzzy_equals.hpp"
#include "utility/mathutil.hpp"

namespace
{
int fuzzy_equal(double a, double b)
{
  return utility::feq< double >(a,b,32);
}

int next_64(int a)
{
  return static_cast<int>( utility::mathutil::next64( static_cast<std::size_t>(a) ) );
}

void pretty_print_number(double *a_dbl, char *a_result, int *sz)
{
  char fmt[16];
  fmt[0]='%';
  if (*sz < 9)
    snprintf(&fmt[1],15,"-%d.%dG",*sz,2);
  else
    snprintf(&fmt[1],15,"-%d.%dG",*sz,*sz-7);
  snprintf(a_result,*sz,fmt,*a_dbl);
}
}
FCALLSCFUN2(LOGICAL,fuzzy_equal,DFEQ,dfeq,DOUBLE,DOUBLE)
FCALLSCSUB3(pretty_print_number,FMTFL2,fmtfl2,PDOUBLE,PSTRING,PINT)
FCALLSCFUN1(INT,next_64,NEXT64,next64,INT)
