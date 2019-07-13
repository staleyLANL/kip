
#pragma once

// -----------------------------------------------------------------------------
// kipand
// -----------------------------------------------------------------------------

template<class real, class tag>
class kipand : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::binary;
   kip_functions(kipand);

   inline point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   inline void propagate_base(const bool force = false) const
      { internal::propagate_binary(*this,force); }


   // kipand()
   inline explicit kipand() :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipand);
      binary.a = NULL;
      binary.b = NULL;
   }


   // kipand(a,b[,tag]), a and b = pointers
   inline explicit kipand(
      const shape<real,tag> *const a,
      const shape<real,tag> *const b
   ) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipand);
      binary.a = a;
      binary.b = b;
   }

   inline explicit kipand(
      const shape<real,tag> *const a,
      const shape<real,tag> *const b, const tag &_tag
   ) :
      shape<real,tag>(this,_tag)
   {
      kip_counter_ctor(kipand);
      binary.a = a;
      binary.b = b;
   }


   // kipand(a,b[,tag]), a and b = references
   inline explicit kipand(
      const shape<real,tag> &a,
      const shape<real,tag> &b
   ) :
      shape<real,tag>(this)
   {
      kip_counter_ctor(kipand);
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }

   inline explicit kipand(
      const shape<real,tag> &a,
      const shape<real,tag> &b, const tag &_tag
   ) :
      shape<real,tag>(this,_tag)
   {
      kip_counter_ctor(kipand);
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }


   // kipand(kipand)
   inline kipand(const kipand &from) :
      shape<real,tag>(from)
   {
      kip_counter_ctor(kipand);
      binary.a = from.binary.a ? from.binary.a->duplicate() : NULL;
      binary.b = from.binary.b ? from.binary.b->duplicate() : NULL;
   }

   // kipand = kipand
   inline kipand &operator=(const kipand &from)
   {
      this->shape<real,tag>::operator=(from);
      delete binary.a; binary.a=from.binary.a ? from.binary.a->duplicate():NULL;
      delete binary.b; binary.b=from.binary.b ? from.binary.b->duplicate():NULL;
      return *this;
   }


   // destructor
   inline ~kipand()
   {
      kip_counter_dtor(kipand);
      delete binary.a;
      delete binary.b;
   }
};



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// process
kip_process(kipand)
{
   // process operands
   binary.a->is_operand = true;
   binary.amin = binary.a->process(eyeball,light,engine,vars);
   if_kip_assert(binary.amin >= 0);

   binary.b->is_operand = true;
   binary.bmin = binary.b->process(eyeball,light,engine,vars);
   if_kip_assert(binary.bmin >= 0);

   // The logical-and operator is reflexive, so we can swap its operands.
   // Putting the operand with the *largest* minimum first tends to speed
   // up the operator.
   if (binary.bmin > binary.amin) {
      std::swap(binary.a, binary.b);
      std::swap(binary.amin, binary.bmin);
   }

   binary.ina = binary.a->interior;
   binary.inb = binary.b->interior;

   interior = binary.ina && binary.inb;

   // Although other primitives are designed to have non-negative minima,
   // the code seems to run faster if we specifically use abs() here.
   return std::abs(
      binary.ina
    ? binary.inb
    ? op::min(binary.amin,binary.bmin)  // in both
    : binary.bmin  // in a only
    : binary.inb
    ? binary.amin  // in b only
    : op::max(binary.amin,binary.bmin)  // in neither
   );
} kip_end



// aabb
kip_aabb(kipand)
{
   const bbox<real> a = binary.a->aabb();
   const bbox<real> b = binary.b->aabb();

   // By construction, if !a.valid() or !b.valid(),
   // then the return value is !valid()
   return bbox<real>(
      false, op::max(a.x.min, b.x.min),   op::min(a.x.max, b.x.max), false,
      false, op::max(a.y.min, b.y.min),   op::min(a.y.max, b.y.max), false,
      false, op::max(a.z.min, b.z.min),   op::min(a.z.max, b.z.max), false
   );
} kip_end



// dry
kip_dry(kipand)
{
   // Could be stronger. The intersection may still be dry, even
   // if neither operand is dry. But that's difficult to determine.
   return binary.a->dry(seg) || binary.b->dry(seg);
} kip_end



// check
kip_check(kipand)
{
   // written the following way so that "a" is checked before "b"
   diagnostic_t d = internal::check_operand("and",binary.a,"a");
   return op::min(d,internal::check_operand("and",binary.b,"b"));
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(kipand)
{
   // INSIDE...
   if (interior) {
      // the overall status changes (from inside to outside) at the point
      // of first intersection with either object's boundary
      inq<real,tag> bq;
      return
           internal::op_first(binary.a, kip_etd, qmin, q, insub)
         ? internal::op_first(binary.b, kip_etd, real(q),bq, insub)
              ? q=bq, // b wins
                true
              : true  // a only
         // b only, or stays in
         : internal::op_first(binary.b, kip_etd, qmin, q, insub)
      ;
   }

   // OUTSIDE...
   // the overall status changes (from outside to inside) at the point
   // after which we're inside both objects

   afew<inq<real,tag>> aq;
   if (!internal::op_all(binary.a, kip_etd, qmin,aq, insub))
      return binary.ina &&
             internal::op_first(binary.b, kip_etd, qmin,q, insub);

   afew<inq<real,tag>> bq;
   if (!internal::op_all(binary.b, kip_etd, qmin,bq, insub))
      return binary.inb && (q = aq[0], true);

   // search for the relevant point
   const size_t anum = aq.size();  size_t an = 0;  bool ina = binary.ina;
   const size_t bnum = bq.size();  size_t bn = 0;  bool inb = binary.inb, is_a;

   do
      if (an < anum && bn < bnum)
         if (aq[an].q < bq[bn].q)
            an++, ina = !ina, is_a = true;
         else
            bn++, inb = !inb, is_a = false;
      else if (inb && an < anum) { an++, is_a = true;  break; }
      else if (ina && bn < bnum) { bn++, is_a = false; break; }
      else return false;
   while (!(ina && inb));

   return q = is_a ? aq[an-1] : bq[bn-1], true;
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(kipand)
{
   afew<inq<real,tag>> aq;
   if (!internal::op_all(binary.a, kip_etd, qmin,aq, insub))
      return binary.ina &&
             internal::op_all(binary.b, kip_etd, qmin,ints, insub);

   afew<inq<real,tag>> bq;
   if (!internal::op_all(binary.b, kip_etd, qmin,bq, insub))
      return binary.inb && ints.assign(aq);

   const size_t anum=aq.size();  size_t an=0;  bool ina=binary.ina, in=interior;
   const size_t bnum=bq.size();  size_t bn=0;  bool inb=binary.inb, is_a;

   for (ints.reset();;) {
      if (an < anum && bn < bnum) is_a = aq[an].q < bq[bn].q;
      else if (inb  && an < anum) is_a = true;
      else if (ina  && bn < bnum) is_a = false;
      else break;

      if (is_a) an++, ina = !ina;
      else      bn++, inb = !inb;

      if ((ina && inb) != in)  // status changed
         ints.push(is_a ? aq[an-1] : bq[bn-1]), in = !in;
   }

   return ints.size() > 0;
} kip_end



// -----------------------------------------------------------------------------
// random
// -----------------------------------------------------------------------------

kip_random(kipand)
{
   shape<real,tag> *aptr;  static bool justspheres = false;
   shape<real,tag> *bptr;  static point<real> loc;

   // Remark, 2019-07-12. This line used to read:
   //    if (justspheres || random<real>() < 0.0) {
   // with the old random<real>() function, which returned a random number
   // in the range [0,1]. I've replaced it with our equivalent new random
   // function, random_unit<real>(), but how did it makes sense in the
   // original to have the < 0.0 test, which would always fail?

   if (justspheres || random_unit<real>() < 0.0) {
      sphere<real,tag> *const a = new sphere<real,tag>;
      sphere<real,tag> *const b = new sphere<real,tag>;

      random(*a);  a->r += a->r;  aptr = a;
      random(*b);  b->r += b->r;  bptr = b;

      if (justspheres) {
         a->c = loc + point<real>(
            real(0.2)*random_unit<real>(),
            real(0.2)*random_unit<real>(),
            real(0.2)*random_unit<real>()
         );
         b->c = loc + point<real>(
            real(0.2)*random_unit<real>(),
            real(0.2)*random_unit<real>(),
            real(0.2)*random_unit<real>()
         );
      } else {
         const real r = a->r + b->r;
         b->c.x = a->c.x + random_half<real>()*r;
         b->c.y = a->c.y + random_half<real>()*r;
         b->c.z = a->c.z + random_half<real>()*r;
      }

   } else {
      justspheres = true;
      random_full(loc);
      kipand<real,tag> *const a = new kipand<real,tag>; random(*a); aptr = a;
      kipand<real,tag> *const b = new kipand<real,tag>; random(*b); bptr = b;
      justspheres = false;
   }

   // operands
   obj.binary.a = aptr;
   obj.binary.b = bptr;

   // tag
   random(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#define   kip_brief and
#include "kip-macro-io-binary.h"
