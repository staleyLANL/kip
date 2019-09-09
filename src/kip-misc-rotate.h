
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
//    rotate<2,real,full,unscaled>: 2-point, no scaling
//    rotate<3,real,full,unscaled>: 3-point, no scaling
//    rotate<3,real,part,unscaled>: 3-point, no scaling, small/simple
//    rotate<3,real,full,  scaled>: 3-point, with scaling
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
};

// mmm * mmm
template<class real>
inline mmm<real> operator*(const mmm<real> &one, const mmm<real> &two)
{
   return mmm<real>(
      point<real>(
         dot(one.a,point<real>(two.a.x,two.b.x,two.c.x)),
         dot(one.a,point<real>(two.a.y,two.b.y,two.c.y)),
         dot(one.a,point<real>(two.a.z,two.b.z,two.c.z))),
      point<real>(
         dot(one.b,point<real>(two.a.x,two.b.x,two.c.x)),
         dot(one.b,point<real>(two.a.y,two.b.y,two.c.y)),
         dot(one.b,point<real>(two.a.z,two.b.z,two.c.z))),
      point<real>(
         dot(one.c,point<real>(two.a.x,two.b.x,two.c.x)),
         dot(one.c,point<real>(two.a.y,two.b.y,two.c.y)),
         dot(one.c,point<real>(two.a.z,two.b.z,two.c.z)))
   );
}

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
// rotate<2,real,full,unscaled>
// -----------------------------------------------------------------------------

/*
Translation and rotation, as follows:

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
      const real cosg = d == 0 ? 1 : (1/d)*diff.x;
      const real sing = d == 0 ? 0 : (1/d)*diff.y;

      h = mod(diff);
      const real cosb = h == 0 ? 1 : (1/h)*d;
      const real sinb = h == 0 ? 0 : (1/h)*diff.z;

      // mat...

      // slower
      // mat = mmm<real>(
      //    point<real>( cosg*cosb,  sing*cosb, sinb),
      //    point<real>(-sing,       cosg,         0),
      //    point<real>(-cosg*sinb, -sing*sinb, cosb)
      // );

      // faster
      const mmm<real> rotz(
         point<real>( cosg, sing, 0),
         point<real>(-sing, cosg, 0),
         point<real>(    0,    0, 1)
      );

      const mmm<real> roty(
         point<real>( cosb, 0, sinb),
         point<real>(   0,  1,    0),
         point<real>(-sinb, 0, cosb)
      );

      mat = roty*rotz;
   }

   // fore(p)
   point<real> fore(const point<real> &p) const { return mat*(p-shift); }
};



// -----------------------------------------------------------------------------
// rotate<3,real,full,unscaled>
// -----------------------------------------------------------------------------

/*
Translation and rotation, as follows:

   fore
      Translate "one" to the origin
      Rotate "two" to the +x axis
      Rotate "three" to the xy plane, with y >= 0

   back
      Do the reverse of fore

Rotation matrix for fore:
   mat.a.x   mat.a.y   mat.a.z
   mat.b.x   mat.b.y   mat.b.z
   mat.c.x   mat.c.y   mat.c.z
*/

template<class real>
class rotate<3,real,op::full,op::unscaled> {
   point<real> val;
   point<real> shift;
public:
   mmm<real> mat;
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
      shift(one)
   {
      const rotate<2,real,op::full,op::unscaled> rot(one,two);
      h = rot.h;

      // t = point "three" before final rotation
      const point<real> t = rot.fore(three);
      ex = t.x;
      ey = std::sqrt(t.y*t.y + t.z*t.z);

      const real cosa = ey == 0 ? 1 : (1/ey)*t.y;
      const real sina = ey == 0 ? 0 : (1/ey)*t.z;

      // mat...

      // slower
      // const mmm<real> rotx(
      //    point<real>( 1,     0,    0 ),
      //    point<real>( 0,  cosa, sina ),
      //    point<real>( 0, -sina, cosa )
      // );
      // mat = rotx*rot.mat;

      // faster
      mat.a = rot.mat.a;

      mat.b.x = rot.mat.b.x*cosa + rot.mat.c.x*sina;
      mat.b.y = rot.mat.b.y*cosa + rot.mat.c.y*sina;
      mat.b.z =                    rot.mat.c.z*sina;

      mat.c.x = rot.mat.c.x*cosa - rot.mat.b.x*sina;
      mat.c.y = rot.mat.c.y*cosa - rot.mat.b.y*sina;
      mat.c.z = rot.mat.c.z*cosa;

      // val
      val = mat*shift;
   }


   // rotate(alpha,beta,gam,p): construct so that back() rotates:
   //    clockwise about x by angle alpha, then...
   //    clockwise about y by angle beta,  then...
   //    clockwise about z by angle gam,
   // and finally translates by (one.x, one.y, one.z).
   explicit rotate(
      const real alpha, const real beta, const real gam,
      const point<real> &one
   ) :
      shift(one)
   {
      // Note: h, ex, and ey are NOT initialized by this constructor!
      const real sina = std::sin(alpha), cosa = std::cos(alpha);
      const real sinb = std::sin(beta ), cosb = std::cos(beta );
      const real sing = std::sin(gam  ), cosg = std::cos(gam  );

      // direct
      // mat.a(cosb*cosg, -cosb*sing, sinb);
      // mat.b(cosa*sing+sina*sinb*cosg, cosa*cosg-sina*sinb*sing, -sina*cosb);
      // mat.c(sina*sing-cosa*sinb*cosg, sina*cosg+cosa*sinb*sing,  cosa*cosb);

      // speed comparable to direct, IF we use the (roty*rotz) parens
      const mmm<real> rotz(
         point<real>( cosg, -sing, 0 ),
         point<real>( sing,  cosg, 0 ),
         point<real>(    0,     0, 1 )
      );
      const mmm<real> roty(
         point<real>(  cosb, 0, sinb ),
         point<real>(    0,  1,    0 ),
         point<real>( -sinb, 0, cosb )
      );
      const mmm<real> rotx(
         point<real>( 1,    0,     0 ),
         point<real>( 0, cosa, -sina ),
         point<real>( 0, sina,  cosa )
      );
      mat = rotx*(roty*rotz);

      // val
      val = mat*shift;
   }


   // --------------------------------
   // fore
   // --------------------------------

   real forex(const point<real> &p) const { return dot(mat.a,p) - val.x; }
   real forey(const point<real> &p) const { return dot(mat.b,p) - val.y; }
   real forez(const point<real> &p) const { return dot(mat.c,p) - val.z; }

   point<real> fore(const point<real> &p) const
   {
      // faster
      return point<real>(forex(p), forey(p), forez(p));

      // slower
      // return mat*(p-shift);
   }


   // --------------------------------
   // back
   // --------------------------------

   point<real> back(const real px, const real py, const real pz) const
      { return shift + px*mat.a + py*mat.b + pz*mat.c; }

   point<real> back(const point<real> &p) const
      { return back(p.x, p.y, p.z); }


   // --------------------------------
   // back specializations
   //    back_n00 (      py= 0, pz=0)
   //    back_0n0 (px=0,        pz=0)
   //    back_00n (px=0, py= 0      )
   //    back_n01 (      py= 0, pz=1)
   //    back_0n1 (px=0,        pz=1)
   //    back_01n (px=0, py= 1      )
   //    back_nm0 (      py=-1, pz=0)
   //    back_0nn (px=0             )
   // --------------------------------

   point<real> back_n00(const real px) const { return shift + px*mat.a; }
   point<real> back_0n0(const real py) const { return shift + py*mat.b; }
   point<real> back_00n(const real pz) const { return shift + pz*mat.c; }

   point<real> back_n01(const real px) const { return shift + px*mat.a + mat.c; }
   point<real> back_0n1(const real py) const { return shift + py*mat.b + mat.c; }
   point<real> back_01n(const real pz) const { return shift + pz*mat.c + mat.b; }

   point<real> back_nm0(const real px) const { return shift + px*mat.a - mat.b; }

   point<real> back_0nn(const real py, const real pz) const
      { return shift + py*mat.b + pz*mat.c; }
};



// -----------------------------------------------------------------------------
// rotate<3,real,part,unscaled>
// Parts of rotate<3,real,full,unscaled> that are needed for forez only
// -----------------------------------------------------------------------------

template<class real>
class rotate<3,real,op::part,op::unscaled> {
public:
   point<real> matc;
   real valz;

   // rotate()
   explicit rotate() { }

   // rotate(one,two,three)
   explicit rotate(
      const point<real> &one,
      const point<real> &two,
      const point<real> &three
   ) {
      const rotate<2,real,op::full,op::unscaled> rot(one,two);

      const point<real> t = rot.fore(three);
      const real ey = std::sqrt(t.y*t.y + t.z*t.z);

      const real cosa = ey == 0 ? 1 : (1/ey)*t.y;
      const real sina = ey == 0 ? 0 : (1/ey)*t.z;

      // like mat.c in the full version
      matc = cosa*rot.mat.c - sina*rot.mat.b;

      // like val.z in the full version
      valz = dot(matc,one);
   }

   // forez, ge, lt
   real forez(const point<real> &p) const { return dot(matc,p) - valz; }

   bool ge(const point<real> &p) const { return dot(matc,p) >= valz; }
   bool lt(const point<real> &p) const { return dot(matc,p) <  valz; }

   bool lt(const point<real> &p, real &rv) const
   {
      return (rv = dot(matc,p)) < valz;
   }
};



// -----------------------------------------------------------------------------
// rotate<3,real,full,scaled>
// -----------------------------------------------------------------------------

/*
Translation, rotation, and scaling, as follows:

   fore
      Translate "one" to the origin
      Rotate "two" to the +x axis
      Rotate "three" to the xy plane, with y >= 0
      Scale by factor

   back
      Do the reverse of fore

Rotation matrix for fore:
   mat.a.x   mat.a.y   mat.a.z
   mat.b.x   mat.b.y   mat.b.z
   mat.c.x   mat.c.y   mat.c.z
*/

template<class real>
class rotate<3,real,op::full,op::scaled> {
   mmm<real> mat;
   point<real> shift;
public:
   real ex, ey;
   real h;

   // rotate()
   explicit rotate() { }

   // rotate(one,two,three,factor)
   explicit rotate(
      const point<real> &one,
      const point<real> &two,
      const point<real> &three,
      const real factor
   ) :
      shift(one)
   {
      const rotate<2,real,op::full,op::unscaled> rot(one,two);
      h = factor*rot.h;

      const point<real> t = rot.fore(three);
      ex = factor*t.x;
      ey = std::sqrt(t.y*t.y + t.z*t.z);

      const real cosa = ey == 0 ? factor : (factor/ey)*t.y;
      const real sina = ey == 0 ? 0      : (factor/ey)*t.z;

      // in contrast to what we saw in the op::unscaled case,
      // this seems to be faster than the written-out version
      const mmm<real> rotx(
         point<real>( factor, 0,    0    ),
         point<real>( 0,      cosa, sina ),
         point<real>( 0,     -sina, cosa )
      );
      mat = rotx*rot.mat;
      ey *= factor;
   }

   // fore(p)
   point<real> fore(const point<real> &p) const
   {
      const point<real> t = p - shift;
      return point<real>(dot(mat.a,t), dot(mat.b,t), dot(mat.c,t));
   }

   // back(p)
   point<real> back(const point<real> &p) const
      { return shift + p.x*mat.a + p.y*mat.b + p.z*mat.c; }
};
