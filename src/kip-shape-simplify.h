
#pragma once

// This file provides simplify() functions for each shape.

// kip_simplify
//    (1) pointer (calls (2))
//    (2) reference
#define kip_simplify(type)\
   template<class real, class base>\
   inline\
   std::pair<const type <real,base> *, const shape<real,base> *>\
   simplify(const type<real,base> *const ptr)\
      { return simplify(*ptr); }\
   \
   template<class real, class base>\
   std::pair<const type <real,base> *, const shape<real,base> *>\
   simplify(const type<real,base> &obj)

/*
(    ) tabular
(    ) surf

( xx ) kipnot

( xx ) kipand
( xx ) kipcut
( xx ) kipor
( xx ) kipxor

(    ) ands
(    ) odd
(    ) even
(    ) some
(    ) one
(    ) ors
*/



// -----------------------------------------------------------------------------
// Generic-pointer simplify()
// -----------------------------------------------------------------------------

template<class real, class base>
std::pair<
   const shape<real,base> *,
   const shape<real,base> *
> simplify(const shape<real,base> *const ptr)
{
   using pair = std::pair<const shape<real,base> *, const shape<real,base> *>;
   const pair nn = pair(NULL,NULL);

   if (ptr == NULL)
      return nn;

#define kip_make_simplify(type)\
   if (ptr->id() == internal::get_shape_id<type>::result)\
      return simplify((const type<real,base> *)ptr)

   kip_expand_semi(kip_make_simplify)
#undef kip_make_simplify

   assert(false);  // shouldn't get here, or else id() didn't check out
   return nn;
}



// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// pNULL: (&obj, NULL)   "invalid"
// psame: (&obj, &obj)   "same - object didn't simplify"
// pdiff: (&obj,  new)   "simplified to 'new' (delete eventually)"
namespace internal {
   template<template<class,class> class type, class real, class base>
   inline std::pair<const type<real,base> *, const shape<real,base> *>
   pNULL(const type<real,base> &obj)
      { return std::make_pair(&obj, (const shape<real,base> *)NULL); }

   template<template<class,class> class type, class real, class base>
   inline std::pair<const type<real,base> *, const shape<real,base> *>
   psame(const type<real,base> &obj)
      { return std::make_pair(&obj, (const shape<real,base> *)&obj); }

   template<template<class,class> class type, class real, class base>
   inline std::pair<const type<real,base> *, const shape<real,base> *>
   pdiff(const type<real,base> &obj, const shape<real,base> *const ptr)
      { return std::make_pair(&obj, ptr); }
}



// make
template<
   template<class,class> class old_t,
   template<class,class> class one_t, class real, class base
>
inline std::pair<const old_t<real,base> *, const shape<real,base> *> make(
   const old_t<real,base> &old,
   const one_t<real,base> *const one
) {
   const std::pair<
      const one_t<real,base> *,
      const shape<real,base> *
   > two = simplify(one);

   assert(one == two.first);

   // one, NULL   (obj, NULL), and delete "one"
   // one, one    (obj, one )
   // one, two    (obj, two ), and delete "one"

   // if further improved, then delete original improvement
   if (two.second != one)
      delete one;

   // done
   return internal::pdiff(old,two.second);
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// PRIMITIVES
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// bicylinder ( a.x,a.y,a.z, b.x,b.y,b.z, r.a,r.b )
// -----------------------------------------------------------------------------

kip_simplify(bicylinder)
{
   const point<real> &a=obj.a, &b=obj.b;
   const real ra=obj.r.a, rb=obj.r.b;

   // a == b: nothing
   // Could have circle, but orientation is indeterminate
   if (a == b)
      return internal::pNULL(obj);

   // r.a == 0 and r.b > 0: change to cone
   if (ra == 0 && rb > 0)
      return make(obj, new cone<real,base>(a,b, rb, obj.base()));

   // r.a > 0 and r.b == 0: change to cone
   if (ra > 0 && rb == 0)
      return make(obj, new cone<real,base>(b,a, ra, obj.base()));

   // r.a <= 0 or r.b <= 0: nothing
   if (ra <= 0 || rb <= 0)
      return internal::pNULL(obj);

   // r.a == r.b: change to cylinder
   if (ra == rb)
      return make(obj, new cylinder<real,base>(a,b, ra, obj.base()));

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// biwasher ( a.x,a.y,a.z, b.x,b.y,b.z, i.a,i.b, o.a,o.b )
// -----------------------------------------------------------------------------

kip_simplify(biwasher)
{
   const point<real> &a=obj.a, &b=obj.b;
   const real ia=obj.i.a, ib=obj.i.b, oa=obj.o.a, ob=obj.o.b;

   // a == b: nothing
   // inners: if either is < 0: nothing
   // outers: if either is < 0, or both are == 0: nothing
   // inners: if both are == outer, or either is > outer: nothing
   if (a == b || ia <  0 || ib <  0 || oa <  0 || ob <  0 ||
      (oa == 0 && ob == 0) || (ia == oa && ib == ob) || ia >  oa || ib >  ob)
      return internal::pNULL(obj);

   // both i == 0: bicylinder
   if (ia == 0 && ib == 0)
      return make(obj, new bicylinder<real,base>(a,b, oa,ob, obj.base()));

   // same i and same o at both ends: washer
   if (ia == ib && oa == ob)
      return make(obj, new washer<real,base>(a,b, ia,oa, obj.base()));

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// box ( c.x,c.y,c.z, a.x,a.y,a.z, r.x,r.y,r.z )
// -----------------------------------------------------------------------------

kip_simplify(box)
{
   const point<real> &r = obj.r;

   // if any radius < 0: nothing
   if (r.x < 0 || r.y < 0 || r.z < 0)
      return internal::pNULL(obj);

   // if two or more radii == 0: nothing
   const unsigned nzero =
      unsigned(r.x == 0) + unsigned(r.y == 0) + unsigned(r.z == 0);
   if (nzero >= 2)
      return internal::pNULL(obj);

   // if one radius == 0: polygon
   if (nzero == 1) {
      using polygon = kip::polygon<real,base>;
      polygon *const s = new polygon(obj.base());  s->table.reserve(4);
      obj.compute_corners();

      s->push(obj.p[0]);
      if (r.x == 0) s->push(obj.p[3]), s->push(obj.p[7]), s->push(obj.p[4]);
      if (r.y == 0) s->push(obj.p[4]), s->push(obj.p[5]), s->push(obj.p[1]);
      if (r.z == 0) s->push(obj.p[1]), s->push(obj.p[2]), s->push(obj.p[3]);

      return make(obj, s);
   }

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// cube ( c.x,c.y,c.z, a.x,a.y,a.z, r )
// -----------------------------------------------------------------------------

kip_simplify(cube)
{
   const real r = obj.r;

   // if r <= 0: nothing
   if (r <= 0)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// circle ( c.x,c.y,c.z, n.x,n.y,n.z, r )
// -----------------------------------------------------------------------------

kip_simplify(circle)
{
   // if n == 0: nothing
   // if r <= 0: nothing
   if (obj.n == point<real>(0,0,0) || obj.r <= 0)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// cone ( a.x,a.y,a.z, b.x,b.y,b.z, r )
// -----------------------------------------------------------------------------

kip_simplify(cone)
{
   // if a == b: nothing (could have circle, but orientation is indeterminate)
   // if r <= 0: nothing
   if (obj.a == obj.b || obj.r <= 0)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// cylinder ( a.x,a.y,a.z, b.x,b.y,b.z, r )
// -----------------------------------------------------------------------------

kip_simplify(cylinder)
{
   // if a == b: nothing (could have circle, but orientation is indeterminate)
   // if r <= 0: nothing
   if (obj.a == obj.b || obj.r <= 0)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// ellipsoid ( c.x,c.y,c.z, a.x,a.y,a.z, r.x,r.y,r.z )
// -----------------------------------------------------------------------------

kip_simplify(ellipsoid)
{
   const point<real> &r=obj.r;

   // if any radius < 0: nothing
   if (r.x < 0 || r.y < 0 || r.z < 0)
      return internal::pNULL(obj);

   // if two or more radii == 0: nothing
   const unsigned nzero =
      unsigned(r.x == 0) + unsigned(r.y == 0) + unsigned(r.z == 0);
   if (nzero >= 2)
      return internal::pNULL(obj);

/*
zzz
Write the following.

Also, bypass spheroid's circle-making inability;
while still in ellipsoid we do know orientation.

Considering note about spheres in spheroid, maybe
go directly to sphere case if all three radii = 0.
*/

   // if all three radii are equal: sphere
   if (r.x == r.y && r.y == r.z) {
   }

   // if two or more radii are equal: spheroid
   if (r.x == r.y) {
   }
   if (r.x == r.z) {
   }
   if (r.y == r.z) {
   }

   // zzz

   // if one radius == 0: ellipse (but we don't have "ellipse" at this time)
   // Note: "ellipse" itself could sometimes reduce to "circle"
   if (nzero == 1)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// everything ( )
// -----------------------------------------------------------------------------

kip_simplify(everything)
{
   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// half ( point.x,point.y,point.z, normal.x,normal.y,normal.z )
// -----------------------------------------------------------------------------

kip_simplify(half)
{
   const point<real> &normal = obj.normal;

   // if normal == 0: nothing
   if (normal == point<real>(0,0,0))
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// nothing ( )
// -----------------------------------------------------------------------------

kip_simplify(nothing)
{
   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// paraboloid ( a.x,a.y,a.z, b.x,b.y,b.z, r )
// -----------------------------------------------------------------------------

kip_simplify(paraboloid)
{
   const point<real> &a=obj.a, &b=obj.b;
   const real r=obj.r;

   // if a == b: nothing
   // Could have circle, but orientation is indeterminate
   if (a == b)
      return internal::pNULL(obj);

   // if r <= 0: nothing
   if (r <= 0)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// pill ( a.x,a.y,a.z, b.x,b.y,b.z, r )
// -----------------------------------------------------------------------------

kip_simplify(pill)
{
   // if a == b: change to sphere
   if (obj.a == obj.b)
      return make(obj, new sphere<real,base>(obj.a, obj.r, obj.base()));

   // if r <= 0: nothing
   if (obj.r <= 0)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// xplane ( x, size.y,size.z )
// -----------------------------------------------------------------------------

kip_simplify(xplane)
{
   // if size.y <= 0: nothing
   if (obj.size.y <= 0) return internal::pNULL(obj);

   // if size.z <= 0: nothing
   if (obj.size.z <= 0) return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// yplane ( y, size.x,size.z )
// -----------------------------------------------------------------------------

kip_simplify(yplane)
{
   // if size.x <= 0: nothing
   if (obj.size.x <= 0) return internal::pNULL(obj);

   // if size.z <= 0: nothing
   if (obj.size.z <= 0) return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// zplane ( z, size.x,size.y )
// -----------------------------------------------------------------------------

kip_simplify(zplane)
{
   // if size.x <= 0: nothing
   if (obj.size.x <= 0) return internal::pNULL(obj);

   // if size.y <= 0: nothing
   if (obj.size.y <= 0) return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// silo ( a.x,a.y,a.z, b.x,b.y,b.z, r )
// -----------------------------------------------------------------------------

kip_simplify(silo)
{
   const point<real> &a=obj.a, &b=obj.b;
   const real r=obj.r;

   // if a == b: nothing
   // Could have hemisphere, but orientation is indeterminate
   // Also, we don't have "hemisphere" at this time!
   if (a == b)
      return internal::pNULL(obj);

   // if r <= 0: nothing
   if (r <= 0)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// sphere ( c.x,c.y,c.z, r )
// -----------------------------------------------------------------------------

kip_simplify(sphere)
{
   const real r=obj.r;

   // if r <= 0: nothing
   if (r <= 0)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// spheroid ( a.x,a.y,a.z, b.x,b.y,b.z, r )
// -----------------------------------------------------------------------------

kip_simplify(spheroid)
{
   const point<real> &a=obj.a, &b=obj.b;
   const real r=obj.r;

   // if a == b: nothing
   // Could have circle, but orientation is indeterminate
   if (a == b)
      return internal::pNULL(obj);

   // if r <= 0: nothing
   if (r <= 0)
      return internal::pNULL(obj);

   // zzz This is another reason to have a close() function,
   // zzz or perhaps to change (a,b) to (a, theta,phi,h) for many objects.
   // if |b-a| == 2*r (== diameter), change to sphere
   if (mod2(b-a) == 4*r*r)
      return make(obj, new sphere<real,base>(0.5*(a+b), r, obj.base()));

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// triangle ( u.x,u.y,u.z, v.x,v.y,v.z, w.x,w.y,w.z )
// -----------------------------------------------------------------------------

kip_simplify(triangle)
{
   const point<real> &u=obj.u, &v=obj.v, &w=obj.w;

   // if u == v: nothing
   // if u == w: nothing
   // if v == w: nothing
   if (u == v || u == w || v == w)
      return internal::pNULL(obj);

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// polygon ( n,
//           table[ 0 ].x, table[ 0 ].y, table[ 0 ].z,
//           ...,
//           table[n-1].x, table[n-1].y, table[n-1].z )
// -----------------------------------------------------------------------------

kip_simplify(polygon)
{
   // create temporary...
   polygon<real,base> *const tmp = new polygon<real,base>(obj);

   // ...remove any successive duplicates
   std::vector<point<real>> &t = tmp->table;
   t.erase(std::unique(t.begin(), t.end(), internal::same<real>()), t.end());

   // ...remove any begin/end duplicate
   unsigned size = t.size();
   if (size >= 2 && t[size-1] == t[0])
      t.pop_back(), size--;

   // if two or fewer points: nothing
   if (size <= 2) {
      delete tmp;
      return internal::pNULL(obj);
   }

   // if polygon was compressed
   if (size != obj.size())
      return internal::pdiff(obj,tmp);

   // verbatim
   delete tmp;
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// washer ( a.x,a.y,a.z, b.x,b.y,b.z, i,o )
// -----------------------------------------------------------------------------

kip_simplify(washer)
{
   const point<real> &a=obj.a, &b=obj.b;
   const real i=obj.i, o=obj.o;

   // if a == b: nothing
   if (a == b) return internal::pNULL(obj);

   // if i >= o: nothing
   if (i >= o) return internal::pNULL(obj);

   // if o <= 0: nothing
   if (o <= 0) return internal::pNULL(obj);

   // if i <  0: nothing
   if (i <  0) return internal::pNULL(obj);

   // if i == 0: change to cylinder
   if (i == 0)
      return make(obj, new cylinder<real,base>(a,b, o, obj.base()));

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// tabular ( a.x,a.y,a.z, b.x,b.y,b.z,
//           n,
//           table[ 0 ].x, table[ 0 ].r,
//           ...,
//           table[n-1].x, table[n-1].r )
// -----------------------------------------------------------------------------

kip_simplify(tabular)
{
   ///const point<real> &a=obj.a, &b=obj.b;

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// surf ( n,
//        node[ 0 ].x, node[ 0 ].y, node[ 0 ].z,
//        ...,
//        node[n-1].x, node[n-1].y, node[n-1].z,
//
//        f,
//        face[ 0 ].u, face[ 0 ].v, face[ 0 ].w,
//        ...,
//        face[n-1].u, face[n-1].v, face[n-1].w )
// -----------------------------------------------------------------------------

kip_simplify(surf)
{
   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// UNARY OPERATORS
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// kipnot ( shape )
// -----------------------------------------------------------------------------

kip_simplify(kipnot)
{
   // simplify argument
   const shape<real,base> *const a = simplify(obj.unary.a).second;

   // not(NULL) = NULL
   if (a == NULL)  // also covers obj.unary.a == NULL
      return internal::pNULL(obj);

   // not(half) = half, with opposite normal
   if (a->id() == id::half) {
      const half<real,base> &arg = *(const half<real,base> *)a;
      const half<real,base> *const ptr =
         new half<real,base>(arg.point, -arg.normal, arg.base());
      if (a != obj.unary.a)
         delete a;
      return internal::pdiff(obj,ptr);
   }

   // not(not(s)) = s
   if (a->id() == id::kipnot) {
      const kipnot<real,base> &arg = *(const kipnot<real,base> *)a;
      const shape<real,base> *const ptr = arg.unary.a->duplicate();
      if (a != obj.unary.a)
         delete a;
      return internal::pdiff(obj,ptr);
   }

   // verbatim
   return internal::psame(obj);
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// BINARY OPERATORS
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// kipand ( shape, shape )
// -----------------------------------------------------------------------------

kip_simplify(kipand)
{
   /*
   and(a, NULL)            NULL
   and(NULL, b)            NULL

   // same/different arguments; will need comparison functionality:
   and(a, a)               a
   and(a,!a)               nothing

   and(not(a), not(b))     not(or(a,b))
   and(a, not(b))          cut(a,b)
   and(not(a), b)          cut(b,a)

   // perhaps perform before earlier block...
   and(a, and(b,c))        ands(a,b,c)
   and(and(a,b), c)        ands(a,b,c)
   and(a, ands(1,2,...))   ands(a,1,2,...)
   and(ands(1,2,...), b)   ands(1,2,...,b)
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// kipcut ( shape, shape )
// -----------------------------------------------------------------------------

kip_simplify(kipcut)
{
   /*
   cut(a, NULL)     a
   cut(NULL, b)     NULL

   // same/different arguments; will need comparison functionality:
   cut(a, a)        nothing
   cut(a,!a)        a

   cut(a, not(b))   and(a,b)
   cut(not(a), b)   not(or(a,b))
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// kipor ( shape, shape )
// -----------------------------------------------------------------------------

kip_simplify(kipor)
{
   /*
   or(a, NULL)           a
   or(NULL, b)           b

   // same/different arguments; will need comparison functionality:
   or(a, a)              a
   or(a,!a)              everything

   or(not(a), not(b))    not(and(a,b))
   or(a, not(b))         not(cut(b,a))
   or(not(a), b)         not(cut(a,b))

   // perhaps perform before earlier block...
   or(a, or(b,c))        ors(a,b,c)
   or(or(a,b), c)        ors(a,b,c)
   or(a, ors(1,2,...))   ors(a,1,2,...)
   or(ors(1,2,...), b)   ors(1,2,...,b)
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// kipxor ( shape, shape )
// -----------------------------------------------------------------------------

kip_simplify(kipxor)
{
   /*
   xor(a, NULL)          a
   xor(NULL, b)          b

   // same/different arguments; will need comparison functionality:
   xor(a, a)             nothing
   xor(a,!a)             everything

   xor(not(a), not(b))   xor(a,b)
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// NARY OPERATORS
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ands ( shape-list )
// all
// -----------------------------------------------------------------------------

kip_simplify(ands)
{
   /*
     ands(a)     a
     ands(a,b)   and(a,b)
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// odd ( shape-list )
// 1, 3, 5, ...
// -----------------------------------------------------------------------------

kip_simplify(odd)
{
   /*
     odd(a)     a
     odd(a,b)   xor(a,b)
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// even ( shape-list )
// 2, 4, 6, ...
// -----------------------------------------------------------------------------

kip_simplify(even)
{
   /*
     even(a)     nothing
     even(a,b)   and(a,b)
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// some ( shape-list )
// at least one, but not all
// -----------------------------------------------------------------------------

kip_simplify(some)
{
   /*
     some(a)     nothing
     some(a,b)   xor(a,b)
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// one ( shape-list )
// exactly one
// -----------------------------------------------------------------------------

kip_simplify(one)
{
   /*
     one(a)     a
     one(a,b)   xor(a,b)
   */

   assert(false);
   return internal::pNULL(obj);
}



// -----------------------------------------------------------------------------
// ors ( shape-list )
// at least one
// -----------------------------------------------------------------------------

kip_simplify(ors)
{
   /*
     ors(a)     a
     ors(a,b)   or(a,b)
   */

   assert(false);
   return internal::pNULL(obj);
}

#undef kip_simplify
