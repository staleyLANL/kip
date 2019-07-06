
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// kipcut: A cut B == A and (not B)
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class kipcut : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::binary;
   kip_functions(kipcut);

   inline point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   inline void propagate_base(const bool force = false) const
      { internal::propagate_binary(*this,force); }


   // kipcut()
   inline explicit kipcut(void) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipcut);
      binary.a = NULL;
      binary.b = NULL;
   }


   // kipcut(a,b[,base]), a and b = pointers
   inline explicit kipcut(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b
   ) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipcut);
      binary.a = _a;
      binary.b = _b;
   }

   inline explicit kipcut(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      kip_counter_ctor(kipcut);
      binary.a = _a;
      binary.b = _b;
   }


   // kipcut(a,b[,base]), a and b = references
   inline explicit kipcut(
      const shape<real,tag> &a,
      const shape<real,tag> &b
   ) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipcut);
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }

   inline explicit kipcut(
      const shape<real,tag> &a,
      const shape<real,tag> &b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      kip_counter_ctor(kipcut);
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }


   // kipcut(kipcut)
   inline kipcut(const kipcut &from) :
      shape<real,tag>(from)
   {
      kip_counter_ctor(kipcut);
      binary.a = from.binary.a ? from.binary.a->duplicate() : NULL;
      binary.b = from.binary.b ? from.binary.b->duplicate() : NULL;
   }

   // kipcut = kipcut
   inline kipcut &operator=(const kipcut &from)
   {
      this->shape<real,tag>::operator=(from);
      delete binary.a; binary.a=from.binary.a ? from.binary.a->duplicate():NULL;
      delete binary.b; binary.b=from.binary.b ? from.binary.b->duplicate():NULL;
      return *this;
   }


   // destructor
   inline ~kipcut(void)
   {
      kip_counter_dtor(kipcut);
      delete binary.a;
      delete binary.b;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(kipcut)
{
   // process operands
   binary.a->is_operand = true;
   binary.amin = binary.a->process(eyeball,light,engine,vars);
   if_kip_assert(binary.amin >= 0);

   binary.b->is_operand = true;
   binary.bmin = binary.b->process(eyeball,light,engine,vars);
   if_kip_assert(binary.bmin >= 0);

   // The logical-cut operator is *not* reflexive, so we can't swap
   // its operands (as we can with certain other operators).

   binary.ina = binary.a->interior;
   binary.inb = binary.b->interior;

   interior = binary.ina && !binary.inb;

   // The following is equivalent to "and" with !inb. The effects are to swap
   // the "both" and "a" cases, and the "b" and "neither" cases.
   return op::abs(
      binary.ina
    ? binary.inb
    ? binary.bmin  // in both
    : op::min(binary.amin,binary.bmin)  // in a only
    : binary.inb
    ? op::max(binary.amin,binary.bmin)  // in b only
    : binary.amin  // in neither
   );
} kip_end



// aabb
kip_aabb(kipcut)
{
   const bbox<real> a = binary.a->aabb();
   const bbox<real> b = binary.b->aabb();

   return kip::bbox<real>(
      // x
      a.x.min < b.x.min && a.x.min.tight(),
      a.x.min,
      a.x.max,
      a.x.max > b.x.max && a.x.max.tight(),

      // y
      a.y.min < b.y.min && a.y.min.tight(),
      a.y.min,
      a.y.max,
      a.y.max > b.y.max && a.y.max.tight(),

      // z
      a.z.min < b.z.min && a.z.min.tight(),
      a.z.min,
      a.z.max,
      a.z.max > b.z.max && a.z.max.tight()
   );
} kip_end



// dry
kip_dry(kipcut)
{
   // Could be stronger; perhaps "b" takes away the entire non-dry portion
   // of "a". But that's difficult to determine.
   return binary.a->dry(seg);
} kip_end



// check
kip_check(kipcut)
{
   // written the following way so that "a" is checked before "b"
   diagnostic_t d = internal::check_operand("cut",binary.a,"a");
   return op::min(d,internal::check_operand("cut",binary.b,"b"));
} kip_end



// random
kip_random(kipcut)
{
   sphere<real,tag> *const a = new sphere<real,tag>;
   sphere<real,tag> *const b = new sphere<real,tag>;

   random(*a);  a->r *= real(1.5);
   random(*b);  b->r *= real(1.5);  const real r = a->r + b->r;

   b->c.x = a->c.x + (random<real>() - real(0.5))*r;
   b->c.y = a->c.y + (random<real>() - real(0.5))*r;
   b->c.z = a->c.z + (random<real>() - real(0.5))*r;

   // operands
   obj.binary.a = a;
   obj.binary.b = b;

   // base
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(kipcut)
{
   // INSIDE...
   if (interior) {
      // the overall status changes (from inside to outside) at the point
      // of first intersection with either object's boundary
      inq<real,tag> bq;
      return
           binary.amin < qmin &&
           internal::op_first(binary.a, kip_etd, qmin, q, insub)
         ? binary.bmin < q    &&
           internal::op_first(binary.b, kip_etd, real(q),bq, insub)
              ? (q=bq).reverse(), // b wins
                 true
              :  true             // a only
         : binary.bmin < qmin &&
           internal::op_first(binary.b, kip_etd, qmin, q, insub)
              ? (q   ).reverse(), // b only
                 true
              :  false  // always (up to qmin) inside
      ;
   }

   // OUTSIDE...
   // the overall status changes (from outside to inside) at the point
   // after which we're inside "a" and outside "b"

   afew<inq<real,tag>> aq;
   if (binary.amin >= qmin ||
      !internal::op_all(binary.a, kip_etd, qmin,aq, insub))
      return
         binary.ina &&
         binary.bmin < qmin &&
         internal::op_first(binary.b, kip_etd, qmin,q, insub) &&
        (q.reverse(), true);

   afew<inq<real,tag>> bq;
   if (binary.bmin >= qmin ||
      !internal::op_all(binary.b, kip_etd, qmin,bq, insub))
      return !binary.inb && (q = aq[0], true);

   // search for the relevant point
   const size_t anum = aq.size();  size_t an = 0;  bool ina = binary.ina;
   const size_t bnum = bq.size();  size_t bn = 0;  bool inb = binary.inb, is_a;

   do
      if (an < anum && bn < bnum)
         if (aq[an].q < bq[bn].q)
            an++, ina = !ina, is_a = true;
         else
            bn++, inb = !inb, is_a = false;
      else if (!inb && an < anum) { an++, is_a = true;  break; }
      else if ( ina && bn < bnum) { bn++, is_a = false; break; }
      else return false;
   while (inb || !ina);

   return q = is_a ? aq[an-1] : bq[bn-1].reverse(), true;
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(kipcut)
{
   afew<inq<real,tag>> aq;
   if (!(binary.amin < qmin &&
         internal::op_all(binary.a, kip_etd, qmin,aq, insub)))
      return
         binary.ina && binary.bmin < qmin &&
         internal::op_all(binary.b, kip_etd, qmin,ints, insub) &&
         ints.reverse();

   afew<inq<real,tag>> bq;
   if (!(binary.bmin < qmin &&
         internal::op_all(binary.b, kip_etd, qmin,bq, insub)))
      return !binary.inb && ints.assign(aq);

   const size_t anum = aq.size();  size_t an = 0;  bool ina = binary.ina;
   const size_t bnum = bq.size();  size_t bn = 0;  bool inb = binary.inb;
   bool in = interior, is_a;

   for (ints.reset();;) {
      if (an < anum && bn < bnum) is_a = aq[an].q < bq[bn].q;
      else if (!inb && an < anum) is_a = true;
      else if ( ina && bn < bnum) is_a = false;
      else break;

      if (is_a) an++, ina = !ina;
      else      bn++, inb = !inb;

      if ((ina && !inb) != in)  // status changed
         ints.push(is_a ? aq[an-1] : bq[bn-1].reverse()), in = !in;
   }

   return ints.size() > 0;
} kip_end

}



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#define   kip_brief cut
#include "kip-macro-io-binary.h"
