
#pragma once

// This file provides the "light" class.



// -----------------------------------------------------------------------------
// light
// -----------------------------------------------------------------------------

template<class real = default_real_t>
class light {

   // light source
   point<real> primary = point<real>(0,-10,5);

public:

   // operator[]
   inline       point<real> &operator[](const size_t)       { return primary; }
   inline const point<real> &operator[](const size_t) const { return primary; }

   // fix
   inline const light &fix() const
   {
      // currently, nothing needs fixing
      return *this;
   }
};
