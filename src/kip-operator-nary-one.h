
// -----------------------------------------------------------------------------
// one
// -----------------------------------------------------------------------------

template<class real = defaults::real, class tag = defaults::base>
class one : public shape<real,tag> {
#define   kip_class one
#include "kip-macro-nary.h"
};



// -----------------------------------------------------------------------------
// process
// -----------------------------------------------------------------------------

kip_process(one)
{
   // process operands
   vec_t &vec = kip_data.vec();
   kip_data.nop = vec.size();
   std::vector<minimum_and_ptr<real,shape<real,tag>>> min_and_op(kip_data.nop);

   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      vec[i].op->isoperand = true;
      min_and_op[i].min = (min_and_op[i].shape = vec[i].op)->
         process(eyeball,light,engine,vars);
      kip_assert(min_and_op[i].min >= 0);
   }

   // The logical-one operator is mutually reflexive, so we can arbitrarily
   // rearrange its operands. Putting operands with smaller minima first tends
   // to speed up the operator.
   std::sort(min_and_op.begin(), min_and_op.end(), detail::less<real,tag>());

   // Bookkeeping
   nary.total_in = 0;
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      vec[i].min = min_and_op[i].min;
      if ((vec[i].in=(vec[i].op=min_and_op[i].shape)->interior))  // =, not ==
         nary.total_in++;
   }
   this->interior = kip_data.nop ? nary.total_in == 1 : true;

   // minimum
   if (kip_data.nop == 0) return 0;

   if (nary.total_in < 2) {
      real rv = min_and_op[0].min;
      for (ulong i = 1;  i < kip_data.nop;  ++i)
         rv = op::min(rv, real(min_and_op[i].min));
      return rv;
   }

   std::vector<real> minin(nary.total_in);  ulong n = 0;
   for (ulong i = 0;  i < kip_data.nop;  ++i)
      if (vec[i].in)
         minin[n++] = min_and_op[i].min;

   std::sort(minin.begin(), minin.end());
   return minin[nary.total_in-2];  // in total_in, require exit from total_in-1
} kip_end



// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// aabb
kip_aabb(one)
{
   bbox<real> rv(false,1,0,false, false,1,0,false, false,1,0,false);

   const vec_t &vec = kip_data.vec();
   kip_data.nop = vec.size();

   for (ulong i = 0;  i < kip_data.nop;  ++i)
      rv = detail::bound_combine(rv, vec[i].op->aabb(), detail::op_less());
   return rv;
} kip_end



// dry
kip_dry(one)
{
   const vec_t &vec = kip_data.vec();
   for (ulong i = 0;  i < kip_data.nop;  ++i)
      if (!vec[i].op->dry(seg)) return false;  // makes the whole thing non-dry
   return kip_data.nop > 0;  // false if no operands, true otherwise
} kip_end



// randomize
kip_randomize(one)
{
   const ulong nop = 4;

   point<real> loc;
   random_full(loc);
   obj.clear();
   obj.nary.vec().reserve(nop);

   for (ulong i = 0;  i < nop;  ++i) {
      sphere<real,tag> *const ptr = new sphere<real,tag>;
      randomize(*ptr);
      obj.nary.push().op = ptr;

      ptr->c = loc + point<real>(
         real(0.2)*random_unit<real>(),
         real(0.2)*random_unit<real>(),
         real(0.2)*random_unit<real>()
      );
   }

   // base
   randomize(obj.base());  obj.baseset = true;
   return obj;
} kip_end



// -----------------------------------------------------------------------------
// infirst
// -----------------------------------------------------------------------------

kip_infirst(one)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop == 0) return false;  // no operands --> no intersection

   // Inside 0 or 1 operands...
   if (nary.total_in < 2) {
      // Find the first hit with anything. It represents one of the following,
      // each of which amounts to a border of the one() object:
      //    in 0 --> in 1
      //    in 1 --> in 0
      //    in 1 --> in 2
      inq<real,tag> qtmp;  q = qmin;
      for (ulong i = 0;  i < kip_data.nop && vec[i].min < q;  ++i)
         if (detail::op_first(vec[i].op, etd, insub, real(q), qtmp))
            q = qtmp;
      return q < qmin;
   }

   // Inside 2 or more operands...
   const ulong num_operand = kip_data.nop;
   using per_operand = afew_book<real,tag>;
#include "kip-macro-workspace.h"

   // compute operand information
   bool found = false;
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      operand[i].size =
         vec[i].min < qmin &&
         detail::op_all(vec[i].op, etd, insub, qmin, operand[i].points)
         ? (found = true, operand[i].points.size())
         :  0;
      operand[i].next = 0, operand[i].in_now = vec[i].in;
   }
   if (!found) return kip_less, false;

   // search for the first relevant intersection.
   ulong num_in = nary.total_in;

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (ulong i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size &&
             (current == -1 || operand[i].q() < operand[current].q()))
            current = int(i);
      if (current == -1) return kip_less, false;

      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (num_in++ < 2)
            return q = operand[current].q().reverse(num_in==2), kip_less, true;
      } else
         // leaving current operand
         if (--num_in < 2)
            return q = operand[current].q().reverse(num_in==1), kip_less, true;
   }

   return kip_less, false;  // unreachable; presence silences g++ warnings
} kip_end



// -----------------------------------------------------------------------------
// inall
// -----------------------------------------------------------------------------

kip_inall(one)
{
   const vec_t &vec = kip_data.vec();
   if (kip_data.nop == 0) return false;  // no operands --> no intersection

   const ulong num_operand = kip_data.nop;
   using per_operand = afew_book<real,tag>;
#include "kip-macro-workspace.h"

   // compute operand information
   bool found = false;
   for (ulong i = 0;  i < kip_data.nop;  ++i) {
      operand[i].size =
         vec[i].min < qmin &&
         detail::op_all(vec[i].op, etd, insub, qmin, operand[i].points)
         ? (found = true, operand[i].points.size())
         :  0;
      operand[i].next = 0, operand[i].in_now = vec[i].in;
   }
   if (!found) return kip_less, false;

   ulong num_in = nary.total_in;
   ints.reset();

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (ulong i = 0;  i < kip_data.nop;  ++i)
         if (operand[i].next < operand[i].size &&
             (current == -1 || operand[i].q() < operand[current].q()))
             current = int(i);
      if (current == -1) return kip_less, ints.size() > 0;

      if ((operand[current].in_now = !operand[current].in_now)) {  // =, not ==
         // entering current operand
         if (num_in++ < 2)
            ints.push(operand[current].q().reverse(num_in==2));
      } else
         // leaving current operand
         if (--num_in < 2)
            ints.push(operand[current].q().reverse(num_in==1));
   }

   return kip_less, false;  // unreachable; presence silences g++ warning
} kip_end



// -----------------------------------------------------------------------------
// i/o
// -----------------------------------------------------------------------------

#include "kip-macro-io-nary.h"
