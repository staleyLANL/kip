
#pragma once

// -----------------------------------------------------------------------------
// point
// -----------------------------------------------------------------------------

template<class real>
class point {
public:
   real x, y, z;

   // point()
   explicit point() { }

   // point(x,y,z)
   explicit point(const real a, const real b, const real c) :
      x(a), y(b), z(c)
   { }

   // point(point<X>)
   template<class X>
   explicit point(const point<X> &from) :
      x(real(from.x)), y(real(from.y)), z(real(from.z))
   { }

   // operator()(x,y,z)
   void operator()(const real a, const real b, const real c)
   {
      x = a, y = b, z = c;
   }

   // operator()(offset, d,theta,phi)
   // Some testing suggests that other ways of expressing
   // the formulas give no discernible speed improvements.
   void operator()(
      const point<real> &offset, const real d, const real theta, const real phi
   ) {
      const real t = theta*(pi<real>*(real(1)/180));
      const real p = phi  *(pi<real>*(real(1)/180)), dcosp = d*std::cos(p);

      x = offset.x + std::cos(t)*dcosp;
      y = offset.y - std::sin(t)*dcosp;
      z = offset.z + std::sin(p)*d;
   }
};



// -----------------------------------------------------------------------------
// functions
// -----------------------------------------------------------------------------

// random_full
template<class real>
inline point<real> &random_full(point<real> &u)
{
   u(random_full<real>(), random_full<real>(), random_full<real>());
   return u;
}

// operator==
template<class real>
inline bool operator==(const point<real> &u, const point<real> &v)
{
   return u.x == v.x && u.y == v.y && u.z == v.z;
}



// u + v
template<class real>
inline point<real> operator+(const point<real> &u, const point<real> &v)
{
   return point<real>(u.x+v.x, u.y+v.y, u.z+v.z);
}

// u - v
template<class real>
inline point<real> operator-(const point<real> &u, const point<real> &v)
{
   return point<real>(u.x-v.x, u.y-v.y, u.z-v.z);
}

// c * u
template<class real>
inline point<real> operator*(const real c, const point<real> &u)
{
   return point<real>(c*u.x, c*u.y, c*u.z);
}

// u *= c
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
inline real dot(const point<real> &u, const point<real> &v)
{
   return u.x*v.x + u.y*v.y + u.z*v.z;
}

// cross(u,v)
template<class real>
inline point<real> cross(const point<real> &u, const point<real> &v)
{
   return point<real>(
      u.y*v.z - u.z*v.y,
      u.z*v.x - u.x*v.z,
      u.x*v.y - u.y*v.x
   );
}



// mod2(u): "mod squared"
template<class real>
inline real mod2(const point<real> &u)
{
   return dot(u,u);
}

// mod(u)
template<class real>
inline real mod(const point<real> &u)
{
   return std::sqrt(mod2(u));
}

// normalize(u)
template<class real>
inline point<real> normalize(const point<real> &u)
{
   return (real(1)/mod(u))*u;
}

// pline(p,u,v)
//
// Distance between the point p and the line containing the points u and v.
// Must have u != v, which is NOT verified here.
//
// Some testing suggests that the following:
//    std::sqrt(mod2(cross(v-u,u-p))/mod2(v-u)) // equivalent
//    std::sqrt(mod2(cross(p-u,p-v))*(1/mod2(v-u))) // w/reciprocal
// were slower or no better, even precomputing v-u in the first one.

template<class real>
inline real pline(
   const point<real> &p,
   const point<real> &u, const point<real> &v
) {
   return std::sqrt(mod2(cross(p-u,p-v))/mod2(v-u));
}
