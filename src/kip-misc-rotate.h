
#pragma once

// Empty classes for rotate's use. The namespace isn't really the best place
// for these, but it's short, and I don't want to pollute the main namespace.
namespace op {
   class full { };
   class part { };
   class unscaled { };
   class scaled { };
}

// rotate: default
// We'll have the following specializations:
//
//    rotate<2, real, op::full, op::unscaled>: 2-point, no scaling
//    rotate<3, real, op::full, op::unscaled>: 3-point, no scaling
//    rotate<3, real, op::part, op::unscaled>: 3-point, no scaling, small/simple
//    rotate<3, real, op::full, op::  scaled>: 3-point, with scaling
//
template<int, class, class, class>
class rotate { };



// -----------------------------------------------------------------------------
// mmm<real>
// -----------------------------------------------------------------------------

template<class real>
class mmm {
public:
   point<real> a; // "row 1"
   point<real> b; // "row 2"
   point<real> c; // "row 3"

   // mmm()
   explicit mmm() { }

   // mmm(one,two,three)
   explicit mmm(
      const point<real> &one,
      const point<real> &two,
      const point<real> &three
   ) :
      a(one),
      b(two),
      c(three)
   { }

   // operator()
   void operator()(
      const point<real> &one,
      const point<real> &two,
      const point<real> &three
   ) {
      a = one;
      b = two;
      c = three;
   }
};

// mmm * point
template<class real>
inline point<real> operator*(const mmm<real> &m, const point<real> &p)
{
   return point<real>(
      dot(m.a,p),
      dot(m.b,p),
      dot(m.c,p)
   );
}



// -----------------------------------------------------------------------------
// rotate<2,real,op::full,op::unscaled>
// -----------------------------------------------------------------------------

/*
Rigid translation and rotation, as follows:

   fore
      Translate "one" to the origin
      Rotate "two" to the +x axis

Rotation matrix for fore:
   mat.a.x   mat.a.y   mat.a.z
   mat.b.x   mat.b.y   mat.b.z=0
   mat.c.x   mat.c.y   mat.c.z
*/

template<class real>
class rotate<2,real,op::full,op::unscaled> {
public:
   mmm<real> mat;
   point<real> shift;
   real h;

   // rotate(one,two)
   explicit rotate(
      const point<real> &one,
      const point<real> &two
   ) :
      shift(one)
   {
      const point<real> diff = two - one;

      const real d = std::sqrt(op::square(diff.x) + op::square(diff.y));
      const real cost = d == 0 ? 1 : (1/d)*diff.x;
      const real sint = d == 0 ? 0 : (1/d)*diff.y;

      h = mod(diff);
      const real cosa = h == 0 ? 1 : (1/h)*d;
      const real sina = h == 0 ? 0 : (1/h)*diff.z;

      mat(
         point<real>( cost*cosa,  sint*cosa, sina),
         point<real>(-sint,       cost,         0),
         point<real>(-cost*sina, -sint*sina, cosa)
      );
   }

   // fore(p)
   point<real> fore(const point<real> &p) const
   {
      return point<real>(mat*(p-shift));
   }
};



// -----------------------------------------------------------------------------
// rotate<3,real,op::full,op::unscaled>
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
class rotate<3,real,op::full,op::unscaled> {
   point<real> val;
public:
   point<real> o, f1, f2, f3;
   real ex, ey;  // x and y coordinates of fore(three)
   real h;       // x coordinate of fore(two)


   // --------------------------------
   // Constructors
   // --------------------------------

   // rotate()
   explicit rotate() { }

   // rotate(one,two,three)
   explicit rotate(
      const point<real> &one,
      const point<real> &two,
      const point<real> &three
   ) :
      o(one)
   {
      const rotate<2,real,op::full,op::unscaled> t(one,two);
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

      f1 = t.mat.a;

      f2.x = t.mat.b.x*cosa + t.mat.c.x*sina;
      f2.y = t.mat.b.y*cosa + t.mat.c.y*sina;
      f2.z =              t.mat.c.z*sina;

      f3.x = t.mat.c.x*cosa - t.mat.b.x*sina;
      f3.y = t.mat.c.y*cosa - t.mat.b.y*sina;
      f3.z = t.mat.c.z*cosa;

      val(dot(f1,o), dot(f2,o), dot(f3,o));
   }

   // rotate(alpha,beta,gam,p): set up this structure so that back() rotates:
   //    clockwise about x by angle alpha, then
   //    clockwise about y by angle beta, then
   //    clockwise about z by angle gam,
   // and finally translates by (p.x, p.y, p.z).
   explicit rotate(
      const real alpha, const real beta, const real gam, const point<real> &p
   ) :
      o(p)
   {

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

   real forex(const point<real> &p) const { return dot(f1,p) - val.x; }
   real forey(const point<real> &p) const { return dot(f2,p) - val.y; }
   real forez(const point<real> &p) const { return dot(f3,p) - val.z; }

   point<real> fore(const point<real> &p) const
      { return point<real>(forex(p), forey(p), forez(p)); }


   // --------------------------------
   // back
   // --------------------------------

   point<real> back(const real px, const real py, const real pz) const
      { return o + px*f1 + py*f2 + pz*f3; }

   point<real> back(const point<real> &p) const
      { return back(p.x, p.y, p.z); }


   // --------------------------------
   // back specializations
   //    back_n00 (      py=0,  pz=0)
   //    back_0n0 (px=0,        pz=0)
   //    back_00n (px=0, py=0       )
   //    back_n01 (      py=0,  pz=1)
   //    back_0n1 (px=0,        pz=1)
   //    back_01n (px=0, py=1       )
   //    back_nm0 (      py=-1, pz=0)
   //    back_0nn (px=0             )
   // --------------------------------

   point<real> back_n00(const real px) const { return o + px*f1; }
   point<real> back_0n0(const real py) const { return o + py*f2; }
   point<real> back_00n(const real pz) const { return o + pz*f3; }

   point<real> back_n01(const real px) const { return o + px*f1 + f3; }
   point<real> back_0n1(const real py) const { return o + py*f2 + f3; }
   point<real> back_01n(const real pz) const { return o + pz*f3 + f2; }

   point<real> back_nm0(const real px) const { return o + px*f1 - f2; }

   point<real> back_0nn(const real py, const real pz) const
      { return o + py*f2 + pz*f3; }
};



// -----------------------------------------------------------------------------
// rotate<3,real,op::part,op::unscaled>
// Parts of rotate<3,real,op::full,op::unscaled> that are needed for forez only
// -----------------------------------------------------------------------------

template<class real>
class rotate<3,real,op::part,op::unscaled> {
public:
   point<real> f;
   real c;

   // rotate()
   explicit rotate() { }

   // rotate(one, two, three)
   explicit rotate(
      const point<real> &one,
      const point<real> &two,
      const point<real> &three
   ) {
      const rotate<2,real,op::full,op::unscaled> t(one,two);

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

      f(
         t.mat.c.x*cosa - t.mat.b.x*sina,
         t.mat.c.y*cosa - t.mat.b.y*sina,
         t.mat.c.z*cosa
      );

      c = dot(f,one);
   }

   // forez, ge, lt
   real forez(const point<real> &p) const { return dot(f,p)  - c; }
   bool    ge(const point<real> &p) const { return dot(f,p) >= c; }
   bool    lt(const point<real> &p) const { return dot(f,p) <  c; }

   bool lt(const point<real> &p, real &rv) const
      { return (rv = dot(f,p)) < c; }
};



// -----------------------------------------------------------------------------
// rotate<3,real,op::full,op::scaled>
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
class rotate<3,real,op::full,op::scaled> {
   point<real> f1, f2, f3;
public:
   point<real> o;
   real ex, ey, h;

   // rotate()
   explicit rotate() { }

   // rotate(one,two,three, factor)
   explicit rotate(
      const point<real> &one, const point<real> &two, const point<real> &three,
      const real factor
   ) :
      o(one)
   {
      const rotate<2,real,op::full,op::unscaled> t(one,two);
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

      f2.x = t.mat.b.x*cosa + t.mat.c.x*sina;  f1 = factor*t.mat.a;
      f2.y = t.mat.b.y*cosa + t.mat.c.y*sina;
      f2.z =              t.mat.c.z*sina;

      f3.x = t.mat.c.x*cosa - t.mat.b.x*sina;  ex *= factor;
      f3.y = t.mat.c.y*cosa - t.mat.b.y*sina;  ey *= factor;
      f3.z = t.mat.c.z*cosa;               h  *= factor;
   }

   // fore(p)
   point<real> fore(const point<real> &p) const
   {
      const point<real> t = p - o;
      return point<real>(dot(f1,t), dot(f2,t), dot(f3,t));
   }

   // back(p)
   point<real> back(const point<real> &p) const
      { return o + p.x*f1 + p.y*f2 + p.z*f3;  }
};
