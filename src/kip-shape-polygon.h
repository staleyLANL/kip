
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// polygon
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class polygon : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   typedef std::vector< point<real> > table_t;

private:
   // modified polygon: (0,0,0), (h,0,0), (ex,ey,0), ... (all with z == 0)
   mutable rotate<3,real> rot;
   mutable array_simple< point_xy<real> > poly;  // rotated to 2d
   mutable real xlo, xhi, ylo, yhi;  // 2d bounding box
   mutable size_t npts;

public:
   using shape<real,tag>::basic;
   kip_functions(polygon);

   // vertices
   table_t table;
   inline size_t size(void) const { return table.size(); }

   inline point<real> back(const point<real> &from) const { return rot.back(from); }


   // polygon([base])
   inline explicit polygon(void) :
      shape<real,tag>(this) { }

   inline explicit polygon(const tag &thebase) :
      shape<real,tag>(this,thebase) { }

   // polygon((x,y,z)[,base])
   inline explicit polygon(const table_t &_table) :
      shape<real,tag>(this), table(_table) { }

   inline explicit polygon(const table_t &_table, const tag &thebase) :
      shape<real,tag>(this,thebase), table(_table) { }

   // polygon(polygon)
   inline polygon(const polygon &from) :
      shape<real,tag>(from), table(from.table) { }

   // polygon = polygon
   inline polygon &operator=(const polygon &from)
   {
      this->shape<real,tag>::operator=(from);
      table = from.table;
      return *this;
   }


   // push
   inline point<real> &push(const point<real> &xyz)
      { return table.push_back(xyz), table.back(); }

   inline point<real> &push(const real &x, const real &y, const real &z)
      { return push(point<real>(x,y,z)); }

   // point_in_poly
   inline bool point_in_poly(const real &x, const real &y) const
   {
      if (x < xlo || x > xhi || y < ylo || y > yhi)
         return false;

      point_xy<real> a = poly[npts-1], b;
      bool apos = a.y >= y, inside = false;

      for (size_t j = 0;  j < npts;  a = b, ++j) {
         b = poly[j];
         if (apos != (b.y >= y) &&
            (apos = !apos) == ((b.y-y)*(a.x-b.x) >= (b.x-x)*(a.y-b.y)))
            inside = !inside;
      }
      return inside;
   }
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(polygon)
{
   interior = false;
   if ((npts = size()) < 3) return 0;

   size_t loc = npts;
   for (size_t i = 0;  i < npts;  ++i)
      if (mod2(cross(
          table[(i+1) % npts] - table[i],
          table[(i+2) % npts] - table[i]
      )) > 0)
         { loc = i;  break; }

   if (loc == npts) return 0;  // shouldn't happen if it's a genuine polygon

   // rot, eye, lie
   rot = rotate<3,real>(
      table[loc],
      table[(loc+1) % npts],
      table[(loc+2) % npts]
   );
   basic.eye() = rot.fore(eyeball);
   basic.lie() = point<float>(rot.fore(light));

   // xy projection of points
   poly.resize(npts);

   poly[0] = point_xy<real>(0,0);
   poly[1] = point_xy<real>(rot.h,0);
   poly[2] = point_xy<real>(rot.ex,rot.ey);

   xlo = op::min(real(0), rot.ex);  ylo = 0;
   xhi = op::max(rot.h,rot.ex);  yhi = rot.ey;

   for (size_t i = 3;  i < npts;  ++i) {
      const real x = rot.forex(table[i]);
      const real y = rot.forey(table[i]);
      poly[i] = point_xy<real>(x,y);
      if (x < xlo) xlo = x; else if (x > xhi) xhi = x;
      if (y < ylo) ylo = y; else if (y > yhi) yhi = y;
   }

   // minimum
   if (point_in_poly(basic.eye().x, basic.eye().y))
      return op::abs(basic.eye().z);

   real minimum = std::numeric_limits<real>::max();
   point_xy<real> a = poly[npts-1], b;

   for (size_t i = 0;  i < npts;  a = b, ++i) {
      b = poly[i];
      const real a1 = basic.eye().x - a.x, a2 = b.x - a.x;
      const real b1 = basic.eye().y - a.y, b2 = b.y - a.y, den = a2*a2 + b2*b2;
      const real c1 = basic.eye().z*basic.eye().z, q = (a1*a2 + b1*b2)/den;

      if (0 < q && q < 1)
         minimum = op::min(minimum, op::sq(b2*a1 - a2*b1)/den + c1);
      minimum = op::min(minimum, a1*a1 + b1*b1 + c1);
   }

   return op::sqrt(minimum);
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(polygon)
{
   if ((npts = size()) == 0)
      return bbox<real>(  // make out-of-order (thus !valid()) in each dimension
         false,1, 0,false,
         false,1, 0,false,
         false,1, 0,false
      );

   real xmin = table[0].x, xmax = xmin;
   real ymin = table[0].y, ymax = ymin;
   real zmin = table[0].z, zmax = zmin;

   for (size_t i = 1;  i < npts;  ++i) {
      xmin = op::min(xmin, table[i].x),   xmax = op::max(xmax, table[i].x);
      ymin = op::min(ymin, table[i].y),   ymax = op::max(ymax, table[i].y);
      zmin = op::min(zmin, table[i].z),   zmax = op::max(zmax, table[i].z);
   }

   return bbox<real>(
      true,xmin, xmax,true,
      true,ymin, ymax,true,
      true,zmin, zmax,true
   );
} kip_end



// dry
kip_dry(polygon)
{
   for (size_t i = 0;  i < npts;  ++i)
      if (seg.lt(table[i]))
         return false;  // at least one point below
   return true;  // all points above
} kip_end



// infirst
kip_infirst(polygon)
{
   if (npts < 3) return false;

   // transform; compute (x,y) of intersection with polygon's plane
   const point<real> tar = rot.fore(target);

   if (basic.eye().z - tar.z == 0) return false;
   q = basic.eye().z/(basic.eye().z - tar.z);
   if (!(0 < q && q < qmin)) return false;

   q.x = basic.eye().x + q*(tar.x - basic.eye().x);
   if (q.x < xlo || q.x > xhi) return false;
   q.y = basic.eye().y + q*(tar.y - basic.eye().y);

   // done
   return point_in_poly(q.x,q.y)
      ? q.z=0, q(0,0,basic.eye().z > 0 ? real(1) : real(-1),this,yesnorm),true
      : false;
} kip_end



// inall
kip_inall(polygon)
{
   return polygon<real,tag>::infirst(kip_etd,qmin,ints.one(),insub)
      ? ints.push(ints[0]), true
      : false;
} kip_end



// check
kip_check(polygon)
{
   diagnostic_t rv = diagnostic_good;
   npts = size();

   // Require that no two successive points be equal
   for (size_t i = 0;  i < npts;  ++i)
      if (table[i] == table[(i+1) % npts]) {
         std::ostringstream oss;
         oss << "polygon has successive coincident vertices [" << i
             << "]=[" << (i+1) % npts << "]=" << table[i];
         rv = error(oss);
      }

   return rv;
} kip_end



// random
kip_random(polygon)
{
   static const size_t max = 16;  // #points = #edges = max
   const size_t npts = 3 + size_t((max-2)*random<real>());

   // rotation to move vertices to their actual locations
   const rotate<3,real> move(
      real(kip_pi)*random<real>(),
      real(kip_pi)*random<real>(),
      real(kip_pi)*random<real>(),
      point<real>(
         op::twice(random<real>() - real(0.5)),
         op::twice(random<real>() - real(0.5)),
         op::twice(random<real>() - real(0.5))
      )
   );

   // table
   obj.table.clear();
   obj.table.reserve(npts);

   for (size_t i = 0;  i < npts;  ++i)
      obj.table.push_back(move.back(
         point<real>(real(0.3)*random<real>(), real(0.3)*random<real>(), 0)
      ));

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
kip_read_value(polygon) {

   // (
   //    size,
   //    x0, y0, z0,
   //    x1, y1, z1,
   //    x2, y2, z2,
   //    ...
   //    base
   // )

   obj.table.clear();
   s.bail = false;
   size_t size = 0;

   bool okay =
      read_left(s) &&
      read_value(s,size);  // don't necessarily read comma; size==0 is possible

   if (okay && size > 0) {
      obj.table.reserve(size);
      point<real> p;

      for (size_t i = 0;  i < size && okay;  ++i)
         if ((okay =
              read_comma(s) && read_value(s, p.x) &&
              read_comma(s) && read_value(s, p.y) &&
              read_comma(s) && read_value(s, p.z)))
            obj.table.push_back(p);
   }

   if (!(okay && read_done(s, obj))) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_error);
   }
   return !s.fail();
}



// kip::ostream
kip_ostream(polygon) {
   bool okay;
   const size_t npts = obj.size();

   // stub
   if (kip::format == kip::format_stub)
      okay = k << "polygon()";

   // one
   // op
   else if (kip::format == kip::format_one ||
            kip::format == kip::format_op) {
      okay = k << "polygon(" << npts;
      for (size_t i = 0;  i < npts && okay;  ++i)
         okay = k << ", " << obj.table[i];
      okay = okay && kip::write_finish(k, obj, true);
   }

   // full
   else {
      okay = k << "polygon(\n   " && k.indent() << npts;
      for (size_t i = 0;  i < npts && okay;  ++i)
         okay = k << ",\n   " && k.indent() << obj.table[i];
      okay = okay && kip::write_finish(k, obj, false);
   }

   kip_ostream_end(polygon);
}

}

#define   kip_class polygon
#include "kip-macro-io.h"
