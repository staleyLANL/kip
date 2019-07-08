
#pragma once

// This file provides the "image" class template.

// Default type: color
using default_color_t = kip::rgba;



// -----------------------------------------------------------------------------
// border_t
// -----------------------------------------------------------------------------

namespace internal {

class border_t {
public:

   // defaults
   static inline bool     default_bin   () { return false; }
   static inline bool     default_object() { return false; }
   static inline unsigned default_small () { return 4; }
   static inline unsigned default_large () { return 2*default_small(); }

   // data
   bool bin;
   bool object;
   unsigned small;
   unsigned large;

   // border_t()
   inline explicit border_t()
 : bin   (default_bin   ()),
   object(default_object()),
   small (default_small ()),
   large (default_large ())
   { }

   // operator()
   inline border_t operator()(
      const bool     _bin,
      const bool     _object = default_object(),
      const unsigned _small  = default_small (),
      const unsigned _large  = default_large ()
   ) {
      bin    = _bin;
      object = _object;
      small  = _small;/// ? _small : default_small();
      large  = _large;/// ? _large : 2*_small;
      return *this;
   }
};

} // namespace internal



// -----------------------------------------------------------------------------
// Defaults
// -----------------------------------------------------------------------------

namespace internal {

// hpixel, vpixel
inline size_t default_hpixel() { return 800; }
inline size_t default_vpixel() { return 800; }

// background
template<class color>
inline color default_background() { return color::background(); }

// aspect
template<class real>
inline real default_aspect() { return real(1); }

// anti
inline unsigned default_anti() { return 1; }

// border
inline border_t default_border() { return border_t(); }

} // namespace internal



// -----------------------------------------------------------------------------
// image
// -----------------------------------------------------------------------------

template<
   class real  = default_real_t,
   class color = default_color_t
>
class image {

   // copy constructor/assignment; deliberately private
   inline image(const image &) :
      hpixel(bitmap.isize()),
      vpixel(bitmap.jsize())
   { }
   inline image &operator=(const image &) { return *this; }

public:

   // --------------------------------
   // Data, constructor
   // --------------------------------

   // bitmap
   array<2,color> bitmap;

   // other data
   const size_t &hpixel;   // references bitmap.isize()
   const size_t &vpixel;   // references bitmap.jsize()
   color background;       // background color
   real  aspect;           // pixel aspect ratio
   mutable unsigned anti;  // antialiasing indicator
   mutable internal::border_t border;  // bin/object border information



   // prior   zzz Eventually make private, so users can't disturb
   class _prior {
   public:
      // targets
      array<2,point<real>> targets;

      // aspect, hpixel, vpixel
      real aspect;
      const size_t &hpixel;  // references targets.isize()
      const size_t &vpixel;  // references targets.jsize()

      // first
      bool first;

      // c'tor
      inline explicit _prior() :
         hpixel(targets.isize()),
         vpixel(targets.jsize()), first(true)
      { }
   } prior;



   // image()
   inline explicit image() :
      bitmap     (internal::default_hpixel(), internal::default_vpixel()),
      hpixel     (bitmap.isize()),
      vpixel     (bitmap.jsize()),
      background (internal::default_background<color>()),
      aspect     (internal::default_aspect<real>()),
      anti       (internal::default_anti()),
      border     (internal::default_border())
   { }



   // --------------------------------
   // Miscellaneous
   // --------------------------------

   // fix
   inline image &fix();

   // resize
   inline image &resize(const size_t h, const size_t v = 0)
      { bitmap.resize(h, v ? v : h);  return *this; }

   // upsize
   inline image &upsize(const size_t h, const size_t v = 0)
      { bitmap.upsize(h, v ? v : h);  return *this; }

   // indexing
   inline const color &operator()(const size_t i, const size_t j) const
      { return bitmap(i,j); }
   inline       color &operator()(const size_t i, const size_t j)
      { return bitmap(i,j); }

   // data access
   inline       color *operator()()       { return bitmap.data(); }
   inline const color *operator()() const { return bitmap.data(); }
};



// -----------------------------------------------------------------------------
// fix
// -----------------------------------------------------------------------------

template<class real, class color>
image<real,color> &image<real,color>::fix()
{
   // bitmap: guarantee size
   upsize(hpixel,vpixel);

   // qqq should probably do something for aspect

   // anti
   if (anti < 1) {
      std::ostringstream oss;
      oss << "Image has anti=" << anti << "; setting to default=";
      oss << (anti = internal::default_anti());
      (void)warning(oss);
   }

   // border
   if (border.small < 1) border.small = 1;
   if (border.large < 1) border.large = 1;

   // done
   return *this;
}
