
// -----------------------------------------------------------------------------
// border_t
// -----------------------------------------------------------------------------

namespace detail {

class border_t {
public:

   // default member-data values
   static constexpr bool     default_object = false;
   static constexpr unsigned default_small  = 4;
   static constexpr unsigned default_large  = 2*default_small;

   // data
   bool bin = false;
   bool object;
   unsigned small;
   unsigned large;

   // border_t()
   explicit border_t() :
      object(default_object),
      small (default_small),
      large (default_large)
   { }

   // operator()
   border_t operator()(
      const bool     _bin,
      const bool     _object = default_object,
      const unsigned _small  = default_small,
      const unsigned _large  = default_large
   ) {
      bin    = _bin;
      object = _object;
      small  = _small;
      large  = _large;
      return *this;
   }
};

} // namespace detail



// -----------------------------------------------------------------------------
// image
// -----------------------------------------------------------------------------

// Default type: color
using default_color = rgba;

template<
   class real  = default_real,
   class color = default_color
>
class image {

   // copy constructor/assignment; deliberately private
   image(const image &) :
      hpixel(bitmap.isize()),
      vpixel(bitmap.jsize())
   { }
   image &operator=(const image &) { return *this; }

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
      explicit _prior() :
         hpixel(targets.isize()),
         vpixel(targets.jsize()), first(true)
      { }
   } prior;

   // image()
   explicit image() :
      bitmap(800,800),
      hpixel(bitmap.isize()),
      vpixel(bitmap.jsize()),
      anti  (default_anti)
   { }

   // --------------------------------
   // Miscellaneous
   // --------------------------------

   // fix
   image &fix();

   // resize
   image &resize(const ulong h, const ulong v = 0)
      { bitmap.resize(h, v ? v : h);  return *this; }

   // upsize
   image &upsize(const ulong h, const ulong v = 0)
      { bitmap.upsize(h, v ? v : h);  return *this; }

   // indexing
   const color &operator()(const ulong i, const ulong j) const
      { return bitmap(i,j); }
   color &operator()(const ulong i, const ulong j)
      { return bitmap(i,j); }

   // data access
   color *operator()()       { return bitmap.data(); }
   const color *operator()() const { return bitmap.data(); }
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
