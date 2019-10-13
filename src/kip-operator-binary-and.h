
// -----------------------------------------------------------------------------
// kipand
// -----------------------------------------------------------------------------

template<class real, class tag>
class kipand : public shape<real,tag> {
   using shape<real,tag>::interior;

public:
   using shape<real,tag>::binary;
   kip_functions(kipand);

   point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   void propagate_base(const bool force = false) const
      { detail::propagate_binary(*this,force); }


   // kipand()
   explicit kipand() :
      shape<real,tag>(this)
   {
      binary.a = nullptr;
      binary.b = nullptr;
   }


   // kipand(a,b[,tag]), a and b = pointers
   explicit kipand(
      const shape<real,tag> *const a,
      const shape<real,tag> *const b
   ) :
      shape<real,tag>(this)
   {
      binary.a = a;
      binary.b = b;
   }

   explicit kipand(
      const shape<real,tag> *const a,
      const shape<real,tag> *const b, const tag &_tag
   ) :
      shape<real,tag>(this,_tag)
   {
      binary.a = a;
      binary.b = b;
   }


   // kipand(a,b[,tag]), a and b = references
   explicit kipand(
      const shape<real,tag> &a,
      const shape<real,tag> &b
   ) :
      shape<real,tag>(this)
   {
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }

   explicit kipand(
      const shape<real,tag> &a,
      const shape<real,tag> &b, const tag &_tag
   ) :
      shape<real,tag>(this,_tag)
   {
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }


   // kipand(kipand)
   kipand(const kipand &from) :
      shape<real,tag>(from)
   {
      binary.a = from.binary.a ? from.binary.a->duplicate() : nullptr;
      binary.b = from.binary.b ? from.binary.b->duplicate() : nullptr;
   }

   // kipand = kipand
   kipand &operator=(const kipand &from)
   {
      this->shape<real,tag>::operator=(from);
      delete binary.a;
      binary.a = from.binary.a ? from.binary.a->duplicate() : nullptr;
      delete binary.b;
      binary.b = from.binary.b ? from.binary.b->duplicate() : nullptr;
      return *this;
   }


   // destructor
  ~kipand()
   {
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
   kip_assert(binary.amin >= 0);

   binary.b->is_operand = true;
   binary.bmin = binary.b->process(eyeball,light,engine,vars);
   kip_assert(binary.bmin >= 0);

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
   diagnostic d = detail::check_operand("and",binary.a,"a");
   return op::min(d,detail::check_operand("and",binary.b,"b"));
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
           detail::op_first(binary.a, kip_etd, qmin, q, insub)
         ? detail::op_first(binary.b, kip_etd, real(q),bq, insub)
              ? q=bq, // b wins
                true
              : true  // a only
         // b only, or stays in
         : detail::op_first(binary.b, kip_etd, qmin, q, insub)
      ;
   }

   // OUTSIDE...
   // the overall status changes (from outside to inside) at the point
   // after which we're inside both objects

   afew<inq<real,tag>> aq;
   if (!detail::op_all(binary.a, kip_etd, qmin,aq, insub))
      return binary.ina &&
             detail::op_first(binary.b, kip_etd, qmin,q, insub);

   afew<inq<real,tag>> bq;
   if (!detail::op_all(binary.b, kip_etd, qmin,bq, insub))
      return binary.inb && (q = aq[0], true);

   // search for the relevant point
   const ulong anum = aq.size();  ulong an = 0;  bool ina = binary.ina;
   const ulong bnum = bq.size();  ulong bn = 0;  bool inb = binary.inb, is_a;

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
   if (!detail::op_all(binary.a, kip_etd, qmin,aq, insub))
      return binary.ina &&
             detail::op_all(binary.b, kip_etd, qmin,ints, insub);

   afew<inq<real,tag>> bq;
   if (!detail::op_all(binary.b, kip_etd, qmin,bq, insub))
      return binary.inb && ints.assign(aq);

   const ulong anum=aq.size();  ulong an=0;  bool ina=binary.ina, in=interior;
   const ulong bnum=bq.size();  ulong bn=0;  bool inb=binary.inb, is_a;

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
// randomize
// -----------------------------------------------------------------------------

kip_randomize(kipand)
{
   // For a random [kip]and, we'll do an and(sphere,sphere).

   // random "starting point"
   point<real> cent;
   random_full(cent);

   // random "difference"
   point<real> diff;
   random_full(diff);
   diff *= real(0.05); // spheres will be twice this distance apart

   // spheres
   sphere<real,tag> *const one = new sphere<real,tag>; // delete in ~kipand()
   one->c = cent - diff;
   one->r = real(0.11) + real(0.05)*random_unit<real>();
   sphere<real,tag> *const two = new sphere<real,tag>; // delete in ~kipand()
   two->c = cent + diff;
   two->r = real(0.11) + real(0.05)*random_unit<real>();

   // and: operands
   obj.binary.a = one;
   obj.binary.b = two;

   // color over the nested objects, in order to give the overall object
   // a consistent color
   randomize(obj.base());
   obj.baseset = true;

   // done
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#define   kip_brief and
#include "kip-macro-io-binary.h"
