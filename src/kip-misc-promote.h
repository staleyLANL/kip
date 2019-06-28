
#pragma once

// This file provides some type-promotion information that will be used
// by the RGB and RGBA classes. What we have here right now is overkill,
// at least for the present time.

namespace kip {

template<class A, class B>
class if_same { /* nothing */ };

template<class T>
class if_same<T,T> { public: typedef T result; };



// -----------------------------------------------------------------------------
// pro: helper class for promote<>
// Reasonable promotion for fundamental types, except that for our purposes
// here, we neglect wchar_t.
// -----------------------------------------------------------------------------

/*
------------------------
Types, excluding wchar_t
------------------------

   bool
   char
   signed char
   unsigned char
   short
   int
   long
   unsigned short
   unsigned int
   unsigned long
   float
   double
   long double

------------------------
Rules
------------------------

   bases:
      bool --> char
           --> short --> int --> long
           --> float --> double --> long double

   signedness:
      (signed, *)  -->  signed
      (*, signed)  -->  signed

      (unsigned, N/A)  -->  unsigned
      (N/A, unsigned)  -->  unsigned

      (N/A, N/A)  -->  same N/A

   where:
      "signed"   is signed char, short, int, long, float, double, long double
      "unsigned" is unsigned char, unsigned short, unsigned int, unsigned long
      "N/A"      is bool, char

For now, I'll just list all the combinations. Certainly, however, this could
be rewritten in a better way.
*/

namespace internal {

   // default
   template<class A, class B>
   class pro { /* nothing */ };

   // others
#define kip_make_pro(one_t, two_t, result_t)\
   template<>\
   class pro<one_t,two_t> { public: typedef result_t result; }



   //          | --------------- | --------------- | --------------- |
   //          | first           | second          | promotion       |
   //          | --------------- | --------------- | --------------- |

   // bool, *
   kip_make_pro(          bool   ,          bool   ,          bool   );
   kip_make_pro(          bool   ,          char   ,          char   );
   kip_make_pro(          bool   ,   signed char   ,   signed char   );
   kip_make_pro(          bool   , unsigned char   , unsigned char   );
   kip_make_pro(          bool   ,   signed short  ,   signed short  );
   kip_make_pro(          bool   ,   signed int    ,   signed int    );
   kip_make_pro(          bool   ,   signed long   ,   signed long   );
   kip_make_pro(          bool   , unsigned short  , unsigned short  );
   kip_make_pro(          bool   , unsigned int    , unsigned int    );
   kip_make_pro(          bool   , unsigned long   , unsigned long   );
   kip_make_pro(          bool   ,          float  ,          float  );
   kip_make_pro(          bool   ,          double ,          double );
   kip_make_pro(          bool   ,     long double ,     long double );

   // char, *
   kip_make_pro(          char   ,          bool   ,          char   );
   kip_make_pro(          char   ,          char   ,          char   );
   kip_make_pro(          char   ,   signed char   ,   signed char   );
   kip_make_pro(          char   , unsigned char   , unsigned char   );
   kip_make_pro(          char   ,   signed short  ,   signed short  );
   kip_make_pro(          char   ,   signed int    ,   signed int    );
   kip_make_pro(          char   ,   signed long   ,   signed long   );
   kip_make_pro(          char   , unsigned short  , unsigned short  );
   kip_make_pro(          char   , unsigned int    , unsigned int    );
   kip_make_pro(          char   , unsigned long   , unsigned long   );
   kip_make_pro(          char   ,          float  ,          float  );
   kip_make_pro(          char   ,          double ,          double );
   kip_make_pro(          char   ,     long double ,     long double );

   // signed char, *
   kip_make_pro(   signed char   ,          bool   ,   signed char   );
   kip_make_pro(   signed char   ,          char   ,   signed char   );
   kip_make_pro(   signed char   ,   signed char   ,   signed char   );
   kip_make_pro(   signed char   , unsigned char   ,   signed char   );
   kip_make_pro(   signed char   ,   signed short  ,   signed short  );
   kip_make_pro(   signed char   ,   signed int    ,   signed int    );
   kip_make_pro(   signed char   ,   signed long   ,   signed long   );
   kip_make_pro(   signed char   , unsigned short  ,   signed short  );
   kip_make_pro(   signed char   , unsigned int    ,   signed int    );
   kip_make_pro(   signed char   , unsigned long   ,   signed long   );
   kip_make_pro(   signed char   ,          float  ,          float  );
   kip_make_pro(   signed char   ,          double ,          double );
   kip_make_pro(   signed char   ,     long double ,     long double );

   // unsigned char, *
   kip_make_pro( unsigned char   ,          bool   , unsigned char   );
   kip_make_pro( unsigned char   ,          char   , unsigned char   );
   kip_make_pro( unsigned char   ,   signed char   ,   signed char   );
   kip_make_pro( unsigned char   , unsigned char   , unsigned char   );
   kip_make_pro( unsigned char   ,   signed short  ,   signed short  );
   kip_make_pro( unsigned char   ,   signed int    ,   signed int    );
   kip_make_pro( unsigned char   ,   signed long   ,   signed long   );
   kip_make_pro( unsigned char   , unsigned short  , unsigned short  );
   kip_make_pro( unsigned char   , unsigned int    , unsigned int    );
   kip_make_pro( unsigned char   , unsigned long   , unsigned long   );
   kip_make_pro( unsigned char   ,          float  ,          float  );
   kip_make_pro( unsigned char   ,          double ,          double );
   kip_make_pro( unsigned char   ,     long double ,     long double );

   // short, *
   kip_make_pro(   signed short  ,          bool   ,   signed short  );
   kip_make_pro(   signed short  ,          char   ,   signed short  );
   kip_make_pro(   signed short  ,   signed char   ,   signed short  );
   kip_make_pro(   signed short  , unsigned char   ,   signed short  );
   kip_make_pro(   signed short  ,   signed short  ,   signed short  );
   kip_make_pro(   signed short  ,   signed int    ,   signed int    );
   kip_make_pro(   signed short  ,   signed long   ,   signed long   );
   kip_make_pro(   signed short  , unsigned short  ,   signed short  );
   kip_make_pro(   signed short  , unsigned int    ,   signed int    );
   kip_make_pro(   signed short  , unsigned long   ,   signed long   );
   kip_make_pro(   signed short  ,          float  ,          float  );
   kip_make_pro(   signed short  ,          double ,          double );
   kip_make_pro(   signed short  ,     long double ,     long double );

   // int, *
   kip_make_pro(   signed int    ,          bool   ,   signed int    );
   kip_make_pro(   signed int    ,          char   ,   signed int    );
   kip_make_pro(   signed int    ,   signed char   ,   signed int    );
   kip_make_pro(   signed int    , unsigned char   ,   signed int    );
   kip_make_pro(   signed int    ,   signed short  ,   signed int    );
   kip_make_pro(   signed int    ,   signed int    ,   signed int    );
   kip_make_pro(   signed int    ,   signed long   ,   signed long   );
   kip_make_pro(   signed int    , unsigned short  ,   signed int    );
   kip_make_pro(   signed int    , unsigned int    ,   signed int    );
   kip_make_pro(   signed int    , unsigned long   ,   signed long   );
   kip_make_pro(   signed int    ,          float  ,          float  );
   kip_make_pro(   signed int    ,          double ,          double );
   kip_make_pro(   signed int    ,     long double ,     long double );

   // long, *
   kip_make_pro(   signed long   ,          bool   ,   signed long   );
   kip_make_pro(   signed long   ,          char   ,   signed long   );
   kip_make_pro(   signed long   ,   signed char   ,   signed long   );
   kip_make_pro(   signed long   , unsigned char   ,   signed long   );
   kip_make_pro(   signed long   ,   signed short  ,   signed long   );
   kip_make_pro(   signed long   ,   signed int    ,   signed long   );
   kip_make_pro(   signed long   ,   signed long   ,   signed long   );
   kip_make_pro(   signed long   , unsigned short  ,   signed long   );
   kip_make_pro(   signed long   , unsigned int    ,   signed long   );
   kip_make_pro(   signed long   , unsigned long   ,   signed long   );
   kip_make_pro(   signed long   ,          float  ,          float  );
   kip_make_pro(   signed long   ,          double ,          double );
   kip_make_pro(   signed long   ,     long double ,     long double );

   // unsigned short, *
   kip_make_pro( unsigned short  ,          bool   , unsigned short  );
   kip_make_pro( unsigned short  ,          char   , unsigned short  );
   kip_make_pro( unsigned short  ,   signed char   ,   signed short  );
   kip_make_pro( unsigned short  , unsigned char   , unsigned short  );
   kip_make_pro( unsigned short  ,   signed short  ,   signed short  );
   kip_make_pro( unsigned short  ,   signed int    ,   signed int    );
   kip_make_pro( unsigned short  ,   signed long   ,   signed long   );
   kip_make_pro( unsigned short  , unsigned short  , unsigned short  );
   kip_make_pro( unsigned short  , unsigned int    , unsigned int    );
   kip_make_pro( unsigned short  , unsigned long   , unsigned long   );
   kip_make_pro( unsigned short  ,          float  ,          float  );
   kip_make_pro( unsigned short  ,          double ,          double );
   kip_make_pro( unsigned short  ,     long double ,     long double );

   // unsigned int, *
   kip_make_pro( unsigned int    ,          bool   , unsigned int    );
   kip_make_pro( unsigned int    ,          char   , unsigned int    );
   kip_make_pro( unsigned int    ,   signed char   ,   signed int    );
   kip_make_pro( unsigned int    , unsigned char   , unsigned int    );
   kip_make_pro( unsigned int    ,   signed short  ,   signed int    );
   kip_make_pro( unsigned int    ,   signed int    ,   signed int    );
   kip_make_pro( unsigned int    ,   signed long   ,   signed long   );
   kip_make_pro( unsigned int    , unsigned short  , unsigned int    );
   kip_make_pro( unsigned int    , unsigned int    , unsigned int    );
   kip_make_pro( unsigned int    , unsigned long   , unsigned long   );
   kip_make_pro( unsigned int    ,          float  ,          float  );
   kip_make_pro( unsigned int    ,          double ,          double );
   kip_make_pro( unsigned int    ,     long double ,     long double );

   // unsigned long, *
   kip_make_pro( unsigned long   ,          bool   , unsigned long   );
   kip_make_pro( unsigned long   ,          char   , unsigned long   );
   kip_make_pro( unsigned long   ,   signed char   ,   signed long   );
   kip_make_pro( unsigned long   , unsigned char   , unsigned long   );
   kip_make_pro( unsigned long   ,   signed short  ,   signed long   );
   kip_make_pro( unsigned long   ,   signed int    ,   signed long   );
   kip_make_pro( unsigned long   ,   signed long   ,   signed long   );
   kip_make_pro( unsigned long   , unsigned short  , unsigned long   );
   kip_make_pro( unsigned long   , unsigned int    , unsigned long   );
   kip_make_pro( unsigned long   , unsigned long   , unsigned long   );
   kip_make_pro( unsigned long   ,          float  ,          float  );
   kip_make_pro( unsigned long   ,          double ,          double );
   kip_make_pro( unsigned long   ,     long double ,     long double );

   // float, *
   kip_make_pro(          float  ,          bool   ,          float  );
   kip_make_pro(          float  ,          char   ,          float  );
   kip_make_pro(          float  ,   signed char   ,          float  );
   kip_make_pro(          float  , unsigned char   ,          float  );
   kip_make_pro(          float  ,   signed short  ,          float  );
   kip_make_pro(          float  ,   signed int    ,          float  );
   kip_make_pro(          float  ,   signed long   ,          float  );
   kip_make_pro(          float  , unsigned short  ,          float  );
   kip_make_pro(          float  , unsigned int    ,          float  );
   kip_make_pro(          float  , unsigned long   ,          float  );
   kip_make_pro(          float  ,          float  ,          float  );
   kip_make_pro(          float  ,          double ,          double );
   kip_make_pro(          float  ,     long double ,     long double );

   // double, *
   kip_make_pro(          double ,          bool   ,          double );
   kip_make_pro(          double ,          char   ,          double );
   kip_make_pro(          double ,   signed char   ,          double );
   kip_make_pro(          double , unsigned char   ,          double );
   kip_make_pro(          double ,   signed short  ,          double );
   kip_make_pro(          double ,   signed int    ,          double );
   kip_make_pro(          double ,   signed long   ,          double );
   kip_make_pro(          double , unsigned short  ,          double );
   kip_make_pro(          double , unsigned int    ,          double );
   kip_make_pro(          double , unsigned long   ,          double );
   kip_make_pro(          double ,          float  ,          double );
   kip_make_pro(          double ,          double ,          double );
   kip_make_pro(          double ,     long double ,     long double );

   // long double, *
   kip_make_pro(     long double ,          bool   ,     long double );
   kip_make_pro(     long double ,          char   ,     long double );
   kip_make_pro(     long double ,   signed char   ,     long double );
   kip_make_pro(     long double , unsigned char   ,     long double );
   kip_make_pro(     long double ,   signed short  ,     long double );
   kip_make_pro(     long double ,   signed int    ,     long double );
   kip_make_pro(     long double ,   signed long   ,     long double );
   kip_make_pro(     long double , unsigned short  ,     long double );
   kip_make_pro(     long double , unsigned int    ,     long double );
   kip_make_pro(     long double , unsigned long   ,     long double );
   kip_make_pro(     long double ,          float  ,     long double );
   kip_make_pro(     long double ,          double ,     long double );
   kip_make_pro(     long double ,     long double ,     long double );

#undef kip_make_pro

}



// -----------------------------------------------------------------------------
// promote<A, B [, at_least]>
// The last template parameter allows us to prescribe "at least" a certain
// precision to be used for the arithmetic and the result, in the contexts
// in which we'll use the promote<> class.
// -----------------------------------------------------------------------------

template<class A, class B, class at_least = B>
class promote {
public:
   typedef typename internal::pro<
      typename internal::pro<A,B>::result, at_least
   >::result result;
};

}
