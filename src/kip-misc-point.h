
#pragma once

// This file defines the point class template and related functions.

// -----------------------------------------------------------------------------
// point
// -----------------------------------------------------------------------------

template<class real = default_real_t>
class point {
public:
   real x, y, z;

   // point()
   inline explicit point()
   { }

   // point(x,y,z)
   inline explicit point(const real _x, const real _y, const real _z) :
      x(_x), y(_y), z(_z)
   { }

   // point(point<TFROM>)
   template<class TFROM>
   inline explicit point(const point<TFROM> &from) :
      x(real(from.x)), y(real(from.y)), z(real(from.z))
   { }


   // operator()(x,y,z)
   inline point &operator()(const real _x, const real _y, const real _z)
   {
      x = _x;
      y = _y;
      z = _z;
      return *this;
   }

   // operator()(offset, d,theta,phi)
   inline point &operator()(
      const point<real> &offset, const real d, const real theta, const real phi
   ) {
      const real p = phi  *(real(kip_pi)/180), cosp = op::cos(p);
      const real t = theta*(real(kip_pi)/180);

      x = offset.x + op::cos(t)*d*cosp;
      y = offset.y - op::sin(t)*d*cosp;
      z = offset.z + op::sin(p)*d;

      return *this;
   }
};



// -----------------------------------------------------------------------------
// point: functions
// -----------------------------------------------------------------------------

// operator==
template<class real>
inline bool operator==(const point<real> &a, const point<real> &b)
{
   return a.x == b.x && a.y == b.y && a.z == b.z;
}

// operator!=
template<class real>
inline bool operator!=(const point<real> &a, const point<real> &b)
{
   return !(a == b);
}



// real*point
template<class real>
inline point<real> operator*(const real c, const point<real> &u)
{
   return point<real>(c*u.x, c*u.y, c*u.z);
}

// point *= real
template<class real>
inline point<real> &operator*=(point<real> &u, const real c)
{
   u.x *= c;
   u.y *= c;
   u.z *= c;
   return u;
}



// dot(u,v)
template<class real>
inline real dot(
   const point<real> &u,
   const point<real> &v
) {
   return u.x*v.x + u.y*v.y + u.z*v.z;
}

// cross(u,v)
template<class real>
inline point<real> cross(
   const point<real> &u,
   const point<real> &v
) {
   return point<real>(
      u.y*v.z - u.z*v.y,
      u.z*v.x - u.x*v.z,
      u.x*v.y - u.y*v.x
   );
}



// -u
template<class real>
inline point<real> operator-(
   const point<real> &u
) {
   return point<real>(-u.x, -u.y, -u.z);
}

// u-v
template<class real>
inline point<real> operator-(
   const point<real> &u,
   const point<real> &v
) {
   return point<real>(u.x-v.x, u.y-v.y, u.z-v.z);
}

// u+v
template<class real>
inline point<real> operator+(
   const point<real> &u,
   const point<real> &v
) {
   return point<real>(u.x+v.x, u.y+v.y, u.z+v.z);
}



// mod2(u)   ("mod2" as in "mod squared")
template<class real>
inline real mod2(const point<real> &u)
{
   return dot(u,u);
}

// mod(u)
template<class real>
inline real mod(const point<real> &u)
{
   return op::sqrt(mod2(u));
}



// normalize(u)
template<class real>
inline point<real> normalize(const point<real> &u)
{
   // this tests as somewhat faster than u/mod(u)...
   const real fac = real(1)/mod(u);
   return fac*u;
}



// pdistance2(u,v): distance squared between the points u and v
template<class real>
inline real pdistance2(
   const point<real> &u,
   const point<real> &v
) {
   return mod2(u-v);
}

// pdistance(u,v): distance between the points u and v
template<class real>
inline real pdistance(
   const point<real> &u,
   const point<real> &v
) {
   return op::sqrt(pdistance2(u,v));
}



// pline2(p,u,v)
template<class real>
inline real pline2(
   const point<real> &p,
   const point<real> &u, const point<real> &v
) {
   return mod2(cross(v-u,u-p)) / mod2(v-u);
}

// pline(p,u,v): distance between the point p and the line containing
// the points u and v. Must have u != v, which is NOT verified here.
template<class real>
inline real pline(
   const point<real> &p,
   const point<real> &u, const point<real> &v
) {
   return op::sqrt(pline2(p,u,v));
}



// skewd2(x1,x2, x3,x4): square of the distance between the (skew) lines:
//    x1 to x2
//    x3 to x4
// Works correctly for parallel lines.

template<class real>
inline real skewd2(
   const point<real> &x1, const point<real> &x2,
   const point<real> &x3, const point<real> &x4
) {
   const point<real> a = x2 - x1;
   const point<real> b = x4 - x3, c = cross(a,b);

/*
zzz think about these...

pline2(x1, x3,x4)
   p = x1
   u = x3
   v = x4

   mod2(cross(x4-x3,x3-x1)) / mod2(x4-x3);
   mod2(cross(b,x3-x1)) / mod2(b);

OR, could use:

pline2(x3, x1,x2)
   p = x3
   u = x1
   v = x2

   mod2(cross(x2-x1,x1-x3)) / mod2(x2-x1);
   mod2(cross(a,x1-x3)) / mod2(a);
   mod2(cross(x3-x1,a)) / mod2(a);
*/

   const real den = mod2(c);
   return den == 0 ? pline2(x1, x3,x4) : op::sq(dot(x3-x1, c)) / den;
}
