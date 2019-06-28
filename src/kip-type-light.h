
#pragma once

// This file provides the "light" class.

namespace kip {



// -----------------------------------------------------------------------------
// Defaults
// -----------------------------------------------------------------------------

namespace internal {

// light[0]
template<class real>
inline point<real> default_light0(void)
{
   return point<real>(real(0), real(-10), real(5));
}

}



// -----------------------------------------------------------------------------
// light
// -----------------------------------------------------------------------------

template<class real = default_real_t>
class light {

   // light source
   point<real> primary;

public:

   // light()
   inline explicit light(void) :
      primary(internal::default_light0<real>())
   { }

   // operator[]
   inline       point<real> &operator[](const size_t)       { return primary; }
   inline const point<real> &operator[](const size_t) const { return primary; }

   // fix
   inline const light &fix(void) const;
};



// -----------------------------------------------------------------------------
// fix
// -----------------------------------------------------------------------------

template<class real>
inline const light<real> &light<real>::fix(void) const
{
   // currently, nothing needs fixing
   return *this;
}

}
