
#pragma once

// This file defines the shape class template, related functions, and other
// functionality for use by shape and by classes derived from shape.

namespace kip {

// vars
namespace internal {
   template<class real = default_real_t, class tag = default_tag_t>
   class vars;
}

// engine
template<class real = default_real_t>
class engine;



// -----------------------------------------------------------------------------
// dummy
// nary_element
// shape_id_t
// -----------------------------------------------------------------------------

// dummy
class dummy { };



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
// Note: I found that by using unsigned char rather than an enum,
// these allow for a smaller sizeof(shape<>).
using shape_id_t = unsigned char;

namespace internal {
   // unique_number
   template<class T>
   inline T unique_number(void)
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
}



// -----------------------------------------------------------------------------
// minend
// subinfo
// -----------------------------------------------------------------------------

// minend
class minend {
public:
   size_t imin, iend, jmin, jend;
   inline explicit minend(void) { }
};



// subinfo
namespace internal {
   class subinfo {
   public:
      const minend &mend;
      size_t i, j;
      const unsigned nzone;

      // zzz remove?
      inline explicit subinfo(void) : mend(*(minend*)NULL), nzone(0) { }

      // subinfo(i,j)
      inline explicit subinfo(
         const size_t &_i, const size_t &_j,
         const unsigned &_nzone,
         const minend/*<I>*/ &_mend//ccc
      ) :
         mend(_mend),//ccc
         i(_i), j(_j), nzone(_nzone)
      { }
   };
}



#ifdef KIP_TOGETHER

// eyetardiff
template<class real>
class eyetardiff {
public:
   const point<real> eyeball;
   const point<real> target;
   const point<real> diff;

   inline explicit eyetardiff(
      const point<real> &e,
      const point<real> &t,
      const point<real> &d
   ) : eyeball(e), target(t), diff(d)
   { }
};

#endif



// -----------------------------------------------------------------------------
// shape
// -----------------------------------------------------------------------------

// forward declarations
template<class real, class tag> class tri;

namespace internal {
   template<class SHAPE>
   class min_and_part;
}

template<class T> class binner;



// shape
template<class real, class tag>
class shape : public tag {
public:

   // --------------------------------
   // Miscellaneous
   // --------------------------------

   mutable bool is_operand : 1;
   /*   */ bool eyelie     : 1;
   /*   */ bool on         : 1;
   /*   */ bool solid      : 1;
   mutable bool isbound    : 1;
   mutable bool baseset    : 1;
   mutable bool interior   : 1;
   mutable bool degenerate : 1;

   // id
   virtual shape_id_t id(void) const = 0;

   // [imin,iend) x [jmin,jend)
   mutable minend/*<>*/ mend;

   // get_interior(), because some g++s don't realize interior is public
   inline bool get_interior(void) const { return interior; }

   // base()
   inline const tag &base(void) const { return *this; }
   inline       tag &base(void)       { return *this; }



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
         inline point<real> &f(void) const
            { return *(point<real> *)(void *)&_f[0]; }
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
         mutable size_t nop;
         inline vec_t &vec(void) const { return *(vec_t *)(void *)&_per[0]; }
         inline element_t &push(void) const
            { return vec().push_back(element_t()), vec().back(); }
         mutable size_t total_in;
      } ands;
   };


   // for nary operators (except ands)
   class nary_type {
      private: using element_t = nary_element<real,tag>;
      public : using vec_t = std::vector<element_t>;
      private: mutable char _per[sizeof(vec_t)];
   public:
      mutable size_t nop;
      inline vec_t &vec(void) const
         { return *(vec_t *)(void *)&_per[0]; }
      inline element_t &push(void) const
         { return vec().push_back(element_t()), vec().back(); }
      mutable size_t total_in;
   };







   /*
   // union'd data, as we only need one-at-a-time
   union {
      // for certain specific shapes
      misc_type misc;

      // for most shapes
      class {
         mutable char _eye[sizeof(point<real >)];
         mutable char _lie[sizeof(point<float>)];
      public:
         inline point<real > &eye(void) const
            { return *(point<real > *)(void *)&_eye[0]; }
         inline point<float> &lie(void) const
            { return *(point<float> *)(void *)&_lie[0]; }
      } basic;

      // for unary operators
      class {
      public:
         mutable real amin;
         shape *a;
         mutable bool ina : 1;
      } unary;

      // for binary operators
      class {
         using shape_ptr = shape *;
      public:
         mutable real amin, bmin;
         mutable shape_ptr a, b;
         mutable bool ina : 1;
         mutable bool inb : 1;
      } binary;

      // for nary operators (except ands)
      nary_type nary;

      // for tri
      class {
      public:
         mutable char ghi[sizeof(point<real>)];
         mutable size_t u, v, w;
      } vertex;

      // for triangle
      class {
      public:
         mutable char ghi[sizeof(point<real>)];
      } tridata;

      // for surf
      class {
      public:
         mutable char mint
            [sizeof(binner<internal::min_and_part<tri<real,tag>>>)];
      } surfdata;
   };
   */

   // for most shapes
   class union_basic {
      mutable char _eye[sizeof(point<real >)];
      mutable char _lie[sizeof(point<float>)];
   public:
      inline point<real > &eye(void) const
         { return *(point<real > *)(void *)&_eye[0]; }
      inline point<float> &lie(void) const
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

   // for tri
   class union_vertex {
   public:
      mutable char ghi[sizeof(point<real>)];
      mutable size_t u, v, w;
   };

   // for triangle
   class union_tridata {
   public:
      mutable char ghi[sizeof(point<real>)];
   };

   // for surf
   class union_surfdata {
   public:
      mutable char mint
         [sizeof(binner<internal::min_and_part<tri<real,tag>>>)];
   };

   // union'd data, as we only need one-at-a-time
   union {
      misc_type      misc;      // for certain specific shapes
      union_basic    basic;     // for most shapes
      union_unary    unary;     // for unary operators
      union_binary   binary;    // for binary operators
      nary_type      nary;      // for nary operators (except ands)
      union_vertex   vertex;    // for tri
      union_tridata  tridata;   // for triangle
      union_surfdata surfdata;  // for surf
   };







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
   inline explicit shape(const derived<_real,_tag> *const) :
      tag    ( tag() ),
      eyelie  ( true   ),
      on      ( true   ),
      solid   ( true   ),
      isbound ( false  ),
      baseset ( false  ),
      user    ( NULL   )
   { }

   // shape(derived *, base)
   template<template<class,class> class derived, class _real, class _tag>
   inline explicit shape(
      const derived<_real,_tag> *const,
      const tag &thebase
   ) :
      tag    ( thebase ),
      eyelie  ( true    ),
      on      ( true    ),
      solid   ( true    ),
      isbound ( false   ),
      baseset ( true    ),
      user    ( NULL    )
   { }

   // shape(shape)
   // zzz Think about some fields, e.g. baseset; correct semantics may
   // depend on context. May need to delegate to two or more copy() functions.
   inline shape(const shape &from) :
      tag    ( from.base()  ),
      eyelie  ( from.eyelie  ),
      on      ( from.on      ),
      solid   ( from.solid   ),
      isbound ( from.isbound ),
      baseset ( from.baseset ),
      user    ( from.user    )
   { }



   // --------------------------------
   // Assignment, Destructor
   // --------------------------------

   // shape = shape
   // zzz Similar remark as that for copy c'tor.
   inline shape &operator=(const shape &from)
   {
      this->tag::operator=(from.base());

      eyelie  = from.eyelie;
      on      = from.on;
      solid   = from.solid;
      isbound = from.isbound;
      baseset = from.baseset;
      user    = from.user;

      return *this;
   }

   // destructor
   virtual inline ~shape(void) { }



   // --------------------------------
   // Virtuals (except id())
   // --------------------------------

   // ----------------
   // PURE...
   // ----------------

   // duplicate, size_of
   virtual shape<real,tag> *duplicate(void) const = 0;
   virtual size_t size_of(void) const = 0;


   // read
   virtual kip::istream &read(kip::istream &) = 0;
   virtual std::istream &read(std::istream &) = 0;

   // write
   virtual kip::ostream &write(kip::ostream &) const = 0;
   virtual std::ostream &write(std::ostream &) const = 0;


   // process
   virtual real process(
      const point<real> &, const point<real> &,
      const engine<real> &, const internal::vars<real,tag> &
   ) const = 0;

   // aabb
   virtual bbox<real> aabb(void) const = 0;

   // dry
   virtual bool dry(const rotate<-3,real> &) const = 0;



#ifdef KIP_TOGETHER

   // infirst
   virtual bool infirst(
      const eyetardiff<real> &,
      const real, inq<real,tag> &, const internal::subinfo &
   ) const = 0;

   // inall
   virtual bool inall(
      const eyetardiff<real> &,
      const real, afew<inq<real,tag>> &, const internal::subinfo &
   ) const = 0;

#else

   // infirst
   virtual bool infirst(
      const point<real> &, const point<real> &, const point<real> &,
      const real,       inq<real,tag>   &, const internal::subinfo &
   ) const = 0;

   // inall
   virtual bool inall(
      const point<real> &, const point<real> &, const point<real> &,
      const real, afew<inq<real,tag>> &, const internal::subinfo &
   ) const = 0;

#endif



   // check
   virtual diagnostic_t check(void) const = 0;

   // back
   virtual point<real> back(const point<real> &) const = 0;


   // ----------------
   // NON-PURE...
   // ----------------

   // propagate_base; overridden with meaningful propagation iff "operator"
   virtual inline void propagate_base(const bool = false) const { }


   // --------------------------------
   // Generic pointer; use as you wish
   // --------------------------------

   void *user;
};



// -----------------------------------------------------------------------------
// Macros - general
// -----------------------------------------------------------------------------

// kip_virtual_id
#define kip_virtual_id(kip_class)\
   inline shape_id_t id(void) const\
      { return internal::get_shape_id<kip::kip_class>::result; }

// kip_destructor
#ifdef KIP_CONSTRUCT_COUNTER
   #define kip_destructor(kip_class)\
      inline ~kip_class(void)\
         { kip_counter_dtor(kip_class); }
#else
   #define kip_destructor(kip_class)
#endif



// Regarding virtuals:
//    Defines:  [id,] [destructor,] duplicate, size_of, read/write kip/std
//    Declares: process, aabb, dry, first, all, check
//    Ignores:  back, propagate_base

#ifdef KIP_TOGETHER

#define kip_functions(kip_class)\
   \
   kip_virtual_id(kip_class)\
   kip_destructor(kip_class)\
   \
   inline kip_class *duplicate(void) const { return new kip_class(*this); }\
   inline size_t size_of(void) const { return sizeof(kip_class); }\
   \
   inline kip::istream &read (kip::istream &k)       { return k >> *this; }\
   inline std::istream &read (std::istream &s)       { return s >> *this; }\
   inline kip::ostream &write(kip::ostream &k) const { return k << *this; }\
   inline std::ostream &write(std::ostream &s) const { return s << *this; }\
   \
   inline real process(\
      const kip::point<real> &,\
      const kip::point<real> &,\
      const engine<real> &,\
      const internal::vars<real,tag> &\
   ) const;\
   \
   inline kip::bbox<real> aabb(void) const;\
   inline bool dry(const rotate<-3,real> &) const;\
   \
   inline bool infirst(\
      const kip::eyetardiff<real> &,\
      const real,\
      inq<real,tag> &,\
      const internal::subinfo &\
   ) const;\
   \
   inline bool inall(\
      const kip::eyetardiff<real> &,\
      const real,\
      afew<inq<real,tag>> &,\
      const internal::subinfo &\
   ) const;\
   \
   inline diagnostic_t check(void) const
   // no ';' at end - semicolons at *invocation* points help emacs indent

#else

#define kip_functions(kip_class)\
   \
   kip_virtual_id(kip_class)\
   kip_destructor(kip_class)\
   \
   inline kip_class *duplicate(void) const { return new kip_class(*this); }\
   inline size_t size_of(void) const { return sizeof(kip_class); }\
   \
   inline kip::istream &read (kip::istream &k)       { return k >> *this; }\
   inline std::istream &read (std::istream &s)       { return s >> *this; }\
   inline kip::ostream &write(kip::ostream &k) const { return k << *this; }\
   inline std::ostream &write(std::ostream &s) const { return s << *this; }\
   \
   inline real process(\
      const kip::point<real> &,\
      const kip::point<real> &,\
      const engine<real> &,\
      const internal::vars<real,tag> &\
   ) const;\
   \
   inline kip::bbox<real> aabb(void) const;\
   inline bool dry(const rotate<-3,real> &) const;\
   \
   inline bool infirst(\
      const kip::point<real> &,\
      const kip::point<real> &,\
      const kip::point<real> &,\
      const real,\
      inq<real,tag> &,\
      const internal::subinfo &\
   ) const;\
   \
   inline bool inall(\
      const kip::point<real> &,\
      const kip::point<real> &,\
      const kip::point<real> &,\
      const real,\
      afew<inq<real,tag>> &,\
      const internal::subinfo &\
   ) const;\
   \
   inline diagnostic_t check(void) const
   // no ';' at end - semicolons at *invocation* points help emacs indent

#endif



// -----------------------------------------------------------------------------
// Macros - specific shape functions
// -----------------------------------------------------------------------------

// kip_process
#define kip_process(type)\
   template<class real, class tag>\
   inline real type<real,tag>::process(\
      const kip::point<real> &eyeball,\
      const kip::point<real> &light,\
      const engine<real> &engine,\
      const internal::vars<real,tag> &vars\
   ) const {\
      (void)eyeball;  (void)light;\
      (void)engine;   (void)vars;

// kip_aabb
#define kip_aabb(type)\
   template<class real, class tag>\
   inline bbox<real> type<real,tag>::aabb(void) const\
   {

// kip_inside
#define kip_inside(type)\
   template<class real, class tag>\
   inline bool type<real,tag>::inside(\
      const kip::point<real> &eyeball\
   ) const\
   {\
      (void)eyeball;

// kip_dry
#define kip_dry(type)\
   template<class real, class tag>\
   inline bool type<real,tag>::dry(const rotate<-3,real> &seg) const\
   {\
      (void)seg;



#ifdef KIP_TOGETHER

// kip_etd
#define kip_etd etd

// kip_infirst
#define kip_infirst(type)\
   template<class real, class tag>\
   inline bool type<real,tag>::infirst(\
      const kip::eyetardiff<real> &etd,\
      const real qmin,\
      inq<real,tag> &q,\
      const internal::subinfo &insub\
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
   inline bool type<real,tag>::inall(\
      const kip::eyetardiff<real> &etd,\
      const real qmin,\
      afew<inq<real,tag>> &ints,\
      const internal::subinfo &insub\
   ) const {\
      const kip::point<real> &eyeball = etd.eyeball;\
      const kip::point<real> &target  = etd.target;\
      const kip::point<real> &diff    = etd.diff;\
      \
      (void)eyeball;  (void)target;  (void)diff;\
      (void)qmin;     (void)ints;    (void)insub;

#else

// kip_etd
#define kip_etd eyeball, target, diff

// kip_infirst
#define kip_infirst(type)\
   template<class real, class tag>\
   inline bool type<real,tag>::infirst(\
      const kip::point<real> &eyeball,\
      const kip::point<real> &target,\
      const kip::point<real> &diff,\
      const real qmin,\
      inq<real,tag> &q,\
      const internal::subinfo &insub\
   ) const {\
      (void)eyeball;  (void)target;  (void)diff;\
      (void)qmin;     (void)q;       (void)insub;

// kip_inall
#define kip_inall(type)\
   template<class real, class tag>\
   inline bool type<real,tag>::inall(\
      const kip::point<real> &eyeball,\
      const kip::point<real> &target,\
      const kip::point<real> &diff,\
      const real qmin,\
      afew<inq<real,tag>> &ints,\
      const internal::subinfo &insub\
   ) const {\
      (void)eyeball;  (void)target;  (void)diff;\
      (void)qmin;     (void)ints;    (void)insub;

#endif



// kip_check
#define kip_check(type)\
   template<class real, class tag>\
   inline diagnostic_t type<real,tag>::check(void) const\
   {

// kip_random
#define kip_random(type)\
   template<class real, class tag>\
   type<real,tag> &random(type<real,tag> &obj)\
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
      kip::ostream &k, const kip::type<real,tag> &obj\
   )


// kip_ostream_end
#define kip_ostream_end(type)\
   if (!okay)\
      (void)kip::error("Could not write " #type);\
   return k



// -----------------------------------------------------------------------------
// onetwor_check
// bound_abr
// -----------------------------------------------------------------------------

namespace internal {

template<class real, class OBJ>
inline diagnostic_t onetwor_check(const char *const name, const OBJ &obj)
{
   diagnostic_t rv = diagnostic_good;

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
   const point<real> &a, const point<real> &b, const real &r,
   point<real> &min, point<real> &max  // output
) {
   const rotate<2,real> m(a,b);

   const real dx = r*op::sqrt(m.m2x*m.m2x + m.m3.x*m.m3.x);
   const real dy = r*op::sqrt(m.m2y*m.m2y + m.m3.y*m.m3.y), dz = r*m.m3.z;

   min(a.x-dx, a.y-dy, a.z-dz);
   max(a.x+dx, a.y+dy, a.z+dz);
}

}



// -----------------------------------------------------------------------------
// bbox_minimum
// -----------------------------------------------------------------------------

namespace internal {

template<class real>
real bbox_minimum(
   const point<real> &e,
   const real &xsize, const real &ysize, const real &zsize
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
            return pline(e,point<real>(xsize,0,zsize),point<real>(xsize,ysize,zsize));
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
            return pline(e,point<real>(xsize,ysize,0),point<real>(xsize,ysize,zsize));
      else
         if (e.x < 0)
            return mod(e - point<real>(0,ysize,zsize));
         else if (e.x < xsize)
            return pline(e,point<real>(0,ysize,zsize),point<real>(xsize,ysize,zsize));
         else
            return mod(e - point<real>(xsize,ysize,zsize));
}

}



// -----------------------------------------------------------------------------
// random_abr: randomize shape with {a,b,r}
// -----------------------------------------------------------------------------

namespace internal {

template<class real, class tag, class SHAPE>
SHAPE &random_abr(SHAPE &obj)
{
   // a
   obj.a(
      op::twice(random<real>() - real(0.5)),
      op::twice(random<real>() - real(0.5)),
      op::twice(random<real>() - real(0.5))
   );

   // b
   obj.b = obj.a + point<real>(
      real(0.6)*(random<real>() - real(0.5)),
      real(0.6)*(random<real>() - real(0.5)),
      real(0.6)*(random<real>() - real(0.5))
   );

   // r
   obj.r = real(0.15*random<real>());

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
}

}



// -----------------------------------------------------------------------------
// check_operand
// op_first (declaration)
// op_all   (declaration)
// -----------------------------------------------------------------------------

namespace internal {

// check_operand
template<class real, class tag>
inline diagnostic_t check_operand(
   const char *const logical,
   const shape<real,tag> *const p,
   const char *const description
) {
   if (p == NULL) {
      std::ostringstream oss;
      oss << "Logical-" << logical << " has undefined (NULL pointer) operand \""
          << description << '\"';
      return error(oss);
   }

   const diagnostic_t diag = p->check();
   if (diag != diagnostic_good) {
      std::ostringstream oss;
      oss << "Detected while checking \"" << description
          << "\" operand of logical-" << logical;
      return addendum(oss,diag);
   }

   return diagnostic_good;
}



// op_first
template<class real, class tag>
inline bool op_first(
   const shape<real,tag> *const,
#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> &,
#else
   const point<real> &, const point<real> &, const point<real> &,
#endif
   const real &, inq<real,tag> &, const subinfo &
);



// op_all
template<class real, class tag>
inline bool op_all(
   const shape<real,tag> *const,
#ifdef KIP_TOGETHER
   const kip::eyetardiff<real> &,
#else
   const point<real> &, const point<real> &, const point<real> &,
#endif
   const real &, afew<inq<real,tag>> &, const subinfo &
);

}



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

   inline explicit minimum_and_ptr(void) { }
   inline explicit minimum_and_ptr(const real &_minimum, SHAPE &_ptr) :
      minimum(minimum_t(_minimum)), shape(&_ptr) { }
};

// minimum_and_shape
template<class real, class base>
class minimum_and_shape : public minimum_and_ptr<real,kip::shape<real,base>> {
public:
   inline explicit minimum_and_shape(void) { }
   inline explicit minimum_and_shape(
      const real &_minimum, kip::shape<real,base> &_shape
   ) : minimum_and_ptr<real, kip::shape<real,base>>(_minimum,_shape) { }
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

namespace internal {

// less
template<class real, class tag>
class less {
public:
   inline explicit less(void) { }

   // for minimum_and_shape
   inline bool operator()(
      const minimum_and_shape<real,tag> &a,
      const minimum_and_shape<real,tag> &b
   ) const {
      return a.minimum < b.minimum;
   }

   // for real
   inline bool operator()(const real &a, const real &b) const { return a < b; }

   // for inq
   inline bool operator()(const inq<real,tag> &a, const inq<real,tag> &b) const
      { return a.q < b.q; }
};



// less_mmi
template<class real> class mmi;

template<class real>
class less_mmi {
public:
   inline explicit less_mmi(void) { }

   // for mmi
   inline bool operator()(
      const mmi<real> &a,
      const mmi<real> &b
   ) const
      { return a.qsq < b.qsq; }
};



// same
template<class real>
class same {
public:
   inline explicit same(void) { }

   // for point
   inline bool operator()(const point<real> &a, const point<real> &b) const
      { return a == b; }
};



// more
template<class real, class tag>
class more {
public:
   inline explicit more(void) { }

   // for minimum_and_shape
   inline bool operator()(
      const minimum_and_shape<real,tag> &a,
      const minimum_and_shape<real,tag> &b
   ) const {
      return a.minimum > b.minimum;
   }

   // for real
   inline bool operator()(const real &a, const real &b) const { return a > b; }
};

}
}
