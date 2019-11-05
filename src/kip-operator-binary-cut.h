
// -----------------------------------------------------------------------------
// kipcut: A cut B == A and (not B)
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class kipcut : public shape<real,tag> {
public:
   using shape<real,tag>::binary;
   kip_functions(kipcut);

   point<real> back(const point<real> &from) const
      { assert(false);  return from; }

   void propagate_base(const bool force = false) const
      { detail::propagate_binary(*this,force); }


   // kipcut()
   explicit kipcut() :
      shape<real,tag>(this)
   {
      binary.a = nullptr;
      binary.b = nullptr;
   }


   // kipcut(a,b[,base]), a and b = pointers
   explicit kipcut(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b
   ) :
      shape<real,tag>(this)
   {
      binary.a = _a;
      binary.b = _b;
   }

   explicit kipcut(
      const shape<real,tag> *const _a,
      const shape<real,tag> *const _b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      binary.a = _a;
      binary.b = _b;
   }


   // kipcut(a,b[,base]), a and b = references
   explicit kipcut(
      const shape<real,tag> &a,
      const shape<real,tag> &b
   ) :
      shape<real,tag>(this)
   {
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }

   explicit kipcut(
      const shape<real,tag> &a,
      const shape<real,tag> &b, const tag &thebase
   ) :
      shape<real,tag>(this,thebase)
   {
      binary.a = a.duplicate();
      binary.b = b.duplicate();
   }


   // kipcut(kipcut)
   kipcut(const kipcut &from) :
      shape<real,tag>(from)
   {
      binary.a = from.binary.a ? from.binary.a->duplicate() : nullptr;
      binary.b = from.binary.b ? from.binary.b->duplicate() : nullptr;
   }

   // kipcut = kipcut
   kipcut &operator=(const kipcut &from)
   {
      this->shape<real,tag>::operator=(from);
      delete binary.a;
      binary.a = from.binary.a ? from.binary.a->duplicate() : nullptr;
      delete binary.b;
      binary.b = from.binary.b ? from.binary.b->duplicate() : nullptr;
      return *this;
   }


   // destructor
  ~kipcut()
   {
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
   kip_assert(binary.amin >= 0);

   binary.b->is_operand = true;
   binary.bmin = binary.b->process(eyeball,light,engine,vars);
   kip_assert(binary.bmin >= 0);

   // The logical-cut operator is *not* reflexive, so we can't swap
   // its operands (as we can with certain other operators).

   binary.ina = binary.a->interior;
   binary.inb = binary.b->interior;

   this->interior = binary.ina && !binary.inb;

   // The following is equivalent to "and" with !inb. The effects are to swap
   // the "both" and "a" cases, and the "b" and "neither" cases.
   return std::abs(
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

   return bbox<real>(
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
   diagnostic d = detail::check_operand("cut",binary.a,"a");
   return op::min(d,detail::check_operand("cut",binary.b,"b"));
} kip_end



// randomize
kip_randomize(kipcut)
{
   // For a random [kip]cut, we'll do a cut(cone,sphere).

   // cone
   cone  <real,tag> *const one = new cone  <real,tag>; // delete in ~kipcut()
   randomize(*one);

   // sphere
   sphere<real,tag> *const two = new sphere<real,tag>; // delete in ~kipcut()
   point<real> tweak; const real fac = random_unit<real>();
   two->c = one->a + fac*(one->b - one->a) + real(0.1)*random_full(tweak);
   two->r = one->r*real(3.0)*random_unit<real>();

   // cut: operands
   obj.binary.a = one;
   obj.binary.b = two;

   // fixme 2019-07-13. Something may be wrong with kipcut's color selection.
   // I had to set obj.base() = one->base(), rather than just randomizing
   // obj.base() as I did in kipand, in order to get something other than
   // two-colored objects, even with obj.baseset = true. Look into this.

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

kip_infirst(kipcut)
{
   // INSIDE...
   if (this->interior) {
      // the overall status changes (from inside to outside) at the point
      // of first intersection with either object's boundary
      inq<real,tag> bq;
      return
           binary.amin < qmin &&
           detail::op_first(binary.a, etd, insub, qmin, q)
         ? binary.bmin < q    &&
           detail::op_first(binary.b, etd, insub, real(q), bq)
              ? (q=bq).reverse(), // b wins
                 true
              :  true             // a only
         : binary.bmin < qmin &&
           detail::op_first(binary.b, etd, insub, qmin, q)
              ? (q   ).reverse(), // b only
                 true
              :  false  // always (up to qmin) inside
      ;
   }

   // OUTSIDE...
   // the overall status changes (from outside to inside) at the point
   // after which we're inside "a" and outside "b"

   afew<real,tag> aq;
   if (binary.amin >= qmin ||
      !detail::op_all(binary.a, etd, insub, qmin, aq))
      return
         binary.ina &&
         binary.bmin < qmin &&
         detail::op_first(binary.b, etd, insub, qmin, q) &&
        (q.reverse(), true);

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
   afew<real,tag> aq;
   if (!(binary.amin < qmin &&
         detail::op_all(binary.a, etd, insub, qmin, aq)))
      return
         binary.ina && binary.bmin < qmin &&
         detail::op_all(binary.b, etd, insub, qmin, ints) &&
         ints.reverse();

   afew<real,tag> bq;
   if (!(binary.bmin < qmin &&
         detail::op_all(binary.b, etd, insub, qmin, bq)))
      return !binary.inb && ints.assign(aq);

   const ulong anum = aq.size();  ulong an = 0;  bool ina = binary.ina;
   const ulong bnum = bq.size();  ulong bn = 0;  bool inb = binary.inb;
   bool in = this->interior, is_a;

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



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#define   kip_brief cut
#include "kip-macro-io-binary.h"
