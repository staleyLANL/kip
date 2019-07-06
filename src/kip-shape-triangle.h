
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// triangle
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class triangle : public shape<real,tag> {
   using T = real;

   // transformation
   mutable affine<real> aff;

public:

   // misc
   kip_functions(triangle);

   // vertices
   point<real> u, v, w;

   // ghi()
   inline point<real> &ghi(void) const
      { return *(point<real> *)(void *)&this->shape<real,tag>::tridata.ghi[0]; }

   // back
   inline point<real> back(const point<real> &from) const { return from; }



   // triangle([u[,v[,w[,tag]]]])
   inline explicit triangle(
      const point<real> &_u = point<real>(real(1), real(0), real(0)),
      const point<real> &_v = point<real>(real(0), real(1), real(0)),
      const point<real> &_w = point<real>(real(0), real(0), real(1))
   ) :
      shape<real,tag>(this),
      u(_u), v(_v), w(_w)
   { this->eyelie = false; }

   inline explicit triangle(
      const point<real> &_u,
      const point<real> &_v,
      const point<real> &_w, const tag &_tag
   ) :
      shape<real,tag>(this,_tag),
      u(_u), v(_v), w(_w)
   { this->eyelie = false; }


   // triangle(ux,uy,uz[,vx,vy,vz[,wx,wy,wz[,tag]]])
   inline explicit triangle(
      const real &ux, const real &uy, const real &uz
   ) :
      shape<real,tag>(this),
      u(ux, uy, uz),
      v(real(0), real(1), real(0)),
      w(real(0), real(0), real(1))
   { this->eyelie = false; }

   inline explicit triangle(
      const real &ux, const real &uy, const real &uz,
      const real &vx, const real &vy, const real &vz
   ) :
      shape<real,tag>(this),
      u(ux, uy, uz),
      v(vx, vy, vz),
      w(real(0), real(0), real(1))
   { this->eyelie = false; }

   inline explicit triangle(
      const real &ux, const real &uy, const real &uz,
      const real &vx, const real &vy, const real &vz,
      const real &wx, const real &wy, const real &wz
   ) :
      shape<real,tag>(this),
      u(ux, uy, uz),
      v(vx, vy, vz),
      w(wx, wy, wz)
   { this->eyelie = false; }

   inline explicit triangle(
      const real &ux, const real &uy, const real &uz,
      const real &vx, const real &vy, const real &vz,
      const real &wx, const real &wy, const real &wz, const tag &_tag
   ) :
      shape<real,tag>(this,_tag),
      u(ux, uy, uz),
      v(vx, vy, vz),
      w(wx, wy, wz)
   { this->eyelie = false; }


   // triangle(triangle)
   inline triangle(const triangle &from) :
      shape<real,tag>(from),
      u(from.u),
      v(from.v),
      w(from.w)
   { this->eyelie = false; }


   // triangle=triangle
   inline triangle &operator=(const triangle &from)
   {
      this->shape<real,tag>::operator=(from);
      u = from.u;
      v = from.v;
      w = from.w;
      return *this;
   }
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(triangle)
{
   // aff, degenerate ( = non-singular, and eyeball not in tri's plane)
   if ((this->degenerate = !aff(u,v,w, eyeball, ghi())))
      return 0;

   // local rot, eye
   const rotate<3,real> rot(u,v,w);
   const point<real> &eye = rot.fore(eyeball);

   // Translated/rotated eyeball (ex,ey,ez), and triangle
   // points (0,0,0), (bx,0,0), (cx,cy,0).
   const real ex = eye.x, ey = eye.y,  ez = eye.z;
   const real bx = rot.h, cx = rot.ex, cy = rot.ey;

   // interior
   this->interior = false;


   // ----------------
   // minimum
   // ----------------

   // straight out=down from (u=0 to v)?
   if (ey <= 0 && 0 <= ex && ex <= bx)
      return op::sqrt(ey*ey + ez*ez);

   // straight out from (v to w)?
   const real bxmex = bx-ex, bxmcx = bx-cx;
   const real exmcx = ex-cx, eymcy = ey-cy, eytcy = ey*cy;
   const bool b = bxmcx*ey >= cy*bxmex;
   if (b && eytcy >= -bxmcx*bxmex && cy*eymcy <= bxmcx*exmcx)
      return pline(eye, point<real>(bx,0,0), point<real>(cx,cy,0));

   // straight out from (w to u)?
   const real extcy = ex*cy;
   const real eytcx = ey*cx;
   if (eytcx >= extcy && cx*ex >= -eytcy && -cy*eymcy >= cx*exmcx)
      return pline(eye, point<real>(cx,cy,0), point<real>(0,0,0));

   // relative to points?
   if (b || ey < 0 || extcy < eytcx)
      return op::sqrt(op::min(
         mod2(eye),
         mod2(point<real>(bxmex, ey,    ez)),
         mod2(point<real>(exmcx, eymcy, ez))
      ));

   // above/below
   return op::abs(ez);
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(triangle)
{
   return kip::bbox<real>(
      true, op::min(u.x, v.x, w.x),   op::max(u.x, v.x, w.x), true,
      true, op::min(u.y, v.y, w.y),   op::max(u.y, v.y, w.y), true,
      true, op::min(u.z, v.z, w.z),   op::max(u.z, v.z, w.z), true
   );
} kip_end



// dry
kip_dry(triangle)
{
   return this->degenerate || (seg.ge(u) && seg.ge(v) && seg.ge(w));
} kip_end



// infirst
kip_infirst(triangle)
{
   real dx, dy, dz;

   return
      0 >= (dx = aff.forex(diff)) &&
      0 >= (dy = aff.forey(diff)) &&
      0 <  (dz = aff.forez(diff,ghi())) &&
      0 <=  dx + dy + dz && aff.den < dz*qmin && (

      q.point<real>::operator=(eyeball - real(q = aff.den/dz)*diff),
      q(ghi(), this, yesnorm), true);
} kip_end



// inall
kip_inall(triangle)
{
   return triangle<real,tag>::infirst(kip_etd,qmin,ints.one(),insub)
      ? ints.push(ints[0]), true
      : false;
} kip_end



// check
kip_check(triangle)
{
   static const char *const badvert = "Triangle has coincident vertices ";
   using ostr_t = std::ostringstream;
   diagnostic_t rv = diagnostic_good;

   if (u == v) { ostr_t oss;  oss << badvert << "u=v=" << u;  rv = error(oss); }
   if (u == w) { ostr_t oss;  oss << badvert << "u=w=" << u;  rv = error(oss); }
   if (v == w) { ostr_t oss;  oss << badvert << "v=w=" << v;  rv = error(oss); }

   return rv;
} kip_end



// random
kip_random(triangle)
{
   // u
   obj.u.x = op::twice(random<real>() - real(0.5));
   obj.u.y = op::twice(random<real>() - real(0.5));
   obj.u.z = op::twice(random<real>() - real(0.5));

   // v
   obj.v.x = obj.u.x + real(0.3)*(random<real>() - real(0.5));
   obj.v.y = obj.u.y + real(0.3)*(random<real>() - real(0.5));
   obj.v.z = obj.u.z + real(0.3)*(random<real>() - real(0.5));

   // w
   obj.w.x = obj.u.x + real(0.3)*(random<real>() - real(0.5));
   obj.w.y = obj.u.y + real(0.3)*(random<real>() - real(0.5));
   obj.w.z = obj.u.z + real(0.3)*(random<real>() - real(0.5));

   // tag
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(triangle) {

   // (
   //    ux, uy, uz,
   //    vx, vy, vz,
   //    wx, wy, wz,
   //    tag
   // )

   s.bail = false;
   if (!(
      read_left(s) &&
      read_value(s, obj.u, "u vertex") && read_comma(s) &&
      read_value(s, obj.v, "v vertex") && read_comma(s) &&
      read_value(s, obj.w, "w vertex") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(triangle) {
   bool okay;

   // stub
   if (kip::format == kip::format_stub)
      okay = k << "triangle()";

   // one
   // op
   else if (kip::format == kip::format_one ||
            kip::format == kip::format_op)
      okay = k << "triangle("
               << obj.u << ", "
               << obj.v << ", "
               << obj.w &&
             kip::write_finish(k, obj, true);

   // full
   else
      okay = k << "triangle(\n   " &&
             k.indent() << obj.u << ",\n   " &&
             k.indent() << obj.v << ",\n   " &&
             k.indent() << obj.w &&
             kip::write_finish(k, obj, false);

   kip_ostream_end(triangle);
}

}

#define   kip_class triangle
#include "kip-macro-io.h"
