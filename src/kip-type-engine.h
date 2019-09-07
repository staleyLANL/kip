
#pragma once

// This file provides the "engine" class.



// -----------------------------------------------------------------------------
// engine
// -----------------------------------------------------------------------------

template<class real>  // template argument was defaulted elsewhere
class engine {
public:
   static constexpr real default_fudge = real(0.99999);

   // ------------------------
   // Settings
   // ------------------------

   // method
   method_t method = method_t::uniform;

   // For method = uniform
   // hzone and vzone are the number of bins, not their size in pixels.
   // They affect the object binning/hashing.
   // hsub and vsub are the number of sub-zones per zone. They determine how
   // "tight" the computed bounds around individual objects will be. Example:
   // say we have an image 1000 pixels wide. You might use 40 bins, in which
   // case each bin is 25 pixels wide. hsub could then be anywhere from 1..25.
   // If 0, the "fix" stuff changes it to the largest value, 25. Otherwise,
   // clip to the allowable range.
   unsigned hzone = 40;
   unsigned vzone = 40;
   unsigned hsub  = 0;
   unsigned vsub  = 0;

   // For method = recursive
   unsigned hdivision = 2;
   unsigned vdivision = 2;
   unsigned min_area  = 800;

   // For method = block
   unsigned xzone = 26;
   unsigned yzone = 26;
   unsigned zzone = 26;

   // For method = uniform and method = recursive
   real     sort_frac = real(0.02);
   unsigned sort_min  = 64;

   // For all methods: fudge factor, leaner memory use flag
   real fudge = default_fudge;
   bool lean  = true;

   // ------------------------
   // Functions
   // ------------------------

   // fix
   // Receives hpixel and vpixel from an "image" object.
   // Intentionally returns a value (not a reference).
   inline engine fix(const ulong, const ulong) const;
};



// -----------------------------------------------------------------------------
// fix: helpers
// -----------------------------------------------------------------------------

namespace detail {

// fix_zone_hv
inline void fix_zone_hv(unsigned &zone, const ulong npixel)
{
   if (zone < 1)
      zone = 1;
   if (zone > npixel)
      zone = unsigned(npixel);
}


// fix_sub_hv (also fixes products with hzone, vzone)
inline void fix_sub_hv(
   unsigned &sub, const unsigned zone,
   const ulong npixel
) {
   if (sub < 1 || sub*zone > npixel+zone-1)
      sub = unsigned((npixel+zone-1)/zone);

   /*
   if (sub < 1)
       sub = 1;
   if (sub*zone > npixel+zone-1)
       sub = (npixel+zone-1)/zone;
   */
}


// fix_zone_xyz
inline void fix_zone_xyz(unsigned &zone)
{
   if (zone < 1)
       zone = 1;
}


// fix_sort
template<class real>
inline void fix_sort(engine<real> &obj)
{
   // sort_frac
   if (obj.sort_frac < real(0))
       obj.sort_frac = real(0);

   // sort_min
   if (obj.sort_min < 1)
       obj.sort_min = 1;
}



// fix_engine
template<class real>
inline void fix_engine(
   engine<real> &obj, const ulong hpixel, const ulong vpixel
) {
   // uniform fix
   if (obj.method == method_t::uniform) {
      // hzone, vzone
      fix_zone_hv(obj.hzone, hpixel);
      fix_zone_hv(obj.vzone, vpixel);

      // hsub, vsub
      fix_sub_hv(obj.hsub, obj.hzone, hpixel);
      fix_sub_hv(obj.vsub, obj.vzone, vpixel);
   }

   // recursive fix
   else if (obj.method == method_t::recursive) {
      // hdivision
      if (obj.hdivision < 2)
         obj.hdivision = 2;
      if (obj.hdivision > hpixel)
         obj.hdivision = unsigned(hpixel);

      // vdivision
      if (obj.vdivision < 2)
         obj.vdivision = 2;
      if (obj.vdivision > vpixel)
         obj.vdivision = unsigned(vpixel);

      // min_area
      if (obj.min_area < 1)
         obj.min_area = 1;
      if (obj.min_area > hpixel*vpixel)
         obj.min_area = unsigned(hpixel*vpixel);
   }

   // block fix
   else {
      // xzone, yzone, zzone
      fix_zone_xyz(obj.xzone);
      fix_zone_xyz(obj.yzone);
      fix_zone_xyz(obj.zzone);
   }

   // sort_frac, sort_min
   fix_sort(obj);

   // fudge factor
   // Definitely require fudge < 1, as a strict inequality. Allowing fudge == 1,
   // while feasible, would require greater care regarding strict vs. non-strict
   // inequalities in all the dry() functions. As for our requiring 0 <= fudge,
   // in fact any fudge < 1 is allowable. But further down than "1-epsilon" has
   // no use, and would slow the code.
   if (!(0 <= obj.fudge && obj.fudge < 1))  // if NOT in [0,1)...
      obj.fudge = engine<real>::default_fudge;
}

} // namespace detail



// -----------------------------------------------------------------------------
// fix
// Intentionally returns value (not reference)
// -----------------------------------------------------------------------------

template<class real>
inline engine<real> engine<real>::fix(
   const ulong hpixel, const ulong vpixel
) const {
   engine<real> rv = *this;
   detail::fix_engine(rv, hpixel,vpixel);
   return rv;
}
