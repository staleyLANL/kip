
#pragma once

// default shape base
using default_base = rgb;

// shape - forward declaration
template<class = default_real, class = default_base>
class shape;



// -----------------------------------------------------------------------------
// read_specific_character
// read_comma
// read_comma_or
// read_semi
// read_semi_or
// read_left
// read_right
// -----------------------------------------------------------------------------

// read_specific_character: helper function
namespace internal {
   template<class ISTREAM>
   bool read_specific_character(
      ISTREAM &s, const std::string &description,
      const char want, char want2 = '\0'
   ) {
      if (want2 == '\0')
         want2 = want;

      if (s.prefix('\0', description, false)) {  // false: eof not okay
         char value = 0;
         s.input(value);
         if (s.verify('\0', description, value == want || value == want2))
            return true;
      }

      if (want == '(') s.level++; else
      if (want == ')') s.level--;
      return false;
   }
}

// comma ,
template<class ISTREAM>
inline bool read_comma(ISTREAM &s)
{
   return internal::read_specific_character(s, "comma ','", ',');
}

// comma , or semicolor ;
// BUT print that right parenthesis could have been there
template<class ISTREAM>
inline bool read_comma_or(ISTREAM &s)
{
   return internal::read_specific_character
      (s, "comma ',' or semicolon ';' or right parenthesis ')'", ',', ';');
}

// semicolon ;
// BUT print that right parenthesis could have been there
template<class ISTREAM>
inline bool read_semi_or(ISTREAM &s)
{
   return internal::read_specific_character
      (s, "semicolon ';' or right parenthesis ')'", ';');
}

// left parenthesis (
template<class ISTREAM>
inline bool read_left(ISTREAM &s)
{
   return internal::read_specific_character(s, "left parenthesis '('", '(');
}

// right parenthesis )
template<class ISTREAM>
inline bool read_right(ISTREAM &s)
{
   return internal::read_specific_character(s, "right parenthesis ')'", ')');
}



// -----------------------------------------------------------------------------
// read_value
// For various basic types
// -----------------------------------------------------------------------------

// read_specific_pod: helper function
namespace internal {
   template<class ISTREAM, class T>
   bool read_specific_pod(
      ISTREAM &s, T &value,
      const std::string &description
   ) {
      if (!s.prefix('\0', description, false))  // false: eof not okay
         return false;
      s.input(value);
      return s.verify
         ('\0', description, true);  // true: no specific value wanted
   }
}


#define kip_make_read_value(T,_description)\
   template<class ISTREAM>\
   inline bool read_value(\
      ISTREAM &s, T &value,\
      const std::string &description = _description\
   ) {\
      return internal::read_specific_pod(s,value,description);\
   }


kip_make_read_value(char, "char")
kip_make_read_value(signed char, "signed char")
kip_make_read_value(unsigned char, "unsigned char")

kip_make_read_value(short, "short")
kip_make_read_value(int, "int")
kip_make_read_value(long, "long")

kip_make_read_value(unsigned short, "unsigned short")
kip_make_read_value(unsigned int, "unsigned int")
kip_make_read_value(unsigned long, "unsigned long")

kip_make_read_value(float, "float")
kip_make_read_value(double, "double")
kip_make_read_value(long double, "long double")

#undef kip_make_read_value



// -----------------------------------------------------------------------------
// read_value(string)
// -----------------------------------------------------------------------------

template<class ISTREAM>
bool read_value(
   ISTREAM &s, std::string &value,
   const std::string &description = "keyword"
) {
   if (!s.prefix('\0', description, true))  // true: eof okay
      return false;

   value = "";

   int ch;
   while (ch = s.get(), s.good())
      if (isalnum(ch) || ch == '_')
         value += char(ch);
      else
         break;

   if (value == "")
      // Set fail() to indicate that our attempt to get a keyword failed,
      // regardless of whether or not the above get() failed in some way.
      s.add(std::ios::failbit);
   else {
      s.unget();
      if (s.fail() && s.eof())
         s.set(s.state() & (std::ios::badbit | std::ios::eofbit));
   }

   return s.verify('\0', description, value != "");  // non-empty expected
}



// -----------------------------------------------------------------------------
// read_color_component - read component (for now, only uchar) of RGB or RGBA
// crayola_or_component
// read_value(RGB)
// read_value(RGBA)
// -----------------------------------------------------------------------------

// read_color_component
// We don't simply use read_value(uchar), because in this context we
// want to allow for more-descriptive diagnostics than that would allow.
template<class ISTREAM>
bool read_color_component(
   ISTREAM &s, uchar &x,  // x in {r,g,b,a}
   const std::string &description = "unsigned char-based color component"
) {
   if (!s.prefix('\0', description, false))  // false: eof not okay
      return false;

   long ell;
   s.input(ell);

   if (!s.fail()) {
      const uchar min = std::numeric_limits<uchar>::min();
      const uchar max = std::numeric_limits<uchar>::max();

      if (ell < min || ell > max) {
         std::ostringstream oss;

         oss << "Value " << ell << " for unsigned char-based color component"
                "\n   is outside allowable range [" << unsigned(min) << ','
             << unsigned(max) << "]\nClipping to ";

         x = ell < min ? min : max;
         oss << unsigned(x);  // unsigned(), so it doesn't print as a character
         s.warning(oss);
      } else
         x = uchar(ell);
   }

   return s.verify('\0', description, true);  // true: no specific value wanted
}



// crayola_or_component
// See what's next, expecting either a crayola color or a color component.
// The character is only peeked.
template<class ISTREAM>
inline bool crayola_or_component(ISTREAM &s, bool &alphabetic)
{
   const std::string &description =
     "crayola color, or unsigned char-based color component";
   if (!s.prefix('\0', description, false))  // false: eof not okay
      return false;

   alphabetic = 0 != isalpha(s.peek());
   return true;
}



// RGB
template<class ISTREAM, class T>
bool read_value(
   ISTREAM &s, RGB<T> &value,
   const std::string &description = "RGB"  /// should improve when <default>
) {
   crayola cray;  bool alphabetic = false;
   s.bail = false;

   if (!(
      crayola_or_component(s,alphabetic) && (
       ( alphabetic &&
         read_value(s,cray)) ||
       (!alphabetic &&
         read_color_component(s,value.r) && read_comma(s) &&
         read_color_component(s,value.g) && read_comma(s) &&
         read_color_component(s,value.b)
   )))) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic_t::diagnostic_error);
   }

   if (alphabetic) {
      const RGB<crayola_rgb_t> from = cray;
      value(from.r, from.g, from.b);
   }

   return !s.fail();
}



// RGBA
template<class ISTREAM, class T>
bool read_value(
   ISTREAM &s, RGBA<T> &value,
   const std::string &description = "RGBA"  /// should improve when <default>
) {
   crayola cray;  bool alphabetic = false;
   s.bail = false;

   if (!(
      crayola_or_component(s,alphabetic) && (
       ( alphabetic &&
         read_value(s,cray)) ||
       (!alphabetic &&
         read_color_component(s,value.r) && read_comma(s) &&
         read_color_component(s,value.g) && read_comma(s) &&
         read_color_component(s,value.b) && read_comma(s) &&
         read_color_component(s,value.a)
   )))) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic_t::diagnostic_error);
   }

   if (alphabetic) {
      const RGB<crayola_rgb_t> from = cray;
      value(from.r, from.g, from.b);
   }

   return !s.fail();
}



// -----------------------------------------------------------------------------
// component_traits
// -----------------------------------------------------------------------------

namespace internal {

template<
   class T,
   bool is_signed  = std::numeric_limits<T>::is_signed,
   bool is_integer = std::numeric_limits<T>::is_integer
>
class component_traits {  // general
public:
   using result = T;
};

template<class T>
class component_traits<T,true,true> {  // for signed integral types
public:
   using result = long;
};

template<class T>
class component_traits<T,false,true> {  // for unsigned integral types
public:
   using result = unsigned long;
};

}



// -----------------------------------------------------------------------------
// istream >> crayola
// ostream << crayola
// -----------------------------------------------------------------------------

// kip::istream >> crayola
inline kip::istream &operator>>(kip::istream &k, crayola &obj)
{
   read_value(k,obj);
   return k;
}

// std::istream >> crayola
inline std::istream &operator>>(std::istream &s, crayola &obj)
{
   kip::istream k(s);
   k >> obj;
   return s;
}

// kip::ostream << crayola
template<class>
kip::ostream &crayola_write(kip::ostream &k, const crayola &obj)
{
   const ulong size = crayola::color_table().size();
   using print_as = unsigned;

   if (obj.id() >= size) {
      std::ostringstream oss;
      oss << "Index " << print_as(obj.id()) << " for crayola color"
         " is outside allowable range [0,"
          << size-1 << "]\nWriting as \"unknown\"";
      warning(oss);
      return k << crayola::color_table()[0].second;
   }

   return k << crayola::color_table()[obj.id()].second;
}

inline kip::ostream &operator<<(kip::ostream &k, const crayola &obj)
{
   return crayola_write<char>(k,obj);
}

// std::ostream << crayola
inline std::ostream &operator<<(std::ostream &s, const crayola &obj)
{
   kip::ostream k(s);
   k << obj;
   return s;
}



// -----------------------------------------------------------------------------
// istream >> RGB
// ostream << RGB
// -----------------------------------------------------------------------------

// kip::istream >> RGB
template<class T>
inline kip::istream &operator>>(kip::istream &k, RGB<T> &obj)
{
   read_value(k,obj);
   return k;
}

// std::istream >> RGB
template<class T>
inline std::istream &operator>>(std::istream &s, RGB<T> &obj)
{
   kip::istream k(s);
   k >> obj;
   return s;
}

// kip::ostream << RGB
template<class T>
inline kip::ostream &operator<<(kip::ostream &k, const RGB<T> &obj)
{
   using print_as = typename internal::component_traits<T>::result;
   return
      k << print_as(obj.r) << ','
        << print_as(obj.g) << ','
        << print_as(obj.b);
}

// std::ostream << RGB
template<class T>
inline std::ostream &operator<<(std::ostream &s, const RGB<T> &obj)
{
   kip::ostream k(s);
   k << obj;
   return s;
}



// -----------------------------------------------------------------------------
// istream >> RGBA
// ostream << RGBA
// -----------------------------------------------------------------------------

// kip::istream >> RGBA
template<class T>
inline kip::istream &operator>>(istream &k, RGBA<T> &obj)
{
   read_value(k,obj);
   return k;
}

// std::istream >> RGBA
template<class T>
inline std::istream &operator>>(std::istream &s, RGBA<T> &obj)
{
   kip::istream k(s);
   k >> obj;
   return s;
}

// kip::ostream << RGBA
template<class T>
inline kip::ostream &operator<<(kip::ostream &k, const RGBA<T> &obj)
{
   using print_as = typename internal::component_traits<T>::result;
   return
      k << print_as(obj.r) << ','
        << print_as(obj.g) << ','
        << print_as(obj.b) << ','
        << print_as(obj.a);
}

// std::ostream << RGBA
template<class T>
inline std::ostream &operator<<(std::ostream &s, const RGBA<T> &obj)
{
   kip::ostream k(s);
   k << obj;
   return s;
}



// -----------------------------------------------------------------------------
// istream >> marble
// ostream << marble
// -----------------------------------------------------------------------------

// kip::istream >> marble
template<class T, class real>
inline kip::istream &operator>>(kip::istream &k, marble<T,real> &obj)
{
   read_value(k,obj);
   return k;
}

// std::istream >> marble
template<class T, class real>
inline std::istream &operator>>(std::istream &s, marble<T,real> &obj)
{
   kip::istream k(s);
   k >> obj;
   return s;
}

// kip::ostream << marble
template<class T, class real>
kip::ostream &operator<<(kip::ostream &k, const marble<T,real> &obj)
{
   using print_as = typename internal::component_traits<T>::result;
   return
      k << print_as(obj.r) << ','
        << print_as(obj.g) << ','
        << print_as(obj.b) << ','

        << obj.amp    << ','
        << obj.ampfac << ','
        << obj.per    << ','
        << obj.perfac << ','
        << obj.nfun   << ','
        << obj.seed   << ','
        <<(obj.swirl ? '1' : '0')
      ;
}

// std::ostream << marble
template<class T, class real>
inline std::ostream &operator<<(std::ostream &s, const marble<T,real> &obj)
{
   kip::ostream k(s);
   k << obj;
   return s;
}



// -----------------------------------------------------------------------------
// read_value(point)
// -----------------------------------------------------------------------------

template<class ISTREAM, class T>
bool read_value(
   ISTREAM &s, point<T> &value,
   const std::string &description = "point"
) {
   // x, y, z
   s.bail = false;
   if (!(
      read_value(s,value.x) && read_comma(s) &&
      read_value(s,value.y) && read_comma(s) &&
      read_value(s,value.z)
   )) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// -----------------------------------------------------------------------------
// istream >> point
// ostream << point
// -----------------------------------------------------------------------------

// kip::istream >> point
template<class T>
inline kip::istream &operator>>(kip::istream &k, point<T> &obj)
{
   read_value(k,obj);
   return k;
}

// std::istream >> point
template<class T>
inline std::istream &operator>>(std::istream &s, point<T> &obj)
{
   kip::istream k(s);
   k >> obj;
   return s;
}

// kip::ostream << point
template<class T>
inline kip::ostream &operator<<(kip::ostream &k, const point<T> &obj)
{
   return k << obj.x << ',' << obj.y << ',' << obj.z;
}

// std::ostream << point
template<class T>
inline std::ostream &operator<<(std::ostream &s, const point<T> &obj)
{
   kip::ostream k(s);
   k << obj;
   return s;
}



// -----------------------------------------------------------------------------
// read_done
// write_finish
// -----------------------------------------------------------------------------

// read_done: reads either
//    )
// or
//    ,base)
// or possibly
//    ;base)
// to finish off a shape
template<class ISTREAM, class T, class tag>
bool read_done(ISTREAM &s, kip::shape<T,tag> &shape, const char sep = ',')
{
   tag &base = shape;
   s.bail = false;

   // prefix
   if (!s.prefix('\0', base.description, false))  // false: eof not okay
      return false;

   // )
   char ch = 0;
   s.input(ch);
   if (ch == ')') {
      base = tag();  // default
      shape.baseset = false;
      return true;
   }

   shape.baseset = true;
   s.unget();
   if (ch == '(') s.level--;

   // ,base)
   if (!(
      (sep == '\0') ||
      (sep == ',' && read_comma_or(s)) ||  // allows ; or ,
      (sep == ';' && read_semi_or (s))     // allows ; only
   ))
      s.add(std::ios::failbit);
   else if (!(read_value(s,base)))
      s.add(std::ios::failbit);
   else if (!read_right(s))  // don't want "detected..." for this
      s.add(std::ios::failbit);

   return !s.fail();
}



// write_finish: writes either
//    )
// or
//    ,base)
// to finish off a shape
template<class T, class tag>
inline bool write_finish(
   kip::ostream &k, const shape<T,tag> &obj,
   const bool oneline
) {
   bool okay = true;

   if (oneline) {
      if (obj.baseset)
         okay = k << ", " << obj.base();
      return okay && k << ')';
   } else {
      if (obj.baseset)
         okay = k << ",\n   " && k.indent() << obj.base();
      return okay && k << '\n' && k.indent() << ')';
   }
}



// -----------------------------------------------------------------------------
// onetwor_write
// -----------------------------------------------------------------------------

namespace internal {

template<class real, class tag>
kip::ostream &onetwor_write(
   kip::ostream &k, const shape<real,tag> &obj,
   const point<real> &one,
   const point<real> &two,
   const real r,
   const std::string &description
) {
   bool okay;

   // stub
   if (format == format_t::format_stub)
      okay = k << description << "()";

   // one
   // op
   else if (format == format_t::format_one ||
            format == format_t::format_op)
      okay = k << description << '('
               << one << ", "
               << two << ", "
               << r &&
             write_finish(k, obj, true);

   // full
   else
      okay = k << description << "(\n   " &&
             k.indent() << one << ",\n   " &&
             k.indent() << two << ",\n   " &&
             k.indent() << r &&
             write_finish(k, obj, false);

   // done
   if (!okay)
      (void)error("Could not write " + description);
   return k;
}

}
