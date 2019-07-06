
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// kipxor
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class kipxor : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::binary;
   kip_functions(kipxor);

   inline point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   inline void propagate_base(const bool force = false) const
      { internal::propagate_binary(*this,force); }


   // kipxor()
   inline explicit kipxor() :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipxor);
      binary.a = NULL;
      binary.b = NULL;
   }


   // kipxor(a,b[,base]), a and b = pointers
   inline explicit kipxor(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b
   ) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipxor);
      binary.a = _a;
      binary.b = _b;
   }

   inline explicit kipxor(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      kip_counter_ctor(kipxor);
      binary.a = _a;
      binary.b = _b;
   }


   // kipxor(a,b[,base]), a and b = references
   inline explicit kipxor(
      const shape<real,tag> &a,
      const shape<real,tag> &b
   ) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipxor);
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }

   inline explicit kipxor(
      const shape<real,tag> &a,
      const shape<real,tag> &b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      kip_counter_ctor(kipxor);
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }


   // kipxor(kipxor)
   inline kipxor(const kipxor &from) :
      shape<real,tag>(from)
   {
      kip_counter_ctor(kipxor);
      binary.a = from.binary.a ? from.binary.a->duplicate() : NULL;
      binary.b = from.binary.b ? from.binary.b->duplicate() : NULL;
   }

   // kipxor = kipxor
   inline kipxor &operator=(const kipxor &from)
   {
      this->shape<real,tag>::operator=(from);
      delete binary.a; binary.a=from.binary.a ? from.binary.a->duplicate():NULL;
      delete binary.b; binary.b=from.binary.b ? from.binary.b->duplicate():NULL;
      return *this;
   }


   // destructor
   inline ~kipxor()
   {
      kip_counter_dtor(kipxor);
      delete binary.a;
      delete binary.b;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(kipxor)
{
   // process operands
   binary.a->is_operand = true;
   binary.amin = binary.a->process(eyeball,light,engine,vars);
   if_kip_assert(binary.amin >= 0);

   binary.b->is_operand = true;
   binary.bmin = binary.b->process(eyeball,light,engine,vars);
   if_kip_assert(binary.bmin >= 0);

   // The logical-xor operator is reflexive, so we can swap its operands.
   // Putting the operand with the *smallest* minimum first tends to speed
   // up the operator.
   if (binary.bmin < binary.amin) {
      std::swap(binary.a, binary.b);
      std::swap(binary.amin, binary.bmin);
   }

   binary.ina = binary.a->interior;
   binary.inb = binary.b->interior;

   interior = binary.ina != binary.inb;

   // Technically should be min(abs,abs), but minima are presumably positive.
   return op::abs(op::min(binary.amin,binary.bmin));
} kip_end



// aabb
kip_aabb(kipxor)
{
   // Almost the same as for or, as exclusiveness can only remove an "inner"
   // portion. However, to ensure tightness we want strict inequalities for
   // bound comparisons. Otherwise, possible exact cancellations could mean
   // an or-tight bound is not xor-tight.

   return internal::bound_combine(
      binary.a->aabb(),
      binary.b->aabb(), internal::op_less()
   );
} kip_end



// dry
kip_dry(kipxor)
{
   // Technically, if the two xor'd operands overlapped perfectly in their
   // non-dry portions, then we could do better than the following. That
   // scenario is unlikely, however, and the following is at least *safe*.
   return binary.a->dry(seg) && binary.b->dry(seg);
} kip_end



// check
kip_check(kipxor)
{
   // written the following way so that "a" is checked before "b"
   diagnostic_t d = internal::check_operand("xor",binary.a,"a");
   return op::min(d,internal::check_operand("xor",binary.b,"b"));
} kip_end



// random
kip_random(kipxor)
{
   sphere<real,tag> *const a = new sphere<real,tag>;
   sphere<real,tag> *const b = new sphere<real,tag>;

   random(*a);
   random(*b);  const real r = a->r + b->r;

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



// infirst
kip_infirst(kipxor)
{
   // the overall status changes (from either inside to outside, or outside to
   // inside) at the point of first intersection with either object's boundary
   inq<real,tag> bq;
   return
        binary.amin < qmin &&
        internal::op_first(binary.a, kip_etd, qmin, q, insub)
      ? binary.bmin < q    &&
        internal::op_first(binary.b, kip_etd, real(q),bq, insub)
           ? (q=bq).reverse(binary.ina), true   // b wins
           : (q    .reverse(binary.inb), true)  // a only
      : binary.bmin < qmin &&
        internal::op_first(binary.b, kip_etd, qmin, q, insub)
           ?  q    .reverse(binary.ina), true   // b only
           :  false  // always (up to qmin) the same status
   ;
} kip_end



// inall
kip_inall(kipxor)
{
   afew<inq<real,tag>> aq;
   if (!(binary.amin < qmin &&
         internal::op_all(binary.a, kip_etd, qmin,aq,   insub)))
      return
         binary.bmin < qmin &&
         internal::op_all(binary.b, kip_etd, qmin,ints, insub) &&
         ints.reverse(binary.ina);

   afew<inq<real,tag>> bq;
   if (!(binary.bmin < qmin &&
         internal::op_all(binary.b, kip_etd, qmin,bq,   insub)))
      return ints.assign(aq).reverse(binary.inb);

   const size_t anum = aq.size();  size_t an = 0;  bool ina = binary.ina;
   const size_t bnum = bq.size();  size_t bn = 0;  bool inb = binary.inb;

   ints.reset();
   while (an < anum || bn < bnum)
      if (an < anum && (bn >= bnum || aq[an].q < bq[bn].q))
         ina = !ina, ints.push(aq[an++].reverse(inb));
      else
         inb = !inb, ints.push(bq[bn++].reverse(ina));
   return true;
} kip_end

}



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#define   kip_brief xor
#include "kip-macro-io-binary.h"
