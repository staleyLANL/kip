
// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

namespace detail {

// readfile
template<class unused>
bool readfile(
   std::vector<
      std::unique_ptr<
         kip::array<2,kip::rgb>
      >
   > &image,
   const ulong hsize,
   const ulong vsize,
   const char *const name
) {
   std::ifstream ifs(name, std::ios::ate | std::ios::binary);
   const std::ios::pos_type size = ifs.tellg();
   assert(size == long(hsize*vsize*sizeof(kip::rgb)));

   ///   static ulong index = 0;
   image.push_back(std::make_unique<kip::array<2,kip::rgb>>());
   image.back()->resize(hsize,vsize);

   ifs.seekg(0, std::ios::beg);
   return ifs.read((char *)(image.back()->data()), size) ? true : false;
}



// readearth
template<class unused>
bool readearth(
   std::vector<
      std::unique_ptr<
         kip::array<2,kip::rgb>
      >
   > &image
) {
   // bookkeeping
   assert(image.size() == 0);
   image.reserve(5);

   // read
   std::cout << "Reading earth images..." << std::endl;
   const bool rv =
      readfile<char>(image, 8192, 4096,
              "maps/earth-08192x04096-shading-ice-clouds.rgb") &&
      readfile<char>(image, 8192, 4096,
              "maps/earth-08192x04096-shading-ice.rgb") &&
      readfile<char>(image, 8192, 4096,
              "maps/earth-08192x04096-shading.rgb") &&
      readfile<char>(image, 8192, 4096,
              "maps/earth-08192x04096-plain.rgb") &&
      readfile<char>(image, 8192, 4096,
              "maps/earth-08192x04096-night.rgb") &&
      ///      readfile<char>(image, 24000, 12000,
      ///              "maps/earth-24000x12000-jumbo.rgb") &&
      true;
   std::cout << "Done." << std::endl;

   // done
   return rv;
}

} // namespace detail



// -----------------------------------------------------------------------------
// picture
// -----------------------------------------------------------------------------

template<class BASE, class real = defaults::real>
class picture {
public:
   // description, for i/o
   static const std::string description;

   // image
   static std::vector<std::unique_ptr<kip::array<2,kip::rgb>>> image;

   // data
   BASE base;  // use this, if no image, or if i == -1, or there's no image[i]
   int i = -1; // use image[i], if i != -1 && image[i] exists

   // initialize
   // Just some hardcoded stuff for now
   void initialize()
   {
      static bool first = true;
      if (first) {
         detail::readearth<char>(image);
         first = false;
      }
   }

   // picture()
   // picture(base)
   explicit picture(const BASE &b = BASE{}) : base(b)
   {
      initialize();
   }
};

// description
template<class BASE, class real>
const std::string picture<BASE,real>::description =
   std::string("picture<") + BASE::description + ">";

// image
// qqq probably don't need kip:: here, and would writing "{}" work? +elsewhere
template<class BASE, class real>
std::vector<std::unique_ptr<kip::array<2,kip::rgb>>>
picture<BASE,real>::image =
   std::vector<std::unique_ptr<kip::array<2,kip::rgb>>>{};



// -----------------------------------------------------------------------------
// randomize
// -----------------------------------------------------------------------------

template<class BASE, class real>
inline picture<BASE,real> &randomize(picture<BASE,real> &obj)
{
   /*
   randomize(obj.base);
   obj.i = (rand() % int(obj.image.size() + 1)) - 1;
   return obj;
   */

   randomize(obj.base);
   const int size = int(obj.image.size());
   obj.i = size ? (rand() % size) : - 1;
   return obj;

   /*
   randomize(obj.base);
   obj.i = 0;
   return obj;
   */
}



// -----------------------------------------------------------------------------
// read_value
// -----------------------------------------------------------------------------

// qqq Eventually allow for picture specification
template<class ISTREAM, class BASE, class real>
bool read_value(ISTREAM &s, picture<BASE,real> &obj)
{
   s.bail = false;

   // base
   bool okay = read_value(s,obj.base);
   if (okay) {
      // prefix
      if (!s.prefix('\0', obj.description, false))  // false: eof not okay
         return false;

      // )
      /*
      char ch = 0;
      s.input(ch);
      */
      const int ch = s.peek();
      if (ch == ')')
         return obj.i = -1, true;

      // ,obj.i
      ///      s.unget();
      if (!(read_comma(s) && read_value(s,obj.i)))
         okay = false;
   }

   if (!okay) {
      s.add(std::ios::failbit);
      addendum("Detected while reading " + obj.description, diagnostic::error);
   }

   return !s.fail();
}



// -----------------------------------------------------------------------------
// convert
// qqq Review again precisely where and how these are used, considering that
// other functions (with intersection etc.) are actually used in order to go
// all the way from "base" to final pixel color.
// -----------------------------------------------------------------------------

// picture<BASE,real> ==> RGB<comp>
template<class BASE, class real, class comp>
inline void convert(const picture<BASE,real> &in, kip::RGB<comp> &out)
{
   convert(in.base,out);
}

// picture<BASE,real> ==> RGBA<comp>
template<class BASE, class real, class comp>
inline void convert(const picture<BASE,real> &in, kip::RGBA<comp> &out)
{
   convert(in.base,out);
}
