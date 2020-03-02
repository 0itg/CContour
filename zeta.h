#pragma once

long int binomialCoeff(int n, int k);
long int Coeff(int n, int j);

template <typename T> T zeta(T s, int prec = 28) {
   T res    = 0;
   int last = 2 * prec - 1;
   for (int i = 0; i < last; i++) {
      res += (T)Coeff(prec, i) / (T)pow(i + 1.0, s);
   }
   res *= (T)-1.0 / (T)(pow(2, prec) * ((T)1 - pow(2.0, (T)1 - s)));
   return res;
}