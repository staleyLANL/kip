
// This file provides a few additional constructs related to the (now-defined)
// shape class template.

// -----------------------------------------------------------------------------
// id-related constructs
// -----------------------------------------------------------------------------

// users may prefer the shorter name, "shape_id", without the "_t" suffix
using shape_id = shape_id_t;

namespace detail {

// id_t
template<class>
class id_t {
public:
   // for "shape" itself
   static const shape_id_t shape;

   // for primitives and operators
   #define kip_make_id(type) static const shape_id_t type
      kip_expand(kip_make_id,;)
      kip_extra (kip_make_id,;)
   #undef kip_make_id
};

// id for shape (base class)
template<class unused>
const shape_id_t id_t<unused>::shape = get_shape_id<kip::shape>::result;

// id for specific shapes
#define kip_make_id(type)\
   template<class unused>\
   const shape_id_t id_t<unused>::type = get_shape_id<kip::type>::result
kip_expand(kip_make_id,;)
kip_extra (kip_make_id,;)
#undef kip_make_id

} // namespace detail

using id = detail::id_t<char>;
