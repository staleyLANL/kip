
#pragma once

// This file provides the "image" class template.

// Default type: color
using default_color = rgba;



// -----------------------------------------------------------------------------
// border_t
// -----------------------------------------------------------------------------

namespace detail {

class border_t {
public:

   // defaults
   static constexpr bool default_object = false;
   static constexpr unsigned default_small = 4;
   static constexpr unsigned default_large = 2*default_small;

   // data
   bool bin = false;
   bool object;
   unsigned small;
   unsigned large;

   // border_t()
   inline explicit border_t() :
      object(default_object),
      small (default_small),
      large (default_large)
   { }

   // operator()
   inline border_t operator()(
      const bool     _bin,
      const bool     _object = default_object,
      const unsigned _small  = default_small,
      const unsigned _large  = default_large
   ) {
      bin    = _bin;
      object = _object;
      small  = _small;/// ? _small : default_small;
      large  = _large;/// ? _large : 2*_small;
      return *this;
   }
};

} // namespace detail



// -----------------------------------------------------------------------------
// image
// -----------------------------------------------------------------------------

template<
   class real  = default_real,
   class color = default_color
>
class image {

   // copy constructor/assignment; deliberately private
   inline image(const image &) :
      hpixel(bitmap.isize()),
      vpixel(bitmap.jsize())
   { }
   inline image &operator=(const image &) { return *this; }

public:
   static constexpr unsigned default_anti = 1;

   // --------------------------------
   // Data, constructor
   // --------------------------------

   // bitmap
   array<2,color> bitmap;

   // other data
   const ulong &hpixel;   // references bitmap.isize()
   const ulong &vpixel;   // references bitmap.jsize()
   color background = color::background();       // background color
   real  aspect = real(1);           // pixel aspect ratio
   mutable unsigned anti;  // antialiasing indicator
   mutable detail::border_t border;  // bin/object border information

   // prior zzz Eventually make private, so users can't disturb
   class _prior {
   public:
      // targets
      array<2,point<real>> targets;

      // aspect, hpixel, vpixel
      real aspect;
      const ulong &hpixel;  // references targets.isize()
      const ulong &vpixel;  // references targets.jsize()

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
      bitmap(800,800),
      hpixel(bitmap.isize()),
      vpixel(bitmap.jsize()),
      anti  (default_anti)
   { }

   // --------------------------------
   // Miscellaneous
   // --------------------------------

   // fix
   inline image &fix();

   // resize
   inline image &resize(const ulong h, const ulong v = 0)
      { bitmap.resize(h, v ? v : h);  return *this; }

   // upsize
   inline image &upsize(const ulong h, const ulong v = 0)
      { bitmap.upsize(h, v ? v : h);  return *this; }

   // indexing
   inline const color &operator()(const ulong i, const ulong j) const
      { return bitmap(i,j); }
   inline       color &operator()(const ulong i, const ulong j)
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
      oss << (anti = default_anti);
      (void)warning(oss);
   }

   // border
   if (border.small < 1) border.small = 1;
   if (border.large < 1) border.large = 1;

   // done
   return *this;
}
