
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
// kipcolor
// -----------------------------------------------------------------------------

// general
template<class COLOR, class real, class BASE>
inline COLOR kipcolor(
   const kip::shape<real,BASE> &,
   const BASE &base,
   const kip::point<real> &
) {
   COLOR color;
   convert(base,color);
   return color;
}



// marble
// T for marble is probably the same as real, but may be different
template<class COLOR, class real, class BASE, class T>
COLOR kipcolor(
   const kip::shape<real,marble<BASE,T>> &shape,
   const marble<BASE,T> &in,
   const kip::point<real> &inter
) {
   const point<real> exact = shape.back(inter);
   T atotal;

   // basic marble texture
   const real noise = detail::noise(
      exact.x + in.seed,
      exact.y + in.seed,
      exact.z + in.seed,
      in.amp, in.ampfac,
      in.per, in.perfac,
      in.nfun,
      atotal // output
   );
   const real fac = atotal*std::sin(noise);

   COLOR color;
   convert(in,color);

   const rgb white(255,255,255);
   const int r = op::clip(0, int(color.r + (white.r-color.r)*fac), 255);
   const int g = op::clip(0, int(color.g + (white.g-color.g)*fac), 255);
   const int b = op::clip(0, int(color.b + (white.b-color.b)*fac), 255);

   // black swirls
   if (in.swirl) {
      const real sw_noise = detail::noise(
         exact.x,/// + in.seed*100,
         exact.y,/// + in.seed*10000,
         exact.z,/// + in.seed*1000000,
         // zzz probably have separate parameters for swirl code
         in.amp, in.ampfac,
         0.5*in.per, in.perfac,
         1, // in.nfun,
         atotal
      );
      const real p = 5*op::min(real(0),0.5+std::cos(20*sw_noise));
      color.r = uchar(op::clip(0, int(r + r*p), 255));
      color.g = uchar(op::clip(0, int(g + g*p), 255));
      color.b = uchar(op::clip(0, int(b + b*p), 255));
   } else
      color.set(uchar(r), uchar(g), uchar(b));

   return color;
}



// -----------------------------------------------------------------------------
// diffuse_specular
// -----------------------------------------------------------------------------

template<class color, class real>
color diffuse_specular(
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
// get_color
// -----------------------------------------------------------------------------

// get_color
template<class color, class real, class base, class pix>
color get_color(
   const point<real> &eyeball,
   const point<real> &light,
   const inq<real,base> &q,
   pix &pixel
) {
   // ----------------
   // Color
   // ----------------

#ifdef KIP_COLOR_FLAT
   // flat
   (void)eyeball; (void)light; (void)pixel;
   const RGB<uchar> c = *q.color;
   ///kipcolor<color>(q, q.fac > 0 ? q.fac*q : point<real>(q));
   const color rv(c.r, c.g, c.b);

#else
   // diffuse or specular
   //    const RGB<uchar> &rgbval,
   //    const real q,
   //    const point<real> &eyeball,
   //    const point<real> &light,
   //    const point<real> &intersection,
   //    const point<real> &normal,
   //    const bool isnormalized

   ///   std::cout << "q.fac = " << q.fac << std::endl;
   ///   std::cout << "point = " << point<real>(q) << std::endl;

   // shape color at ray intersection
   const color cshape = kipcolor<color>(
     *q.shape,
     *q.color,
      q.fac > 0 ? q.fac*q : point<real>(q)
   );

   // final color, after effect from light
   const color rv = diffuse_specular<color>(
      cshape,

      // scaled q needed by diffuse_specular()
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
      point<float>(q),

      // normal; scaling N/A
      point<float>(q.n),

      // normal-is-normalized
      q.isnormalized == normalized::yes
   );
#endif

   // done
   pixel.set(q);
   return rv;
}

} // namespace detail
