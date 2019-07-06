
#pragma once

// This file provides a few additional constructs related to the (now-defined)
// shape class template.

// -----------------------------------------------------------------------------
// id-related constructs
// -----------------------------------------------------------------------------

namespace kip {

// users may prefer the shorter name, "shape_id", without the "_t" suffix
using shape_id = shape_id_t;

namespace internal {

// id_t
template<class>
class id_t {
public:
   inline explicit id_t(void) { }

   // for "shape" itself
   static const shape_id_t shape;

   // for primitives and operators
   #define kip_make_id(type) static const shape_id_t type
      kip_expand_semi(kip_make_id)
      kip_extra_semi (kip_make_id)
   #undef kip_make_id
};



// id for shape (base class)
template<class unused>
const shape_id_t id_t<unused>::shape = get_shape_id<kip::shape>::result;

// id for specific shapes
#define kip_make_id(type)\
   template<class unused>\
   const shape_id_t id_t<unused>::type = get_shape_id<kip::type>::result

kip_expand_semi(kip_make_id)
kip_extra_semi (kip_make_id)
#undef kip_make_id

}

using id = internal::id_t<char>;

}
