
#pragma once

// This file provides functions that support procedural textures.

#define KIP_COSINE
#define KIP_SMOOTH

class nothing_per_pixel {
public:
   inline void initialize() const
   {
      assert(false);
   }

   template<class real, class base>
   inline void set(const inq<real,base> &) const
   {
      // assert(false);
   }
};



// -----------------------------------------------------------------------------
// linear
// cosine
// interpolate
// ran
// smooth
// -----------------------------------------------------------------------------

namespace internal {

// linear
template<class real>
inline real linear(const real a, const real t, const real b)
   { return a*(1-t) + b*t; }

template<class real>
inline real linear(const real a, const real t, const real b, const char)
   { return a*(1-t) + b*t; }

// cosine
template<class real>
inline real cosine(const real a, const real t, const real b)
   { return linear(a, real(0.5)*(1-op::cos(real(kip_pi)*t)), b); }

template<class real>
inline real cosine(const real a, const real t, const real b, const char)
   { return linear(a, t, b); }



// interpolate
template<class real>
inline real interpolate(const real a, const real t, const real b)
{
#ifdef KIP_COSINE
   return cosine(a,t,b);
#else
   return linear(a,t,b);
#endif
}

template<class real>
inline real interpolate(const real a, const real t, const real b, const char ch)
{
#ifdef KIP_COSINE
   return cosine(a,t,b,ch);
#else
   return linear(a,t,b,ch);
#endif
}



// ran
template<class real>
inline real ran(
   const unsigned i,
   const int x,
   const int y,
   const int z
) {
   /*
   // zzz Need to change srand48() and drand48() calls, so that we instead
   // zzz use kip's own random<>() function.
   /// assert(false);
   return random<real>();

   srand48((long(i)<<6) + (long(x)<<12) + (long(y)<<18) + (long(z)<<24));
   const double seed = drand48();
   ///const long *const s = (const long *)&seed;
   // zzz darn --- the following doesn't get rid of g++'s warning about
   // type punning. Oh well, we should rewrite this function anyway, as
   // it's not thread-safe in its present form.
   const long *const s = reinterpret_cast<const long *>(&seed);
   srand48(s[0]^s[1]);
   return drand48();
   */

   (void)i;
   (void)x;
   (void)y;
   (void)z;
   return 0.12345678;
}

// smooth
template<class real>
inline real smooth(const unsigned i, const int x, const int y, const int z)
{
   return real(0.5)*(
      ran<real>(i,x,  y,  z  ) + real(1)/6*(
    + ran<real>(i,x+1,y,  z  )
    + ran<real>(i,x-1,y,  z  )
    + ran<real>(i,x,  y+1,z  )
    + ran<real>(i,x,  y-1,z  )
    + ran<real>(i,x,  y,  z+1)
    + ran<real>(i,x,  y,  z-1)
   ));
}

} // namespace internal



// -----------------------------------------------------------------------------
// noise (single function)
// -----------------------------------------------------------------------------

namespace internal {

template<class real>
inline real noise(
   const unsigned i, const real x, const real y, const real z
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
   const real tmp1 = smooth<real>(i, xint,  yint,   zint  );
   const real tmp2 = smooth<real>(i, xint+1,yint,   zint  );
   const real tmp3 = smooth<real>(i, xint,  yint+1, zint  );
   const real tmp4 = smooth<real>(i, xint+1,yint+1, zint  );
   const real tmp5 = smooth<real>(i, xint,  yint,   zint+1);
   const real tmp6 = smooth<real>(i, xint+1,yint,   zint+1);
   const real tmp7 = smooth<real>(i, xint,  yint+1, zint+1);
   const real tmp8 = smooth<real>(i, xint+1,yint+1, zint+1);
#else
   const real tmp1 = ran   <real>(i, xint,  yint,   zint  );
   const real tmp2 = ran   <real>(i, xint+1,yint,   zint  );
   const real tmp3 = ran   <real>(i, xint,  yint+1, zint  );
   const real tmp4 = ran   <real>(i, xint+1,yint+1, zint  );
   const real tmp5 = ran   <real>(i, xint,  yint,   zint+1);
   const real tmp6 = ran   <real>(i, xint+1,yint,   zint+1);
   const real tmp7 = ran   <real>(i, xint,  yint+1, zint+1);
   const real tmp8 = ran   <real>(i, xint+1,yint+1, zint+1);
#endif

   const real tx = real(0.5)*(1 - op::cos(real(kip_pi)*(x-xint)));
   const real ty = real(0.5)*(1 - op::cos(real(kip_pi)*(y-yint)));

   return interpolate(
      interpolate(
         interpolate(tmp1,tx,tmp2,'c'),
         ty,
         interpolate(tmp3,tx,tmp4,'c'), 'c'
      ),

      z-zint,

      interpolate(
         interpolate(tmp5,tx,tmp6,'c'),
         ty,
         interpolate(tmp7,tx,tmp8,'c'), 'c'
      )
   );
}

} // namespace internal



// -----------------------------------------------------------------------------
// noise (sum of functions)
// -----------------------------------------------------------------------------

template<class real>
inline real noise(
   const real x, const real y, const real z,
   const real amp, const real ampfac,
   const real per, const real perfac, const unsigned nfun,
   real &atotal
) {
   // normalize by total possible amplitude; return value in [-1,1]
   if (nfun == 0) {
      return atotal = 0;
   } else if (nfun == 1) {
      atotal = amp;
      return op::twice(internal::noise(1, x/per, y/per, z/per) - 0.5);
   } else {
      real val = 0, a = amp, p = per;
      atotal = 0;
      for (unsigned i = 0;  i < nfun;  ++i, a *= ampfac, p *= perfac) {
         atotal += a;
         val += a*(internal::noise(i+1, x/p, y/p, z/p) - 0.5);
      }
      return (val+val)/atotal;
   }

   // Note: in practice, and especially with smoothing on, I've found that this
   // function tends to return values somewhat INSIDE of [-1,1]; that is, it's
   // rare to see values very close to -1 or to 1. I should look into this more,
   // as it may be desirable to get values throughout [-1,1] more often.
}



// -----------------------------------------------------------------------------
// diffuse_specular
// -----------------------------------------------------------------------------

namespace internal {

template<class outcolor, class T>
inline outcolor diffuse_specular(
   const kip::RGB<unsigned char> &rgbval,
   const T &q,
   const point<T> &eyeball,
   const point<T> &light,
   const point<T> &intersection,
   const point<T> &normal,
   const bool normalized
) {
   // Assumptions: eyeball != intersection, light != intersection,
   // normal != (0,0,0).
   using ctype = typename outcolor::value_t;

   const point<T> n   = normalized ? normal : normalize(normal);
   const point<T> i2l = light - intersection;

   const T modi =  mod(i2l);
   const T dotp =  dot(n,i2l);
   const T diff = (dotp+modi)/(modi+modi);

#ifdef KIP_COLOR_DIFFUSE
   (void)q;
   (void)eyeball;

   // diffuse
   return outcolor(
      ctype(diff*T(rgbval.r)),
      ctype(diff*T(rgbval.g)),
      ctype(diff*T(rgbval.b))
   );

#else

   // diffuse + specular
   const T d = dot((dotp+dotp)*n - i2l, eyeball - intersection);
   if (d < 0)
      return outcolor(
         ctype(diff*T(rgbval.r)),
         ctype(diff*T(rgbval.g)),
         ctype(diff*T(rgbval.b))
      );

   const T s = /*op::*/std::pow(d/(q*modi),T(20));
   const T f = diff*(1-s);
   const T spec = s*endcolor<T,ctype>();

   return outcolor(
      ctype(f*T(rgbval.r) + spec),
      ctype(f*T(rgbval.g) + spec),
      ctype(f*T(rgbval.b) + spec)
   );

#endif
}

} // namespace internal



// -----------------------------------------------------------------------------
// kipcolor
// -----------------------------------------------------------------------------

// general
template<class out, class real, class base>
inline out kipcolor(
   const shape<real,base> *const, const base &qcolor,
   const point<real> &
) {
   return qcolor;
}



   /*
// RGB
template<class real, class rgb_t>
inline RGB<rgb_t> kipcolor(
   const shape<real,RGB<rgb_t>> *const,
   const RGB<rgb_t> &qcolor,
   const point<real> &
) {
   return qcolor;
}

// crayola
template<class real>
inline RGB<crayola_rgb_t> kipcolor(
   const shape<real,crayola> *const,
   const crayola &qcolor,
   const point<real> &
) {
   return qcolor;
}
   */



// marble
template<class out, class real, class rgb_t, class mreal>
inline out kipcolor(
   const shape<real,marble<rgb_t,mreal>> *const qshape,
   const marble<rgb_t,mreal> &qcolor,
   const point<real> &_intersection
) {
   const point<real> intersection = qshape->back(_intersection);
   const real seed = qcolor.seed;
   mreal atotal;

   // basic marble texture
   real noise = kip::noise(
      intersection.x + seed,
      intersection.y + seed,
      intersection.z + seed,
      qcolor.amp, qcolor.ampfac,
      qcolor.per, qcolor.perfac,
      qcolor.nfun, atotal
   );
   const real fac = atotal*op::sin(noise);

   int r = qcolor.r;
   int g = qcolor.g;
   int b = qcolor.b;

   r = op::clip(0, int(r + (255-r)*fac), 255);
   g = op::clip(0, int(g + (255-g)*fac), 255);
   b = op::clip(0, int(b + (255-b)*fac), 255);

   // black swirls
   if (qcolor.swirl) {
      real sw_noise = kip::noise(
         intersection.x + 100*seed,
         intersection.y + 10000*seed,
         intersection.z + 1000000*seed,
         qcolor.amp, qcolor.ampfac,
         qcolor.per, qcolor.perfac,
         qcolor.nfun, atotal
      );
      const real p = 5*op::min(real(0),0.8+op::cos(20*sw_noise));
      r = op::clip(0, int(r + r*p), 255);
      g = op::clip(0, int(g + g*p), 255);
      b = op::clip(0, int(b + b*p), 255);
   }

   return out(rgb_t(r), rgb_t(g), rgb_t(b));
}



// -----------------------------------------------------------------------------
// get_color
// -----------------------------------------------------------------------------

namespace internal {

// get_color
template<class out, class real, class base, class pix>
inline out get_color(
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
   const RGB<unsigned char> c = *q.color;
   ///kip::kipcolor<out>(q, q.fac > 0 ? q.fac*q : point<real>(q));
   const out rv(c.r, c.g, c.b);

#else
   // diffuse or specular
   //    const kip::RGB<unsigned char> &rgbval,
   //    const T &q,
   //    const point<T> &eyeball,
   //    const point<T> &light,
   //    const point<T> &intersection,
   //    const point<T> &normal,
   //    const bool normalized

   /*
   assert(sizeof(out) == 4);
   assert(sizeof(out().r) == 1);
   */

   const out rv = diffuse_specular<out>(
      // RGB
      kip::kipcolor<RGB<unsigned char>>(
         q.shape,
        *q.color,
         q.fac > 0 ? q.fac*q : point<real>(q)  // true intersection
      ),

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
      q.normalized == normalized_t::yesnorm
   );
#endif

   // done
   pixel.set(q);
   return rv;
}

} // namespace internal
