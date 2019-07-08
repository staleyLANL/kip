
// This file is #included in various places to help create stream i/o
// for nary operator shapes.

// read_value
template<class ISTREAM, class real, class tag>
bool read_value(
   ISTREAM &s, kip_class<real,tag> &obj,
   const std::string &description = kip_description(kip_class)
) {
   // Note: for binary types, we allowed:
   //    binary(a,b[,base])
   // Now, for nary operator types, we allow:
   //    nary(list[;base])
   // where list is empty, or is a comma-separated list of shapes.

   obj.clear();
   s.bail = false;

   if (!(
      read_left(s) &&
      read_shapes<real,tag>(s, obj.kip_data) &&
      read_done(s, obj, ';')
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   } else if (s.level == 0)
      obj.propagate_base();

   return !s.fail();
}



// kip::ostream
template<class real, class tag>
kip::ostream &operator<<(
   kip::ostream &k, const kip_class<real,tag> &obj
) {
   const size_t nop = obj.size();
   bool okay;

   // stub
   if (format == format_t::format_stub) {
      okay = k << kip_quote(kip_class) "(";
      for (size_t n = 0;  n < nop && okay;  ++n)
         okay = (n ? bool(k << ", ") : true) &&
                 obj.kip_data.vec()[n].op->write(k);
      okay = okay && k << ')';
   }

   // one
   else if (format == format_t::format_one) {
      okay = k << kip_quote(kip_class) "(";

      for (size_t n = 0;  n < nop && okay;  ++n)
         okay = (n ? bool(k << ", ") : true) &&
                 obj.kip_data.vec()[n].op->write(k);

      if (obj.baseset)
         okay = okay && k << "; " << obj.base();
      okay = okay && k << ')';
   }

   // op
   // full
   else if ((okay = k << kip_quote(kip_class) "(\n")) {
      k.level++;

      for (size_t n = 0;  n < nop && okay;  ++n)
         okay = (k << (n ? ",\n" : "")) &&
                 obj.kip_data.vec()[n].op->write(k.indent());

      if (obj.baseset)
         okay = okay && (nop ? k : k.indent()) << ";\n"
                     &&  k.indent() << obj.base() << '\n';
      else if (nop)
         okay = okay &&  k.indent() << '\n';

      k.level--;
      okay = okay && k.indent() << ')';
   }

   // done
   if (!okay)
      (void)error("Could not write " kip_description(kip_class));
   return k;
}

#undef kip_type
#undef kip_data

#include "kip-macro-io.h"
