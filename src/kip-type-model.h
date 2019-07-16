
#pragma once

// This file provides the "model" class, objects of which are designed
// to contain models for ray tracing.



// -----------------------------------------------------------------------------
// model
// -----------------------------------------------------------------------------

template<
   class real = default_real_t,
   class base = default_tag_t
>
class model {
   // copy constructor/assignment; deliberately private
   inline model(const model &) : append(false) { }
   inline model &operator=(const model &) { return *this; }

public:

   // Data
   #ifdef KIP_BLOCK
      kip::bbox<real> bbox;
   #endif
   bool append;

   // Constructor
   inline explicit model() : append(false) { }



   // --------------------------------
   // containers
   // push
   // --------------------------------

   /*
   For type = each shape:
      (1) container<type>

   Specific push() functions:
      (2) type &push(const type &)
      (3) type &push(const type *const)

   Generic push() functions:
      (4) shape *push(const shape *const)
      (5) shape *push(const std::pair<const type  *, const shape *> &)
      (6) shape *push(const std::pair<const shape *, const shape *> &)

   Regarding push():
      Inputs are entirely const. Copies of inputs, never inputs themselves,
      are actually pushed. Because models onto which we're *pushing* cannot
      be const, we don't want const push()es. Nor is there a necessity, then,
      to return const type & (or const shape *) rather than their non-const
      counterparts. (While allowable, this would needlessly restrict users.)

      The shape * push() dereferences, to the exact type, based on ptr->id().
      Then it pushes a copy, as other push() functions do. Regarding its
      return value: again we don't need const; and, *const has no meaning
      here. Note also that our pointer return is consistent with our pointer
      input - and appropriate, as the exact type is determined at run-time.

      The std::pair push() call (5) allows for pushing of the output from a
      simplify() call. In view of what simplify() produces: if first==second
      then first (whose type is known at compile-time) is pushed; otherwise,
      second (whose type is known only at run-time) is pushed. Finally, (6)
      is a more-generic version of (5), used e.g. for simplify() output when
      a generic shape * (not something more specific) was sent to simplify().
   */

   // (1) container<type>
   // (2) type &push(const type &)
   // (3) type &push(const type *const)
   #define kip_make_primitive(type,prop)\
      std::vector<kip::type<real,base>> type;\
      \
      inline kip::type<real,base> &push(const kip::type<real,base> &obj)\
      {\
         type.push_back(obj);\
         if (prop) type.back().propagate_base();\
         return type.back();\
      }\
      \
      inline kip::type<real,base> &push(const kip::type<real,base> *const ptr)\
      {\
         internal::tclass<kip::type<real,base>> t;\
         return ptr ? push(*ptr) : default_parameter(t);\
      }

   // operators
   kip_make_primitive(kipnot, true)

   kip_make_primitive(kipand, true)
   kip_make_primitive(kipcut, true)
   kip_make_primitive(kipor,  true)
   kip_make_primitive(kipxor, true)

   kip_make_primitive(ands,   true)
   kip_make_primitive(odd,    true)
   kip_make_primitive(even,   true)
   kip_make_primitive(some,   true)
   kip_make_primitive(one,    true)
   kip_make_primitive(ors,    true)

   // primitives
   kip_make_primitive(bicylinder, false)
   kip_make_primitive(biwasher,   false)
   kip_make_primitive(box,        false)
   kip_make_primitive(cube,       false)
   kip_make_primitive(cone,       false)
   kip_make_primitive(silo,       false)
   kip_make_primitive(cylinder,   false)
   kip_make_primitive(pill,       false)
   kip_make_primitive(ellipsoid,  false)
   kip_make_primitive(paraboloid, false)
   kip_make_primitive(surf,       false)
   kip_make_primitive(nothing,    false)
   kip_make_primitive(everything, false)
   kip_make_primitive(half,       false)
   kip_make_primitive(circle,     false)
   kip_make_primitive(xplane,     false)
   kip_make_primitive(yplane,     false)
   kip_make_primitive(zplane,     false)
   kip_make_primitive(sphere,     false)
   kip_make_primitive(spheroid,   false)
   kip_make_primitive(triangle,   false)
   kip_make_primitive(polygon,    false)
   kip_make_primitive(washer,     false)
   kip_make_primitive(tabular,    false)

   #undef kip_make_primitive



   // (4) shape *push(const shape *const); defined later
   shape<real,base> *push(const shape<real,base> *const);


   // (5) shape *push(const std::pair<const type *, const shape *> &)
   template<template<class,class> class type>
   inline shape<real,base> *push(
      const std::pair<
         const type <real,base> *,
         const shape<real,base> *
      > &pair
   ) {
      if (pair.first == pair.second)
         // calls (3); compile-time select
         return &push(pair.first);
      else {
         // calls (4); run-time select
         shape<real,base> *const rv = push(pair.second);
         delete pair.second;
         return rv;
      }
   }


   // (6) shape *push(const std::pair<const shape *, const shape *> &)
   inline shape<real,base> *push(
      const std::pair<
         const shape<real,base> *,
         const shape<real,base> *
      > &pair
   ) {
      shape<real,base> *const rv = push(pair.second);
      if (pair.first != pair.second)
         delete pair.second;
      return rv;
   }



   // --------------------------------
   // push(bbox,...)
   // --------------------------------

private:

   // bbox_representation_t, and_tight_t
   using bbox_representation_t = kip::ors<real,base>;
   using and_tight_t = internal::and_tight<real>;

   // rpush
   void rpush(
      bbox_representation_t &, const real, const base &,
      const and_tight_t &, const and_tight_t &, const and_tight_t &,
      const and_tight_t &, const and_tight_t &, const and_tight_t &
   );

   // push_bbox_ptr(bbox,shape*, scale, tight,loose)
   bbox_representation_t &push_bbox_ptr(
      const kip::bbox<real> &,  // bounding box
      const shape<real,base> *const,  // associated shape

      const real = real(0),  // scale

      // bases
      const base & = default_parameter(internal::tclass<base>()),
      const base & = default_parameter(internal::tclass<base>()),
      const base & = default_parameter(internal::tclass<base>())
   );

public:

   // push_bbox(bbox,shape&, scale, tight,partial,loose)
   bbox_representation_t &push_bbox(
      const kip::bbox<real> &,  // bounding box
      const shape<real,base> &,  // associated shape

      const real = real(0),  // scale

      // for tight/partial/loose bbox representation
      const base & = default_parameter(internal::tclass<base>()),
      const base & = default_parameter(internal::tclass<base>()),
      const base & = default_parameter(internal::tclass<base>())
   );



   // --------------------------------
   // Misc
   // --------------------------------

   // clear, assign, size
   void clear();
   model &assign(const model &);
   size_t size() const;

   // unbound
   void unbound();

   // aabb
   kip::bbox<real> aabb(
      const bool = false,  // each
      const bool = false,  // overall
      const real   = real(0),  // scale
      const base & = default_parameter(internal::tclass<base>()),  // tight
      const base & = default_parameter(internal::tclass<base>()),  // partial
      const base & = default_parameter(internal::tclass<base>())   // loose
   );
};



// -----------------------------------------------------------------------------
// loop
// For looping over a model's shapes (generically, as shapes)
// -----------------------------------------------------------------------------

template<
   class real = default_real_t,
   class base = default_tag_t
>
class loop {
   using pshape = const shape<real,base> *;

   // helper: class three
   class three {
   public:
      unsigned size;
      const char *begin;
      unsigned element_size;
   };

   // data
   std::vector<three> shapes;  unsigned nshapes;
   unsigned ROW;  mutable unsigned row, col;
   mutable pshape ptr;

   // conversion to bool
   // Private; prevents users from writing "while (i)" instead of "while (i())"
   // for a loop i
   inline operator bool() const { return true; }

public:

   // reset
   inline void reset() const
   {
      row = ROW;
      col = 0;
   }

   // constructor: loop(model)
   explicit loop(const model<real,base> &m);


   // operator()
   bool operator()() const;

   // dereference (gives shape *)
   inline pshape operator*() const { return ptr; }
};



// -----------------------------------------------------------------------------
// constructor: loop::loop(model)
// loop::operator()
// -----------------------------------------------------------------------------

// loop()
template<class real, class base>
loop<real,base>::loop(const model<real,base> &m)
{
   three t;

#define kip_make_loop(type)\
   t.begin = (t.size = m.type.size()) ? (const char *)&m.type[0] : nullptr;\
   t.element_size = sizeof(kip::type<real,base>);\
   shapes.push_back(t);

   kip_expand_semi(kip_make_loop)
#undef  kip_make_loop

   nshapes = shapes.size();
   for (ROW = 0;  ROW < nshapes && !shapes[ROW].size;  ++ROW)
      ;
   reset();
}



// operator()
template<class real, class base>
bool loop<real,base>::operator()() const
{
   if (ROW == nshapes)
      return false;

   if (col < shapes[row].size)
      return (ptr = pshape(shapes[row].begin + col++*shapes[row].element_size));

   while (++row < nshapes)
      if (shapes[row].size)
         return ptr = pshape(shapes[row].begin), col = 1;

   return reset(), false;
}



// -----------------------------------------------------------------------------
// push(shape *)
// Checks id() to determine specific type, then delegates
// -----------------------------------------------------------------------------

// push
template<class real, class base>
shape<real,base> *model<real,base>::push(const shape<real,base> *const ptr)
{
   if (ptr == nullptr) return nullptr;

#define kip_make_push(type)\
   if (ptr->id() == internal::get_shape_id<kip::type>::result)\
      return &push((const kip::type<real,base> *)ptr)

   kip_expand_semi(kip_make_push)
#undef kip_make_push

   assert(false);  // shouldn't get here, or else id() didn't check out
   return nullptr;
}



// -----------------------------------------------------------------------------
// rpush
// push_bbox_ptr
// push_bbox
// -----------------------------------------------------------------------------

// rpush
template<class real, class base>
void model<real,base>::rpush(
   bbox_representation_t &rep, const real r, const base &thebase,
   const and_tight_t &ax, const and_tight_t &ay, const and_tight_t &az,
   const and_tight_t &bx, const and_tight_t &by, const and_tight_t &bz
) {
   if (ax.finite() && ay.finite() && az.finite() &&
       bx.finite() && by.finite() && bz.finite()) {

      const point<real> a(ax,ay,az);
      const point<real> b(bx,by,bz);

      if (a != b) {
         const point<real> tad = real(0.2) * r * normalize(b-a);
         rep.push(kip::pill<real,base>(a-tad,b+tad, r,thebase)).isbound = true;
      }
   }
}



// push_bbox_ptr
template<class real, class base>
typename model<real,base>::bbox_representation_t &
model<real,base>::push_bbox_ptr(
   const kip::bbox<real> &b,
   const shape<real,base> *const shape,

   const real scale,

   const base &_tight_base,
   const base &_partial_base,
   const base &_loose_base
) {
   real sum = 0;  unsigned num = 0;
   if (b.x.finite()) sum += b.x.max - b.x.min, num++;
   if (b.y.finite()) sum += b.y.max - b.y.min, num++;
   if (b.z.finite()) sum += b.z.max - b.z.min, num++;

   // don't bound shapes that represent bounding boxes themselves, or shapes for
   // which the bounding box is not *entirely* valid or has no finite extent.
   if ((shape && shape->isbound) || !b.valid() || !num)
      return default_parameter(internal::tclass<bbox_representation_t>());

   // initialize rep, a shape that graphically represents bbox
   bbox_representation_t rep;  rep.isbound = true;

   // bases
   const base &default_base = default_parameter(internal::tclass<base>());
   const base
          tight_base = &  _tight_base != &default_base
     ?   _tight_base : crayola::black,
        partial_base = &_partial_base != &default_base
     ? _partial_base : crayola::gray_medium,
          loose_base = &  _loose_base != &default_base
     ?   _loose_base : crayola::white;

   const base xbase =
      b.x.min.tight() && b.x.max.tight() ?   tight_base
    : b.x.min.tight() || b.x.max.tight() ? partial_base : loose_base;
   const base ybase =
      b.y.min.tight() && b.y.max.tight() ?   tight_base
    : b.y.min.tight() || b.y.max.tight() ? partial_base : loose_base;
   const base zbase =
      b.z.min.tight() && b.z.max.tight() ?   tight_base
    : b.z.min.tight() || b.z.max.tight() ? partial_base : loose_base;

   // scaling factor
   //    if scale <  0: |scale|
   //    if scale >  0: average bounding box size * scale
   //    if scale == 0: average bounding box size * a default factor
   const real r = scale < 0
      ? -scale
      :  sum/real(num)*(scale > 0 ? scale : real(0.01));

   // build...
   #define kip_rpush(one,two,three, four,five,six, dir)\
      rpush(rep,r,kip_paste(dir,base), b.one,b.two,b.three, b.four,b.five,b.six)

   // ...x direction
   kip_rpush(x.min,y.min,z.min, x.max,y.min,z.min, x); if (b.z.max != b.z.min)
   kip_rpush(x.min,y.min,z.max, x.max,y.min,z.max, x); if (b.y.max != b.y.min) {
   kip_rpush(x.min,y.max,z.min, x.max,y.max,z.min, x); if (b.z.max != b.z.min)
   kip_rpush(x.min,y.max,z.max, x.max,y.max,z.max, x); }

   // ...y direction
   kip_rpush(x.min,y.min,z.min, x.min,y.max,z.min, y); if (b.z.max != b.z.min)
   kip_rpush(x.min,y.min,z.max, x.min,y.max,z.max, y); if (b.x.max != b.x.min) {
   kip_rpush(x.max,y.min,z.min, x.max,y.max,z.min, y); if (b.z.max != b.z.min)
   kip_rpush(x.max,y.min,z.max, x.max,y.max,z.max, y); }

   // ...z direction
   kip_rpush(x.min,y.min,z.min, x.min,y.min,z.max, z); if (b.y.max != b.y.min)
   kip_rpush(x.min,y.max,z.min, x.min,y.max,z.max, z); if (b.x.max != b.x.min) {
   kip_rpush(x.max,y.min,z.min, x.max,y.min,z.max, z); if (b.y.max != b.y.min)
   kip_rpush(x.max,y.max,z.min, x.max,y.max,z.max, z); }

   #undef kip_rpush

   // finish
   return rep.size()
      ? push(rep)
      : default_parameter(internal::tclass<bbox_representation_t>());
}



// push_bbox
template<class real, class base>
inline typename model<real,base>::bbox_representation_t &
model<real,base>::push_bbox(
   const kip::bbox<real> &b,
   const shape<real,base> &shape,

   const real scale,

   const base &tight_base,
   const base &partial_base,
   const base &loose_base
) {
   return push_bbox_ptr(b,&shape, scale, tight_base,partial_base,loose_base);
}



// -----------------------------------------------------------------------------
// clear
// assign
// size
// -----------------------------------------------------------------------------

// clear
namespace internal {
   class functor_clear {
   public:
      inline functor_clear() { }

      template<class CONTAINER>
      inline void operator()(CONTAINER &c) const { c.clear(); }
   };
}

template<class real, class base>
void model<real,base>::clear()
{
   const internal::functor_clear f;
   internal::allshape(*this, f);
}



// assign
template<class real, class base>
model<real,base> &model<real,base>::assign(const model<real,base> &from)
{
#define kip_make_assign(type) type = from.type
   kip_expand_semi(kip_make_assign)
#undef  kip_make_assign

   return *this;
}



// size
namespace internal {
   class functor_size {
   public:
      mutable size_t size;
      inline functor_size() : size(0) { }

      template<class CONTAINER>
      inline void operator()(CONTAINER &c) const { size += size_t(c.size()); }
   };
}

template<class real, class base>
size_t model<real,base>::size() const
{
   const internal::functor_size f;
   internal::allshape(*this, f);
   return f.size;
}



// -----------------------------------------------------------------------------
// unbound
// -----------------------------------------------------------------------------

// isbound - condition for std::remove_if() in unbound()
namespace internal {
   template<class SHAPE>
   class isbound {
   public:
      // operator()(SHAPE)
      inline bool operator()(const SHAPE &s) const { return s.isbound; }
   };
}



// unbound
namespace internal {
   class functor_unbound {
   public:
      inline functor_unbound() { }

      template<class CONTAINER>
      inline void operator()(CONTAINER &c) const
      {
         c.erase(
            std::remove_if(
               c.begin(), c.end(), isbound<typename CONTAINER::value_type>()
            ),
            c.end()
         );
      }
   };
}

template<class real, class base>
void model<real,base>::unbound()
{
   /// std::cout << "   unbound!" << std::endl;

   const internal::functor_unbound f;
   internal::allshape(*this, f);
}



// -----------------------------------------------------------------------------
// aabb
// -----------------------------------------------------------------------------

namespace internal {
   template<class real, class base>
   class functor_bound {
      kip::model<real,base> &model;  bbox<real> &rv;  const bool each;
      const real scale;  const base &tight, &partial, &loose;

   public:

      inline functor_bound(
         kip::model<real,base> &_model, bbox<real> &_rv,
         const bool &_each, const real _scale,
         const base &_tight, const base &_partial, const base &_loose
      ) :
         model(_model), rv(_rv), each(_each), scale(_scale),
         tight(_tight), partial(_partial), loose(_loose)
      { }

      template<class CONTAINER>
      inline void operator()(CONTAINER &obj) const
      {
         const size_t size = obj.size();
         for (size_t i = 0;  i < size;  ++i)
            if (obj[i].on && !obj[i].isbound) {
               // next bounding box
               const bbox<real> b = obj[i].aabb();

               // possibly attach to model
               if (each) model.push_bbox(
                  b, obj[i], scale,

                  &tight   == &default_parameter(internal::tclass<base>())
                     ? obj[i] : tight,

                  &partial == &default_parameter(internal::tclass<base>())
                     ? obj[i] : partial,

                  &loose   == &default_parameter(internal::tclass<base>())
                     ? obj[i] : loose
               );

               // update overall result; similar, e.g., to ors::aabb()
               rv = internal::bound_combine(rv, b, internal::op_leq());
            }
      }
   };
}

template<class real, class base>
bbox<real> model<real,base>::aabb(
   const bool each, const bool overall, const real scale,
   const base &tight, const base &partial, const base &loose
) {
   // chuck any existing clutter
   unbound();

   /// Remove...
   /// std::cout << "   bound " << each << ' ' << overall << std::endl;

   // new return value, to be computed
   kip::bbox<real> rv(false,1,0,false, false,1,0,false, false,1,0,false);

   // compute
   const internal::functor_bound<real,base>
      f(*this, rv,each, scale, tight,partial,loose);
   internal::allshape(*this, f);

   // finish
   if (overall)
      push_bbox_ptr(rv,nullptr);
   return rv;
}



// -----------------------------------------------------------------------------
// shape_select
// allocate_and_read
// read_shape
// read_shapes
//
// These really should go in a different file, as they don't relate to class
// model per se. They're used by the logical operators: and, not, etc.
// -----------------------------------------------------------------------------

// shape_select
// Note: the return value is true iff the shape selection succeeded, regardless
// of whether or not the action succeeded. In fact, the action is taken iff the
// shape selection did succeed, and in this case, the action's status should be
// checked elsewhere.
template<class real, class base, class ACTION>
bool shape_select(const std::string &name, ACTION &action)
{
   if (false) ;

   // shortened names
   #define kip_if_shape(str,type)\
      else if (name == str) { action.fun(internal::tclass<type<real,base>>()); }
   kip_if_shape("not",  kipnot)
   kip_if_shape("and",  kipand)
   kip_if_shape("cut",  kipcut)
   kip_if_shape("or",   kipor )
   kip_if_shape("xor",  kipxor)
   #undef  kip_if_shape

   // full names
   #define kip_if_shape(shp)\
      else if (name == #shp) { action.fun(internal::tclass<shp<real,base>>()); }
   kip_expand_plain(kip_if_shape)
   #undef  kip_if_shape

   else return false;
   return true;  // true iff shape selection (not necessarily action) succeeded

   #undef kip_if_shape
}



namespace internal {
   // allocate_and_read
   template<class real, class base>
   class allocate_and_read {
      kip::istream &k;
      shape<real,base> * &ptr;

   public:

      inline explicit allocate_and_read(
         kip::istream &_k, shape<real,base> * &_ptr
      )
    : k(_k), ptr(_ptr)
      { }

      template<class SHAPE>
      inline void fun(const tclass<SHAPE> &)
      {
         SHAPE *const p = new SHAPE;
         if (p && k >> *p)
            ptr = p;
         else
            delete p;
      }
   };
}



// read_shape
template<class real, class tag>
bool read_shape(
   kip::istream &k, shape<real,tag> * &ptr,
   const std::string &description = "shape"
) {
   // prefix
   if (!k.prefix('\0', description, false))  // false: eof not okay
      return false;

   // read keyword
   static std::string word;
   if (!read_value(k,word,description))
      return false;

   // select shape and take action
   internal::allocate_and_read<real,tag> action(k,ptr);
   if (!shape_select<real,tag>(word,action)) {
      std::ostringstream oss;
      oss << "Unknown shape \"" << word << '"';
      k.error(oss);
      k.set(std::ios::failbit);
   }
   return !k.fail();
}



// read_shapes
// read some number (can be zero) of comma-separated shapes. Cases:
//    0. no shapes
//    1. shape
//    2. shape, shape
//    3. shape, shape, shape
// Etc.
// By context, after the last shape we should see either ';' or ')'
template<class real, class tag, class NARY>
bool read_shapes(kip::istream &k, NARY &obj)
{
   std::vector<shape<real,tag> *> op;

   for (;;) {
      std::ostringstream oss;
      oss << "shape operand [" << op.size() << ']';

      if (!k.prefix('\0', oss.str(), false))  // false: eof not okay
         return false;

      // look ahead; do we see ';' or ')'?
      const int ch = k.peek();
      if (ch == ';' || ch == ')') {
         // transfer op to obj.vec(); then done
         obj.vec().clear();
         const size_t nop = op.size();
         obj.vec().reserve(nop);

         for (size_t n = 0;  n < nop;  ++n)
            obj.push().op = op[n];
         return true;
      }

      // should be a shape
      shape<real,tag> *ptr;
      if ((op.size() && !read_comma(k)) || !read_shape(k,ptr,oss.str()))
         return false;

      op.push_back(ptr);
   }
}



// -----------------------------------------------------------------------------
// i/o for model
// -----------------------------------------------------------------------------

namespace internal {
   // read_and_submit
   template<class real, class base>
   class read_and_submit {
      kip::istream &k;
      kip::model<real,base> &model;

   public:

      inline explicit read_and_submit(
         kip::istream &_k, kip::model<real,base> &_model
      )
    : k(_k), model(_model)
      { }

      template<class SHAPE>
      inline void fun(const tclass<SHAPE> &)
      {
         static SHAPE shape;
         if (k >> shape)
            model.push(shape);
      }
   };
}



// read_value
template<class ISTREAM, class real, class tag>
bool read_value(
   ISTREAM &s, model<real,tag> &value,
   const std::string &description = "model"
) {
   // clear model
   if (!value.append) value.clear();

   // bookkeeping
   s.nerr = 0;
   if (s.maxerr == 0)
      note("Maximum number of errors = 0 is interpreted as no maximum");
   s.bail = false;

   // read
   for (;;) {
      static std::string word;
      s.level = 0;

      // read keyword
      if (!read_value(s,word)) {
         // unable to read keyword
         if (!s.bad() && s.eof()) { s.clear();  return false; }
         addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
         if (!s.recover('\0')) return false;
         continue;
      }

      // select shape and take action
      internal::read_and_submit<real,tag> action(s,value);
      if (!shape_select<real,tag>(word,action)) {
         std::ostringstream oss;
         oss << "Unknown keyword \"" << word << '"';
         s.error(oss);
         s.set(std::ios::failbit);  // set fail bit
      }

      // shape input failed, or keyword wasn't recognized
      if (s.fail()) {
         addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
         if (!s.recover('\0')) return false;
      }

      if (s.maxerr && s.nerr >= s.maxerr) {
         std::ostringstream oss;
         oss << "Maximum number of errors (" << s.maxerr
             << ") reached; bailing out";
         note(oss);
         return false;
      }
   }
}



namespace internal {
   inline bool &write_shape_start()
   {
      static bool value = true;
      return value;
   }

   template<class VEC>
   inline bool write_shape(kip::ostream &k, const VEC &obj)
   {
      const typename VEC::const_iterator end = obj.end();
      for (typename VEC::const_iterator i = obj.begin();  k && i != end;  ++i)
         if (i->on) {
            if (!write_shape_start())
               k << "\n\n";
            k << *i;
            write_shape_start() = false;
         }
      return k;
   }
}



// kip::ostream
template<class real, class tag>
kip::ostream &operator<<(
   kip::ostream &k, const model<real,tag> &obj
) {
   // check
   // Actually, don't check the model as a whole. Instead, allow each object
   // to be checked (and written, if it's fine) individually. No need to foil
   // the whole model because of specific bad objects.

   // write...
   internal::write_shape_start() = true;

#define kip_write_shape(shape) internal::write_shape(k, obj.shape) &&
   kip_expand_plain(kip_write_shape)
#undef  kip_write_shape

   // done
   true;
   return k;
}

// model isn't a shape, but the following does the right thing
#define   kip_class model
#include "kip-macro-io.h"
