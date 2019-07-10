
#pragma once

// This file defines the marble class template and related functions.



// -----------------------------------------------------------------------------
// marble
// zzz make this more hierarchical, e.g. based on full RGB<T> or crayola
// -----------------------------------------------------------------------------

template<class T = unsigned char, class real = double>
class marble {
public:
   static const char *const description;
   using value_t = T;

   // components
   union { mutable value_t r, red; };
   union { mutable value_t g, green; };
   union { mutable value_t b, blue; };

   // noise information
   mutable real amp, ampfac;
   mutable real per, perfac;
   mutable unsigned nfun, seed;
   mutable bool swirl;

   // marble()
   inline explicit marble() :
      r(internal::midcolor<value_t>()),
      g(internal::midcolor<value_t>()),
      b(internal::midcolor<value_t>()),
      amp(1), ampfac(0.5),
      per(1), perfac(0.5),
      nfun(1), seed(0),
      swirl(false)
   { }

   // marble(r,g,b[, amp[,ampfac[, per[,perfac[, nfun[,seed[, swirl]]]]]]])
   inline explicit marble(
      const value_t _r, const value_t _g, const value_t _b,
      const real _amp = 1, const real _ampfac = 0.5,
      const real _per = 1, const real _perfac = 0.5,
      const unsigned _nfun = 1,
      const unsigned _seed = 0,
      const bool _swirl = false
   ) :
      r(_r), g(_g), b(_b),
      amp(_amp), ampfac(_ampfac),
      per(_per), perfac(_perfac),
      nfun(_nfun), seed(_seed),
      swirl(_swirl)
   { }
};

template<class T, class real>
const char *const marble<T,real>::description = "marble";



// random(marble)
template<class C, class real>
inline marble<C,real> &random(marble<C,real> &obj)
{
   RGB<C> tmp;
   random(tmp);

   obj.r = tmp.r;
   obj.g = tmp.g;
   obj.b = tmp.b;

   obj.amp    = 1 + random<real>();
   obj.ampfac = 1;

   obj.per    = 0.005*(1 + random<real>());
   obj.perfac = 1;

   obj.nfun = 1;
   obj.seed = unsigned(1000000*random<real>());

   obj.swirl = false;
   return obj;
}



// -----------------------------------------------------------------------------
// read_value(marble)
// -----------------------------------------------------------------------------

template<class ISTREAM, class C, class real>
bool read_value(
   ISTREAM &s, marble<C,real> &value,
   const std::string &description = "marble"
) {
   // r,g,b, amp,ampfac, per,perfac, nfun,seed, swirl

   int swirl;  // so we read as int, not bool
   s.bail = false;

   if (!(
      read_color_component(s,value.r) && read_comma(s) &&
      read_color_component(s,value.g) && read_comma(s) &&
      read_color_component(s,value.b) && read_comma(s) &&

      read_value(s,value.amp   ) && read_comma(s) &&
      read_value(s,value.ampfac) && read_comma(s) &&

      read_value(s,value.per   ) && read_comma(s) &&
      read_value(s,value.perfac) && read_comma(s) &&

      read_value(s,value.nfun) && read_comma(s) &&
      read_value(s,value.seed) && read_comma(s) &&
      read_value(s,swirl)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }

   value.swirl = swirl;
   return !s.fail();
}

/*
   const real _amp = 1, const real _ampfac = 0.5,
   const real _per = 1, const real _perfac = 0.5,
   const unsigned _nfun = 1,
   const unsigned _seed = 0,
   const bool _swirl = false
*/
