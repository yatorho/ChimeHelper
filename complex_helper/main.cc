#include <cstdint>
#include <iostream>
#include <complex>


int main () {
  std::complex<float> c1(1.f, 2.f);
  std::complex<float> c2(3.f, 4.f);
  // std::cout << c1 + c2 << std::endl;

  long double d1 = 1.;
  __uint128_t  d2 = 2;
  std::cout << "size of float128: " << sizeof(__int128) << std::endl;
}
