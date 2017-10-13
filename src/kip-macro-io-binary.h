/* -*- C++ -*- */

// This file is #included in various places to help create stream i/o
// for binary operator shapes.

#define kip_class kip_paste(kip,kip_brief)

namespace kip {

// read_value
template<class ISTREAM, class real, class tag>
bool read_value(
   ISTREAM &s, kip::kip_class<real,tag> &obj,
   const std::string &description = kip_description(kip_brief)
) {
   // For whatever reason, writing this directly for kip_class gives smaller
   // code (in g++, at least) than forwarding.

   delete obj.binary.a;
   delete obj.binary.b;
   s.bail = false;

   if (!(
      read_left(s) &&
      read_shape(s, obj.binary.a, "shape operand \"a\"") && read_comma(s) &&
      read_shape(s, obj.binary.b, "shape operand \"b\"") &&
      read_done(s, obj)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_error);
   } else if (s.level == 0)
      obj.propagate_base();

   return !s.fail();
}



// kip::ostream
template<class real, class tag>
kip::ostream &operator<<(
   kip::ostream &k, const kip::kip_class<real,tag> &obj
) {
   // For whatever reason, writing this directly for kip_class gives smaller
   // code (in g++, at least) than forwarding.

   bool okay;

   // stub
   if (kip::format == kip::format_stub)
      okay = k << kip_quote(kip_brief) "(" &&
             obj.binary.a->write(k) << ", " &&
             obj.binary.b->write(k) << ')';

   // one
   else if (kip::format == kip::format_one) {
      okay = k << kip_quote(kip_brief) "(" &&
             obj.binary.a->write(k) << ", " &&
             obj.binary.b->write(k);
      if (obj.baseset)
         okay = okay && k << ", " << obj.base();
      okay = okay && k << ')';
   }

   // op
   // full
   else if ((okay = k << kip_quote(kip_brief) "(\n")) {
      k.level++;

      okay = obj.binary.a->write(k.indent()) << ",\n" &&
             obj.binary.b->write(k.indent());
      if (obj.baseset)
         okay = okay && k << ",\n" && k.indent() << obj.base();
      okay = okay && k << '\n';

      k.level--;
      okay = okay && k.indent() << ')';
   }

   // done
   if (!okay)
      (void)kip::error("Could not write " kip_description(kip_brief));
   return k;
}

}

#include "kip-macro-io.h"
