
#pragma once

// -----------------------------------------------------------------------------
// one
// -----------------------------------------------------------------------------

template<class real = default_real_t, class tag = default_tag_t>
class one : public shape<real,tag> {
   using shape<real,tag>::interior;

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
   std::vector<minimum_and_shape<real,tag>> min_and_op(kip_data.nop);

   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      vec[i].op->is_operand = true;
      min_and_op[i].minimum = minimum_t((min_and_op[i].shape = vec[i].op)->
         process(eyeball,light,engine,vars));
      if_kip_assert(min_and_op[i].minimum >= 0);
   }

   // The logical-one operator is mutually reflexive, so we can arbitrarily
   // rearrange its operands. Putting operands with smaller minima first tends
   // to speed up the operator.
   std::sort(min_and_op.begin(), min_and_op.end(), internal::less<real,tag>());

   // Bookkeeping
   nary.total_in = 0;
   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      vec[i].min = min_and_op[i].minimum;
      if ((vec[i].in=(vec[i].op=min_and_op[i].shape)->interior))  // =, not ==
         nary.total_in++;
   }
   interior = kip_data.nop ? nary.total_in == 1 : true;

   // minimum
   if (kip_data.nop == 0) return 0;

   if (nary.total_in < 2) {
      real rv = min_and_op[0].minimum;
      for (size_t i = 1;  i < kip_data.nop;  ++i)
         rv = op::min(rv, real(min_and_op[i].minimum));
      return rv;
   }

   std::vector<real> minin(nary.total_in);  size_t n = 0;
   for (size_t i = 0;  i < kip_data.nop;  ++i)
      if (vec[i].in)
         minin[n++] = min_and_op[i].minimum;

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

   for (size_t i = 0;  i < kip_data.nop;  ++i)
      rv = internal::bound_combine(rv, vec[i].op->aabb(), internal::op_less());
   return rv;
} kip_end



// dry
kip_dry(one)
{
   const vec_t &vec = kip_data.vec();
   for (size_t i = 0;  i < kip_data.nop;  ++i)
      if (!vec[i].op->dry(seg)) return false;  // makes the whole thing non-dry
   return kip_data.nop > 0;  // false if no operands, true otherwise
} kip_end



// randomize
kip_randomize(one)
{
   const size_t nop = 4;

   point<real> loc;
   random_full(loc);
   obj.clear();
   obj.nary.vec().reserve(nop);

   for (size_t i = 0;  i < nop;  ++i) {
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
      for (size_t i = 0;  i < kip_data.nop && vec[i].min < q;  ++i)
         if (internal::op_first(vec[i].op, kip_etd, real(q),qtmp, insub))
            q = qtmp;
      return q < qmin;
   }

   // Inside 2 or more operands...
   const size_t num_operand = kip_data.nop;
   using per_operand = afew_book<inq<real,tag>>;
#include "kip-macro-workspace.h"

   // compute operand information
   bool found = false;
   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      operand[i].size =
         vec[i].min < qmin &&
         internal::op_all(vec[i].op, kip_etd,
                      qmin,operand[i].points, insub)
         ? (found = true, operand[i].points.size())
         :  0;
      operand[i].next = 0, operand[i].in_now = vec[i].in;
   }
   if (!found) return kip_less, false;

   // search for the first relevant intersection.
   size_t num_in = nary.total_in;

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (size_t i = 0;  i < kip_data.nop;  ++i)
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

   const size_t num_operand = kip_data.nop;
   using per_operand = afew_book<inq<real,tag>>;
#include "kip-macro-workspace.h"

   // compute operand information
   bool found = false;
   for (size_t i = 0;  i < kip_data.nop;  ++i) {
      operand[i].size =
         vec[i].min < qmin &&
         internal::op_all(vec[i].op, kip_etd,
                      qmin,operand[i].points, insub)
         ? (found = true, operand[i].points.size())
         :  0;
      operand[i].next = 0, operand[i].in_now = vec[i].in;
   }
   if (!found) return kip_less, false;

   size_t num_in = nary.total_in;
   ints.reset();

   for (int current = -1 ;; ++operand[current].next, current = -1) {
      for (size_t i = 0;  i < kip_data.nop;  ++i)
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
