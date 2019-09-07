
#pragma once

// This file defines the RGB and RGBA class templates, and related functions.

// Default component type for red, green, and blue (and alpha)
using default_comp = uchar;

// Forward
class crayola;



// -----------------------------------------------------------------------------
// helper functions
// -----------------------------------------------------------------------------

namespace detail {

// if_true
template<bool, class A, class B = A>
class if_true { };

template<class A, class B>
class if_true<true,A,B> {
public:
   using result = B;
};

// is_floating: default
template<class T> class is_floating
   { public: static const bool result = false; };

// is_floating: float, double, long double
template<> class is_floating<float>
   { public: static const bool result = true; };
template<> class is_floating<double>
   { public: static const bool result = true; };
template<> class is_floating<long double>
   { public: static const bool result = true; };



// mincolor, for unsigned integral
// mincolor, for floating
template<class T>
inline
   typename if_true<
     (std::numeric_limits<T>::is_integer &&
     !std::numeric_limits<T>::is_signed) ||
      is_floating<T>::result,
      T
   >::result
mincolor()
{
   return T(0);
}



// maxcolor, for unsigned integral
template<class T>
inline
   typename if_true<
      std::numeric_limits<T>::is_integer &&
     !std::numeric_limits<T>::is_signed,
      T
   >::result
maxcolor()
{
   return std::numeric_limits<T>::max();
}

// maxcolor, for floating
template<class T>
inline
   typename if_true<
      is_floating<T>::result,
      T
   >::result
maxcolor()
{
   return T(1);
}



// midcolor, for unsigned integral
// midcolor, for floating
template<class T>
inline T midcolor()
{
   return T(
      (detail::maxcolor<T>() -
       detail::mincolor<T>()
      )/2
   );
}



// endcolor, for unsigned integral
// Result type: "to"
template<class to, class T>
inline
   typename if_true<
      std::numeric_limits<T>::is_integer &&
     !std::numeric_limits<T>::is_signed,
      T,
      to
   >::result
endcolor()
{
   // e.g., 256 (not 255) for 8-bit uchar
   return to(maxcolor<T>()) + to(1);
}

// endcolor, for floating
// Result type: "to"
template<class to, class T>
inline
   typename if_true<
      is_floating<T>::result,
      T,
      to
   >::result
endcolor()
{
   // same as maxcolor
   return to(1);
}

} // namespace detail



// -----------------------------------------------------------------------------
// RGB (templated)
// rgb (not templated)
// -----------------------------------------------------------------------------

template<class rgb_t = default_comp>
class RGB {
public:
   static const char *const description;
   using value_t = rgb_t;


   // components
   union { value_t r, red  ; };
   union { value_t g, green; };
   union { value_t b, blue ; };


   // RGB()
   inline explicit RGB() : r(0), g(0), b(0) { }

   // RGB(r,g,b)
   inline explicit RGB(
      const value_t &_r, const value_t &_g, const value_t &_b
   ) : r(_r), g(_g), b(_b) { }

   // conversion from crayola
   inline RGB(const crayola &from);

   // operator()(r,g,b), for convenience
   inline RGB &operator()(
      const value_t &_r, const value_t &_g, const value_t &_b
   ) {
      r = _r;
      g = _g;
      b = _b;
      return *this;
   }
};

// description
template<class rgb_t> const char *const RGB<rgb_t>::description = "RGB";///

// for users
using rgb = RGB<>;



// -----------------------------------------------------------------------------
// RGBA (templated)
// rgba (not templated)
// -----------------------------------------------------------------------------

// transparent
template<class component_t>
inline component_t transparent()
{
   return detail::mincolor<component_t>();
}

// opaque
template<class component_t>
inline component_t opaque()
{
   return detail::maxcolor<component_t>();
}



// RGBA
template<class rgb_t = default_comp>
class RGBA {
public:
   static const char *const description;
   using value_t = rgb_t;


   // components
   union { value_t r, red  ; };
   union { value_t g, green; };
   union { value_t b, blue ; };
   union { value_t a, alpha; };


   // RGBA()
   inline explicit RGBA() { }

   // RGBA(r,g,b[,a])
   inline explicit RGBA(
      const value_t &_r, const value_t &_g, const value_t &_b,
      const value_t &_a = opaque<rgb_t>()
   ) : r(_r), g(_g), b(_b), a(_a) { }

   // conversion from RGB
   inline RGBA(const RGB<rgb_t> &from) :
      r(from.r), g(from.g), b(from.b), a(opaque<rgb_t>())
   { }

   // conversion from crayola
   inline RGBA(const crayola &from);

   // operator()(r,g,b[,a]), for convenience
   inline RGBA &operator()(
      const value_t &_r, const value_t &_g, const value_t &_b
      // "a" not given
   ) {
      r = _r;
      g = _g;
      b = _b;
      // keep existing "a"
      return *this;
   }

   inline RGBA &operator()(
      const value_t &_r, const value_t &_g, const value_t &_b,
      // "a" given
      const value_t &_a
   ) {
      r = _r;
      g = _g;
      b = _b;
      a = _a;  // use given "a"
      return *this;
   }

   // alpha meanings
   // static const value_t transparent;
   // static const value_t opaque;

   // background
   static inline RGBA background()
   {
      const value_t mid = detail::midcolor<value_t>();
      return RGBA(mid,mid,mid);
   }

   // border
   static inline RGBA border(const ulong num, const ulong den)
   {
      const value_t max = detail::maxcolor<value_t>();

      // both nonzero: use fraction
      if (num && den) {
         if (!(num <= den))
            return RGBA(max, 0, 0);  // shouldn't happen; give red as warning
         const value_t level = value_t(max*num/den);
         return RGBA(level, level, level);
      }

      // nonzero, zero: means second was unknown (as with recursive trace)
      if (num)
         return RGBA(max, max, max);

      // bin size is zero, in any event
      return RGBA(0, 0, 0);
   }

   // border
   static inline RGBA border()
   {
      const value_t max = detail::maxcolor<value_t>();
      return RGBA(max,max,0);
   }
};

// description
template<class rgb_t> const char *const RGBA<rgb_t>::description = "RGBA";

// for users
using rgba = RGBA<>;



// -----------------------------------------------------------------------------
// randomize(RGB)
// -----------------------------------------------------------------------------

// randomize(RGB<rgb_t>), for rgb_t = unsigned integral
template<class rgb_t>
inline RGB<
   typename detail::if_true<
      std::numeric_limits<rgb_t>::is_integer &&
     !std::numeric_limits<rgb_t>::is_signed,
      rgb_t
   >::result
> &randomize(RGB<rgb_t> &obj)
{
   static const double scale = detail::endcolor<double,rgb_t>();

   obj.r = rgb_t(random_unit<double>()*scale);
   obj.g = rgb_t(random_unit<double>()*scale);
   obj.b = rgb_t(random_unit<double>()*scale);

   return obj;
}



// randomize(RGB<rgb_t>), for rgb_t = floating
template<class rgb_t>
inline RGB<
   typename detail::if_true<
      detail::is_floating<rgb_t>::result,
      rgb_t
   >::result
> &randomize(RGB<rgb_t> &obj)
{
   obj.r = random_unit<rgb_t>();
   obj.g = random_unit<rgb_t>();
   obj.b = random_unit<rgb_t>();

   return obj;
}



// -----------------------------------------------------------------------------
// Some operators for RGBA
// -----------------------------------------------------------------------------

// RGBA<OUT> += RGBA<IN>
template<class OUT, class IN>
inline RGBA<OUT> &operator+=(RGBA<OUT> &lhs, const RGBA<IN> &rhs)
{
   lhs.r += OUT(rhs.r);
   lhs.g += OUT(rhs.g);
   lhs.b += OUT(rhs.b);
   lhs.a += OUT(rhs.a);
   return lhs;
}

// op::div(RGBA,den)
namespace op {
   template<class OUT, class IN>
   inline RGBA<OUT> div(const RGBA<IN> &value, const unsigned den)
   {
      return RGBA<OUT>(
         OUT((value.r + den/2) / den),
         OUT((value.g + den/2) / den),
         OUT((value.b + den/2) / den),
         OUT((value.a + den/2) / den)
      );
   }
}
