
// This file defines the shape class template, related functions, and other
// functionality for use by shape and by classes derived from shape.

// vars
namespace detail {
   template<class real = default_real, class tag = default_base>
   class vars;
}

// engine
template<class real = default_real>
class engine;

// dummy
class dummy { };



// -----------------------------------------------------------------------------
// nary_element
// shape_id_t
// -----------------------------------------------------------------------------

// nary_element (for all nary operators except ands)
template<class real, class tag, class MIN = real>
class nary_element {
public:
   MIN min;
   shape<real,tag> *op;
   bool in;
};

// nary_element (for ands)
template<class real, class tag>
class nary_element<real,tag,dummy> {
public:
   shape<real,tag> *op;
   bool in;
};



// shape_id_t
// Note: I found that by using uchar, not an enum,
// these allow for a smaller sizeof(shape<>).
using shape_id_t = uchar;

namespace detail {
   // unique_number
   template<class T>
   inline T unique_number()
   {
      static T value = T(0);
      return value++;
   }

   // get_shape_id
   // Unique per shape, independent of shape's template parameters
   template<template<class,class> class>
   class get_shape_id {
   public:
      static const shape_id_t result;
   };

   template<template<class,class> class SHAPE>
   const shape_id_t get_shape_id<SHAPE>::result = unique_number<shape_id_t>();
} // namespace detail



// -----------------------------------------------------------------------------
// minend
// subinfo
// eyetardiff
// -----------------------------------------------------------------------------

// minend
class minend {
public:
   u32 imin, iend, jmin, jend;
};

// subinfo
namespace detail {
   class subinfo {
   public:
      minend mend;
      u32 i, j;
      unsigned nzone;

      explicit subinfo(
         const u32 _i,
         const u32 _j,
         const unsigned _nzone,
         const minend _mend
      ) :
         mend(_mend),
         i(_i), j(_j),
         nzone(_nzone)
      { }
   };
}

// eyetardiff
template<class real>
class eyetardiff {
public:
   point<real> eyeball;
   point<real> target;
   point<real> diff;

   explicit eyetardiff(
      const point<real> e,
      const point<real> t,
      const point<real> d
   ) :
      eyeball(e),
      target(t),
      diff(d)
   { }
};



// -----------------------------------------------------------------------------
// shape
// -----------------------------------------------------------------------------

template<class real, class tag>
class shape {
public:

   // --------------------------------
   // Miscellaneous
   // --------------------------------

   // id
   virtual shape_id_t id() const = 0;

   // base()
   const tag &base() const { return thebase; }
   tag &base() { return thebase; }


   // --------------------------------
   // Block data, if necessary
   // --------------------------------

   #ifdef KIP_BLOCK
      mutable real minimum;
      mutable int  lastpix;
   #endif


   // --------------------------------
   // Primary data
   // --------------------------------

   // for certain specific shapes
   union misc_type {
      // half
      class { public: mutable real tmp; } half;

      // sphere
      class {
         mutable char _f[sizeof(point<real>)];
      public:
         point<real> &f() const { return *(point<real> *)(void *)&_f[0]; }
         mutable real m;
      } sphere;

      // plane
      class { public: mutable real h, norm; } plane;

      // ands
      class ands_type {
         private: using element_t = nary_element<real,tag,dummy>;
         public : using vec_t = std::vector<element_t>;
         private: mutable char _per[sizeof(vec_t)];
      public:
         mutable ulong nop;
         vec_t &vec() const { return *(vec_t *)(void *)&_per[0]; }
         element_t &push() const
         { return vec().push_back(element_t()), vec().back(); }
         mutable ulong total_in;
      } ands;
   };

   // for nary operators (except ands)
   class nary_type {
      private: using element_t = nary_element<real,tag>;
      public : using vec_t = std::vector<element_t>;
      private: mutable char _per[sizeof(vec_t)];
   public:
      mutable ulong nop;
      vec_t &vec() const
         { return *(vec_t *)(void *)&_per[0]; }
      element_t &push() const
         { return vec().push_back(element_t()), vec().back(); }
      mutable ulong total_in;
   };

   // for most shapes
   class union_basic {
      mutable char _eye[sizeof(point<real >)];
      mutable char _lie[sizeof(point<float>)];
   public:
      point<real > &eye() const
         { return *(point<real > *)(void *)&_eye[0]; }
      point<float> &lie() const
         { return *(point<float> *)(void *)&_lie[0]; }
   };

   // for unary operators
   class union_unary {
   public:
      mutable real amin;
      shape *a;
      mutable bool ina : 1;
   };

   // for binary operators
   class union_binary {
      using shape_ptr = shape *;
   public:
      mutable real amin, bmin;
      mutable shape_ptr a, b;
      mutable bool ina : 1;
      mutable bool inb : 1;
   };

   /*
   // for tri
   class union_vertex {
   public:
      mutable char ghi[sizeof(point<real>)];
      mutable ulong u, v, w;
   };
   */


   // --------------------------------
   // Data
   // --------------------------------

   // size 8. virtual function table

   // size 16.
   // [imin,iend) x [jmin,jend)
   mutable minend mend;

   // size 48.
   // union'd data, as we only need one-at-a-time
   union {
      misc_type     misc;    // size 40. for certain specific shapes
      union_basic   basic;   // size 36. for most shapes
      union_unary   unary;   // size 24. for unary operators
      union_binary  binary;  // size 40. for binary operators
      nary_type     nary;    // size 40. for nary operators (except ands)
      ///      union_vertex  vertex;  // size 48. for tri
   };

   // size 3. for rgb
   tag thebase;

   // size 1.
   mutable bool is_operand : 1;
   /*   */ bool eyelie     : 1;
   /*   */ bool on         : 1;
   /*   */ bool solid      : 1;
   mutable bool isbound    : 1;
   mutable bool baseset    : 1;
   mutable bool interior   : 1;
   mutable bool degenerate : 1;


   // --------------------------------
   // Constructors
   // --------------------------------

   // For constructors, remember to initialize:
   //    tag
   //    eyelie
   //    on
   //    solid
   //    isbound
   //    baseset

   // shape(derived *)
   template<template<class,class> class derived, class _real, class _tag>
   explicit shape(const derived<_real,_tag> *const) :
      thebase ( tag() ),
      eyelie  ( true  ),
      on      ( true  ),
      solid   ( true  ),
      isbound ( false ),
      baseset ( false )
   { }

   // shape(derived *, base)
   template<template<class,class> class derived, class _real, class _tag>
   explicit shape(
      const derived<_real,_tag> *const,
      const tag &_thebase
   ) :
      thebase (_thebase ),
      eyelie  ( true    ),
      on      ( true    ),
      solid   ( true    ),
      isbound ( false   ),
      baseset ( true    )
   { }

   // shape(shape)
   // zzz Think about some fields, e.g. baseset; correct semantics may
   // depend on context. May need to delegate to two or more copy() functions.
   shape(const shape &from) :
      thebase ( from.base()  ),
      eyelie  ( from.eyelie  ),
      on      ( from.on      ),
      solid   ( from.solid   ),
      isbound ( from.isbound ),
      baseset ( from.baseset )
   { }


   // --------------------------------
   // Assignment, Destructor
   // --------------------------------

   // shape = shape
   // zzz Similar remark as that for copy c'tor.
   shape &operator=(const shape &from)
   {
      thebase = from.base();
      eyelie  = from.eyelie;
      on      = from.on;
      solid   = from.solid;
      isbound = from.isbound;
      baseset = from.baseset;

      return *this;
   }

   // destructor
   virtual ~shape() { }


   // --------------------------------
   // Virtuals (except id())
   // --------------------------------

   // ----------------
   // PURE...
   // ----------------

   // duplicate, size_of
   virtual shape<real,tag> *duplicate() const = 0;
   virtual ulong size_of() const = 0;

   // write
   virtual kip::ostream &write(kip::ostream &) const = 0;

   // process
   virtual real process(
      const point<real> &, const point<real> &,
      const engine<real> &, const detail::vars<real,tag> &
   ) const = 0;

   // aabb
   virtual bbox<real> aabb() const = 0;

   // dry
   virtual bool dry(const rotate<3,real,op::part,op::unscaled> &) const = 0;

   // infirst
   virtual bool infirst(
      const eyetardiff<real> &,
      const real, inq<real,tag> &, const detail::subinfo &
   ) const = 0;

   // inall
   virtual bool inall(
      const eyetardiff<real> &,
      const real, afew<inq<real,tag>> &, const detail::subinfo &
   ) const = 0;

   // check
   virtual diagnostic check() const = 0;

   // back
   virtual point<real> back(const point<real> &) const = 0;


   // ----------------
   // NON-PURE...
   // ----------------

   // propagate_base; overridden with meaningful propagation iff "operator"
   virtual void propagate_base(const bool = false) const { }
};



// -----------------------------------------------------------------------------
// Macros - general
// -----------------------------------------------------------------------------

// Regarding virtuals:
//    Defines:  [id,] [destructor,] duplicate, size_of, read/write kip/std
//    Declares: process, aabb, dry, first, all, check
//    Ignores:  back, propagate_base

#define kip_functions(kip_class)\
   \
   shape_id_t id() const\
      { return detail::get_shape_id<kip_class>::result; }\
   \
   kip_class *duplicate() const { return new kip_class(*this); }\
   ulong size_of() const { return sizeof(kip_class); }\
   \
   real process(\
      const kip::point<real> &,\
      const kip::point<real> &,\
      const engine<real> &,\
      const detail::vars<real,tag> &\
   ) const;\
   \
   kip::ostream &write(kip::ostream &k) const { return k << *this; }\
   bbox<real> aabb() const;\
   bool dry(const rotate<3,real,op::part,op::unscaled> &) const;\
   \
   bool infirst(\
      const eyetardiff<real> &,\
      const real,\
      inq<real,tag> &,\
      const detail::subinfo &\
   ) const;\
   \
   bool inall(\
      const eyetardiff<real> &,\
      const real,\
      afew<inq<real,tag>> &,\
      const detail::subinfo &\
   ) const;\
   \
   diagnostic check() const
// no ';' at end - semicolons at *invocation* points help emacs indent



// -----------------------------------------------------------------------------
// Macros - specific shape functions
// -----------------------------------------------------------------------------

// kip_process
#define kip_process(type)\
   template<class real, class tag>\
   real type<real,tag>::process(\
      const kip::point<real> &eyeball,\
      const kip::point<real> &light,\
      const engine<real> &engine,\
      const detail::vars<real,tag> &vars\
   ) const {\
      (void)eyeball;  (void)light;\
      (void)engine;   (void)vars;

// kip_aabb
#define kip_aabb(type)\
   template<class real, class tag>\
   bbox<real> type<real,tag>::aabb() const\
   {

// kip_inside
#define kip_inside(type)\
   template<class real, class tag>\
   bool type<real,tag>::inside(\
      const kip::point<real> &eyeball\
   ) const\
   {\
      (void)eyeball;

// kip_dry
#define kip_dry(type)\
   template<class real, class tag>\
   bool type<real,tag>::dry(\
      const rotate<3,real,op::part,op::unscaled> &seg\
   ) const {\
      (void)seg;



// kip_etd
#define kip_etd etd

// kip_infirst
#define kip_infirst(type)\
   template<class real, class tag>\
   bool type<real,tag>::infirst(\
      const eyetardiff<real> &etd,\
      const real qmin,\
      inq<real,tag> &q,\
      const detail::subinfo &insub\
   ) const {\
      const kip::point<real> &eyeball = etd.eyeball;\
      const kip::point<real> &target  = etd.target;\
      const kip::point<real> &diff    = etd.diff;\
      \
      (void)eyeball;  (void)target;  (void)diff;\
      (void)qmin;     (void)q;       (void)insub;

// kip_inall
#define kip_inall(type)\
   template<class real, class tag>\
   bool type<real,tag>::inall(\
      const eyetardiff<real> &etd,\
      const real qmin,\
      afew<inq<real,tag>> &ints,\
      const detail::subinfo &insub\
   ) const {\
      const kip::point<real> &eyeball = etd.eyeball;\
      const kip::point<real> &target  = etd.target;\
      const kip::point<real> &diff    = etd.diff;\
      \
      (void)eyeball;  (void)target;  (void)diff;\
      (void)qmin;     (void)ints;    (void)insub;

// kip_check
#define kip_check(type)\
   template<class real, class tag>\
   diagnostic type<real,tag>::check() const\
   {

// kip_randomize
#define kip_randomize(type)\
   template<class real, class tag>\
   type<real,tag> &randomize(type<real,tag> &obj)\
   {\
      (void)obj;

// kip_end
#define kip_end }


// kip_read_value
#define kip_read_value(type)\
   template<class ISTREAM, class real, class tag>\
   bool read_value(\
      ISTREAM &s, type<real,tag> &obj,\
      const std::string &description = #type\
   )


// kip_ostream
#define kip_ostream(type)\
   template<class real, class tag>\
   kip::ostream &operator<<(\
      kip::ostream &k, const type<real,tag> &obj\
   )


// kip_ostream_end
#define kip_ostream_end(type)\
   if (!okay)\
      (void)error("Could not write " #type);\
   return k



// -----------------------------------------------------------------------------
// onetwor_check
// bound_abr
// -----------------------------------------------------------------------------

namespace detail {

// onetwor_check
template<class real, class OBJ>
inline diagnostic onetwor_check(const char *const name, const OBJ &obj)
{
   diagnostic rv = diagnostic::good;

   // r
   if (obj.r <= real(0)) {
      std::ostringstream oss;
      oss << name << " has non-positive radius r=" << obj.r;
      rv = error(oss);
   }

   // require a != b
   if (obj.a == obj.b) {
      std::ostringstream oss;
      oss << name << " has coincident points a=b=" << obj.a;
      rv = error(oss);
   }

   return rv;
}



// bound_abr
template<class real>
inline void bound_abr(
   const point<real> &a, const point<real> &b, const real r,
   point<real> &min, point<real> &max  // output
) {
   const rotate<2,real,op::full,op::unscaled> m(a,b);

   const real dx = r*std::sqrt(op::square(m.mat.b.x) + op::square(m.mat.c.x));
   const real dy = r*std::sqrt(op::square(m.mat.b.y) + op::square(m.mat.c.y));
   const real dz = r*m.mat.c.z;

   min(a.x-dx, a.y-dy, a.z-dz);
   max(a.x+dx, a.y+dy, a.z+dz);
}

} // namespace detail



// -----------------------------------------------------------------------------
// bbox_minimum
// -----------------------------------------------------------------------------

namespace detail {

template<class real>
real bbox_minimum(
   const point<real> &e,
   const real xsize, const real ysize, const real zsize
) {
   if (e.y < 0)
      if (e.z < 0)
         if (e.x < 0)
            return mod(e - point<real>(0,0,0));
         else if (e.x < xsize)
            return pline(e,point<real>(0,0,0),point<real>(xsize,0,0));
         else
            return mod(e - point<real>(xsize,0,0));
      else if (e.z < zsize)
         if (e.x < 0)
            return pline(e,point<real>(0,0,0),point<real>(0,0,zsize));
         else if (e.x < xsize)
            return-e.y;
         else
            return pline(e,point<real>(xsize,0,0),point<real>(xsize,0,zsize));
      else
         if (e.x < 0)
            return mod(e - point<real>(0,0,zsize));
         else if (e.x < xsize)
            return pline(e,point<real>(0,0,zsize),point<real>(xsize,0,zsize));
         else
            return mod(e - point<real>(xsize,0,zsize));
   else if (e.y < ysize)
      if (e.z < 0)
         if (e.x < 0)
            return pline(e,point<real>(0,0,0),point<real>(0,ysize,0));
         else if (e.x < xsize)
            return-e.z;
         else
            return pline(e,point<real>(xsize,0,0),point<real>(xsize,ysize,0));
      else if (e.z < zsize)
         if (e.x < 0)
            return-e.x;
         else if (e.x < xsize)
            return op::min(
               e.x, xsize - e.x,
               e.y, ysize - e.y,
               e.z, zsize - e.z
            );
         else
            return e.x-xsize;
      else
         if (e.x < 0)
            return pline(e,point<real>(0,0,zsize),point<real>(0,ysize,zsize));
         else if (e.x < xsize)
            return e.z-zsize;
         else
            return pline(e,
                         point<real>(xsize,0,zsize),
                         point<real>(xsize,ysize,zsize));
   else
      if (e.z < 0)
         if (e.x < 0)
            return mod(e - point<real>(0,ysize,0));
         else if (e.x < xsize)
            return pline(e,point<real>(0,ysize,0),point<real>(xsize,ysize,0));
         else
            return mod(e - point<real>(xsize,ysize,0));
      else if (e.z < zsize)
         if (e.x < 0)
            return pline(e,point<real>(0,ysize,0),point<real>(0,ysize,zsize));
         else if (e.x < xsize)
            return e.y-ysize;
         else
            return pline(e,
                         point<real>(xsize,ysize,0),
                         point<real>(xsize,ysize,zsize));
      else
         if (e.x < 0)
            return mod(e - point<real>(0,ysize,zsize));
         else if (e.x < xsize)
            return pline(e,
                         point<real>(0,ysize,zsize),
                         point<real>(xsize,ysize,zsize));
         else
            return mod(e - point<real>(xsize,ysize,zsize));
}

} // namespace detail



// -----------------------------------------------------------------------------
// random_abr: randomize shape with {a,b,r}
// -----------------------------------------------------------------------------

namespace detail {

template<class real, class tag, class SHAPE>
SHAPE &random_abr(SHAPE &obj)
{
   // a
   random_full(obj.a);

   // b
   obj.b = obj.a + point<real>(
      real(0.6)*random_half<real>(),
      real(0.6)*random_half<real>(),
      real(0.6)*random_half<real>()
   );

   // r
   obj.r = real(0.15)*random_unit<real>();

   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
}

} // namespace detail



// -----------------------------------------------------------------------------
// check_operand
// op_first (declaration)
// op_all   (declaration)
// -----------------------------------------------------------------------------

namespace detail {

// check_operand
template<class real, class tag>
inline diagnostic check_operand(
   const char *const logical,
   const shape<real,tag> *const p,
   const char *const description
) {
   if (p == nullptr) {
      std::ostringstream oss;
      oss << "Logical-" << logical << " has undefined (nullptr) operand \""
          << description << '\"';
      return error(oss);
   }

   const diagnostic diag = p->check();
   if (diag != diagnostic::good) {
      std::ostringstream oss;
      oss << "Detected while checking \"" << description
          << "\" operand of logical-" << logical;
      return addendum(oss,diag);
   }

   return diagnostic::good;
}

// op_first
template<class real, class tag>
inline bool op_first(
   const shape<real,tag> *const,
   const eyetardiff<real> &,
   const real,
   inq<real,tag> &,
   const subinfo &
);

// op_all
template<class real, class tag>
inline bool op_all(
   const shape<real,tag> *const,
   const eyetardiff<real> &,
   const real,
   afew<inq<real,tag>> &,
   const subinfo &
);

} // namespace detail



// -----------------------------------------------------------------------------
// minimum_and_ptr
// minimum_and_shape
// vec_reset
// -----------------------------------------------------------------------------

// minimum_t
using minimum_t = float;

// minimum_and_ptr
template<class real, class SHAPE>
class minimum_and_ptr {
public:
   minimum_t minimum;
   SHAPE *shape;

   explicit minimum_and_ptr() { }
   explicit minimum_and_ptr(const real _minimum, SHAPE &_ptr) :
      minimum(minimum_t(_minimum)), shape(&_ptr) { }
};

// minimum_and_shape
template<class real, class base>
class minimum_and_shape : public minimum_and_ptr<real,shape<real,base>> {
public:
   explicit minimum_and_shape() { }
   explicit minimum_and_shape(
      const real _minimum, shape<real,base> &_shape
   ) : minimum_and_ptr<real, shape<real,base>>(_minimum,_shape) { }
};



// vec_reset
template<class real, class base>
class vec_reset {
public:
   std::vector<minimum_and_shape<real,base>> vec;
   bool reset;
};



// -----------------------------------------------------------------------------
// less, same, more
// -----------------------------------------------------------------------------

namespace detail {

// less
template<class real, class tag>
class less {
public:
   // for minimum_and_shape
   bool operator()(
      const minimum_and_shape<real,tag> &a,
      const minimum_and_shape<real,tag> &b
   ) const {
      return a.minimum < b.minimum;
   }

   // for real
   bool operator()(const real a, const real b) const { return a < b; }

   // for inq
   bool operator()(const inq<real,tag> &a, const inq<real,tag> &b) const
      { return a.q < b.q; }
};



// less_mmi
template<class real> class mmi;

template<class real>
class less_mmi {
public:
   // for mmi
   bool operator()(
      const mmi<real> &a,
      const mmi<real> &b
   ) const
      { return a.qsq < b.qsq; }
};



// same
template<class real>
class same {
public:
   // for point
   bool operator()(const point<real> &a, const point<real> &b) const
      { return a == b; }
};



// more
template<class real, class tag>
class more {
public:
   // for minimum_and_shape
   bool operator()(
      const minimum_and_shape<real,tag> &a,
      const minimum_and_shape<real,tag> &b
   ) const {
      return a.minimum > b.minimum;
   }

   // for real
   bool operator()(const real a, const real b) const { return a > b; }
};

} // namespace detail
