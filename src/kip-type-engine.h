
#pragma once

// This file provides the "engine" class.

namespace kip {



// -----------------------------------------------------------------------------
// Defaults
// -----------------------------------------------------------------------------

namespace internal {

// method
inline method_t default_method() { return uniform; }

   // For method==uniform: hzone, hsub, vzone, vsub
   // Comment, 2017-04-08:
   // hzone and vzone are the number of bins, not their size in pixels. They
   // affect the object binning/hashing. hsub and vsub are the number of sub-
   // zones per zone. They determine how "tight" the computed bounds around
   // individual objects will be. Example: say we have an image 1000 pixels
   // wide. You might use 40 bins, in which case each bin is 25 pixels wide.
   // hsub could then be anywhere from 1..25. If 0, the "fix" stuff changes
   // it to the largest value, 25. Otherwise, clip to the allowable range.
   inline unsigned default_hzone() { return 40; }
   inline unsigned default_hsub () { return  0; }
   inline unsigned default_vzone() { return 40; }
   inline unsigned default_vsub () { return  0; }

   // For method==recursive: hdivision, vdivision, min_area
   inline unsigned default_hdivision() { return 2; }
   inline unsigned default_vdivision() { return 2; }
   inline unsigned default_min_area () { return 800; }

   // For method==block: xzone, yzone, zzone
   inline unsigned default_xzone() { return 26; }
   inline unsigned default_yzone() { return 26; }
   inline unsigned default_zzone() { return 26; }

// For method==uniform and method==recursive: sort_frac, sort_min
template<class real>
inline real default_sort_frac() { return real(0.02); }
inline unsigned default_sort_min() { return 64; }

// For all methods: "fudge factor," and "low memory" flag
template<class real>
inline real default_fudge() { return real(0.99999); }

// Lean[er] memory usage?
inline bool default_lean() { return true; }

}



// -----------------------------------------------------------------------------
// engine
// -----------------------------------------------------------------------------

template<class real>  // template argument defaulted elsewhere
class engine {
public:

   // ------------------------
   // Settings
   // ------------------------

   // method
   method_t method;

      // For method==uniform
      unsigned hzone, hsub;
      unsigned vzone, vsub;

      // For method==recursive
      unsigned hdivision;
      unsigned vdivision, min_area;

      // For method==block
      unsigned xzone;
      unsigned yzone;
      unsigned zzone;

   // For method==uniform and method==recursive
   real     sort_frac;
   unsigned sort_min;

   // For all methods
   real fudge;
   bool lean;



   // ------------------------
   // Constructors
   // ------------------------

   // engine()
   inline explicit engine() :
      method   (internal::default_method()),
      hzone    (internal::default_hzone()), hsub(internal::default_hsub()),
      vzone    (internal::default_vzone()), vsub(internal::default_vsub()),
      hdivision(internal::default_hdivision()),
      vdivision(internal::default_vdivision()),
      min_area (internal::default_min_area()),
      xzone    (internal::default_xzone()),
      yzone    (internal::default_yzone()),
      zzone    (internal::default_zzone()),
      sort_frac(internal::default_sort_frac<real>()),
      sort_min (internal::default_sort_min()),
      fudge    (internal::default_fudge<real>()),
      lean     (internal::default_lean())
   { }



   // ------------------------
   // Functions
   // ------------------------

   // fix
   // Receives hpixel and vpixel from an "image" object.
   // Intentionally returns a value (not a reference).
   inline engine fix(const size_t, const size_t) const;
};



// -----------------------------------------------------------------------------
// fix: helpers
// -----------------------------------------------------------------------------

namespace internal {

// fix_zone_hv
inline void fix_zone_hv(unsigned &zone, const size_t npixel)
{
   if (zone < 1)
      zone = 1;
   if (zone > npixel)
      zone = unsigned(npixel);
}


// fix_sub_hv (also fixes products with hzone, vzone)
inline void fix_sub_hv(
   unsigned &sub, const unsigned zone,
   const size_t npixel
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
   engine<real> &obj, const size_t hpixel, const size_t vpixel
) {
   // method
   if (obj.method != uniform && obj.method != recursive && obj.method != block)
      obj.method = internal::default_method();

   // uniform fix
   if (obj.method == uniform) {
      // hzone, vzone
      fix_zone_hv(obj.hzone, hpixel);
      fix_zone_hv(obj.vzone, vpixel);

      // hsub, vsub
      fix_sub_hv(obj.hsub, obj.hzone, hpixel);
      fix_sub_hv(obj.vsub, obj.vzone, vpixel);
   }

   // recursive fix
   else if (obj.method == recursive) {
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
      obj.fudge = internal::default_fudge<real>();
}

}



// -----------------------------------------------------------------------------
// fix
// Intentionally returns value (not reference)
// -----------------------------------------------------------------------------

template<class real>
inline engine<real> engine<real>::fix(
   const size_t hpixel, const size_t vpixel
) const {
   engine<real> rv = *this;
   internal::fix_engine(rv, hpixel,vpixel);
   return rv;
}

}
