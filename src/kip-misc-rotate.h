
#pragma once

// This file provides the rotate class template, which supports certain
// translations/rotations.

template<signed char, class, bool = false>
class rotate { };



// -----------------------------------------------------------------------------
// rotate<2,real[,false]>
// -----------------------------------------------------------------------------

/*
Rigid translation and rotation, as follows:

   fore
      Translate "one" to the origin
      Rotate "two" to the x axis, with x >= 0

Rotation matrix for fore:
   m1.x   m1.y   m1.z
   m2x    m2y       0
   m3.x   m3.y   m3.z
*/

template<class real>
class rotate<2,real> {
   point<real> val;
public:
   point<real> o, m1, m3;
   real m2x, m2y, h;  // h = the x coordinate of fore(two)

   // rotate(one,two)
   inline explicit rotate(const point<real> &one, const point<real> &two) : o(one)
   {
      const point<real> t = two - one;
      const real tmp = t.x*t.x + t.y*t.y, rxy = std::sqrt(tmp);
      h = std::sqrt(tmp + t.z*t.z);

      real cosg, sing;
      if (rxy == 0) {
         cosg = 1;
         sing = 0;
      } else {
         const real rec = 1/rxy;
         cosg = t.x*rec;
         sing = t.y*rec;
      }

      real cosb, sinb;
      if (h == 0) {
         cosb = 1;
         sinb = 0;
      } else {
         const real rec = 1/h;
         cosb = rxy*rec;
         sinb = t.z*rec;
      }

      m1( cosb*cosg,  cosb*sing, sinb);  m2x = -sing;
      m3(-sinb*cosg, -sinb*sing, cosb);  m2y =  cosg;

      val(dot(m1,o), m2x*o.x + m2y*o.y, dot(m3,o));
   }

   // fore(p)
   inline point<real> fore(const point<real> &p) const
      { return point<real>(dot(m1,p), m2x*p.x + m2y*p.y, dot(m3,p)) - val; }
};



// -----------------------------------------------------------------------------
// rotate<3,real[,false]>
// -----------------------------------------------------------------------------

/*
Rigid translation and rotation, as follows:

   fore
      Translate "one" to the origin
      Rotate "two" to the x axis, with x >= 0
      Rotate "three" to the xy plane, with y >= 0

   back
      Do the reverse of fore

Rotation matrix for fore:
   f1.x   f1.y   f1.z
   f2.x   f2.y   f2.z
   f3.x   f3.y   f3.z
*/

template<class real>
class rotate<3,real> {
   point<real> val;
public:
   point<real> o, f1, f2, f3;
   real ex, ey;  // x and y coordinates of fore(three)
   real h;       // x coordinate of fore(two)


   // --------------------------------
   // Constructors
   // --------------------------------

   // rotate()
   inline explicit rotate() { }


   // rotate(one,two,three)
   inline explicit rotate(
      const point<real> &one, const point<real> &two, const point<real> &three
   ) : o(one) {

      const rotate<2,real> t(one,two);
      h = t.h;

      // tprime = point "three" before final rotation
      const point<real> tprime = t.fore(three);
      ex = tprime.x;
      ey = std::sqrt(tprime.y*tprime.y + tprime.z*tprime.z);

      real cosa, sina;
      if (ey == 0) {
         cosa = 1;
         sina = 0;
      } else {
         const real rec = 1/ey;
         cosa = tprime.y*rec;
         sina = tprime.z*rec;
      }

      f1 = t.m1;

      f2.x = t.m2x*cosa + t.m3.x*sina;
      f2.y = t.m2y*cosa + t.m3.y*sina;
      f2.z =              t.m3.z*sina;

      f3.x = t.m3.x*cosa - t.m2x*sina;
      f3.y = t.m3.y*cosa - t.m2y*sina;
      f3.z = t.m3.z*cosa;

      val(dot(f1,o), dot(f2,o), dot(f3,o));
   }


   // rotate(alpha,beta,gam,p): set up this structure so that back() rotates:
   //    clockwise about x by angle alpha, then
   //    clockwise about y by angle beta, then
   //    clockwise about z by angle gam,
   // and finally translates by (p.x, p.y, p.z).
   inline explicit rotate(
      const real alpha, const real beta, const real gam, const point<real> &p
   ) : o(p) {

      // Note: h, ex, and ey are not initialized by this constructor!
      const real sina = std::sin(alpha), cosa = std::cos(alpha);
      const real sinb = std::sin(beta ), cosb = std::cos(beta );
      const real sing = std::sin(gam  ), cosg = std::cos(gam  );

      f1(cosb*cosg, -cosb*sing, sinb);
      f2(cosa*sing + sina*sinb*cosg, cosa*cosg - sina*sinb*sing, -sina*cosb);
      f3(sina*sing - cosa*sinb*cosg, sina*cosg + cosa*sinb*sing,  cosa*cosb);

      val(dot(f1,o), dot(f2,o), dot(f3,o));
   }


   // --------------------------------
   // fore
   // --------------------------------

   inline real forex(const point<real> &p) const { return dot(f1,p) - val.x; }
   inline real forey(const point<real> &p) const { return dot(f2,p) - val.y; }
   inline real forez(const point<real> &p) const { return dot(f3,p) - val.z; }

   inline point<real> fore(const point<real> &p) const
      { return point<real>(forex(p), forey(p), forez(p)); }


   // --------------------------------
   // back
   // --------------------------------

   inline point<real> back(const real px, const real py, const real pz) const
      { return o + px*f1 + py*f2 + pz*f3; }

   inline point<real> back(const point<real> &p) const
      { return back(p.x, p.y, p.z); }


   // --------------------------------
   // back (special)
   //    back_n00 (py=0, pz=0)   back_n01 (py=0, pz=1)
   //    back_0n0 (px=0, pz=0)   back_0n1 (px=0, pz=1)   back_nm0 (py=-1, pz=0)
   //    back_00n (px=0, py=0)   back_01n (px=0, py=1)   back_0nn (px=0)
   // --------------------------------

   inline point<real> back_n00(const real px) const { return o + px*f1; }
   inline point<real> back_0n0(const real py) const { return o + py*f2; }
   inline point<real> back_00n(const real pz) const { return o + pz*f3; }

   inline point<real> back_n01(const real px) const { return o + px*f1 + f3; }
   inline point<real> back_0n1(const real py) const { return o + py*f2 + f3; }
   inline point<real> back_01n(const real pz) const { return o + pz*f3 + f2; }

   inline point<real> back_nm0(const real px) const { return o + px*f1 - f2; }

   inline point<real> back_0nn(const real py, const real pz) const
      { return o + py*f2 + pz*f3; }
};



// -----------------------------------------------------------------------------
// rotate<-3,real[,false]>
// Parts of rotate<3,real> that are needed for forez only
// -----------------------------------------------------------------------------

template<class real>
class rotate<-3,real> {
public:
   point<real> f;
   real c;

   // rotate()
   inline explicit rotate() { }

   // rotate(one, two, three)
   inline explicit rotate(
      const point<real> &one, const point<real> &two, const point<real> &three
   ) {
      const rotate<2,real> t(one,two);

      const point<real> tprime = t.fore(three);
      const real ey = std::sqrt(tprime.y*tprime.y + tprime.z*tprime.z);

      real cosa, sina;
      if (ey == 0) {
         cosa = 1;
         sina = 0;
      } else {
         const real rec = 1/ey;
         cosa = tprime.y*rec;
         sina = tprime.z*rec;
      }

      f(t.m3.x*cosa - t.m2x*sina,
        t.m3.y*cosa - t.m2y*sina, t.m3.z*cosa);

      c = dot(f,one);
   }

   // forez, ge, lt
   inline real forez(const point<real> &p) const { return dot(f,p)  - c; }
   inline bool    ge(const point<real> &p) const { return dot(f,p) >= c; }
   inline bool    lt(const point<real> &p) const { return dot(f,p) <  c; }

   inline bool lt(const point<real> &p, real &rv) const
      { return (rv = dot(f,p)) <  c; }
};



// -----------------------------------------------------------------------------
// rotate<3,real, true>
// -----------------------------------------------------------------------------

/*
Rigid translation, rotation, and scaling, as follows:

   fore
      Translate "one" to the origin
      Rotate "two" to the x axis, with x >= 0
      Rotate "three" to the xy plane, with y >= 0
      Scale by factor

   back
      Do the reverse of fore

Rotation matrix for fore:
   f1.x   f1.y   f1.z
   f2.x   f2.y   f2.z
   f3.x   f3.y   f3.z
*/

template<class real>
class rotate<3,real,true> {
   point<real> f1, f2, f3;
public:
   point<real> o;
   real ex, ey, h;


   // rotate()
   inline explicit rotate() { }

   // rotate(one,two,three, factor)
   inline explicit rotate(
      const point<real> &one, const point<real> &two, const point<real> &three,
      const real factor
   ) : o(one)
   {
      const rotate<2,real> t(one,two);
      h = t.h;

      const point<real> tprime = t.fore(three);
      ex = tprime.x;
      ey = std::sqrt(tprime.y*tprime.y + tprime.z*tprime.z);

      real cosa, sina;
      if (ey == 0) {
         cosa = factor;
         sina = 0;
      } else {
         const real rec = factor/ey;
         cosa = tprime.y*rec;
         sina = tprime.z*rec;
      }

      f2.x = t.m2x*cosa + t.m3.x*sina;  f1 = factor*t.m1;
      f2.y = t.m2y*cosa + t.m3.y*sina;
      f2.z =              t.m3.z*sina;

      f3.x = t.m3.x*cosa - t.m2x*sina;  ex *= factor;
      f3.y = t.m3.y*cosa - t.m2y*sina;  ey *= factor;
      f3.z = t.m3.z*cosa;               h  *= factor;
   }


   // fore(p)
   inline point<real> fore(const point<real> &p) const
   {
      const point<real> t = p - o;
      return point<real>(dot(f1,t), dot(f2,t), dot(f3,t));
   }

   // back(p)
   inline point<real> back(const point<real> &p) const
      { return o + p.x*f1 + p.y*f2 + p.z*f3;  }
};
