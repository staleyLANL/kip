
// -----------------------------------------------------------------------------
// affine
// -----------------------------------------------------------------------------

template<class real>
class affine {
   point<real> abc, def;
public:
   real den;

   // operator()(u,v,w, e,ghi)
   // Returns true iff non-degenerate and e not in uvw plane
   bool operator()(
      const point<real> &u, const point<real> &v, const point<real> &w,
      const point<real> &e, point<real> &ghi
   ) {
      const point<real> s = e - u;
      const point<real> r = w - u;
      const point<real> q = v - u;

      ghi = cross(r,q);
      real fac = mod(ghi);
      if (fac == 0) return false;  // triangle (u,v,w) is degenerate

      abc = cross(s,r);
      den = dot(q,abc);
      if (den == 0) return false;  // eyeball is in (u,v,w) plane

      // den
      fac = (den < 0 ? real(-1) : real(1))/fac;  // +-1/mod(ghi)
      den *= fac;

      // abc, def, ghi
      abc *= fac;             // ( a b c ) ~ cross(e-u, w-u), or cross(s,r)
      def  = fac*cross(q,s);  // ( d e f ) ~ cross(v-u, e-u), or cross(q,s)
      ghi *= fac;             // ( g h i ) ~ cross(w-u, v-u), or cross(r,q)

      return true;
   }

   // forex, forey, forez
   real forex(const point<real> &p) const { return dot(abc,p); }
   real forey(const point<real> &p) const { return dot(def,p); }
   real forez(const point<real> &p, const point<real> &ghi) const
      { return dot(ghi,p); }
};



// -----------------------------------------------------------------------------
// tri
// -----------------------------------------------------------------------------

template<class real = default_real, class tag = default_base>
class tri : public shape<real,tag> {

   // transformation
   mutable affine<real> aff;

public:
   using shape<real,tag>::degenerate;
   kip_functions(tri);

   // u(), v(), w(): retrieve vertices
   using shape<real,tag>::vertex;
   ulong &u() const { return vertex.u; }
   ulong &v() const { return vertex.v; }
   ulong &w() const { return vertex.w; }

   // ghi()
   point<real> &ghi() const
      { return *(point<real> *)(void *)&vertex.ghi[0]; }


   // ------------------------
   // Constructors, Assignment
   // ------------------------

   // tri()
   explicit tri() :
      shape<real,tag>(this)
   {  this->eyelie = false; }

   // tri(u,v,w)
   explicit tri(
      const ulong &a, const ulong &b, const ulong &c
   ) :
      shape<real,tag>(this)
   {
      u() = a;
      v() = b;
      w() = c;
      this->eyelie = false;
   }

   // tri(tri)
   tri(const tri &from) :
      shape<real,tag>(from)
   {
      u() = from.u();
      v() = from.v();
      w() = from.w();
      this->eyelie = false;
   }

   // tri=tri
   tri &operator=(const tri &from)
   {
      this->shape<real,tag>::operator=(from);
      u() = from.u();
      v() = from.v();
      w() = from.w();
      return *this;
   }


   // ------------------------
   // Functions
   // ------------------------

   // back
   point<real> back(const point<real> &from) const { return from; }

   // operator()(u,v,w)
   tri &operator()(
      const ulong &a, const ulong &b, const ulong &c
   ) {
      u() = a;
      v() = b;
      w() = c;
      return *this;
   }

   // process - custom version
   real process(
      const std::vector<point<real>> &, const point<real> &,
      const engine<real> &, const detail::vars<real,tag> &
   ) const;
};



// Shouldn't call these; call enclosing surf's instead
kip_process(tri) { assert(false); return real(0);         } kip_end
kip_dry    (tri) { assert(false); return false;           } kip_end
kip_check  (tri) { assert(false); return diagnostic::good; } kip_end
kip_inall  (tri) { assert(false); return false;           } kip_end

kip_aabb  (tri) {
   assert(false);

   return bbox<real>(
      false,1, 0,false,
      false,1, 0,false,
      false,1, 0,false
   );
} kip_end



// -----------------------------------------------------------------------------
// process - custom version
// infirst
// -----------------------------------------------------------------------------

// process
template<class real, class tag>
inline real tri<real,tag>::process(
   const std::vector<point<real>> &node,
   const point<real> &eyeball,
   const engine<real> &, const detail::vars<real,tag> &
) const {
   // aff, degenerate ( = non-singular, and eyeball not in tri's plane)
   if ((degenerate = !aff(node[u()], node[v()], node[w()], eyeball, ghi())))
      return 0;

   // local rot, eye
   const rotate<3,real,op::full,op::unscaled>
      rot(node[u()], node[v()], node[w()]);
   const point<real> eye = rot.fore(eyeball);

   // minimum   zzz Simplification is probably possible here (and in triangle).
   const real ex = eye.x, bx = rot.h;
   const real ey = eye.y, cx = rot.ex;
   const real ez = eye.z, cy = rot.ey;

   // straight out=down from (u=0 to v)
   if (ey <= 0 && 0 <= ex && ex <= bx)
      return std::sqrt(op::square(ey) + op::square(ez));

   // straight out from (v to w)
   const real bxmex = bx-ex, bxmcx = bx-cx;
   const real exmcx = ex-cx, eymcy = ey-cy, eytcy = ey*cy;
   const bool b = bxmcx*ey >= cy*bxmex;
   if (eytcy >= -bxmcx*bxmex && cy*eymcy <= bxmcx*exmcx && b)
      return pline(eye, point<real>(bx,0,0), point<real>(cx,cy,0));

   // straight out from (w to u)
   const real extcy = ex*cy;
   const real eytcx = ey*cx;
   if (eytcx >= extcy && cx*ex >= -eytcy && -cy*eymcy >= cx*exmcx)
      return pline(eye, point<real>(cx,cy,0), point<real>(0,0,0));

   // relative to points
   if (b || ey < 0 || extcy < eytcx)
      return std::sqrt(op::min(
         mod2(eye),
         mod2(point<real>(bxmex, ey,    ez)),
         mod2(point<real>(exmcx, eymcy, ez))
      ));

   // above/below
   return std::abs(ez);
}



// infirst
kip_infirst(tri)
{
   real dx, dy, dz;

   return
      0 >= (dx = aff.forex(diff)) &&
      0 >= (dy = aff.forey(diff)) &&
      0 <  (dz = aff.forez(diff,ghi())) &&
      0 <=  dx + dy + dz && aff.den < dz*qmin && (

      q.point<real>::operator=(eyeball - real(q = aff.den/dz)*diff),
      q(ghi(), this, normalized::yes), true);
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(tri) {

   // u, v, w

   s.bail = false;
   if (!(
      read_value(s, obj.u()) && read_comma(s) &&
      read_value(s, obj.v()) && read_comma(s) &&
      read_value(s, obj.w())
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   }

   return !s.fail();
}



// kip::ostream
kip_ostream(tri) {
   return k << obj.u() << ',' << obj.v() << ',' << obj.w();
}

#define   kip_class tri
#include "kip-macro-io.h"
