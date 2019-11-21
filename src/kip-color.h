
///#define KIP_SMOOTH

class nothing_per_pixel {
public:
   void initialize() const
   {
      assert(false);
   }

   template<class real, class base>
   void set(const inq<real,base> &) const
   { }
};

// largely for testing
inline bool flat = false;
inline int npic = -1;

// for the rest of this file
namespace detail {



// -----------------------------------------------------------------------------
// linear
// cosine
// ran
// smooth
// -----------------------------------------------------------------------------

// linear interpolation
template<class real>
inline real linear(const real a, const real t, const real b)
   { return a*(1-t) + b*t; }

// cosine interpolation
template<class real>
inline real cosine(const real a, const real t, const real b)
   { return linear(a, real(0.5)*(1-std::cos(pi<real>*t)), b); }



// fastrand
inline int fastrand(const unsigned seed)
{ 
   return ((214013 * seed + 2531011) >> 16) & 0x7FFF; 
} 

// ran
template<class real>
inline real ran(
   const unsigned n,
   const int i,
   const int j,
   const int k
) {
   // perhaps a bit less direction-biased that our other variant
   return fastrand(
      (n <<  8) +
      (i << 10) +
      (j << 12) +
      (k << 14)
   )/real(0x7FFF);
}



// smooth
template<class real>
inline real smooth(
   const unsigned n,
   const int i,
   const int j,
   const int k
) {
   return real(0.5)*(
      ran<real>( n, i,   j,   k   ) + real(1)/6*(
    + ran<real>( n, i-1, j,   k   )
    + ran<real>( n, i+1, j,   k   )
    + ran<real>( n, i,   j-1, k   )
    + ran<real>( n, i,   j+1, k   )
    + ran<real>( n, i,   j,   k-1 )
    + ran<real>( n, i,   j,   k+1 )
   ));
}



// -----------------------------------------------------------------------------
// noise (single function)
// -----------------------------------------------------------------------------

template<class real>
real noise(
   const unsigned n,
   const real x,
   const real y,
   const real z
) {
   // These assume that we haven't passed some absurdly large x, y, or z value
   // that doesn't properly convert to int. If we did that, then the "z-zint"
   // parameter in the final interpolation below is outside of [0,1], and we
   // end up with an extrapolation, which can give us a final return value
   // that is outside of the intended range of [0,1).
   const int xint = x < 0 ? int(x)-1 : int(x);
   const int yint = y < 0 ? int(y)-1 : int(y);
   const int zint = z < 0 ? int(z)-1 : int(z);

   /*
     7--------8
    /|       /|
   5-+------6 |
   | |      | |
   | 3------+-4
   |/       |/
   1--------2
   */

#ifdef KIP_SMOOTH
   const real tmp1 = smooth<real>(n, xint,  yint,   zint  );
   const real tmp2 = smooth<real>(n, xint+1,yint,   zint  );
   const real tmp3 = smooth<real>(n, xint,  yint+1, zint  );
   const real tmp4 = smooth<real>(n, xint+1,yint+1, zint  );
   const real tmp5 = smooth<real>(n, xint,  yint,   zint+1);
   const real tmp6 = smooth<real>(n, xint+1,yint,   zint+1);
   const real tmp7 = smooth<real>(n, xint,  yint+1, zint+1);
   const real tmp8 = smooth<real>(n, xint+1,yint+1, zint+1);
#else
   const real tmp1 = ran   <real>(n, xint,  yint,   zint  );
   const real tmp2 = ran   <real>(n, xint+1,yint,   zint  );
   const real tmp3 = ran   <real>(n, xint,  yint+1, zint  );
   const real tmp4 = ran   <real>(n, xint+1,yint+1, zint  );
   const real tmp5 = ran   <real>(n, xint,  yint,   zint+1);
   const real tmp6 = ran   <real>(n, xint+1,yint,   zint+1);
   const real tmp7 = ran   <real>(n, xint,  yint+1, zint+1);
   const real tmp8 = ran   <real>(n, xint+1,yint+1, zint+1);
#endif

   const real tx = real(0.5)*(1 - std::cos(pi<real>*(x-xint)));
   const real ty = real(0.5)*(1 - std::cos(pi<real>*(y-yint)));

   return cosine(
      linear(linear(tmp1,tx,tmp2), ty, linear(tmp3,tx,tmp4)),
      z-zint,
      linear(linear(tmp5,tx,tmp6), ty, linear(tmp7,tx,tmp8))
   );
}



// -----------------------------------------------------------------------------
// noise (sum of functions)
// -----------------------------------------------------------------------------

template<class real>
real noise(
   const real x,
   const real y,
   const real z,
   const real amp, const real ampfac,
   const real per, const real perfac,
   const unsigned nfun,
   real &atotal // output
) {
   // normalize by total possible amplitude; return value in [-1,1]
   if (nfun == 0) {
      return atotal = 0;
   } else if (nfun == 1) {
      atotal = amp;
      return 2*noise(1,x/per,y/per,z/per) - 1;
   } else {
      real val = 0, a = amp, p = per;
      atotal = 0;
      for (unsigned i = 0;  i < nfun;  ++i, a *= ampfac, p *= perfac) {
         atotal += a;
         val += a*(noise(i+1, x/p, y/p, z/p) - 0.5);
      }
      return (val+val)/atotal;
   }

   // Note: in practice, and especially with smoothing on, I've found that this
   // function tends to return values somewhat INSIDE of [-1,1]; that is, it's
   // rare to see values very close to -1 or to 1. I should look into this more,
   // as it may be desirable to get values throughout [-1,1] more often.
}



// -----------------------------------------------------------------------------
// shape_color
//
// The two bools are:
//
//  - IN. Should shape.back() be called (true) to get the actual (neither
//    rotated nor scaled) intersection? False == no, that was done already.
//
//  - OUT (but caller initializes to true). Should the caller continue on
//    and modify the returned color to account for a light-based highlight?
//
// Note 1: The general case doesn't [need to] do anything with those.
//
// Note 2: Because shape.back() is virtual, we don't just call it directly
// from pixel_color() later in this file, if it's needed, avoiding the worry
// about conditionally doing so later. As far as the optimizer knows, the
// back() call (remember, virtual and probably indeterminate at compile-time)
// may have side effects, so that it probably can't optimize it away even if
// the shape_color() function that receives back()'s result doesn't use it.
// -----------------------------------------------------------------------------

// general
template<class COLOR, class SHAPE, class IN, class real>
inline COLOR shape_color(
   const SHAPE &,
   const IN &in,
   const kip::point<real> &,
   const bool,  // "back": call shape.back()?
   const bool & // "hilite": default=true; should caller highlight?
) {
   // The general case just reports the object's basic color,
   // and doesn't need to deal with the bools
   COLOR out;
   convert(in,out);
   return out;
}



// marble
// T for marble is probably the same as real, but doesn't need to be
template<class COLOR, class SHAPE, class BASE, class T, class real>
COLOR shape_color(
   const SHAPE &shape,
   const marble<BASE,T> &mar,
   const kip::point<real> &intersection,
   bool  back,
   bool & // hilite
) {
   const point<real> exact = back ? shape.back(intersection) : intersection;
   back = false;

   // basic marble texture
   T atotal;
   const real noise = detail::noise(
      exact.x + mar.seed,
      exact.y + mar.seed,
      exact.z + mar.seed,
      mar.amp, mar.ampfac,
      mar.per, mar.perfac,
      mar.nfun,
      atotal // output
   );
   const real fac = atotal*std::sin(noise);

   COLOR out;
   convert(mar,out);

   const rgb white(255,255,255);
   const int r = op::clip(0, int(out.r + (white.r-out.r)*fac), 255);
   const int g = op::clip(0, int(out.g + (white.g-out.g)*fac), 255);
   const int b = op::clip(0, int(out.b + (white.b-out.b)*fac), 255);

   // black swirls
   if (mar.swirl) {
      const real sw_noise = detail::noise(
         exact.x,/// + mar.seed*100,
         exact.y,/// + mar.seed*10000,
         exact.z,/// + mar.seed*1000000,
         // zzz probably have separate parameters for swirl code
         mar.amp, mar.ampfac,
         0.5*mar.per, mar.perfac,
         1, // mar.nfun,
         atotal
      );
      const real p = 5*op::min(0,0.5+std::cos(20*sw_noise));
      out.r = uchar(op::clip(0, int(r + r*p), 255));
      out.g = uchar(op::clip(0, int(g + g*p), 255));
      out.b = uchar(op::clip(0, int(b + b*p), 255));
   } else
      out.set(uchar(r), uchar(g), uchar(b));

   return out;
}



// picture
// T for picture is probably the same as real, but doesn't need to be
template<class COLOR, class SHAPE, class BASE, class T, class real>
COLOR shape_color(
   const SHAPE &shape,
   const picture<BASE,T> &pic,
   const kip::point<real> &intersection,
   bool  back,
   bool &hilite
) {
   // use base, if pic.i is out of range
   const int nimage = int(pic.image.size());
   if (!(0 <= pic.i && pic.i < nimage))
      return shape_color<COLOR>(shape, pic.base, intersection, back, hilite);

   // select image
   const kip::array<2,kip::rgb> &arr =
      *pic.image[ulong(kip::npic < 0 ? pic.i : (kip::npic %= nimage))];

   // use base, if image is empty
   if (arr.size() == 0)
      return shape_color<COLOR>(shape, pic.base, intersection, back, hilite);

   // OK, good to go...
   const point<real> exact = back ? shape.back(intersection) : intersection;
   back = false;

   // zzz Look over the following carefully, for correctness; also simplify

   // For now, consider "exact" relative to (0,0,0)
   /*
   const real x = exact.x;
   const real y = exact.y;
   const real z = exact.z;
   */
   const real r = mod(exact);
   const real p = atan2(exact.y,exact.x); // [-pi..pi]
   const real t = acos(exact.z/r);

   const int isize = int(arr.isize());
   const int jsize = int(arr.jsize());

   const real pfrac = std::abs(p + pi<real>)/(2*pi<real>);
   const real tfrac = t/pi<real>;

   const kip::rgb in = arr(
      ulong(int(pfrac * isize) % isize),
      ulong(int(tfrac * jsize) % jsize)
   );
   COLOR out;
   convert(in,out);

   // done
   hilite = false;
   return out;
}



// -----------------------------------------------------------------------------
// highlight
// -----------------------------------------------------------------------------

template<class color, class real>
color highlight(
   const color &shapecol,
   const real q,
   const point<real> &eyeball,
   const point<real> &light,
   const point<real> &intersection,
   const point<real> &normal,
   const bool isnormalized
) {
   // Assumptions:
   // eyeball != intersection
   // light   != intersection,
   // normal  != (0,0,0).

   const point<real> n   = isnormalized ? normal : normalize(normal);
   const point<real> i2l = light - intersection;

   const real modi =  mod(i2l);
   const real dotp =  dot(n,i2l);
   const real diff = (dotp+modi)/(modi+modi);

#ifdef KIP_COLOR_DIFFUSE

   // diffuse
   return color(diff*shapecol);

#else

   // diffuse + specular
   const real d = dot((dotp+dotp)*n - i2l, eyeball - intersection);
   if (d < 0)
      return color(diff*shapecol);

   const real s = std::pow(d/(q*modi),real(20));
   const real f = diff*(1-s);
   using ctype = uchar; // qqq do something about this...
   const real spec = s*endcolor<real,ctype>();

   return color(f*shapecol + spec);

#endif
}



// -----------------------------------------------------------------------------
// pixel_color
// -----------------------------------------------------------------------------

// Normally I'd have ordered the template parameters (real,base,color), as is
// done elsewhere. Here, however, at the time of this writing, color doesn't
// appear in the argument list, and we need the output to be of this type. So,
// we're putting color first so it's easy to call as pixel_color<color>(...).

template<class color, class real, class base, class EXTRA>
color pixel_color(
   const point<real> &eyeball,
   const point<real> &light,
   const inq<real,base> &q,
   // extra per-pixel information (not pixel color, which is returned)
   EXTRA &pixinfo
) {
   // Extra per-pixel information
   pixinfo.set(q);

   // Flat?
   // Then simple base ==> color conversion...
   if (kip::flat) {
      color out;
      convert(*q.color,out);
      return out;
   }

   // Not flat?
   // Then the general case...

   // Shape color at ray intersection. In the general case this is just copied
   // from the second argument, but it can be different for special base types
   // such as marble (which generally computes to something else) or picture.
   bool hilite = true; // unless overridden...
   const color c = shape_color<color>(
     *q.shape, // the shape itself
     *q.color, // usually shape->color, but (e.g. for [xyz]plane) can differ
      q.fac > 0 ? q.fac*q.inter : q.inter,
      true,    // true == must use back() on above if need true intersection
      hilite   // defaulted to true above; callee can change
   );
   if (!hilite) return c;

   // Final color, after effect from light
   const color out = highlight(
      // nominal shape color at intersection, from above
      c,
      // scaled q
      float(q.fac > 0 ? q.q/q.fac : q.q),
      // scaled eyeball
      q.shape->eyelie
         ? point<float>(q.shape->basic.eye())
         : point<float>(eyeball),
      // scaled light
      q.shape->eyelie
         ? q.shape->basic.lie()
         : point<float>(light),
      // scaled intersection
      point<float>(q.inter),
      // normal; scaling irrelevant
      point<float>(q.n),
      // normal-is-normalized
      q.isnormalized == normalized::yes
   );

   // Done
   return out;
}

} // namespace detail
