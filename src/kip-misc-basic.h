
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
namespace detail { template<class T> class tclass; }
template<class T>
inline T &default_parameter(const detail::tclass<T> &)
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

namespace detail {
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
// -----------------------------------------------------------------------------

// random_unit: [0,1)
template<class real>
inline real random_unit()
{
   return real(drand48());
}

// random_full: [-1,1)
template<class real>
inline real random_full()
{
   return real(2*drand48()-1);
}

// random_half: [-0.5,0.5)
template<class real>
inline real random_half()
{
   return real(drand48()-0.5);
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
   // or we shouldn't have called this function...
   assert(format != format_t::format_stub);

   return format == format_t::format_one  || format == format_t::format_op
      ? " "
      : "\n   ";
}

// oform (for operators)
inline const char *oform()
{
   return format == format_t::format_stub || format == format_t::format_one
      ? " "
      : "\n   ";
}



// -----------------------------------------------------------------------------
// Diagnostics - miscellaneous
// -----------------------------------------------------------------------------

// diagnostic
enum class diagnostic {
   good    = 3,
   note    = 2,
   warning = 1,
   error   = 0
};

// forward: warning, no-context
template<class MESSAGE>
inline diagnostic warning(const MESSAGE &message);

// for the user
inline bool warnings = true;
inline bool notes    = true;
inline bool addenda  = true;



// -----------------------------------------------------------------------------
// Diagnostics - functions
// -----------------------------------------------------------------------------

namespace detail {

// tostring: helper function
inline std::string tostring(const char *const         str) { return str; }
inline std::string tostring(const std::string        &str) { return str; }
inline std::string tostring(const std::ostringstream &oss) { return oss.str(); }

// diagprint: helper function
template<class CONTEXT, class MESSAGE>
void diagprint(
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

} // namespace detail



// error
template<class CONTEXT, class MESSAGE>
inline diagnostic error(const CONTEXT &context, const MESSAGE &message)
{
   detail::diagprint("error", context, message);
   return diagnostic::error;
}

// no-context
template<class MESSAGE>
inline diagnostic error(const MESSAGE &message)
{
   return error("", message);
}



// warning
template<class CONTEXT, class MESSAGE>
inline diagnostic warning(const CONTEXT &context, const MESSAGE &message)
{
   return warnings
      ? detail::diagprint("warning", context, message), diagnostic::warning
      : diagnostic::good;
}

// no-context
template<class MESSAGE>
inline diagnostic warning(const MESSAGE &message)
{
   return warning("", message);
}



// note
template<class CONTEXT, class MESSAGE>
inline diagnostic note(const CONTEXT &context, const MESSAGE &message)
{
   return notes
      ? detail::diagprint("note", context, message), diagnostic::note
      : diagnostic::good;
}

// no-context
template<class MESSAGE>
inline diagnostic note(const MESSAGE &message)
{
   return note("", message);
}



// addendum
// addendum to previous error, warning, or note (indicated by argument d);
// always no-context
template<class MESSAGE>
inline diagnostic addendum(const MESSAGE &message, const diagnostic d)
{
   if (addenda && (
       d == diagnostic::error                ||
      (d == diagnostic::warning && warnings) ||
      (d == diagnostic::note    && notes   )
   ))
      detail::diagprint("", "", message);
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
