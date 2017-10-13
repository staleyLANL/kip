/* -*- C++ -*- */

// OpenMP- and compiler-specific nonsense

#ifndef _OPENMP

      // ================
      // serial
      // ================

      static grower< per_operand > work;
      per_operand *const operand = work.more(num_operand);
      #undef  kip_less
      #define kip_less work.less(num_operand)

#else

   // Apparently, Intel C++ #defines __GNUC__...
   #if !defined(_MSC_VER) && (!defined(__GNUC__) || defined(__INTEL_COMPILER))

      // ================
      // openmp
      // ================

      static grower< per_operand > work;
      #pragma omp threadprivate(work)
      per_operand *const operand = work.more(num_operand);
      #undef  kip_less
      #define kip_less work.less(num_operand)

   #else

      // ================
      // openmp: g++
      // ================

      // Note: use the general case after g++'s openmp bug is fixed

      const unsigned unsigned_num_operand = unsigned(num_operand);
      array< 1, per_operand > operand(unsigned_num_operand);
      #undef  kip_less
      #define kip_less internal::no_action()

   #endif

#endif
