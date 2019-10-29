
// -----------------------------------------------------------------------------
// bbox - helpers
// -----------------------------------------------------------------------------

namespace detail {

// and_tight
template<class real>
class and_tight {
   real  value;
   bool _tight;

public:

   // and_tight(value,tight)
   explicit and_tight(const real _value, const bool tight_) :
      value(_value),
     _tight(tight_)
   { }

   // assignment = real
   and_tight &operator=(const real _value)
      { return value = _value, *this; }

   // conversion to real
   operator       real&()       { return value; }
   operator const real&() const { return value; }

   // tight()
         bool &tight()       { return _tight; }
   const bool &tight() const { return _tight; }

   // finite()
   bool finite() const
   {
      return
         value != -std::numeric_limits<real>::max() &&
         value !=  std::numeric_limits<real>::max();
   }
};

} // namespace detail



// -----------------------------------------------------------------------------
// bbox_component
// -----------------------------------------------------------------------------

template<class real>
class bbox_component {
public:
   // min, max
   detail::and_tight<real> min, max;

   // bbox_component(min_tight,min, max,max_tight)
   explicit bbox_component(
      const bool _min_tight, const real _min,
      const real _max, const bool _max_tight
   ) :
      min(_min,_min_tight),
      max(_max,_max_tight)
   { }

   // tight, finite, valid
   bool tight () const { return min.tight () && max.tight (); }
   bool finite() const { return min.finite() && max.finite(); }
   bool valid () const { return min <= max; }
};



// operator<<
template<class real>
inline std::ostream &operator<<(
   std::ostream &s, const bbox_component<real> &obj
) {
   return s
      << (obj.min.tight() ? '[' : '(')
      <<  obj.min << ", "
      <<  obj.max
      << (obj.max.tight() ? ']' : ')');
}



// -----------------------------------------------------------------------------
// bbox
// -----------------------------------------------------------------------------

/*
Consider a bbox<real> object called obj, and let q = x, y, or z.
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

template<class real>
class bbox {
public:

   // data
   bbox_component<real> x, y, z;

   // bbox(
   //    xmin_tight,xmin, xmax,xmax_tight,
   //    ymin_tight,ymin, ymax,ymax_tight,
   //    zmin_tight,zmin, zmax,zmax_tight
   // )
   explicit bbox(
      const bool _xmin_tight, const real _xmin,
      const real _xmax, const bool _xmax_tight,
      const bool _ymin_tight, const real _ymin,
      const real _ymax, const bool _ymax_tight,
      const bool _zmin_tight, const real _zmin,
      const real _zmax, const bool _zmax_tight
   ) :
      x(_xmin_tight,_xmin, _xmax,_xmax_tight),
      y(_ymin_tight,_ymin, _ymax,_ymax_tight),
      z(_zmin_tight,_zmin, _zmax,_zmax_tight)
   { }

   // bbox(bbox_component<real>, bbox_component<real>, bbox_component<real>)
   explicit bbox(
      const bbox_component<real> &_x,
      const bbox_component<real> &_y,
      const bbox_component<real> &_z
   ) : x(_x), y(_y), z(_z)
   { }

   // tight, finite, valid
   bool tight () const
      { return x.tight () && y.tight () && z.tight (); }
   bool finite() const
      { return x.finite() && y.finite() && z.finite(); }
   bool valid () const
      { return x.valid () && y.valid () && z.valid (); }

   // min, max
   point<real> min() const
      { return point<real>(x.min, y.min, z.min); }
   point<real> max() const
      { return point<real>(x.max, y.max, z.max); }
};



// operator<<
template<class real>
inline std::ostream &operator<<(
   std::ostream &s, const bbox<real> &obj
) {
   return s
      << "   x: " << obj.x << '\n'
      << "   y: " << obj.y << '\n'
      << "   z: " << obj.z
   ;
}



// -----------------------------------------------------------------------------
// bound_combine
// -----------------------------------------------------------------------------

namespace detail {

// op_leq
class op_leq {
public:
   template<class real>
   bool operator()(
      const bbox<real> &a,
      const bbox<real> &b
   ) const {
      return a <= b;
   }

   template<class real>
   bool operator()(
      const and_tight<real> &a,
      const and_tight<real> &b
   ) const {
      return a <= b;
   }
};

// op_less
class op_less {
public:
   template<class real>
   bool operator()(
      const bbox<real> &a,
      const bbox<real> &b
   ) const {
      return a < b;
   }

   template<class real>
   bool operator()(
      const and_tight<real> &a,
      const and_tight<real> &b
   ) const {
      return a < b;
   }
};



// bound_combine_component
template<class real, class COMPARE>
bbox_component<real> bound_combine_component(
   const bbox_component<real> &a,
   const bbox_component<real> &b, const COMPARE &compare
) {
   return
      a.valid() && b.valid()
    ? bbox_component<real>(
         // min tight?
         ((compare(a.min, b.min) || !b.min.finite()) && a.min.tight()) ||
         ((compare(b.min, a.min) || !a.min.finite()) && b.min.tight()),

         // min
         a.min <= b.min
            ? a.min.finite() ? a.min : b.min
            : b.min.finite() ? b.min : a.min,

         // max
         b.max <= a.max
            ? a.max.finite() ? a.max : b.max
            : b.max.finite() ? b.max : a.max,

         // max tight?
         ((compare(a.max, b.max) || !a.max.finite()) && b.max.tight()) ||
         ((compare(b.max, a.max) || !b.max.finite()) && a.max.tight())
      )

    : a.valid() ? a
    : b.valid() ? b

    : bbox_component<real>(false,1,0,false);
}



// bound_combine
template<class real, class COMPARE>
inline bbox<real> bound_combine(
   const bbox<real> &a, const bbox<real> &b, const COMPARE &compare
) {
   return bbox<real>(
      bound_combine_component(a.x, b.x, compare),
      bound_combine_component(a.y, b.y, compare),
      bound_combine_component(a.z, b.z, compare)
   );
}

} // namespace detail
