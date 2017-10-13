/* -*- C++ -*- */

#ifndef KIP_TYPE_RAY
#define KIP_TYPE_RAY

// This file provides the "indsn" (intersection, normal, distance, shape,
// normalized) class template.



// =============================================================================
// indsn
// =============================================================================

namespace kip {

template<class real, class base>
class indsn {
public:
   // data
   point<real> intersection;            // ray/shape intersection
   point<real> normal;                  // shape's surface normal
   real distance;                       // eyeball/intersection distance
   const kip::shape<real,base> *shape;  // shape at this pixel
   bool normalized;                     // is |normal| == 1?

   // conversion to bool (meaning: ray hits shape)
   inline operator bool(void) const
   {
      // zzz Maybe just make to shape*; then same usage, plus more.
      return shape != NULL;
   }

   // initialize
   inline void initialize(void)
   {
      shape = NULL;
   }

   // indsn: set
   inline void set(const inq<real,base> &q)
   {
      // true ray/shape intersection
      intersection = q.shape->back(q.fac > 0 ? q.fac*q : point<real>(q));

      // true shape normal
      // zzz somehow tell back() to *rotate* only (not also translate)
      normal = q.shape->back(q.n) - q.shape->back(point<real>(0,0,0));

      // true eyeball/intersection distance
      distance = q.q;  // zzz q.fac scaling?

      // intersected shape; always a primitive, never an operator!
      shape = q.shape;

      // is |normal| == 1 ?
      normalized = q.normalized == yesnorm;
   }
};

}

#endif
