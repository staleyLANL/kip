
#pragma once

// -----------------------------------------------------------------------------
// colormap
// -----------------------------------------------------------------------------

class colormap {
public:
   // zzz Use as base class for colormap_jet, etc., in some way
};



// -----------------------------------------------------------------------------
// jet
// -----------------------------------------------------------------------------

// colormap_jet
template<class unused>
class colormap_jet {
public:
   // component, color, vec
   using component = double;
   using color = RGB<component>;
   static std::vector<color> vec;

   // size()
   unsigned size() const { return vec.size(); }


   // ------------------------
   // push
   // ------------------------

   // push(color)
   color &push(const color &from) const
   {
      vec.push_back(from);
      return vec.back();
   }

   // push(r,g,b)
   color &push(
      const component &r,
      const component &g,
      const component &b
   ) const {
      return push(color(r,g,b));
   }


   // ------------------------
   // initialize
   // ------------------------

   void initialize() const
   {
      static bool first = true;
      if (first) {
         first = false;

         // color entries
         push( 0.00000, 0.00000, 0.50000 );  // deep blue
         push( 0.00000, 0.00000, 0.56349 );
         push( 0.00000, 0.00000, 0.62698 );
         push( 0.00000, 0.00000, 0.69048 );
         push( 0.00000, 0.00000, 0.75397 );
         push( 0.00000, 0.00000, 0.81746 );
         push( 0.00000, 0.00000, 0.88095 );
         push( 0.00000, 0.00000, 0.94444 );
         push( 0.00000, 0.00794, 1.00000 );
         push( 0.00000, 0.07143, 1.00000 );
         push( 0.00000, 0.13492, 1.00000 );
         push( 0.00000, 0.19841, 1.00000 );
         push( 0.00000, 0.26190, 1.00000 );
         push( 0.00000, 0.32540, 1.00000 );
         push( 0.00000, 0.38889, 1.00000 );
         push( 0.00000, 0.45238, 1.00000 );
         push( 0.00000, 0.51587, 1.00000 );
         push( 0.00000, 0.57937, 1.00000 );
         push( 0.00000, 0.64286, 1.00000 );
         push( 0.00000, 0.70635, 1.00000 );
         push( 0.00000, 0.76984, 1.00000 );
         push( 0.00000, 0.83333, 1.00000 );
         push( 0.00000, 0.89683, 1.00000 );
         push( 0.00000, 0.96032, 1.00000 );
         push( 0.02381, 1.00000, 0.97619 );
         push( 0.08730, 1.00000, 0.91270 );
         push( 0.15079, 1.00000, 0.84921 );
         push( 0.21429, 1.00000, 0.78571 );
         push( 0.27778, 1.00000, 0.72222 );
         push( 0.34127, 1.00000, 0.65873 );
         push( 0.40476, 1.00000, 0.59524 );
         push( 0.46825, 1.00000, 0.53175 );
         push( 0.53175, 1.00000, 0.46825 );
         push( 0.59524, 1.00000, 0.40476 );
         push( 0.65873, 1.00000, 0.34127 );
         push( 0.72222, 1.00000, 0.27778 );
         push( 0.78571, 1.00000, 0.21429 );
         push( 0.84921, 1.00000, 0.15079 );
         push( 0.91270, 1.00000, 0.08730 );
         push( 0.97619, 1.00000, 0.02381 );
         push( 1.00000, 0.96032, 0.00000 );
         push( 1.00000, 0.89683, 0.00000 );
         push( 1.00000, 0.83333, 0.00000 );
         push( 1.00000, 0.76984, 0.00000 );
         push( 1.00000, 0.70635, 0.00000 );
         push( 1.00000, 0.64286, 0.00000 );
         push( 1.00000, 0.57937, 0.00000 );
         push( 1.00000, 0.51587, 0.00000 );
         push( 1.00000, 0.45238, 0.00000 );
         push( 1.00000, 0.38889, 0.00000 );
         push( 1.00000, 0.32540, 0.00000 );
         push( 1.00000, 0.26190, 0.00000 );
         push( 1.00000, 0.19841, 0.00000 );
         push( 1.00000, 0.13492, 0.00000 );
         push( 1.00000, 0.07143, 0.00000 );
         push( 1.00000, 0.00794, 0.00000 );
         push( 0.94444, 0.00000, 0.00000 );
         push( 0.88095, 0.00000, 0.00000 );
         push( 0.81746, 0.00000, 0.00000 );
         push( 0.75397, 0.00000, 0.00000 );
         push( 0.69048, 0.00000, 0.00000 );
         push( 0.62698, 0.00000, 0.00000 );
         push( 0.56349, 0.00000, 0.00000 );
         push( 0.50000, 0.00000, 0.00000 );  // deep red
      }
   }


   // ------------------------
   // operator[]
   // ------------------------

   // zzz Need to work on generality here!!!
   // zzz For now, assuming double index, and rgb output

   rgb operator[](const double d) const
   {
      initialize();

      const unsigned n = size();  // zzz Could be static.
      assert(1 < n);  // zzz Need, for general case.

      const double t = d < 0 ? 0 : 1 < d ? 1 : d;
      const double m = (n-1)*t;

      const double f = floor(m);  const unsigned lo = unsigned(f + 0.1);
      const double c = ceil (m);  const unsigned hi = unsigned(c + 0.1);

      const double one = c-m;
      const double two = 1-one;

      const double r = one*vec[lo].r + two*vec[hi].r;
      const double g = one*vec[lo].g + two*vec[hi].g;
      const double b = one*vec[lo].b + two*vec[hi].b;

      return rgb(uchar(255*r), uchar(255*g), uchar(255*b));

      /*
      const real rcolor = real(0.9999)*(ncolor-1)*(h-hmin)/(hmax-hmin);
      const uchar icolor = uchar(rcolor);
      const real f = rcolor - icolor;
      const real a = 255*(1-f);
      const real b = 255*f;

      const RGB<uchar> color(
         uchar(a*spectrum[icolor][0] + b*spectrum[icolor+1][0]),
         uchar(a*spectrum[icolor][1] + b*spectrum[icolor+1][1]),
         uchar(a*spectrum[icolor][2] + b*spectrum[icolor+1][2])
      );
      */
   }
};

// colormap_jet::vec
template<class unused>
std::vector<typename colormap_jet<unused>::color> colormap_jet<unused>::vec;

// jet
inline const colormap_jet<char> jet;
