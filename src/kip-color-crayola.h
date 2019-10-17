
namespace crayola {

// -----------------------------------------------------------------------------
// macros
// -----------------------------------------------------------------------------

#define kip_make_table(type,name,alt,r,g,b) \
   { #name, rgb(r,g,b) },

#define kip_make_decl( type,name,alt,r,g,b) \
   static const type name;

#define kip_make_alias(type,name,alt,r,g,b) \
   static constexpr const type &alt = name;

#define kip_make_defn( type,name,alt,r,g,b) \
   template<class unused> \
   const type<unused> type<unused>::name(#name);

#define kip_make_standard(theclass,macro) \
   macro( theclass, PureBlack,   pureblack,     0,   0,   0 ) \
   macro( theclass, PureWhite,   purewhite,   255, 255, 255 ) \
   \
   macro( theclass, PureRed,     purered,     255,   0,   0 ) \
   macro( theclass, PureGreen,   puregreen,     0, 255,   0 ) \
   macro( theclass, PureBlue,    pureblue,      0,   0, 255 ) \
   \
   macro( theclass, PureCyan,    purecyan,      0, 255, 255 ) \
   macro( theclass, PureMagenta, puremagenta, 255,   0, 255 ) \
   macro( theclass, PureYellow,  pureyellow,  255, 255,   0 ) \
   \
   macro( theclass, GrayDark,    graydark,     64,  64,  64 ) \
   macro( theclass, GrayMedium,  graymedium,  128, 128, 128 ) \
   macro( theclass, GrayLight,   graylight,   192, 192, 192 ) \
   \
   macro( theclass, AxisOrigin,  axisorigin,    0,   0,   0 ) \
   macro( theclass, AxisXPos,    axisxpos,    203,  65,  84 ) \
   macro( theclass, AxisXNeg,    axisxneg,    255, 188, 217 ) \
   macro( theclass, AxisYPos,    axisypos,     59, 176, 143 ) \
   macro( theclass, AxisYNeg,    axisyneg,    168, 228, 160 ) \
   macro( theclass, AxisZPos,    axiszpos,     43, 108, 196 ) \
   macro( theclass, AxisZNeg,    axiszneg,    154, 206, 235 )

   // Note: the six x/y/z pos/neg axis colors are, respectively,
   // the same as the following colors from crayola::complete:
   //
   // AxisXPos == BrickRed
   // AxisXNeg == CottonCandy
   // AxisYPos == JungleGreen
   // AxisYNeg == GrannySmithApple
   // AxisZPos == Denim
   // AxisZNeg == Cornflower

// MAGIC NUMBER ALERT
// Number of values given in the above list of colors
namespace detail {
   inline const ulong crayola_standard_count = 18;
}



// -----------------------------------------------------------------------------
// crayola::base
// -----------------------------------------------------------------------------

template<class derived>
class base {

   // data
   uchar index;

   // ------------------------
   // constructors
   // ------------------------

protected:

   // base(string)
   explicit base(const std::string &name)
   {
      for (ulong n = size();  n-- ; )
         if (name == derived::table()[n].first) {
            index = uchar(n);
            return;
         }
      assert(false);
   }

public:

   // base()
   // base(n)
   explicit base(const uchar n = 0) : index(n)
   {
      kip_assert(n < size());
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
   uchar r() const { return derived::table()[index].second.r; }
   uchar g() const { return derived::table()[index].second.g; }
   uchar b() const { return derived::table()[index].second.b; }

   // operator[]
   const std::pair<std::string,rgb> &operator[](const ulong n) const
   {
      return derived::table()[n];
   }

   // randomize()
   void randomize()
   {
      kip_assert(size() > detail::crayola_standard_count);
      index = uchar(rand() % (size() - detail::crayola_standard_count));
   }

   // ------------------------
   // functions: static
   // ------------------------

   // size()
   static ulong size()
   {
      return derived::table().size();
   }

   // description()
   static std::string description() { return derived::description; }
};


// randomize
// External; not the one in the class (but forwards to it)
template<class derived>
inline void randomize(base<derived> &obj)
{
   obj.randomize();
}



// -----------------------------------------------------------------------------
// crayola::Pure<>
// crayola::pure
//
// With these and our other color sets, below, we have both a templated and
// a non-templated class, instead of just the latter, so that our member
// color definitions, done with kip_make_defn, work correctly in the context
// of a header-only library. Essentially, we need to make those definitions
// be templated, or we'll end up with multiple definitions.
// -----------------------------------------------------------------------------

#define kip_make_pure(macro) \
   macro( Pure, Black,   black,     0,   0,   0 ) \
   macro( Pure, White,   white,   255, 255, 255 ) \
   macro( Pure, Red,     red,     255,   0,   0 ) \
   macro( Pure, Green,   green,     0, 255,   0 ) \
   macro( Pure, Blue,    blue,      0,   0, 255 ) \
   macro( Pure, Cyan,    cyan,      0, 255, 255 ) \
   macro( Pure, Magenta, magenta, 255,   0, 255 ) \
   macro( Pure, Yellow,  yellow,  255, 255,   0 ) \
   macro( Pure, Dark,    dark,     64,  64,  64 ) \
   macro( Pure, Medium,  medium,  128, 128, 128 ) \
   macro( Pure, Light,   light,   192, 192, 192 ) \
   kip_make_standard(Pure,macro) // give last

template<class unused = char>
class Pure : public base<Pure<unused>> {
public:
   static constexpr const char *const description = "crayola::pure";

   explicit Pure() : base<Pure>() { }
   explicit Pure(const std::string &name) : base<Pure>(name) { }

   static auto &table()
   {
      static const std::vector<std::pair<std::string,rgb>>
         tab { kip_make_pure(kip_make_table) };
      return tab;
   }

   kip_make_pure(kip_make_decl)
   kip_make_pure(kip_make_alias)
};

kip_make_pure(kip_make_defn)
using pure = Pure<>;



// -----------------------------------------------------------------------------
// Silver Swirls
// -----------------------------------------------------------------------------

#define kip_make_silver(macro) \
   macro( Silver, AztecGold,        aztec,       195, 153,  83 ) \
   macro( Silver, BurnishedBrown,   burnished,   161, 122, 116 ) \
   macro( Silver, CeruleanFrost,    cerulean,    109, 155, 195 ) \
   macro( Silver, CinnamonSatin,    cinnamon,    205,  96, 126 ) \
   macro( Silver, CopperPenny,      copper,      173, 111, 105 ) \
   macro( Silver, CosmicCobalt,     cosmic,       46,  45, 136 ) \
   macro( Silver, GlossyGrape,      glossy,      171, 146, 179 ) \
   macro( Silver, GraniteGray,      granite,     103, 103, 103 ) \
   macro( Silver, GreenSheen,       green,       110, 174, 161 ) \
   macro( Silver, LilacLuster,      lilac,       174, 152, 170 ) \
   macro( Silver, MistyMoss,        misty,       187, 180, 119 ) \
   macro( Silver, MysticMaroon,     mystic,      173,  67, 121 ) \
   macro( Silver, PearlyPurple,     pearly,      183, 104, 162 ) \
   macro( Silver, PewterBlue,       pewter,      139, 168, 183 ) \
   macro( Silver, PolishedPine,     polished,     93, 164, 147 ) \
   macro( Silver, QuickSilver,      quick,       166, 166, 166 ) \
   macro( Silver, RoseDust,         rose,        158,  94, 111 ) \
   macro( Silver, RustyRed,         rusty,       218,  44,  67 ) \
   macro( Silver, ShadowBlue,       shadow,      119, 139, 165 ) \
   macro( Silver, ShinyShamrock,    shiny,        95, 167, 120 ) \
   macro( Silver, SteelTeal,        steel,        95, 138, 139 ) \
   macro( Silver, SugarPlum,        sugar,       145,  78, 117 ) \
   macro( Silver, TwilightLavender, twilight,    138,  73, 107 ) \
   macro( Silver, WintergreenDream, wintergreen,  86, 136, 125 ) \
   kip_make_standard(Silver,macro) // give last

template<class unused = char>
class Silver : public base<Silver<unused>> {
public:
   static constexpr const char *const description = "crayola::silver";

   explicit Silver() : base<Silver>() { }
   explicit Silver(const std::string &name) : base<Silver>(name) { }

   static auto &table()
   {
      static const std::vector<std::pair<std::string,rgb>>
         tab { kip_make_silver(kip_make_table) };
      return tab;
   }

   kip_make_silver(kip_make_decl)
   kip_make_silver(kip_make_alias)
};

kip_make_silver(kip_make_defn)
using silver = Silver<>;

// alternative name
using SilverSwirls = silver;



// -----------------------------------------------------------------------------
// Gem Tones
// -----------------------------------------------------------------------------

#define kip_make_gem(macro) \
   macro( Gem, Amethyst,    amethyst,  100,  96, 154 ) \
   macro( Gem, Citrine,     citrine,   147,  55,   9 ) \
   macro( Gem, Emerald,     emerald,    20, 169, 137 ) \
   macro( Gem, Jade,        jade,       70, 154, 132 ) \
   macro( Gem, Jasper,      jasper,    208,  83,  64 ) \
   macro( Gem, LapisLazuli, lapis,      67, 108, 185 ) \
   macro( Gem, Malachite,   malachite,  70, 148, 150 ) \
   macro( Gem, Moonstone,   moonstone,  58, 168, 193 ) \
   macro( Gem, Onyx,        onyx,       53,  56,  57 ) \
   macro( Gem, Peridot,     peridot,   171, 173,  72 ) \
   macro( Gem, PinkPearl,   pink,      176, 112, 128 ) \
   macro( Gem, RoseQuartz,  rose,      189,  85, 156 ) \
   macro( Gem, Ruby,        ruby,      170,  64, 105 ) \
   macro( Gem, Sapphire,    sapphire,   45,  93, 161 ) \
   macro( Gem, SmokeyTopaz, smokey,    131,  42,  13 ) \
   macro( Gem, TigersEye,   tigers,    181, 105,  23 ) \
   kip_make_standard(Gem,macro) // give last

template<class unused = char>
class Gem : public base<Gem<unused>> {
public:
   static constexpr const char *const description = "crayola::gem";

   explicit Gem() : base<Gem>() { }
   explicit Gem(const std::string &name) : base<Gem>(name) { }

   static auto &table()
   {
      static const std::vector<std::pair<std::string,rgb>>
         tab { kip_make_gem(kip_make_table) };
      return tab;
   }

   kip_make_gem(kip_make_decl)
   kip_make_gem(kip_make_alias)
};

kip_make_gem(kip_make_defn)
using gem = Gem<>;

// alternative name
using GemTones = gem;



// -----------------------------------------------------------------------------
// Metallic FX
// -----------------------------------------------------------------------------

#define kip_make_metallic(macro) \
   macro( Metallic, AlloyOrange,         alloy,        196,  98,  16 ) \
   macro( Metallic, BdazzledBlue,        bdazzled,      46,  88, 148 ) \
   macro( Metallic, BigDipORuby,         big,          156,  37,  66 ) \
   macro( Metallic, BittersweetShimmer,  bittersweet,  191,  79,  81 ) \
   macro( Metallic, BlastOffBronze,      blast,        165, 113, 100 ) \
   macro( Metallic, CyberGrape,          cyber,         88,  66, 124 ) \
   macro( Metallic, DeepSpaceSparkle,    deep,          74, 100, 108 ) \
   macro( Metallic, GoldFusion,          gold,         133, 117,  78 ) \
   macro( Metallic, IlluminatingEmerald, illuminating,  49, 145, 119 ) \
   macro( Metallic, MetallicSeaweed,     seaweed,       10, 126, 140 ) \
   macro( Metallic, MetallicSunburst,    sunburst,     156, 124,  56 ) \
   macro( Metallic, RazzmicBerry,        razzmic,      141,  78, 133 ) \
   macro( Metallic, SheenGreen,          sheen,        143, 212,   0 ) \
   macro( Metallic, ShimmeringBlush,     shimmering,   217, 134, 149 ) \
   macro( Metallic, SonicSilver,         sonic,        117, 117, 117 ) \
   macro( Metallic, SteelBlue,           steel,          0, 129, 171 ) \
   kip_make_standard(Metallic,macro) // give last

// Above, for the original names:
//    Metallic Seaweed
//    Metallic Sunburst
// we used the second word instead of the first as the short names, for the
// obvious reason. Elsewhere, our convention has been to use the first word.

template<class unused = char>
class Metallic : public base<Metallic<unused>> {
public:
   static constexpr const char *const description = "crayola::metallic";

   explicit Metallic() : base<Metallic>() { }
   explicit Metallic(const std::string &name) : base<Metallic>(name) { }

   static auto &table()
   {
      static const std::vector<std::pair<std::string,rgb>>
         tab { kip_make_metallic(kip_make_table) };
      return tab;
   }

   kip_make_metallic(kip_make_decl)
   kip_make_metallic(kip_make_alias)
};

kip_make_metallic(kip_make_defn)
using metallic = Metallic<>;

// alternative name
using MetallicFX = metallic;



// -----------------------------------------------------------------------------
// crayola::Complete
// -----------------------------------------------------------------------------

#define kip_make_complete(macro) \
   macro( Complete, Almond,                 almond,          239, 219, 197 ) \
   macro( Complete, AntiqueBrass,           antique,         205, 149, 117 ) \
   macro( Complete, Apricot,                apricot,         253, 217, 181 ) \
   macro( Complete, Aquamarine,             aquamarine,      120, 219, 226 ) \
   macro( Complete, Asparagus,              asparagus,       135, 169, 107 ) \
   macro( Complete, AtomicTangerine,        atomic,          255, 164, 116 ) \
   macro( Complete, BananaMania,            banana,          250, 231, 181 ) \
   macro( Complete, Beaver,                 beaver,          159, 129, 112 ) \
   macro( Complete, Bittersweet,            bittersweet,     253, 124, 110 ) \
   macro( Complete, Black,                  black,            35,  35,  35 ) \
   macro( Complete, Blue,                   blue,             31, 117, 254 ) \
   macro( Complete, BlueBell,               bluebell,        173, 173, 214 ) \
   macro( Complete, BlueGreen,              bluegreen,        25, 158, 189 ) \
   macro( Complete, BlueViolet,             blueviolet,      115, 102, 189 ) \
   macro( Complete, Bluetiful,              bluetiful,        46,  80, 144 ) \
   macro( Complete, Blush,                  blush,           222,  93, 131 ) \
   macro( Complete, BrickRed,               brick,           203,  65,  84 ) \
   macro( Complete, Brown,                  brown,           180, 103,  77 ) \
   macro( Complete, BurntOrange,            burntorange,     255, 127,  73 ) \
   macro( Complete, BurntSienna,            burntsienna,     234, 126,  93 ) \
   macro( Complete, CadetBlue,              cadet,           176, 183, 198 ) \
   macro( Complete, Canary,                 canary,          255, 255, 159 ) \
   macro( Complete, CaribbeanGreen,         caribbean,        28, 211, 162 ) \
   macro( Complete, CarnationPink,          carnation,       255, 170, 204 ) \
   macro( Complete, Cerise,                 cerise,          221,  68, 146 ) \
   macro( Complete, Cerulean,               cerulean,         29, 172, 214 ) \
   macro( Complete, Chestnut,               chestnut,        188,  93,  88 ) \
   macro( Complete, Copper,                 copper,          221, 148, 117 ) \
   macro( Complete, Cornflower,             cornflower,      154, 206, 235 ) \
   macro( Complete, CottonCandy,            cotton,          255, 188, 217 ) \
   macro( Complete, Denim,                  denim,            43, 108, 196 ) \
   macro( Complete, DesertSand,             desert,          239, 205, 184 ) \
   macro( Complete, Eggplant,               eggplant,        110,  81,  96 ) \
   macro( Complete, ElectricLime,           electric,         29, 249,  20 ) \
   macro( Complete, Fern,                   fern,            113, 188, 120 ) \
   macro( Complete, ForestGreen,            forest,          109, 174, 129 ) \
   macro( Complete, Fuchsia,                fuchsia,         195, 100, 197 ) \
   macro( Complete, FuzzyWuzzyBrown,        fuzzy,           204, 102, 102 ) \
   macro( Complete, Gold,                   gold,            231, 198, 151 ) \
   macro( Complete, Goldenrod,              goldenrod,       255, 217, 117 ) \
   macro( Complete, GrannySmithApple,       granny,          168, 228, 160 ) \
   macro( Complete, Gray,                   gray,            149, 145, 140 ) \
   macro( Complete, Green,                  green,            28, 172, 120 ) \
   macro( Complete, GreenYellow,            greenyellow,     240, 232, 145 ) \
   macro( Complete, HotMagenta,             hot,             255,  29, 206 ) \
   macro( Complete, InchWorm,               inch,            178, 236,  93 ) \
   macro( Complete, Indigo,                 indigo,           93, 118, 203 ) \
   macro( Complete, JazzberryJam,           jazzberry,       202,  55, 103 ) \
   macro( Complete, JungleGreen,            jungle,           59, 176, 143 ) \
   macro( Complete, LaserLemon,             laser,           253, 252, 116 ) \
   macro( Complete, Lavender,               lavender,        252, 180, 213 ) \
   macro( Complete, MacaroniandCheese,      macaroni,        255, 189, 136 ) \
   macro( Complete, Magenta,                magenta,         246, 100, 175 ) \
   macro( Complete, Mahogany,               mahogany,        205,  74,  74 ) \
   macro( Complete, Manatee,                manatee,         151, 154, 170 ) \
   macro( Complete, MangoTango,             mango,           255, 130,  67 ) \
   macro( Complete, Maroon,                 maroon,          200,  56,  90 ) \
   macro( Complete, Mauvelous,              mauvelous,       239, 152, 170 ) \
   macro( Complete, Melon,                  melon,           253, 188, 180 ) \
   macro( Complete, MidnightBlue,           midnight,         26,  72, 118 ) \
   macro( Complete, MountainMeadow,         mountain,         48, 186, 143 ) \
   macro( Complete, NavyBlue,               navy,             25, 116, 210 ) \
   macro( Complete, NeonCarrot,             neon,            255, 163,  67 ) \
   macro( Complete, OliveGreen,             olive,           186, 184, 108 ) \
   macro( Complete, Orange,                 orange,          255, 117,  56 ) \
   macro( Complete, Orchid,                 orchid,          230, 168, 215 ) \
   macro( Complete, OuterSpace,             outer,            65,  74,  76 ) \
   macro( Complete, OutrageousOrange,       outrageous,      255, 110,  74 ) \
   macro( Complete, PacificBlue,            pacific,          28, 169, 201 ) \
   macro( Complete, Peach,                  peach,           255, 207, 171 ) \
   macro( Complete, Periwinkle,             periwinkle,      197, 208, 230 ) \
   macro( Complete, PiggyPink,              piggy,           253, 215, 228 ) \
   macro( Complete, PineGreen,              pine,             21, 128, 120 ) \
   macro( Complete, PinkFlamingo,           pinkflamingo,    252, 116, 253 ) \
   macro( Complete, PinkSherbet,            pinksherbet,     247, 128, 161 ) \
   macro( Complete, Plum,                   plum,            142,  69, 133 ) \
   macro( Complete, PurpleHeart,            purpleheart,     116,  66, 200 ) \
   macro( Complete, PurpleMountainsMajesty, purplemountains, 157, 129, 186 ) \
   macro( Complete, PurplePizza,            purplepizza,     255,  29, 206 ) \
   macro( Complete, RadicalRed,             radical,         255,  73, 107 ) \
   macro( Complete, RawSienna,              raw,             214, 138,  89 ) \
   macro( Complete, RazzleDazzleRose,       razzle,          255,  72, 208 ) \
   macro( Complete, Razzmatazz,             razzmatazz,      227,  37, 107 ) \
   macro( Complete, Red,                    red,             238,  32,  77 ) \
   macro( Complete, RedOrange,              redorange,       255,  83,  73 ) \
   macro( Complete, RedViolet,              redviolet,       192,  68, 143 ) \
   macro( Complete, RobinEggBlue,           robin,            31, 206, 203 ) \
   macro( Complete, RoyalPurple,            royal,           120,  81, 169 ) \
   macro( Complete, Salmon,                 salmon,          255, 155, 170 ) \
   macro( Complete, Scarlet,                scarlet,         242,  40,  71 ) \
   macro( Complete, ScreaminGreen,          screamin,        118, 255, 122 ) \
   macro( Complete, SeaGreen,               sea,             159, 226, 191 ) \
   macro( Complete, Sepia,                  sepia,           165, 105,  79 ) \
   macro( Complete, Shadow,                 shadow,          138, 121,  93 ) \
   macro( Complete, Shamrock,               shamrock,         69, 206, 162 ) \
   macro( Complete, ShockingPink,           shocking,        251, 126, 253 ) \
   macro( Complete, Silver,                 silver,          205, 197, 194 ) \
   macro( Complete, SkyBlue,                sky,             128, 218, 235 ) \
   macro( Complete, SpringGreen,            spring,          236, 234, 190 ) \
   macro( Complete, Sunglow,                sunglow,         255, 207,  72 ) \
   macro( Complete, SunsetOrange,           sunset,          253,  94,  83 ) \
   macro( Complete, Tan,                    tan,             250, 167, 108 ) \
   macro( Complete, TickleMePink,           tickle,          252, 137, 172 ) \
   macro( Complete, Timberwolf,             timberwolf,      219, 215, 210 ) \
   macro( Complete, TropicalRainForest,     tropical,         23, 128, 109 ) \
   macro( Complete, Tumbleweed,             tumbleweed,      222, 170, 136 ) \
   macro( Complete, TurquoiseBlue,          turquoise,       119, 221, 231 ) \
   macro( Complete, UnmellowYellow,         unmellow,        253, 252, 116 ) \
   macro( Complete, Violet, /* see below */ violet,          146, 110, 174 ) \
   macro( Complete, Purple, /* see below */ purple,          146, 110, 174 ) \
   macro( Complete, VioletRed,              violetred,       247,  83, 148 ) \
   macro( Complete, VividTangerine,         vividtangerine,  255, 160, 137 ) \
   macro( Complete, VividViolet,            vividviolet,     143,  80, 157 ) \
   macro( Complete, White,                  white,           237, 237, 237 ) \
   macro( Complete, WildBlueWonder,         wildblue,        162, 173, 208 ) \
   macro( Complete, WildStrawberry,         wildstrawberry,  255,  67, 164 ) \
   macro( Complete, WildWatermelon,         wildwatermelon,  252, 108, 133 ) \
   macro( Complete, Wisteria,               wisteria,        205, 164, 222 ) \
   macro( Complete, Yellow,                 yellow,          252, 232, 131 ) \
   macro( Complete, YellowGreen,            yellowgreen,     197, 227, 132 ) \
   macro( Complete, YellowOrange,           yelloworange,    255, 182,  83 ) \
   kip_make_standard(Complete,macro) // give last

// Above, Violet and Purple are the same color. We have both simply because
// we found both terms being used. They're given as two different entries,
// as opposed to one being a reference to the other, so that the associated
// string name (for example, with I/O) will reflect the one that's used.

template<class unused = char>
class Complete : public base<Complete<unused>> {
public:
   static constexpr const char *const description = "crayola::complete";

   explicit Complete() : base<Complete>() { }
   explicit Complete(const std::string &name) : base<Complete>(name) { }

   static auto &table()
   {
      static const std::vector<std::pair<std::string,rgb>>
         tab { kip_make_complete(kip_make_table) };
      return tab;
   }

   kip_make_complete(kip_make_decl)
   kip_make_complete(kip_make_alias)
};

kip_make_complete(kip_make_defn)
using complete = Complete<>;

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
// Same remark may apply to read_value for marble.
template<class ISTREAM, class derived>
bool read_value(ISTREAM &s, crayola::base<derived> &obj)
{
   s.bail = false;
   const std::string label = obj.description();

   std::string word;
   if (read_value(s,word)) {
      for (ulong n = obj.size();  n-- ; )
         if (word == obj[n].first)
            return obj = crayola::base<derived>(uchar(n)), !s.fail();

      const derived Default = derived::PureBlack;
      std::ostringstream oss;
      oss << "Unknown " << label << " color \"" << word << "\"\n"
          << "Setting to " << label << "::" << obj[Default.id()].first;
      s.warning(oss);
      return obj = Default, !s.fail();
   }

   s.add(std::ios::failbit);
   addendum("Detected while reading " + label, diagnostic::error);
   return !s.fail();
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
