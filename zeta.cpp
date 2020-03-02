
#include <cmath>
#include <complex>
#include "zeta.h"

typedef std::complex<double> cplx;

long int binomialCoeff(int n, int k) {
   int res = 1;

   if (k > n - k)
      k = n - k;
   for (int i = 0; i < k; ++i) {
      res *= (n - i);
      res /= (i + 1);
   }

   return res;
}
long int Coeff(int n, int j) {
   int res = 0;
   if (j > n) {
      int last = j - n;
      for (int k = 0; k <= last; k++) {
         res += binomialCoeff(n, k);
      }
   }
   res -= pow(2, n);
   if (j % 2)
      return -res;
   return res;
}