
#pragma once



// -----------------------------------------------------------------------------
// mincolor
// maxcolor
// endcolor
// avgcolor
//
// I'd have preferred midcolor (middle color, midway between min and max colors)
// instead of avgcolor, but "midcolor" reads a little too close to "mincolor".
// -----------------------------------------------------------------------------

namespace detail {

// mincolor, for unsigned integral and floating-point
template<class comp>
inline constexpr typename std::enable_if<
  (std::is_unsigned<comp>::value &&
   std::is_integral<comp>::value) ||
   std::is_floating_point<comp>::value,
   comp
>::type mincolor()
{
   // 0, or 0.0
   return comp(0);
}



// maxcolor, for unsigned integral
template<class comp>
inline constexpr typename std::enable_if<
   std::is_unsigned<comp>::value &&
   std::is_integral<comp>::value,
   comp
>::type maxcolor()
{
   // e.g. 255 for comp = 8-bit unsigned char
   return std::numeric_limits<comp>::max();
}

// maxcolor, for floating-point
template<class comp>
inline constexpr typename std::enable_if<
   std::is_floating_point<comp>::value,
   comp
>::type maxcolor()
{
   // 1.0, as for a normalized, 0.0..1.0 floating-point color channel
   return comp(1);
}



// endcolor, for unsigned integral
// Result type: to
template<class to, class comp>
inline constexpr typename std::enable_if<
   std::is_unsigned<comp>::value &&
   std::is_integral<comp>::value,
   to
>::type endcolor()
{
   // e.g. 256 (not 255) for comp = 8-bit unsigned char; the
   // result type "to" should be larger, to handle the +1
   return to(maxcolor<comp>()) + to(1);
}

// endcolor, for floating
// Result type: to
template<class to, class comp>
inline constexpr typename std::enable_if<
   std::is_floating_point<comp>::value,
   to
>::type endcolor()
{
   // same as maxcolor()
   return to(1);
}



// avgcolor, for unsigned integral and floating-point
template<class comp>
inline constexpr typename std::enable_if<
  (std::is_unsigned<comp>::value &&
   std::is_integral<comp>::value) ||
   std::is_floating_point<comp>::value,
   comp
>::type avgcolor()
{
   // e.g. 127 (=255/2) for comp = 8-bit unsigned char; 0.5 for floating-point
   return comp((maxcolor<comp>() - mincolor<comp>())/2);
}

} // namespace detail



// -----------------------------------------------------------------------------
// RGB<comp>
// rgb
// -----------------------------------------------------------------------------

template<class comp>
class RGB {
public:
   // for i/o
   static constexpr const char *const description = "RGB";

   // channels
   union { comp r, red  ; };
   union { comp g, green; };
   union { comp b, blue ; };

   // RGB()
   explicit RGB()
      // I'm not sure I want any initialization in the default constructor,
      // but at the moment I'm getting "...may be used uninitialized..."
      // errors if I don't have it.
    : r(comp(0)),
      g(comp(0)),
      b(comp(0))
   { }

   // RGB(r,g,b)
   explicit RGB(const comp rfrom, const comp gfrom, const comp bfrom)
    : r(rfrom),
      g(gfrom),
      b(bfrom)
   { }

   // set(r,g,b)
   void set(const comp rfrom, const comp gfrom, const comp bfrom)
   {
      r = rfrom;
      g = gfrom;
      b = bfrom;
   }
};

/*
// description
template<class comp> const std::string RGB<comp>::description = "RGB";
*/

// for users
using rgb = RGB<uchar>;



// -----------------------------------------------------------------------------
// RGBA<comp>
// rgba
// -----------------------------------------------------------------------------

// transparent
template<class comp>
inline constexpr comp transparent()
   { return detail::mincolor<comp>(); }

// opaque
template<class comp>
inline constexpr comp opaque()
   { return detail::maxcolor<comp>(); }



// RGBA
template<class comp>
class RGBA {
public:
   // for i/o
   static constexpr const char *const description = "RGBA";

   // channels
   union { comp r, red  ; };
   union { comp g, green; };
   union { comp b, blue ; };
   union { comp a, alpha; };

   // RGBA()
   explicit RGBA()
   /*
   // zzz Check if we need these when used as input/base (not just output/color)
    : r(comp(0)),
      g(comp(0)),
      b(comp(0)),
      a(opaque<comp>())
   */
   { }

   // RGBA(r,g,b[,a])
   explicit RGBA(
      const comp rfrom, const comp gfrom, const comp bfrom,
      const comp afrom = opaque<comp>()
   )
    : r(rfrom),
      g(gfrom),
      b(bfrom),
      a(afrom)
   { }

   /*
   // RGBA(RGB[,a])
   explicit RGBA(
      const RGB<comp> from,
      const comp afrom = opaque<comp>()
   )
    : r(from.r),
      g(from.g),
      b(from.b),
      a(afrom)
   { }
   */

   template<
      class real,
      class = typename std::enable_if<
         std::is_floating_point<real>::value,
         void
      >::type
   >
   explicit RGBA(const RGBA<real> rhs)
   {
      r = comp(rhs.r);
      g = comp(rhs.g);
      b = comp(rhs.b);
   }

   // set(r,g,b[,a])
   void set(
      const comp rfrom, const comp gfrom, const comp bfrom,
      const comp afrom = opaque<comp>()
   ) {
      r = rfrom;
      g = gfrom;
      b = bfrom;
      a = afrom;
   }

   // zzz think about the following...

   // background()
   static constexpr RGBA background()
   {
      constexpr comp mid = detail::avgcolor<comp>();
      return RGBA(mid,mid,mid);
   }

   // border()
   static constexpr RGBA border()
   {
      constexpr comp max = detail::maxcolor<comp>();
      return RGBA(max,max,0);
   }

   // border(num,den)
   static const/*zzzexpr*/ RGBA border(const ulong num, const ulong den)
   {
      constexpr comp max = detail::maxcolor<comp>();

      // both nonzero: use fraction
      if (num && den) {
         if (!(num <= den))
            return RGBA(max, 0, 0);  // shouldn't happen; give red as warning
         const comp level = comp(max*num/den);
         return RGBA(level, level, level);
      }

      // nonzero, zero: means second was unknown (as with recursive trace)
      if (num)
         return RGBA(max, max, max);

      // bin size is zero, in any event
      return RGBA(0, 0, 0);
   }
};

/*
// description
template<class comp> const std::string RGBA<comp>::description = "RGBA";
*/

// for users
using rgba = RGBA<uchar>;



// -----------------------------------------------------------------------------
// randomize(RGB)
// randomize(RGBA)
// -----------------------------------------------------------------------------

namespace detail {

// helper: randomize_channel_ui
template<class comp>
inline typename std::enable_if<
   sizeof(comp) < sizeof(unsigned),
   comp
>::type randomize_channel_ui()
{
   /*
   Consider that rand() returns an int \in [0..RAND_MAX], where RAND_MAX,
   per the standard, must be at least 32767. In practice, say that ints
   are 32-bit. Then, RAND_MAX might be (but is not guaranteed to be)
   pow(2,31)-1, or 2147483647, the same as 32-bit int's maximum possible
   value. Next, consider that we'd like to produce a random value of type
   comp, \in [0..maxcolor<comp>()]. For example, with an 8-bit unsigned
   char for comp, we want a value in [0..255]. Or, for a 16-bit unsigned
   short for comp, we want a value in [0..65535]. Beginning with a rand()
   value computed via rand(), we must modulo with 256 (or 65536). These
   one-beyond-the-max values come from end = endcolor<unsigned,comp>()
   below. Our strict less-than condition, sizeof(comp) < sizeof(unsigned),
   is required so we don't try to compute one-beyond-the-max, of unsigned,
   into unsigned itself (which of course would overflow). Unsigned itself
   was chosen, in turn, for size compatibility with int, as from rand().
   If someone ever needs to handle larger sizeof(comp)s, we'll consider
   modifying this function at that time. Note: we suggest that callers
   of this function do the sizeof check themselves, so a compiler emits
   better errors regarding any misuse.
   */
   static constexpr unsigned end = endcolor<unsigned,comp>();
   return comp(unsigned(rand()) % end);
}

// helper: randomize_channel_fp
template<class comp>
inline comp randomize_channel_fp()
{
   return random_unit<comp>();
}

} // namespace detail



// randomize(RGB<comp>), for unsigned integral
template<class comp>
inline typename std::enable_if<
   std::is_unsigned<comp>::value &&
   std::is_integral<comp>::value &&
   sizeof(comp) < sizeof(unsigned), // see remark in randomize_channel_ui()
   RGB<comp>
>::type &randomize(RGB<comp> &obj)
{
   obj.r = detail::randomize_channel_ui<comp>();
   obj.g = detail::randomize_channel_ui<comp>();
   obj.b = detail::randomize_channel_ui<comp>();
   return obj;
}

// randomize(RGB<comp>), for floating-point
template<class comp>
inline typename std::enable_if<
   std::is_floating_point<comp>::value,
   RGB<comp>
>::type &randomize(RGB<comp> &obj)
{
   obj.r = detail::randomize_channel_fp<comp>();
   obj.g = detail::randomize_channel_fp<comp>();
   obj.b = detail::randomize_channel_fp<comp>();
   return obj;
}



// randomize(RGBA<comp>), for unsigned integral
template<class comp>
inline typename std::enable_if<
   std::is_unsigned<comp>::value &&
   std::is_integral<comp>::value &&
   sizeof(comp) < sizeof(unsigned), // see remark in randomize_channel_ui()
   RGBA<comp>
>::type &randomize(RGBA<comp> &obj)
{
   obj.r = detail::randomize_channel_ui<comp>();
   obj.g = detail::randomize_channel_ui<comp>();
   obj.b = detail::randomize_channel_ui<comp>();
   obj.a = detail::randomize_channel_ui<comp>();
   return obj;
}

// randomize(RGBA<comp>), for floating-point
template<class comp>
inline typename std::enable_if<
   std::is_floating_point<comp>::value,
   RGBA<comp>
>::type &randomize(RGBA<comp> &obj)
{
   obj.r = detail::randomize_channel_fp<comp>();
   obj.g = detail::randomize_channel_fp<comp>();
   obj.b = detail::randomize_channel_fp<comp>();
   obj.a = detail::randomize_channel_fp<comp>();
   return obj;
}



// -----------------------------------------------------------------------------
// convert
// -----------------------------------------------------------------------------

// RGB<comp> ==> RGB<comp>
template<class comp>
inline void convert(const RGB<comp> in, RGB<comp> &out)
{
   out = in;
}

// RGB<comp> ==> RGBA<comp>
template<class comp>
inline void convert(const RGB<comp> in, RGBA<comp> &out)
{
   out.set(in.r, in.g, in.b);
}



// -----------------------------------------------------------------------------
// Some operators for RGBA
// -----------------------------------------------------------------------------

// RGBA<OUT> += RGBA<IN>
template<class OUT, class IN>
inline RGBA<OUT> &operator+=(RGBA<OUT> &lhs, const RGBA<IN> rhs)
{
   lhs.r += OUT(rhs.r);
   lhs.g += OUT(rhs.g);
   lhs.b += OUT(rhs.b);
   lhs.a += OUT(rhs.a);
   return lhs;
}

// op::div(RGBA,den)
namespace op {
   template<class OUT, class IN>
   inline RGBA<OUT> div(const RGBA<IN> value, const unsigned den)
   {
      return RGBA<OUT>(
         OUT((value.r + den/2) / den),
         OUT((value.g + den/2) / den),
         OUT((value.b + den/2) / den),
         OUT((value.a + den/2) / den)
      );
   }
}



// -----------------------------------------------------------------------------
// Arithmetic
// -----------------------------------------------------------------------------

// real * RGBA
template<class real, class comp>
inline typename std::enable_if<
   std::is_floating_point<real>::value,
   RGBA<real>
>::type
operator*(const real lhs, const RGBA<comp> rhs)
{
   return RGBA<real>(
      lhs*rhs.r,
      lhs*rhs.g,
      lhs*rhs.b,
      lhs*rhs.a
   );
}

// RGBA + real
template<class real, class comp>
inline typename std::enable_if<
   std::is_floating_point<real>::value,
   RGBA<real>
>::type
operator+(const RGBA<comp> obj, const real f)
{
   return RGBA<real>(
      obj.r + f,
      obj.g + f,
      obj.b + f,
      obj.a + f
   );
}
