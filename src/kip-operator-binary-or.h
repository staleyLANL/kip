
#pragma once

namespace kip {

// -----------------------------------------------------------------------------
// kipor
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class kipor : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::binary;
   kip_functions(kipor);

   inline point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   inline void propagate_base(const bool force = false) const
      { internal::propagate_binary(*this,force); }


   // kipor()
   inline explicit kipor(void) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipor);
      binary.a = NULL;
      binary.b = NULL;
   }


   // kipor(a,b[,base]), a and b = pointers
   inline explicit kipor(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b
   ) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipor);
      binary.a = _a;
      binary.b = _b;
   }

   inline explicit kipor(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      kip_counter_ctor(kipor);
      binary.a = _a;
      binary.b = _b;
   }


   // kipor(a,b[,base]), a and b = references
   inline explicit kipor(
      const shape<real,tag> &a,
      const shape<real,tag> &b
   ) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipor);
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }

   inline explicit kipor(
      const shape<real,tag> &a,
      const shape<real,tag> &b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      kip_counter_ctor(kipor);
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }


   // kipor(kipor)
   inline kipor(const kipor &from) :
      shape<real,tag>(from)
   {
      kip_counter_ctor(kipor);
      binary.a = from.binary.a ? from.binary.a->duplicate() : NULL;
      binary.b = from.binary.b ? from.binary.b->duplicate() : NULL;
   }

   // kipor = kipor
   inline kipor &operator=(const kipor &from)
   {
      this->shape<real,tag>::operator=(from);
      delete binary.a; binary.a=from.binary.a ? from.binary.a->duplicate():NULL;
      delete binary.b; binary.b=from.binary.b ? from.binary.b->duplicate():NULL;
      return *this;
   }


   // destructor
   inline ~kipor(void)
   {
      kip_counter_dtor(kipor);
      delete binary.a;
      delete binary.b;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(kipor)
{
   // process operands
   binary.a->is_operand = true;
   binary.amin = binary.a->process(eyeball,light,engine,vars);
   if_kip_assert(binary.amin >= 0);

   binary.b->is_operand = true;
   binary.bmin = binary.b->process(eyeball,light,engine,vars);
   if_kip_assert(binary.bmin >= 0);

   // The logical-or operator is reflexive, so we can swap its operands.
   // Putting the operand with the *smallest* minimum first tends to speed
   // up the operator.
   if (binary.bmin < binary.amin) {
      std::swap(binary.a, binary.b);
      std::swap(binary.amin, binary.bmin);
   }

   binary.ina = binary.a->interior;
   binary.inb = binary.b->interior;

   interior = binary.ina || binary.inb;

   // minimum
   return op::abs(
      binary.ina
    ? binary.inb
    ? op::max(binary.amin,binary.bmin)  // in both - use furthest exit
    : binary.amin  // in a only - use a's exit
    : binary.inb
    ? binary.bmin  // in b only - use b's exit
    : op::min(binary.amin,binary.bmin)  // in neither - use closest entry
   );
} kip_end



// aabb
kip_aabb(kipor)
{
   return internal::bound_combine(
      binary.a->aabb(),
      binary.b->aabb(), internal::op_leq()
   );
} kip_end



// dry
kip_dry(kipor)
{
   return binary.a->dry(seg) && binary.b->dry(seg);
} kip_end



// check
kip_check(kipor)
{
   // written the following way so that "a" is checked before "b"
   diagnostic_t d = internal::check_operand("or",binary.a,"a");
   return op::min(d,internal::check_operand("or",binary.b,"b"));
} kip_end



// random
kip_random(kipor)
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



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(kipor)
{
   if (!interior) {
      // the overall status changes (from outside to inside) at the point
      // of first intersection with either object's boundary
      inq<real,tag> bq;
      return
           binary.amin < qmin &&
           internal::op_first(binary.a, kip_etd, qmin, q, insub)
         ? binary.bmin < q    &&
           internal::op_first(binary.b, kip_etd, real(q),bq, insub)
              ? q=bq, // b wins
                true
              : true  // a only
         : binary.bmin < qmin &&
           internal::op_first(binary.b, kip_etd, qmin, q, insub)
              // b only, or stays outside
      ;
   }

   // INSIDE...
   // the overall status changes (from inside to outside) at the point
   // after which we're outside both objects

   afew< inq<real,tag> > aq;
   if (binary.amin >= qmin ||
      !internal::op_all(binary.a, kip_etd, qmin,aq, insub))
      return
        !binary.ina && binary.bmin < qmin &&
         internal::op_first(binary.b, kip_etd, qmin,q, insub);

   afew< inq<real,tag> > bq;
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
      else if (!ina && bn < bnum) { bn++, is_a = false; break; }
      else return false;
   while (ina || inb);

   return q = is_a ? aq[an-1] : bq[bn-1], true;
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(kipor)
{
   afew< inq<real,tag> > aq;
   if (!(binary.amin < qmin &&
         internal::op_all(binary.a, kip_etd, qmin,aq,   insub)))
      return
        !binary.ina && binary.bmin < qmin &&
         internal::op_all(binary.b, kip_etd, qmin,ints, insub);

   afew< inq<real,tag> > bq;
   if (!(binary.bmin < qmin &&
         internal::op_all(binary.b, kip_etd, qmin,bq,   insub)))
      return !binary.inb && ints.assign(aq);

   const size_t anum=aq.size();  size_t an=0;  bool ina=binary.ina, in=interior;
   const size_t bnum=bq.size();  size_t bn=0;  bool inb=binary.inb, is_a;

   for (ints.reset();;) {
      if (an < anum && bn < bnum) is_a = aq[an].q < bq[bn].q;
      else if (!inb && an < anum) is_a = true;
      else if (!ina && bn < bnum) is_a = false;
      else break;

      if (is_a) an++, ina = !ina;
      else      bn++, inb = !inb;

      if ((ina || inb) != in)  // status changed
         ints.push(is_a ? aq[an-1] : bq[bn-1]), in = !in;
   }

   return ints.size() > 0;
} kip_end

}



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#define   kip_brief or
#include "kip-macro-io-binary.h"