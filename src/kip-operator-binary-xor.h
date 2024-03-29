
// -----------------------------------------------------------------------------
// kipxor
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class kipxor : public shape<real,tag> {
public:
   using shape<real,tag>::binary;
   kip_functions(kipxor);

   point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   void propagate_base(const bool force = false) const
      { detail::propagate_binary(*this,force); }


   // kipxor()
   explicit kipxor() :
      shape<real,tag>(this)
   {
      binary.a = nullptr;
      binary.b = nullptr;
   }


   // kipxor(a,b[,base]), a and b = pointers
   explicit kipxor(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b
   ) :
      shape<real,tag>(this)
   {
      binary.a = _a;
      binary.b = _b;
   }

   explicit kipxor(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      binary.a = _a;
      binary.b = _b;
   }


   // kipxor(a,b[,base]), a and b = references
   explicit kipxor(
      const shape<real,tag> &a,
      const shape<real,tag> &b
   ) :
      shape<real,tag>(this)
   {
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }

   explicit kipxor(
      const shape<real,tag> &a,
      const shape<real,tag> &b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }


   // kipxor(kipxor)
   kipxor(const kipxor &from) :
      shape<real,tag>(from)
   {
      binary.a = from.binary.a ? from.binary.a->duplicate() : nullptr;
      binary.b = from.binary.b ? from.binary.b->duplicate() : nullptr;
   }

   // kipxor = kipxor
   kipxor &operator=(const kipxor &from)
   {
      this->shape<real,tag>::operator=(from);
      delete binary.a;
      binary.a = from.binary.a ? from.binary.a->duplicate() : nullptr;
      delete binary.b;
      binary.b = from.binary.b ? from.binary.b->duplicate() : nullptr;
      return *this;
   }


   // destructor
   ~kipxor()
   {
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
   binary.a->isoperand = true;
   binary.amin = binary.a->process(eyeball,light,engine,vars);
   kip_assert(binary.amin >= 0);

   binary.b->isoperand = true;
   binary.bmin = binary.b->process(eyeball,light,engine,vars);
   kip_assert(binary.bmin >= 0);

   // The logical-xor operator is reflexive, so we can swap its operands.
   // Putting the operand with the *smallest* minimum first tends to speed
   // up the operator.
   if (binary.bmin < binary.amin) {
      std::swap(binary.a, binary.b);
      std::swap(binary.amin, binary.bmin);
   }

   binary.ina = binary.a->interior;
   binary.inb = binary.b->interior;

   this->interior = binary.ina != binary.inb;

   // Technically should be min(abs,abs), but minima are presumably positive.
   return std::abs(op::min(binary.amin,binary.bmin));
} kip_end



// aabb
kip_aabb(kipxor)
{
   // Almost the same as for or, as exclusiveness can only remove an "inner"
   // portion. However, to ensure tightness we want strict inequalities for
   // bound comparisons. Otherwise, possible exact cancellations could mean
   // an or-tight bound is not xor-tight.

   return detail::bound_combine(
      binary.a->aabb(),
      binary.b->aabb(), detail::op_less()
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
   diagnostic d = detail::check_operand("xor",binary.a,"a");
   return op::min(d,detail::check_operand("xor",binary.b,"b"));
} kip_end



// randomize
kip_randomize(kipxor)
{
   // For a random [kip]xor, we'll do an xor(cone,sphere).

   // cone
   cone  <real,tag> *const one = new cone  <real,tag>; // delete in ~kipxor()
   randomize(*one);

   // sphere
   sphere<real,tag> *const two = new sphere<real,tag>; // delete in ~kipxor()
   point<real> tweak; const real fac = random_unit<real>();
   two->c = one->a + fac*(one->b - one->a) + real(0.1)*random_full(tweak);
   two->r = one->r*random_unit<real>();

   // xor: operands
   obj.binary.a = one;
   obj.binary.b = two;

   // fixme 2019-07-14. Something may be wrong with kipxor's color selection.
   // See similar remark re: kipcut.

   // color over the nested objects, in order to give the overall object
   // a consistent color
   // randomize(obj.base());
   obj.base() = one->base();
   obj.baseset = true;

   // done
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
        detail::op_first(binary.a, etd, insub, qmin, q)
      ? binary.bmin < q    &&
        detail::op_first(binary.b, etd, insub, real(q), bq)
           ? (q=bq).reverse(binary.ina), true   // b wins
           : (q    .reverse(binary.inb), true)  // a only
      : binary.bmin < qmin &&
        detail::op_first(binary.b, etd, insub, qmin, q)
           ?  q    .reverse(binary.ina), true   // b only
           :  false  // always (up to qmin) the same status
   ;
} kip_end



// inall
kip_inall(kipxor)
{
   afew<real,tag> aq;
   if (!(binary.amin < qmin &&
         detail::op_all(binary.a, etd, insub, qmin, aq)))
      return
         binary.bmin < qmin &&
         detail::op_all(binary.b, etd, insub, qmin, ints) &&
         ints.reverse(binary.ina);

   afew<real,tag> bq;
   if (!(binary.bmin < qmin &&
         detail::op_all(binary.b, etd, insub, qmin, bq)))
      return ints.assign(aq).reverse(binary.inb);

   const ulong anum = aq.size();  ulong an = 0;  bool ina = binary.ina;
   const ulong bnum = bq.size();  ulong bn = 0;  bool inb = binary.inb;

   ints.reset();
   while (an < anum || bn < bnum)
      if (an < anum && (bn >= bnum || aq[an].q < bq[bn].q))
         ina = !ina, ints.push(aq[an++].reverse(inb));
      else
         inb = !inb, ints.push(bq[bn++].reverse(ina));
   return true;
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#define   kip_brief xor
#include "kip-macro-io-binary.h"
