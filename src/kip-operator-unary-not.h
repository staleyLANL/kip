
// -----------------------------------------------------------------------------
// kipnot
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class kipnot : public shape<real,tag> {
public:
   using shape<real,tag>::unary;
   kip_functions(kipnot);

   point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   void propagate_base(const bool force = false) const
      { detail::propagate_unary(*this,force); }


   // kipnot()
   explicit kipnot() :
      shape<real,tag>(this)
   { unary.a = nullptr; }


   // kipnot(a[,base]), a = pointer
   explicit kipnot(
      const shape<real,tag> *const _a
   ) :
      shape<real,tag>(this)
   { unary.a = _a; }

   explicit kipnot(
      const shape<real,tag> *const _a, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   { unary.a = _a; }


   // kipnot(a[,base]), a = reference
   explicit kipnot(
      const shape<real,tag> &a
   ) :
      shape<real,tag>(this)
   { unary.a = a.duplicate(); }

   explicit kipnot(
      const shape<real,tag> &a, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   { unary.a = a.duplicate(); }


   // kipnot(kipnot)
   kipnot(const kipnot &from) :
      shape<real,tag>(from)
   {
      unary.a = from.unary.a ? from.unary.a->duplicate() : nullptr;
   }

   // kipnot = kipnot
   kipnot &operator=(const kipnot &from)
   {
      this->shape<real,tag>::operator=(from);
      delete unary.a;
      unary.a = from.unary.a ? from.unary.a->duplicate() : nullptr;
      return *this;
   }


   // destructor
   ~kipnot()
   {
      delete unary.a;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(kipnot)
{
   unary.a->is_operand = true;
   unary.amin = unary.a->process(eyeball,light,engine,vars);
   kip_assert(unary.amin >= 0);

   unary.ina = unary.a->interior;
   this->interior = !unary.ina;

   return std::abs(unary.amin);
} kip_end



// aabb
kip_aabb(kipnot)
{
   const real infinity = std::numeric_limits<real>::max();
   return bbox<real>(
      false,-infinity, infinity,false,
      false,-infinity, infinity,false,
      false,-infinity, infinity,false
   );
} kip_end



// dry
kip_dry(kipnot)
{
   // I suppose this is all we can say for sure about a not-ed object
   return false;
} kip_end



// check
kip_check(kipnot)
{
   return detail::check_operand("not",unary.a,"a");
} kip_end



// randomize
kip_randomize(kipnot)
{
   // For a random [kip]not, we'll do a not(not(sphere)).
   // Random objects are largely for speed testing.

   // Note: I see from testing that these randomly-generated not objects
   // are very slow. This is no doubt because screen segmenting basically
   // doesn't happen for not. Really, these should only be used in other
   // objects, when absolutely necessary. Generally, for what nots would
   // usually be used for, a "cut" object is probably better.

   // sphere
   sphere<real,tag> *const sptr = new sphere<real,tag>; // delete in ~kipnot()
   randomize(*sptr);

   // not: inner
   kipnot<real,tag> *const nptr = new kipnot<real,tag>; // delete in ~kipnot()
   nptr->unary.a = sptr;

   // not: outer
   obj.unary.a = nptr;

   // don't color over the nested objects, but instead maintain
   // the original sphere's color
   // randomize(obj.base());
   // obj.baseset = true;

   // done
   return obj;
} kip_end



// infirst
kip_infirst(kipnot)
{
   return detail::op_first(unary.a, kip_etd, qmin,q, insub)
      ? q.reverse(), true : false;
} kip_end

// inall
kip_inall(kipnot)
{
   return detail::op_all(unary.a, kip_etd, qmin,ints, insub)
      ? ints.reverse() : false;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

// read_value
template<class ISTREAM, class real, class tag>
bool read_value(
   ISTREAM &s, kip::kipnot<real,tag> &obj,
   const std::string &description = "logical-not operator"
) {
   // (
   //    shape,
   //    base
   // )

   delete obj.unary.a;
   s.bail = false;

   if (!(
      read_left(s) &&
      read_shape(s, obj.unary.a, "shape operand") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic::error);
   } else if (s.level == 0)
      obj.propagate_base();

   return !s.fail();
}



// kip::ostream
kip_ostream(kipnot) {
   bool okay;

   // stub
   if (kip::format == kip::format_t::format_stub)
      okay = k << "not(" &&
             obj.unary.a->write(k) << ')';

   // one
   else if (kip::format == kip::format_t::format_one) {
      okay = k << "not(" &&
             obj.unary.a->write(k);
      if (obj.baseset)
         okay = okay && k << ", " << obj.base();
      okay = okay && k << ')';
   }

   // op
   // full
   else if ((okay = k << "not(\n")) {
      k.level++;

      okay = obj.unary.a->write(k.indent());
      if (obj.baseset)
         okay = okay && k << ",\n" && k.indent() << obj.base();
      okay = okay && k << '\n';

      k.level--;
      okay = okay && k.indent() << ')';
   }

   // done
   if (!okay)
      (void)kip::error("Could not write logical-not operator");
   return k;
}

#define   kip_class kipnot
#include "kip-macro-io.h"
