
#pragma once

// This file provides some basic macros, types, functions, classes, etc.

// -----------------------------------------------------------------------------
// Miscellaneous
// -----------------------------------------------------------------------------

// u*
// I prefer this slightly shorter, no-underscore terminology
using uchar = unsigned char;
using ulong = std::size_t;

// Default type: real
using default_real = double;

// Key - like key() below, but always on (not turned off by KIP_NOKEY)
inline void Key()
{
   std::cout << "press <enter> to continue...";
   char ch;
   std::cin.get(ch);
}

// key
inline void key()
{
   #ifndef KIP_NOKEY
      Key();
   #endif
}

// default_parameter
namespace internal { template<class T> class tclass; }
template<class T>
inline T &default_parameter(const internal::tclass<T> &)
{
   static T value;
   return value;
}

// pi
template<class real>
inline constexpr real pi = real(
3.141592653589793238462643383279502884197169399375105820974944592307816406286L);



// kip::cerr
#ifdef kip_cerr_is_not_cout
   inline std::ostream &cerr = std::cerr;
#else
   inline std::ostream &cerr = std::cout;
#endif


// KIP_ASSERT
#ifdef KIP_ASSERT
   #define kip_assert(test) assert(test)
#else
   #define kip_assert(test)
#endif

// KIP_ASSERT_INDEX
#ifdef KIP_ASSERT_INDEX
   #define kip_assert_index(test) assert(test)
#else
   #define kip_assert_index(test)
#endif


// print
template<class T>
inline void print(const T &value)
{
   std::cout << value << std::endl;
}

// ray tracing method
enum class method_t {
   uniform,
   recursive,
   block
};

namespace internal {
   // no_action
   inline void no_action() { }

   // tclass
   template<class T> class tclass { };
}

namespace op {
   // clip
   template<class T>
   inline T clip(const T lower, const T value, const T upper)
   {
      return
         value < lower ? lower
       : value > upper ? upper
       : value;
   }
}



// -----------------------------------------------------------------------------
// random_*
// int rand() \in [0..RAND_MAX]
// -----------------------------------------------------------------------------

// random_unit: [0,1]
template<class real>
inline real random_unit()
{
   return rand()*(real(1)/RAND_MAX);
}

// random_full: [-1,1)
template<class real>
inline real random_full()
{
   return (rand()-RAND_MAX/2)*(real(1)/(-1-RAND_MAX/2));
}

// random_half: [-0.5,0.5)
template<class real>
inline real random_half()
{
   return (rand()-RAND_MAX/2)*(real(1)/(-1-RAND_MAX));
}



// -----------------------------------------------------------------------------
// Formatting (of shape output)
// -----------------------------------------------------------------------------

// format_t
enum class format_t {
   // ------------- primitives --- operators
   // -----------------------------------------
   format_stub,  // name()         one-line
   format_one,   // one-line       one-line
   format_op,    // one-line       multi-line
   format_full,  // multi-line     multi-line
};

// for the user
inline format_t format = format_t::format_op;


// pform (for primitives)
inline const char *pform()
{
   assert(format != format_t::format_stub);  // or we shouldn't have called this function
   return format == format_t::format_one  || format == format_t::format_op  ? " " : "\n   ";
}

// oform (for operators)
inline const char *oform()
{
   return format == format_t::format_stub || format == format_t::format_one ? " " : "\n   ";
}



// -----------------------------------------------------------------------------
// Diagnostics - miscellaneous
// -----------------------------------------------------------------------------

// diagnostic_t
enum class diagnostic_t {
   diagnostic_good    = 3,
   diagnostic_note    = 2,
   diagnostic_warning = 1,
   diagnostic_error   = 0
};

// no-context warning: forward declaration
template<class MESSAGE>
inline diagnostic_t warning(const MESSAGE &message);

// for the user
inline bool warnings = true;
inline bool notes    = true;
inline bool addenda  = true;



// -----------------------------------------------------------------------------
// Diagnostics - functions
// -----------------------------------------------------------------------------

namespace internal {

// tostring: helper function
inline std::string tostring(const char *const         str) { return str; }
inline std::string tostring(const std::string        &str) { return str; }
inline std::string tostring(const std::ostringstream &oss) { return oss.str(); }

// diagnostic: helper function
template<class CONTEXT, class MESSAGE>
void diagnostic(
   const char *const type, const CONTEXT &_context, const MESSAGE &_message
) {
// static const char *const prefix = "    | ";  // possibly use this
   static const char *const prefix = "      ";

   if (type[0]) {
      // introduction
      kip::cerr << "\n[kip] " << type;

      // context
      const std::string context = tostring(_context);
      if (context != "") {
         kip::cerr << ": ";
         for (const char *s = &context[0];  *s;  ++s)
            if (*s == '\n')
               kip::cerr << '\n' << prefix;
            else
               kip::cerr << *s;
      }
      kip::cerr << '\n';
   }

   // message
   const std::string message = tostring(_message);
   kip::cerr << prefix;
   for (const char *s = &message[0];  *s;  ++s)
      if (*s == '\n')
         kip::cerr << '\n' << prefix;
      else
         kip::cerr << *s;
   kip::cerr << std::endl;
}

} // namespace internal



// error
template<class CONTEXT, class MESSAGE>
inline diagnostic_t error(const CONTEXT &context, const MESSAGE &message)
{
   internal::diagnostic("error", context, message);
   return diagnostic_t::diagnostic_error;
}

// no-context
template<class MESSAGE>
inline diagnostic_t error(const MESSAGE &message)
{
   return error("", message);
}



// warning
template<class CONTEXT, class MESSAGE>
inline diagnostic_t warning(const CONTEXT &context, const MESSAGE &message)
{
   return warnings
      ? internal::diagnostic("warning", context, message), diagnostic_t::diagnostic_warning
      : diagnostic_t::diagnostic_good;
}

// no-context
template<class MESSAGE>
inline diagnostic_t warning(const MESSAGE &message)
{
   return warning("", message);
}



// note
template<class CONTEXT, class MESSAGE>
inline diagnostic_t note(const CONTEXT &context, const MESSAGE &message)
{
   return notes
      ? internal::diagnostic("note", context, message), diagnostic_t::diagnostic_note
      : diagnostic_t::diagnostic_good;
}

// no-context
template<class MESSAGE>
inline diagnostic_t note(const MESSAGE &message)
{
   return note("", message);
}



// addendum
// addendum to previous error, warning, or note (indicated by argument d);
// always no-context
template<class MESSAGE>
inline diagnostic_t addendum(const MESSAGE &message, const diagnostic_t d)
{
   if (addenda && (
       d == diagnostic_t::diagnostic_error                ||
      (d == diagnostic_t::diagnostic_warning && warnings) ||
      (d == diagnostic_t::diagnostic_note    && notes   )
   ))
      internal::diagnostic("", "", message);
   return d;
}



// -----------------------------------------------------------------------------
// Miscellaneous arithmetic functions
// -----------------------------------------------------------------------------

namespace op {

// round
template<class INTEGER, class T>
inline INTEGER round(const T val)
{
   return INTEGER(val < T(0) ? val-T(0.5) : val+T(0.5));
}

// twice
template<class T>
inline T twice(const T val)
{
   return val+val;
}

// square
template<class T>
inline T square(const T val)
{
   return val*val;
}

} // namespace op



// -----------------------------------------------------------------------------
// Multi-argument (up to 6) min, max
// -----------------------------------------------------------------------------

namespace op {

// min
template<class T>
inline const T &min(const T &a, const T &b)
   { return std::min(a,b); }

template<class T>
inline const T &min(const T &a, const T &b, const T &c)
   { return op::min(a,op::min(b,c)); }

template<class T>
inline const T &min(const T &a, const T &b, const T &c, const T &d)
   { return op::min(a,op::min(b,c,d)); }

template<class T>
inline const T &min(const T &a, const T &b, const T &c, const T &d, const T &e)
   { return op::min(a,op::min(b,c,d,e)); }

template<class T>
inline const T &min(const T &a, const T &b, const T &c,
              const T &d, const T &e, const T &f)
   { return op::min(a,op::min(b,c,d,e,f)); }

// max
template<class T>
inline const T &max(const T &a, const T &b)
   { return std::max(a,b); }

template<class T>
inline const T &max(const T &a, const T &b, const T &c)
   { return op::max(a,op::max(b,c)); }

} // namespace op



// -----------------------------------------------------------------------------
// kip_expand
// kip_extra
// -----------------------------------------------------------------------------

// kip_expand
#define kip_expand(macro,sym) \
   \
   macro(kipnot) sym \
   \
   macro(kipand) sym   macro(kipcut) sym \
   macro(kipor ) sym   macro(kipxor) sym \
   \
   macro(ands) sym   macro(odd) sym   macro(even) sym \
   macro(some) sym   macro(one) sym   macro(ors ) sym \
   \
   macro(bicylinder) sym  macro(biwasher  ) sym  macro(box       ) sym \
   macro(circle    ) sym  macro(cone      ) sym  macro(cylinder  ) sym \
   macro(ellipsoid ) sym  macro(half      ) sym  macro(paraboloid) sym \
   macro(cube      ) sym  macro(nothing   ) sym  macro(everything) sym \
   macro(pill      ) sym  macro(polygon   ) sym  macro(silo      ) sym \
   macro(sphere    ) sym  macro(spheroid  ) sym  macro(tabular   ) sym \
   macro(triangle  ) sym  macro(washer    ) sym  macro(xplane    ) sym \
   macro(yplane    ) sym  macro(zplane    ) sym  macro(surf      ) sym

// kip_extra
#define kip_extra(macro,sym) \
   \
   macro(tri) sym



// -----------------------------------------------------------------------------
// threads etc.
// -----------------------------------------------------------------------------

// threads (user-settable)
inline int threads = 0; // 0 = default (means to ask the system)

// get_nthreads
// set_nthreads
// this_thread
#ifdef _OPENMP
   inline int get_nthreads()
   {
      return threads
         // IF specified, number of specified threads (clipped to #processors)
         ? op::min(omp_get_num_procs(), threads)
         // else, default to #processors
         : omp_get_num_procs();
   }

   inline void set_nthreads(const int nthreads)
   {
      omp_set_num_threads(nthreads);
   }

   inline int this_thread()
   {
      return omp_get_thread_num();
   }
#else
   inline int  get_nthreads() { return 1; }
   inline void set_nthreads(const int) { }
   inline int  this_thread () { return 0; }
#endif
