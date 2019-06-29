
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// kipnot
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class kipnot : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::unary;
   kip_functions(kipnot);

   inline point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   inline void propagate_base(const bool force = false) const
      { internal::propagate_unary(*this,force); }


   // kipnot()
   inline explicit kipnot(void) :
      shape<real,tag>(this)
   { kip_counter_ctor(kipnot);  unary.a = NULL; }


   // kipnot(a[,base]), a = pointer
   inline explicit kipnot(
      const shape<real,tag> *const _a
   ) :
      shape<real,tag>(this)
   { kip_counter_ctor(kipnot);  unary.a = _a; }

   inline explicit kipnot(
      const shape<real,tag> *const _a, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   { kip_counter_ctor(kipnot);  unary.a = _a; }


   // kipnot(a[,base]), a = reference
   inline explicit kipnot(
      const shape<real,tag> &a
   ) :
      shape<real,tag>(this)
   { kip_counter_ctor(kipnot);  unary.a = a.duplicate(); }

   inline explicit kipnot(
      const shape<real,tag> &a, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   { kip_counter_ctor(kipnot);  unary.a = a.duplicate(); }


   // kipnot(kipnot)
   inline kipnot(const kipnot &from) :
      shape<real,tag>(from)
   {
      kip_counter_ctor(kipnot);
      unary.a = from.unary.a ? from.unary.a->duplicate() : NULL;
   }

   // kipnot = kipnot
   inline kipnot &operator=(const kipnot &from)
   {
      this->shape<real,tag>::operator=(from);
      delete unary.a; unary.a=from.unary.a ? from.unary.a->duplicate():NULL;
      return *this;
   }


   // destructor
   inline ~kipnot(void)
   {
      kip_counter_dtor(kipnot);
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
   if_kip_assert(unary.amin >= 0);

   unary.ina = unary.a->interior;
   interior = !unary.ina;

   return op::abs(unary.amin);
} kip_end



// aabb
kip_aabb(kipnot)
{
   const real infinity = std::numeric_limits<real>::max();
   return kip::bbox<real>(
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
   return internal::check_operand("not",unary.a,"a");
} kip_end



// random
kip_random(kipnot)
{
   sphere<real,tag> *const a = new sphere<real,tag>;
   random(*a);
   // delete for the above new is in kipnot's destructor

   // operand
   obj.unary.a = a;

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// infirst
kip_infirst(kipnot)
{
   return internal::op_first(unary.a, kip_etd, qmin,q, insub)
      ? q.reverse(), true : false;
} kip_end



// inall
kip_inall(kipnot)
{
   return internal::op_all(unary.a, kip_etd, qmin,ints, insub)
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
      addendum("Detected while reading "+description, diagnostic_error);
   } else if (s.level == 0)
      obj.propagate_base();

   return !s.fail();
}



// kip::ostream
kip_ostream(kipnot) {
   bool okay;

   // stub
   if (kip::format == kip::format_stub)
      okay = k << "not(" &&
             obj.unary.a->write(k) << ')';

   // one
   else if (kip::format == kip::format_one) {
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

}

#define   kip_class kipnot
#include "kip-macro-io.h"