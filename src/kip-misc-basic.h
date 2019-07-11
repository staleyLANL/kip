
#pragma once

// This file provides some basic macros, types, functions, classes, etc.

// -----------------------------------------------------------------------------
// Miscellaneous
// -----------------------------------------------------------------------------

// Default type: real
using default_real_t = double;

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

// random
template<class real>
inline real random()
{
   return real(rand())*(1/real(RAND_MAX));
}



// pi
template<class real>
inline constexpr real pi = real(
3.141592653589793238462643383279502884197169399375105820974944592307816406286L);



// kip::cerr
#ifdef kip_cerr_is_not_cout
   // cerr is cerr
   inline std::ostream &cerr = std::cerr;
#else
   // cerr is cout
   inline std::ostream &cerr = std::cout;
#endif


// KIP_ASSERT
#ifdef KIP_ASSERT
   #define if_kip_assert(test) assert(test)
#else
   #define if_kip_assert(test)
#endif


// KIP_ASSERT_INDEX
#ifdef KIP_ASSERT_INDEX
   #define if_kip_assert_index(test) assert(test)
#else
   #define if_kip_assert_index(test)
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
      template<class T> class tclass
      {
      public:
         inline explicit tclass() { }
      };
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
   diagnostic_good    = 3,  // everything is perfect
   diagnostic_note    = 2,  // for-your-information
   diagnostic_warning = 1,  // likely problem
   diagnostic_error   = 0   // definite error
};


// no-context warning: forward declaration
template<class MESSAGE>
inline diagnostic_t warning(const MESSAGE &message);



// helper classes/functions
namespace internal {
   // errors
   class errors_class {
   public:
      static const bool value = true;
      inline explicit errors_class() { }
      inline const errors_class &operator=(const bool _value) const
      {
         if (!_value)
            warning("kip::errors = false?\n"
                    "(un)fortunately, you can't turn off errors");
         return *this;
      }
      inline operator bool() const { return true; }
   };
}

// for the user
inline internal::errors_class errors;
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

// sgn
//   -1, if x < 0
//    1, if x > 0
//    0, else
template<class T>
inline int sgn(const T &x)
{
   return x < 0 ? -1 : x > 0 ? 1 : 0;
}



// abs
template<class T>
inline T abs(const T &val)
   { return val < T(0) ? -val : val; }

inline unsigned abs(const int val)
   { return unsigned(std::abs(val)); }

inline unsigned long abs(const long val)
   { using ulong = unsigned long; return ulong(std::labs(val)); }

inline float abs(const float val)
   { return ((float (*)(const float))(std::abs))(val); }

inline long double abs(const long double val)
   { return ((long double (*)(const long double))(std::abs))(val); }

inline double abs(const double val)
   { return std::fabs(val); }



// sqrt
inline float sqrt(const float val)
   { return ((float (*)(const float))(std::sqrt))(val); }

inline double sqrt(const double val)
   { return std::sqrt(val); }

inline long double sqrt(const long double val)
   { return ((long double (*)(const long double))(std::sqrt))(val); }



   /*
// pow
inline float pow(const float val, const float n)
   { return ((float (*)(const float, const float))(std::pow))(val,n); }

inline float pow(const float val, const int n)
   { return ((float (*)(const float, const int))(std::pow))(val,n); }

inline double pow(const double val, const double n)
   { return ((double (*)(const double, const double))(std::pow))(val,n); }

inline double pow(const double val, const int n)
   { return ((double (*)(const double, const int))(std::pow))(val,n); }

inline long double pow(const long double val, const long double n)
   { return ((long double (*)(const long double, const long double))
             (std::pow))(val,n); }

inline long double pow(const long double val, const int n)
   { return ((long double (*)(const long double, const int))
             (std::pow))(val,n); }
   */



// fmod
inline float fmod(const float a, const float b)
   { return ((float (*)(const float, const float))(std::fmod))(a,b); }

inline long double fmod(const long double a, const long double b)
   { return ((long double (*)(const long double, const long double))
             (std::fmod))(a,b); }

inline double fmod(const double a, const double b)
   { return ((double (*)(const double, const double))(std::fmod))(a,b); }



// sin
inline float sin(const float val)
   { return ((float (*)(const float))(std::sin))(val); }

inline double sin(const double val)
   { return std::sin(val); }

inline long double sin(const long double val)
   { return ((long double (*)(const long double))(std::sin))(val); }


// cos
inline float cos(const float val)
   { return ((float (*)(const float))(std::cos))(val); }

inline double cos(const double val)
   { return std::cos(val); }

inline long double cos(const long double val)
   { return ((long double (*)(const long double))(std::cos))(val); }


// tan
inline float tan(const float val)
   { return ((float (*)(const float))(std::tan))(val); }

inline double tan(const double val)
   { return std::tan(val); }

inline long double tan(const long double val)
   { return ((long double (*)(const long double))(std::tan))(val); }



// rnd (round a value)
template<class INTEGER, class T>
inline INTEGER rnd(const T &val)
{
   return INTEGER(val < T(0) ? val-T(0.5) : val+T(0.5));
}


// twice (two times a value)
template<class T>
inline T twice(const T &val)
{
   return val+val;
}


// sq (square a value)
template<class T>
inline T sq(const T &val)
{
   return val*val;
}

} // namespace op



// -----------------------------------------------------------------------------
// Multi-argument (up to 6) min, max
// -----------------------------------------------------------------------------

namespace op {

// min
template<class T> inline T
min(const T &a, const T &b)
   { return std::min(a,b); }

template<class T> inline T
min(const T &a, const T &b, const T &c)
   { return op::min(a,op::min(b,c)); }

template<class T> inline T
min(const T &a, const T &b, const T &c, const T &d)
   { return op::min(a,op::min(b,c,d)); }

template<class T> inline T
min(const T &a, const T &b, const T &c, const T &d, const T &e)
   { return op::min(a,op::min(b,c,d,e)); }

template<class T> inline T
min(const T &a, const T &b, const T &c, const T &d, const T &e, const T &f)
   { return op::min(a,op::min(b,c,d,e,f)); }

// max
template<class T> inline T
max(const T &a, const T &b)
   { return std::max(a,b); }

template<class T> inline T
max(const T &a, const T &b, const T &c)
   { return op::max(a,op::max(b,c)); }

} // namespace op



// -----------------------------------------------------------------------------
// kip_expand*
// kip_extra*
// -----------------------------------------------------------------------------

// ------------------------
// kip_expand*
// ------------------------

// without semicolon
#define kip_expand_plain(fun)\
   \
   fun( kipnot ) \
   \
   fun( kipand )   fun( kipcut ) \
   fun( kipor  )   fun( kipxor ) \
   \
   fun( ands   )   fun( odd    )   fun( even ) \
   fun( some   )   fun( one    )   fun( ors  ) \
   \
   fun( bicylinder )  fun( biwasher   )  fun( box        ) \
   fun( circle     )  fun( cone       )  fun( cylinder   ) \
   fun( ellipsoid  )  fun( half       )  fun( paraboloid ) \
   fun( cube       )  fun( nothing    )  fun( everything ) \
   fun( pill       )  fun( polygon    )  fun( silo       ) \
   fun( sphere     )  fun( spheroid   )  fun( tabular    ) \
   fun( triangle   )  fun( washer     )  fun( xplane     ) \
   fun( yplane     )  fun( zplane     )  fun( surf       )

// with semicolon
#define kip_expand_semi(fun)\
   \
   fun( kipnot );\
   \
   fun( kipand );  fun( kipcut );\
   fun( kipor  );  fun( kipxor );\
   \
   fun( ands   );  fun( odd    );  fun( even );\
   fun( some   );  fun( one    );  fun( ors  );\
   \
   fun( bicylinder );  fun( biwasher   );  fun( box        );\
   fun( circle     );  fun( cone       );  fun( cylinder   );\
   fun( ellipsoid  );  fun( half       );  fun( paraboloid );\
   fun( cube       );  fun( nothing    );  fun( everything );\
   fun( pill       );  fun( polygon    );  fun( silo       );\
   fun( sphere     );  fun( spheroid   );  fun( tabular    );\
   fun( triangle   );  fun( washer     );  fun( xplane     );\
   fun( yplane     );  fun( zplane     );  fun( surf       );



// ------------------------
// kip_extra*
// ------------------------

// without semicolon
#define kip_extra_plain(fun)\
   \
   fun( tri )

// with semicolon
#define kip_extra_semi(fun)\
   \
   fun( tri );



// -----------------------------------------------------------------------------
// Constructor/destructor call-counters
// -----------------------------------------------------------------------------

#ifdef KIP_CONSTRUCT_COUNTER
   class counter {
   public:
      #define kip_construct_counter(type) static long int type
      kip_expand_semi(kip_construct_counter)
      kip_extra_semi (kip_construct_counter)
      #undef  kip_construct_counter
   };

   #define kip_construct_counter(type) long int counter::type = 0
   kip_expand_semi(kip_construct_counter)
   kip_extra_semi (kip_construct_counter)
   #undef  kip_construct_counter

   #define kip_counter_ctor(type) counter::type++
   #define kip_counter_dtor(type) counter::type--

#else
   #define kip_counter_ctor(type) {}
   #define kip_counter_dtor(type) {}
#endif



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

   inline int this_thread() {
      return omp_get_thread_num();
   }
#else
   inline int  get_nthreads() { return 1; }
   inline void set_nthreads(const int) { }
   inline int  this_thread () { return 0; }
#endif
