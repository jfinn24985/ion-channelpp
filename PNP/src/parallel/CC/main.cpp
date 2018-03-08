//  2 a*a [p/2&1=0]
//  3 2*a
//  4 2*2 [p/2&1=0]
//  5 4*a
//  6 4*2 [p/2&1=1]
//  7 6*a
//  8 4*4 [p/2&1=0]
//  9 8*a
//  10 6*4
//  

int main()
{
//  std::cout << gpl::pow< 2 >(2) << "\n";
//  std::cout << gpl::pow< 3 >(2) << "\n";
//  std::cout << gpl::pow< 4 >(2) << "\n";
//  std::cout << gpl::pow< 5 >(2) << "\n";
//  std::cout << gpl::pow< 6 >(2) << "\n";
//  std::cout << gpl::pow< 7 >(2) << "\n";
//  std::cout << gpl::pow< 8 >(2) << "\n";
//  std::cout << gpl::pow< 9 >(2) << "\n";
//  std::cout << gpl::pow< 10 >(2) << "\n\n";

//  std::cout << gpl::powr< 2 >(2) << "\n";
//    std::cout << gpl::powr(2, 3) << "\n";
//    std::cout << gpl::powr(2, 4) << "\n";
//    std::cout << gpl::powr(2, 5) << "\n";
//    std::cout << gpl::powr(2, 6) << "\n";
//    std::cout << gpl::powr(2, 7) << "\n";
//    std::cout << gpl::powr(2, 8) << "\n";
//    std::cout << gpl::powr(2, 9) << "\n";
//    std::cout << gpl::powr(2, 10) << "\n";
//  
  double length = 10.0;

  for (int xdx = -9 ; xdx != 19; ++xdx)
  {
     const double z (xdx);
     const double p (2.5);


     std::cout << "(P,Z) = " << p << "," << z << " z-p = " <<  std::abs(z - p)<< " dim(z-p, l) = " << std::tr1::fdim(z - p, length) << " dim(l+z-p,l) = " << std::tr1::fdim(z - p, 0.0) << "\n";
  }   




  return 0;
}
