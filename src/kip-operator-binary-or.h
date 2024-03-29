
// -----------------------------------------------------------------------------
// kipor
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class kipor : public shape<real,tag> {
public:
   using shape<real,tag>::binary;
   kip_functions(kipor);

   point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   void propagate_base(const bool force = false) const
      { detail::propagate_binary(*this,force); }


   // kipor()
   explicit kipor() :
      shape<real,tag>(this)
   {
      binary.a = nullptr;
      binary.b = nullptr;
   }


   // kipor(a,b[,base]), a and b = pointers
   explicit kipor(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b
   ) :
      shape<real,tag>(this)
   {
      binary.a = _a;
      binary.b = _b;
   }

   explicit kipor(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      binary.a = _a;
      binary.b = _b;
   }


   // kipor(a,b[,base]), a and b = references
   explicit kipor(
      const shape<real,tag> &a,
      const shape<real,tag> &b
   ) :
      shape<real,tag>(this)
   {
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }

   explicit kipor(
      const shape<real,tag> &a,
      const shape<real,tag> &b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }


   // kipor(kipor)
   kipor(const kipor &from) :
      shape<real,tag>(from)
   {
      binary.a = from.binary.a ? from.binary.a->duplicate() : nullptr;
      binary.b = from.binary.b ? from.binary.b->duplicate() : nullptr;
   }

   // kipor = kipor
   kipor &operator=(const kipor &from)
   {
      this->shape<real,tag>::operator=(from);
      delete binary.a;
      binary.a = from.binary.a ? from.binary.a->duplicate() : nullptr;
      delete binary.b;
      binary.b = from.binary.b ? from.binary.b->duplicate() : nullptr;
      return *this;
   }


   // destructor
   ~kipor()
   {
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
   binary.a->isoperand = true;
   binary.amin = binary.a->process(eyeball,light,engine,vars);
   kip_assert(binary.amin >= 0);

   binary.b->isoperand = true;
   binary.bmin = binary.b->process(eyeball,light,engine,vars);
   kip_assert(binary.bmin >= 0);

   // The logical-or operator is reflexive, so we can swap its operands.
   // Putting the operand with the *smallest* minimum first tends to speed
   // up the operator.
   if (binary.bmin < binary.amin) {
      std::swap(binary.a, binary.b);
      std::swap(binary.amin, binary.bmin);
   }

   binary.ina = binary.a->interior;
   binary.inb = binary.b->interior;

   this->interior = binary.ina || binary.inb;

   // minimum
   return std::abs(
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
   return detail::bound_combine(
      binary.a->aabb(),
      binary.b->aabb(), detail::op_leq()
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
   diagnostic d = detail::check_operand("or",binary.a,"a");
   return op::min(d,detail::check_operand("or",binary.b,"b"));
} kip_end



// randomize
kip_randomize(kipor)
{
   // For a random [kip]or, we'll do an or(cone,sphere).

   // cone
   cone  <real,tag> *const one = new cone  <real,tag>; // delete in ~kipor()
   randomize(*one);

   // sphere
   sphere<real,tag> *const two = new sphere<real,tag>; // delete in ~kipor()
   point<real> tweak; const real fac = random_unit<real>();
   two->c = one->a + fac*(one->b - one->a) + real(0.1)*random_full(tweak);
   two->r = one->r*random_unit<real>();

   // or: operands
   obj.binary.a = one;
   obj.binary.b = two;

   // fixme 2019-07-14. Something may be wrong with kipor's color selection.
   // See similar remark re: kipcut.

   // color over the nested objects, in order to give the overall object
   // a consistent color
   // randomize(obj.base());
   obj.base() = one->base();
   obj.baseset = true;

   // done
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(kipor)
{
   if (!this->interior) {
      // the overall status changes (from outside to inside) at the point
      // of first intersection with either object's boundary
      inq<real,tag> bq;
      return
           binary.amin < qmin &&
           detail::op_first(binary.a, etd, insub, qmin, q)
         ? binary.bmin < q    &&
           detail::op_first(binary.b, etd, insub, real(q), bq)
              ? q=bq, // b wins
                true
              : true  // a only
         : binary.bmin < qmin &&
           detail::op_first(binary.b, etd, insub, qmin, q)
              // b only, or stays outside
      ;
   }

   // INSIDE...
   // the overall status changes (from inside to outside) at the point
   // after which we're outside both objects

   afew<real,tag> aq;
   if (binary.amin >= qmin || !detail::op_all(binary.a, etd, insub, qmin, aq))
      return
        !binary.ina && binary.bmin < qmin &&
         detail::op_first(binary.b, etd, insub, qmin, q);

   afew<real,tag> bq;
   if (binary.bmin >= qmin ||
      !detail::op_all(binary.b, etd, insub, qmin, bq))
      return !binary.inb && (q = aq[0], true);

   // search for the relevant point
   const ulong anum = aq.size();  ulong an = 0;  bool ina = binary.ina;
   const ulong bnum = bq.size();  ulong bn = 0;  bool inb = binary.inb, is_a;

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
   afew<real,tag> aq;
   if (!(binary.amin < qmin && detail::op_all(binary.a, etd, insub, qmin, aq)))
      return
        !binary.ina && binary.bmin < qmin &&
         detail::op_all(binary.b, etd, insub, qmin, ints);

   afew<real,tag> bq;
   if (!(binary.bmin < qmin && detail::op_all(binary.b, etd, insub, qmin, bq)))
      return !binary.inb && ints.assign(aq);

   const ulong anum = aq.size(); ulong an = 0;
   bool ina = binary.ina, in = this->interior;
   const ulong bnum = bq.size(); ulong bn = 0;
   bool inb = binary.inb, is_a;

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



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#define   kip_brief or
#include "kip-macro-io-binary.h"
