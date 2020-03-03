#pragma once

#include <array>

// Template metaprogram to generate table of binomial coefficients.
// Credit: qwe, Stack Overflow.

typedef short int index_t;
typedef unsigned long long int int_t;

// standard recursive template for coefficient values, used as generator
template <index_t n, index_t k> struct coeff {
   static int_t const value =
       coeff<n - 1, k - 1>::value + coeff<n - 1, k>::value;
};
template <index_t n> struct coeff<n, 0> { static int_t const value = 1; };
template <index_t n> struct coeff<n, n> { static int_t const value = 1; };

// helper template, just converts its variadic arguments to array initializer
// list
template <int_t... values> struct int_ary {
   static int_t const value[sizeof...(values)];
};
template <int_t... values>
int_t const int_ary<values...>::value[] = {values...};

// decrement k, pile up variadic argument list using generator
template <index_t n, index_t k, int_t... values>
struct rec : rec<n, k - 1, coeff<n, k - 1>::value, values...> {};
// when done (k == 0), derive from int_ary
template <index_t n, int_t... values>
struct rec<n, 0, values...> : int_ary<values...> {};

// initialise recursion
template <index_t n> struct binomial : rec<n, n + 1> {};


// Constexpr integer power function
template <typename T>
constexpr T ipow(T num, unsigned int exp) {
   if (exp >= sizeof(T) * 8)
      return 0;
   else if (exp == 0)
      return 1;
   else
       return num * ipow(num, exp - 1);
}

// "An Efficient Algorithm for the Riemann Zeta Function". P. Borwein.
// http://www.cecm.sfu.ca/personal/pborwein/PAPERS/P155.pdf

constexpr int ZETA_PRECISION = 15;

   constexpr long int zetaCoeff(int j) {
   long long int res = -ipow(2, ZETA_PRECISION);
      if (j > ZETA_PRECISION) {
         int last = j - ZETA_PRECISION;
         for (int k = 0; k <= last; k++) {
            res += binomial<ZETA_PRECISION>::value[k];
         }
      }
      if (j % 2)
         return -res;
      return res;
   }

template <typename T> T zeta(T s) {
   T res    = 0;
   int last = 2 * ZETA_PRECISION - 1;
   for (int i = 0; i < last; i++) {
      res += (T)zetaCoeff(i) / (T)pow(i + 1.0, s);
   }
   res *=
       (T)-1.0 / (T)((T)ipow(2, ZETA_PRECISION) * ((T)1 - pow(2.0, (T)1 - s)));
   return res;
}