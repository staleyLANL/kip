
#pragma once

// This file provides the bbox class and related functionality.

// -----------------------------------------------------------------------------
// bbox - helpers
// -----------------------------------------------------------------------------

namespace kip {
namespace internal {

// and_tight
template<class real>
class and_tight {
   real  value;
   bool _tight;

public:

   // and_tight(value,tight)
   inline explicit and_tight(const real &_value, const bool tight_) :
      value(_value),
     _tight(tight_)
   { }

   // assignment = real
   inline and_tight &operator=(const real &_value)
      { return value = _value, *this; }

   // conversion to real
   inline operator       real&(void)       { return value; }
   inline operator const real&(void) const { return value; }

   // tight()
   inline       bool &tight(void)       { return _tight; }
   inline const bool &tight(void) const { return _tight; }

   // finite()
   inline bool finite(void) const
   {
      return
         value != -std::numeric_limits<real>::max() &&
         value !=  std::numeric_limits<real>::max();
   }
};

}
}



// -----------------------------------------------------------------------------
// bbox_component
// -----------------------------------------------------------------------------

namespace kip {

template<class real>
class bbox_component {
public:
   // min, max
   internal::and_tight<real> min, max;

   // bbox_component(min_tight,min, max,max_tight)
   inline explicit bbox_component(
      const bool _min_tight, const real &_min,
      const real &_max, const bool _max_tight
   ) :
      min(_min,_min_tight),
      max(_max,_max_tight)
   { }

   // tight, finite, valid
   inline bool tight (void) const { return min.tight () && max.tight (); }
   inline bool finite(void) const { return min.finite() && max.finite(); }
   inline bool valid (void) const { return min <= max; }
};



// operator<<
template<class real>
inline std::ostream &operator<<(
   std::ostream &s, const kip::bbox_component<real> &obj
) {
   return s
      << (obj.min.tight() ? '[' : '(')
      <<  obj.min << ", "
      <<  obj.max
      << (obj.max.tight() ? ']' : ')');
}

}



// -----------------------------------------------------------------------------
// bbox
// -----------------------------------------------------------------------------

/*
Consider a kip::bbox<real> object called obj, and let q = x, y, or z.
Then, you can do the following with obj:

   obj.q                // q component (of type bbox_component<real>)

   obj.q.min            // minimum bound, in the q direction
   obj.q.min.tight ()   //    is this known to be a tight bound?
   obj.q.min.finite()   //    is this != +-"infinity"?

   obj.q.max            // maximum bound, in the q direction
   obj.q.max.tight ()   //    is this known to be a tight bound?
   obj.q.max.finite()   //    is this != +-"infinity"?

   obj.q.tight ()       // q's {min,max} both tight
   obj.q.finite()       // q's {min,max} both finite
   obj.q.valid ()       // q.min <= q.max

   obj.tight ()         // obj's {x,y,z} all tight
   obj.finite()         // obj's {x,y,z} all finite
   obj.valid ()         // obj's {x,y,z} all valid
*/



namespace kip {

template<class real = default_real_t>
class bbox {
public:

   // data
   bbox_component<real> x, y, z;

   // bbox(
   //    xmin_tight,xmin, xmax,xmax_tight,
   //    ymin_tight,ymin, ymax,ymax_tight,
   //    zmin_tight,zmin, zmax,zmax_tight
   // )
   inline explicit bbox(
      const bool _xmin_tight, const real &_xmin,
      const real &_xmax, const bool _xmax_tight,
      const bool _ymin_tight, const real &_ymin,
      const real &_ymax, const bool _ymax_tight,
      const bool _zmin_tight, const real &_zmin,
      const real &_zmax, const bool _zmax_tight
   ) :
      x(_xmin_tight,_xmin, _xmax,_xmax_tight),
      y(_ymin_tight,_ymin, _ymax,_ymax_tight),
      z(_zmin_tight,_zmin, _zmax,_zmax_tight)
   { }

   // bbox(bbox_component<real>, bbox_component<real>, bbox_component<real>)
   inline explicit bbox(
      const bbox_component<real> &_x,
      const bbox_component<real> &_y,
      const bbox_component<real> &_z
   ) : x(_x), y(_y), z(_z)
   { }



   // tight, finite, valid
   inline bool tight (void) const
      { return x.tight () && y.tight () && z.tight (); }
   inline bool finite(void) const
      { return x.finite() && y.finite() && z.finite(); }
   inline bool valid (void) const
      { return x.valid () && y.valid () && z.valid (); }

   // min, max
   inline point<real> min(void) const
      { return point<real>(x.min, y.min, z.min); }
   inline point<real> max(void) const
      { return point<real>(x.max, y.max, z.max); }
};



// operator<<
template<class real>
inline std::ostream &operator<<(
   std::ostream &s, const kip::bbox<real> &obj
) {
   return s
      << "   x: " << obj.x << '\n'
      << "   y: " << obj.y << '\n'
      << "   z: " << obj.z
   ;
}

}



// -----------------------------------------------------------------------------
// bound_combine
// -----------------------------------------------------------------------------

namespace kip {
namespace internal {

// op_leq
class op_leq { public:
   inline explicit op_leq(void) { }

   template<class T>
   inline bool operator()(const T &a, const T &b) const { return a <= b; }
};

// op_less
class op_less { public:
   inline explicit op_less(void) { }

   template<class T>
   inline bool operator()(const T &a, const T &b) const { return a < b; }
};



// bound_combine_component
template<class real, class COMP>
bbox_component<real> bound_combine_component(
   const bbox_component<real> &a,
   const bbox_component<real> &b, const COMP &comp
) {
   return
      a.valid() && b.valid()
    ? bbox_component<real>(
         // min tight?
         ((comp(a.min, b.min) || !b.min.finite()) && a.min.tight()) ||
         ((comp(b.min, a.min) || !a.min.finite()) && b.min.tight()),

         // min
         a.min <= b.min
            ? a.min.finite() ? a.min : b.min
            : b.min.finite() ? b.min : a.min,

         // max
         b.max <= a.max
            ? a.max.finite() ? a.max : b.max
            : b.max.finite() ? b.max : a.max,

         // max tight?
         ((comp(a.max, b.max) || !a.max.finite()) && b.max.tight()) ||
         ((comp(b.max, a.max) || !b.max.finite()) && a.max.tight())
      )

    : a.valid() ? a
    : b.valid() ? b

    : bbox_component<real>(false,1,0,false);
}



// bound_combine
template<class real, class COMP>
inline bbox<real> bound_combine(
   const bbox<real> &a, const bbox<real> &b, const COMP &comp
) {
   return kip::bbox<real>(
      bound_combine_component(a.x, b.x, comp),
      bound_combine_component(a.y, b.y, comp),
      bound_combine_component(a.z, b.z, comp)
   );
}

}
}
