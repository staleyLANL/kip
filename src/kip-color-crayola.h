
#pragma once

namespace crayola {



// -----------------------------------------------------------------------------
// macros
// -----------------------------------------------------------------------------

#define kip_make_table(type,str,alt,r,g,b) { #str, rgb(r,g,b) },
#define kip_make_decl( type,str,alt,r,g,b) static const type str;
#define kip_make_alias(type,str,alt,r,g,b) static constexpr const type &alt=str;
#define kip_make_defn( type,str,alt,r,g,b) const type type::str(#str);

#define kip_make_standard(theclass,macro) \
   macro( theclass, PureBlack,   pureblack,     0,   0,   0 ) \
   macro( theclass, PureWhite,   purewhite,   255, 255, 255 ) \
   macro( theclass, PureRed,     purered,     255,   0,   0 ) \
   macro( theclass, PureGreen,   puregreen,     0, 255,   0 ) \
   macro( theclass, PureBlue,    pureblue,      0,   0, 255 ) \
   macro( theclass, PureCyan,    purecyan,      0, 255, 255 ) \
   macro( theclass, PureMagenta, puremagenta, 255,   0, 255 ) \
   macro( theclass, PureYellow,  pureyellow,  255, 255,   0 ) \
   macro( theclass, GrayDark,    graydark,     64,  64,  64 ) \
   macro( theclass, GrayMedium,  graymedium,  128, 128, 128 ) \
   macro( theclass, GrayLight,   graylight,   192, 192, 192 )

// MAGIC NUMBER ALERT
// Number of values given in the above list of colors
namespace detail {
   inline const ulong crayola_standard_count = 11;
}



// -----------------------------------------------------------------------------
// crayola::base
// -----------------------------------------------------------------------------

template<class derived>
class base {
   using pair = std::pair<std::string,rgb>;

protected:
   static std::vector<pair> vec;
   uchar index;

public:

   // ------------------------
   // constructors
   // ------------------------

   // base(n)
   explicit base(const uchar n = 0) : index(n)
   {
      kip_assert(n < vec.size());
   }

   // base(string)
   explicit base(const std::string &str)
   {
      static bool first = true;
      first = first && (derived::initialize(), false);

      for (ulong n = vec.size();  n-- ; )
         if (str == vec[n].first) {
            index = uchar(n);
            return;
         }
      assert(false);
   }

   // ------------------------
   // functions: regular
   // ------------------------

   // id()
   uchar id() const
   {
      return index;
   }

   // r(), g(), b()
   uchar r() const { return vec[index].second.r; }
   uchar g() const { return vec[index].second.g; }
   uchar b() const { return vec[index].second.b; }

   // randomize()
   void randomize()
   {
      kip_assert(vec.size() > detail::crayola_standard_count);
      index = uchar(rand() % (vec.size() - detail::crayola_standard_count));
   }

   // ------------------------
   // functions: static
   // ------------------------

   // table()
   static const std::vector<pair> &table()
   {
      return vec;
   }

   // description()
   static std::string description() { return derived::description; }
};


// vec
template<class derived>
std::vector<std::pair<std::string,rgb>> base<derived>::vec;


// randomize
template<class derived>
inline void randomize(base<derived> &obj)
{
   obj.randomize();
}



// -----------------------------------------------------------------------------
// crayola::pure
// -----------------------------------------------------------------------------

#define kip_make_pure(macro) \
   macro( pure, Black,   black,     0,   0,   0 ) \
   macro( pure, White,   white,   255, 255, 255 ) \
   macro( pure, Red,     red,     255,   0,   0 ) \
   macro( pure, Green,   green,     0, 255,   0 ) \
   macro( pure, Blue,    blue,      0,   0, 255 ) \
   macro( pure, Cyan,    cyan,      0, 255, 255 ) \
   macro( pure, Magenta, magenta, 255,   0, 255 ) \
   macro( pure, Yellow,  yellow,  255, 255,   0 ) \
   macro( pure, Dark,    dark,     64,  64,  64 ) \
   macro( pure, Medium,  medium,  128, 128, 128 ) \
   macro( pure, Light,   light,   192, 192, 192 ) \
   kip_make_standard(pure,macro) // give last

class pure : public base<pure> {
public:
   static constexpr const char *const description = "crayola::pure";

   explicit pure() : base<pure>() { }
   explicit pure(const std::string &str) : base<pure>(str) { }

   static void initialize()
   {
      assert(vec.size() == 0);
      vec = { kip_make_pure(kip_make_table) };
   }

   kip_make_pure(kip_make_decl)
   kip_make_pure(kip_make_alias)
};

kip_make_pure(kip_make_defn)



// -----------------------------------------------------------------------------
// Silver Swirls
// -----------------------------------------------------------------------------

#define kip_make_silver(macro) \
   macro( silver, AztecGold,        aztec,       195, 153,  83 ) \
   macro( silver, BurnishedBrown,   burnished,   161, 122, 116 ) \
   macro( silver, CeruleanFrost,    cerulean,    109, 155, 195 ) \
   macro( silver, CinnamonSatin,    cinnamon,    205,  96, 126 ) \
   macro( silver, CopperPenny,      copper,      173, 111, 105 ) \
   macro( silver, CosmicCobalt,     cosmic,       46,  45, 136 ) \
   macro( silver, GlossyGrape,      glossy,      171, 146, 179 ) \
   macro( silver, GraniteGray,      granite,     103, 103, 103 ) \
   macro( silver, GreenSheen,       green,       110, 174, 161 ) \
   macro( silver, LilacLuster,      lilac,       174, 152, 170 ) \
   macro( silver, MistyMoss,        misty,       187, 180, 119 ) \
   macro( silver, MysticMaroon,     mystic,      173,  67, 121 ) \
   macro( silver, PearlyPurple,     pearly,      183, 104, 162 ) \
   macro( silver, PewterBlue,       pewter,      139, 168, 183 ) \
   macro( silver, PolishedPine,     polished,     93, 164, 147 ) \
   macro( silver, QuickSilver,      quick,       166, 166, 166 ) \
   macro( silver, RoseDust,         rose,        158,  94, 111 ) \
   macro( silver, RustyRed,         rusty,       218,  44,  67 ) \
   macro( silver, ShadowBlue,       shadow,      119, 139, 165 ) \
   macro( silver, ShinyShamrock,    shiny,        95, 167, 120 ) \
   macro( silver, SteelTeal,        steel,        95, 138, 139 ) \
   macro( silver, SugarPlum,        sugar,       145,  78, 117 ) \
   macro( silver, TwilightLavender, twilight,    138,  73, 107 ) \
   macro( silver, WintergreenDream, wintergreen,  86, 136, 125 ) \
   kip_make_standard(silver,macro) // give last

class silver : public base<silver> {
public:
   static constexpr const char *const description = "crayola::silver";

   explicit silver() : base<silver>() { }
   explicit silver(const std::string &str) : base<silver>(str) { }

   static void initialize()
   {
      assert(vec.size() == 0);
      vec = { kip_make_silver(kip_make_table) };
   }

   kip_make_silver(kip_make_decl)
   kip_make_silver(kip_make_alias)
};

kip_make_silver(kip_make_defn)

// alternative name
using SilverSwirls = silver;



// -----------------------------------------------------------------------------
// Gem Tones
// -----------------------------------------------------------------------------

#define kip_make_gem(macro) \
   macro( gem, Amethyst,    amethyst,  100,  96, 154 ) \
   macro( gem, Citrine,     citrine,   147,  55,   9 ) \
   macro( gem, Emerald,     emerald,    20, 169, 137 ) \
   macro( gem, Jade,        jade,       70, 154, 132 ) \
   macro( gem, Jasper,      jasper,    208,  83,  64 ) \
   macro( gem, LapisLazuli, lapis,      67, 108, 185 ) \
   macro( gem, Malachite,   malachite,  70, 148, 150 ) \
   macro( gem, Moonstone,   moonstone,  58, 168, 193 ) \
   macro( gem, Onyx,        onyx,       53,  56,  57 ) \
   macro( gem, Peridot,     peridot,   171, 173,  72 ) \
   macro( gem, PinkPearl,   pink,      176, 112, 128 ) \
   macro( gem, RoseQuartz,  rose,      189,  85, 156 ) \
   macro( gem, Ruby,        ruby,      170,  64, 105 ) \
   macro( gem, Sapphire,    sapphire,   45,  93, 161 ) \
   macro( gem, SmokeyTopaz, smokey,    131,  42,  13 ) \
   macro( gem, TigersEye,   tigers,    181, 105,  23 ) \
   kip_make_standard(gem,macro) // give last

class gem : public base<gem> {
public:
   static constexpr const char *const description = "crayola::gem";

   explicit gem() : base<gem>() { }
   explicit gem(const std::string &str) : base<gem>(str) { }

   static void initialize()
   {
      assert(vec.size() == 0);
      vec = { kip_make_gem(kip_make_table) };
   }

   kip_make_gem(kip_make_decl)
   kip_make_gem(kip_make_alias)
};

kip_make_gem(kip_make_defn)

// alternative name
using GemTones = gem;



// -----------------------------------------------------------------------------
// Metallic FX
// -----------------------------------------------------------------------------

#define kip_make_metallic(macro) \
   macro( metallic, AlloyOrange,         alloy,        196,  98,  16 ) \
   macro( metallic, BdazzledBlue,        bdazzled,      46,  88, 148 ) \
   macro( metallic, BigDipORuby,         big,          156,  37,  66 ) \
   macro( metallic, BittersweetShimmer,  bittersweet,  191,  79,  81 ) \
   macro( metallic, BlastOffBronze,      blast,        165, 113, 100 ) \
   macro( metallic, CyberGrape,          cyber,         88,  66, 124 ) \
   macro( metallic, DeepSpaceSparkle,    deep,          74, 100, 108 ) \
   macro( metallic, GoldFusion,          gold,         133, 117,  78 ) \
   macro( metallic, IlluminatingEmerald, illuminating,  49, 145, 119 ) \
   macro( metallic, MetallicSeaweed,     seaweed,       10, 126, 140 ) \
   macro( metallic, MetallicSunburst,    sunburst,     156, 124,  56 ) \
   macro( metallic, RazzmicBerry,        razzmic,      141,  78, 133 ) \
   macro( metallic, SheenGreen,          sheen,        143, 212,   0 ) \
   macro( metallic, ShimmeringBlush,     shimmering,   217, 134, 149 ) \
   macro( metallic, SonicSilver,         sonic,        117, 117, 117 ) \
   macro( metallic, SteelBlue,           steel,          0, 129, 171 ) \
   kip_make_standard(metallic,macro) // give last

// Above, for the original names:
//    Metallic Seaweed
//    Metallic Sunburst
// we used the second word instead of the first as the short names, for the
// obvious reason. Elsewhere, our convention has been to use the first word.

class metallic : public base<metallic> {
public:
   static constexpr const char *const description = "crayola::metallic";

   explicit metallic() : base<metallic>() { }
   explicit metallic(const std::string &str) : base<metallic>(str) { }

   static void initialize()
   {
      assert(vec.size() == 0);
      vec = { kip_make_metallic(kip_make_table) };
   }

   kip_make_metallic(kip_make_decl)
   kip_make_metallic(kip_make_alias)
};

kip_make_metallic(kip_make_defn)

// alternative name
using MetallicFX = metallic;



// -----------------------------------------------------------------------------
// crayola::complete
// -----------------------------------------------------------------------------

#define kip_make_complete(macro) \
   macro( complete, Almond,                 almond,          239, 219, 197 ) \
   macro( complete, AntiqueBrass,           antique,         205, 149, 117 ) \
   macro( complete, Apricot,                apricot,         253, 217, 181 ) \
   macro( complete, Aquamarine,             aquamarine,      120, 219, 226 ) \
   macro( complete, Asparagus,              asparagus,       135, 169, 107 ) \
   macro( complete, AtomicTangerine,        atomic,          255, 164, 116 ) \
   macro( complete, BananaMania,            banana,          250, 231, 181 ) \
   macro( complete, Beaver,                 beaver,          159, 129, 112 ) \
   macro( complete, Bittersweet,            bittersweet,     253, 124, 110 ) \
   macro( complete, Black,                  black,            35,  35,  35 ) \
   macro( complete, Blue,                   blue,             31, 117, 254 ) \
   macro( complete, BlueBell,               bluebell,        173, 173, 214 ) \
   macro( complete, BlueGreen,              bluegreen,        25, 158, 189 ) \
   macro( complete, BlueViolet,             blueviolet,      115, 102, 189 ) \
   macro( complete, Bluetiful,              bluetiful,        46,  80, 144 ) \
   macro( complete, Blush,                  blush,           222,  93, 131 ) \
   macro( complete, BrickRed,               brick,           203,  65,  84 ) \
   macro( complete, Brown,                  brown,           180, 103,  77 ) \
   macro( complete, BurntOrange,            burntorange,     255, 127,  73 ) \
   macro( complete, BurntSienna,            burntsienna,     234, 126,  93 ) \
   macro( complete, CadetBlue,              cadet,           176, 183, 198 ) \
   macro( complete, Canary,                 canary,          255, 255, 159 ) \
   macro( complete, CaribbeanGreen,         caribbean,        28, 211, 162 ) \
   macro( complete, CarnationPink,          carnation,       255, 170, 204 ) \
   macro( complete, Cerise,                 cerise,          221,  68, 146 ) \
   macro( complete, Cerulean,               cerulean,         29, 172, 214 ) \
   macro( complete, Chestnut,               chestnut,        188,  93,  88 ) \
   macro( complete, Copper,                 copper,          221, 148, 117 ) \
   macro( complete, Cornflower,             cornflower,      154, 206, 235 ) \
   macro( complete, CottonCandy,            cotton,          255, 188, 217 ) \
   macro( complete, Denim,                  denim,            43, 108, 196 ) \
   macro( complete, DesertSand,             desert,          239, 205, 184 ) \
   macro( complete, Eggplant,               eggplant,        110,  81,  96 ) \
   macro( complete, ElectricLime,           electric,         29, 249,  20 ) \
   macro( complete, Fern,                   fern,            113, 188, 120 ) \
   macro( complete, ForestGreen,            forest,          109, 174, 129 ) \
   macro( complete, Fuchsia,                fuchsia,         195, 100, 197 ) \
   macro( complete, FuzzyWuzzyBrown,        fuzzy,           204, 102, 102 ) \
   macro( complete, Gold,                   gold,            231, 198, 151 ) \
   macro( complete, Goldenrod,              goldenrod,       255, 217, 117 ) \
   macro( complete, GrannySmithApple,       granny,          168, 228, 160 ) \
   macro( complete, Gray,                   gray,            149, 145, 140 ) \
   macro( complete, Green,                  green,            28, 172, 120 ) \
   macro( complete, GreenYellow,            greenyellow,     240, 232, 145 ) \
   macro( complete, HotMagenta,             hot,             255,  29, 206 ) \
   macro( complete, InchWorm,               inch,            178, 236,  93 ) \
   macro( complete, Indigo,                 indigo,           93, 118, 203 ) \
   macro( complete, JazzberryJam,           jazzberry,       202,  55, 103 ) \
   macro( complete, JungleGreen,            jungle,           59, 176, 143 ) \
   macro( complete, LaserLemon,             laser,           253, 252, 116 ) \
   macro( complete, Lavender,               lavender,        252, 180, 213 ) \
   macro( complete, MacaroniandCheese,      macaroni,        255, 189, 136 ) \
   macro( complete, Magenta,                magenta,         246, 100, 175 ) \
   macro( complete, Mahogany,               mahogany,        205,  74,  74 ) \
   macro( complete, Manatee,                manatee,         151, 154, 170 ) \
   macro( complete, MangoTango,             mango,           255, 130,  67 ) \
   macro( complete, Maroon,                 maroon,          200,  56,  90 ) \
   macro( complete, Mauvelous,              mauvelous,       239, 152, 170 ) \
   macro( complete, Melon,                  melon,           253, 188, 180 ) \
   macro( complete, MidnightBlue,           midnight,         26,  72, 118 ) \
   macro( complete, MountainMeadow,         mountain,         48, 186, 143 ) \
   macro( complete, NavyBlue,               navy,             25, 116, 210 ) \
   macro( complete, NeonCarrot,             neon,            255, 163,  67 ) \
   macro( complete, OliveGreen,             olive,           186, 184, 108 ) \
   macro( complete, Orange,                 orange,          255, 117,  56 ) \
   macro( complete, Orchid,                 orchid,          230, 168, 215 ) \
   macro( complete, OuterSpace,             outer,            65,  74,  76 ) \
   macro( complete, OutrageousOrange,       outrageous,      255, 110,  74 ) \
   macro( complete, PacificBlue,            pacific,          28, 169, 201 ) \
   macro( complete, Peach,                  peach,           255, 207, 171 ) \
   macro( complete, Periwinkle,             periwinkle,      197, 208, 230 ) \
   macro( complete, PiggyPink,              piggy,           253, 215, 228 ) \
   macro( complete, PineGreen,              pine,             21, 128, 120 ) \
   macro( complete, PinkFlamingo,           pinkflamingo,    252, 116, 253 ) \
   macro( complete, PinkSherbet,            pinksherbet,     247, 128, 161 ) \
   macro( complete, Plum,                   plum,            142,  69, 133 ) \
   macro( complete, PurpleHeart,            purpleheart,     116,  66, 200 ) \
   macro( complete, PurpleMountainsMajesty, purplemountains, 157, 129, 186 ) \
   macro( complete, PurplePizza,            purplepizza,     255,  29, 206 ) \
   macro( complete, RadicalRed,             radical,         255,  73, 107 ) \
   macro( complete, RawSienna,              raw,             214, 138,  89 ) \
   macro( complete, RazzleDazzleRose,       razzle,          255,  72, 208 ) \
   macro( complete, Razzmatazz,             razzmatazz,      227,  37, 107 ) \
   macro( complete, Red,                    red,             238,  32,  77 ) \
   macro( complete, RedOrange,              redorange,       255,  83,  73 ) \
   macro( complete, RedViolet,              redviolet,       192,  68, 143 ) \
   macro( complete, RobinEggBlue,           robin,            31, 206, 203 ) \
   macro( complete, RoyalPurple,            royal,           120,  81, 169 ) \
   macro( complete, Salmon,                 salmon,          255, 155, 170 ) \
   macro( complete, Scarlet,                scarlet,         242,  40,  71 ) \
   macro( complete, ScreaminGreen,          screamin,        118, 255, 122 ) \
   macro( complete, SeaGreen,               sea,             159, 226, 191 ) \
   macro( complete, Sepia,                  sepia,           165, 105,  79 ) \
   macro( complete, Shadow,                 shadow,          138, 121,  93 ) \
   macro( complete, Shamrock,               shamrock,         69, 206, 162 ) \
   macro( complete, ShockingPink,           shocking,        251, 126, 253 ) \
   macro( complete, Silver,                 silver,          205, 197, 194 ) \
   macro( complete, SkyBlue,                sky,             128, 218, 235 ) \
   macro( complete, SpringGreen,            spring,          236, 234, 190 ) \
   macro( complete, Sunglow,                sunglow,         255, 207,  72 ) \
   macro( complete, SunsetOrange,           sunset,          253,  94,  83 ) \
   macro( complete, Tan,                    tan,             250, 167, 108 ) \
   macro( complete, TickleMePink,           tickle,          252, 137, 172 ) \
   macro( complete, Timberwolf,             timberwolf,      219, 215, 210 ) \
   macro( complete, TropicalRainForest,     tropical,         23, 128, 109 ) \
   macro( complete, Tumbleweed,             tumbleweed,      222, 170, 136 ) \
   macro( complete, TurquoiseBlue,          turquoise,       119, 221, 231 ) \
   macro( complete, UnmellowYellow,         unmellow,        253, 252, 116 ) \
   macro( complete, Violet, /* see below */ violet,          146, 110, 174 ) \
   macro( complete, Purple, /* see below */ purple,          146, 110, 174 ) \
   macro( complete, VioletRed,              violetred,       247,  83, 148 ) \
   macro( complete, VividTangerine,         vividtangerine,  255, 160, 137 ) \
   macro( complete, VividViolet,            vividviolet,     143,  80, 157 ) \
   macro( complete, White,                  white,           237, 237, 237 ) \
   macro( complete, WildBlueWonder,         wildblue,        162, 173, 208 ) \
   macro( complete, WildStrawberry,         wildstrawberry,  255,  67, 164 ) \
   macro( complete, WildWatermelon,         wildwatermelon,  252, 108, 133 ) \
   macro( complete, Wisteria,               wisteria,        205, 164, 222 ) \
   macro( complete, Yellow,                 yellow,          252, 232, 131 ) \
   macro( complete, YellowGreen,            yellowgreen,     197, 227, 132 ) \
   macro( complete, YellowOrange,           yelloworange,    255, 182,  83 ) \
   kip_make_standard(complete,macro) // give last

// Above, Violet and Purple are the same color. We have both simply because
// we found both terms being used. They're given as two different entries,
// as opposed to one being a reference to the other, so that the associated
// string name (for example, with I/O) will reflect the one that's used.

class complete : public base<complete> {
public:
   static constexpr const char *const description = "crayola::complete";

   explicit complete() : base<complete>() { }
   explicit complete(const std::string &str) : base<complete>(str) { }

   static void initialize()
   {
      assert(vec.size() == 0);
      vec = { kip_make_complete(kip_make_table) };
   }

   kip_make_complete(kip_make_decl)
   kip_make_complete(kip_make_alias)
};

kip_make_complete(kip_make_defn)

// alternative name
// The above are from the 120 count box, so I'll give them the following name.
// Note that we actually have more than 120 because we have our standard list
// including black, white, grays, etc. and the Violet/Purple issue (see above).
using OneTwenty = complete;



// -----------------------------------------------------------------------------
// read_value
// -----------------------------------------------------------------------------

// 2019-sep-18
// I think ISTREAM is always kip::istream, but in order to make compilation
// work with kip::istream directly, some reordering is apparently necessary.
template<class ISTREAM, class derived>
bool read_value(ISTREAM &s, crayola::base<derived> &obj)
{
   s.bail = false;
   const std::string label = obj.description();

   std::string word;
   if (read_value(s,word)) {
      for (ulong n = obj.table().size();  n-- ; )
         if (word == obj.table()[n].first)
            return obj = crayola::base<derived>(uchar(n)), !s.fail();

      const derived Default = derived::PureBlack;
      std::ostringstream oss;
      oss << "Unknown " << label << " color \"" << word << "\"\n"
          << "Setting to " << label << "::" << obj.table()[Default.id()].first;
      s.warning(oss);
      return obj = Default, !s.fail();
   }

   s.add(std::ios::failbit);
   addendum("Detected while reading " + label, diagnostic::error);
   return !s.fail();
}



// -----------------------------------------------------------------------------
// istream >> crayola::base
// ostream << crayola::base
// -----------------------------------------------------------------------------

// kip::istream >> crayola::base
template<class derived>
inline kip::istream &operator>>(
   kip::istream &k,
   base<derived> &obj
) {
   read_value(k,obj);
   return k;
}

// std::istream >> crayola::base
template<class derived>
inline std::istream &operator>>(
   std::istream &s,
   base<derived> &obj
) {
   kip::istream k(s);
   k >> obj;
   return s;
}

// kip::ostream << crayola::base
template<class derived>
kip::ostream &operator<<(
   kip::ostream &k,
   const base<derived> &obj
) {
   const ulong size = obj.table().size();

   if (ulong(obj.id()) >= size) {
      const std::string label = obj.description();

      const derived Default = derived::PureBlack;
      std::ostringstream oss;
      oss << "Index " << ulong(obj.id()) << " for " << label << " color "
          << "is outside valid range [0," <<  size-1 << "]\n"
          << "Writing as " << label << "::" << obj.table()[Default.id()].first;
      warning(oss);
      return k << obj.table()[Default.id()].first;
   }

   return k << obj.table()[obj.id()].first;
}

// std::ostream << crayola::base
template<class derived>
inline std::ostream &operator<<(
   std::ostream &s,
   const base<derived> &obj
) {
   kip::ostream k(s);
   k << obj;
   return s;
}



// -----------------------------------------------------------------------------
// convert
// -----------------------------------------------------------------------------

// crayola::base<derived> ==> RGB<comp>
template<class derived, class comp>
inline void convert(const crayola::base<derived> &in, RGB<comp> &out)
{
   out.set(in.r(), in.g(), in.b());
}

// crayola::base<derived> ==> RGBA<comp>
template<class derived, class comp>
inline void convert(const crayola::base<derived> &in, RGBA<comp> &out)
{
   out.set(in.r(), in.g(), in.b());
}

} // namespace crayola
