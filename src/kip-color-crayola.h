
#pragma once

namespace crayola {



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
      kip_assert(vec.size() != 0);
      index = uchar(rand() % vec.size());
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
// -----------------------------------------------------------------------------
// crayola::*
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define kip_make_table(ns,str,alt,r,g,b) { #str, rgb(r,g,b) },
#define kip_make_decl( ns,str,alt,r,g,b) static const ns str;
#define kip_make_alias(ns,str,alt,r,g,b) static constexpr const ns &alt=str;
#define kip_make_defn( ns,str,alt,r,g,b) const ns ns::str(#str);



// -----------------------------------------------------------------------------
// crayola::pure
// -----------------------------------------------------------------------------

#define kip_make_pure(macro) \
   macro( pure, Black, black,   0,   0,   0 ) \
   macro( pure, White, white, 255, 255, 255 ) \
   macro( pure, Red,     red,     255,   0,   0 ) \
   macro( pure, Green,   green,     0, 255,   0 ) \
   macro( pure, Blue,    blue,      0,   0, 255 ) \
   macro( pure, Cyan,    cyan,      0, 255, 255 ) \
   macro( pure, Magenta, magenta, 255,   0, 255 ) \
   macro( pure, Yellow,  yellow,  255, 255,   0 )

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

// colors
kip_make_pure(kip_make_defn)



// -----------------------------------------------------------------------------
// Silver Swirls
// -----------------------------------------------------------------------------

#define kip_make_silver(macro) \
   macro( silver, Black, black, 0,   0,     0 ) \
   macro( silver, White, white, 255, 255, 255 ) \
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
   macro( silver, WintergreenDream, wintergreen,  86, 136, 125 )

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

// colors
kip_make_silver(kip_make_defn)

// alternative name
using SilverSwirls = silver;



// -----------------------------------------------------------------------------
// Gem Tones
// -----------------------------------------------------------------------------

#define kip_make_gem(macro) \
   macro( gem, Black, black,   0,   0,   0 ) \
   macro( gem, White, white, 255, 255, 255 ) \
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
   macro( gem, TigersEye,   tigers,    181, 105,  23 )

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

// colors
kip_make_gem(kip_make_defn)

// alternative name
using GemTones = gem;



// -----------------------------------------------------------------------------
// Metallic FX
// -----------------------------------------------------------------------------

#define kip_make_metallic(macro) \
   macro( metallic, Black, black,   0,   0,   0 ) \
   macro( metallic, White, white, 255, 255, 255 ) \
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
   macro( metallic, SteelBlue,           steel,          0, 129, 171 )

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

// colors
kip_make_metallic(kip_make_defn)

// alternative name
using MetallicFX = metallic;



// -----------------------------------------------------------------------------
// crayola::complete
// zzz Put in all of these!!
// -----------------------------------------------------------------------------

/*

Almond               EFDBC5   239,219,197
Antique Brass        CD9575   205,149,117
Apricot              FDD9B5   253,217,181
Aquamarine           78DBE2   120,219,226
Asparagus            87A96B   135,169,107
Atomic Tangerine     FFA474   255,164,116
Banana Mania         FAE7B5   250,231,181
Beaver               9F8170   159,129,112
Bittersweet          FD7C6E   253,124,110
Black                232323   35,35,35
Blue                 1F75FE   31,117,254
Blue Bell            ADADD6   173,173,214
Blue Green           199EBD   25,158,189
Blue Violet          2E5090   115,102,189
Bluetiful            7366BD   46,80,144
Blush                DE5D83   222,93,131
Brick Red            CB4154   203,65,84
Brown                B5674D   180,103,77
Burnt Orange         FF7F49   255,127,73
Burnt Sienna         EA7E5D   234,126,93
Cadet Blue           B0B7C6   176,183,198
Canary               FFFF99   255,255,159
Caribbean Green      1CD3A2   28,211,162
Carnation Pink       FFAACC   255,170,204
Cerise               FF43A4   221,68,146
Cerulean             1DACD6   29,172,214
Chesnut              BC5D58   188,93,88
Copper               DD9475   221,148,117
Cornflower           9ACEEB   154,206,235
Cotton Candy         FFBCD9   255,188,217
Denim                2B6CC4   43,108,196
Desert Sand          EFCDB8   239,205,184
Eggplant             DD4492   110,81,96
Electric Lime        1DF914   29,249,20
Fern                 71BC78   113,188,120
Forest Green         6DAE81   109,174,129
Fuchisia             C364C5   195,100,197
Fuzzy Wuzzy Brown    CC6666   204,102,102
Gold                 E7C697   231,198,151
Goldenrod            FCD975   255,217,117
Granny Smith Apple   A8E4A0   168,228,160
Gray                 95918C   149,145,140
Green                1CAC78   28,172,120
Green Yellow         F0E891   240,232,145
Hot Magenta          FF1DCE   255,29,206
Inch Worm            B2EC5D   178,236,93
Indigo               5D76CB   93,118,203
Jazzberry Jam        CA3767   202,55,103
Jungle Green         3BB08F   59,176,143
Laser Lemon          FDFC74   253,252,116
Lavender             FCB4D5   252,180,213
Macaroni and Cheese  FFBD88   255,189,136
Magenta              FCB4D5   246,100,175
Mahogany             CD4A4A   205,74,74
Manatee              979AAA   151,154,170
Mango Tango          FF8243   255,130,67
Maroon               C8385A   200,56,90
Mauvelous            EF98AA   239,152,170
Melon                FDBCB4   253,188,180
Midnight Blue        1A4876   26,72,118
Mountain Meadow      30BA8F   48,186,143
Navy Blue            1974D2   25,116,210
Neon Carrot          FFA343   255,163,67
Olive Green          BAB86C   186,184,108
Orange               FF7538   255,117,56
Orichid              C0448F   230,168,215
Outer Space          414AAC   65,74,76
Outrageous Orange    FF6E4A   255,110,74
Pacific Blue         1CA9C9   28,169,201
Peach                FFCFAB   255,207,171
Periwinkle           C5D0E6   197,208,230
Piggy Pink           FDD7E4   253,215,228
Pine Green           158078   21,128,120
Pink Flamingo        FC74FD   252,116,253
Pink Sherbet         F780A1   247,128,161
Plum                 8E4584   142,69,133
Purple Heart         7442C8   116,66,200
Purple Mountains' Magesty  9D81BA   157,129,186
Purple Pizza         FF1DCE   255,29,206
Radical Red          FF496C   255,73,107
Raw Sienna           D68A59   214,138,89
Razzle Dazzle Rose   E6A8D7   255,72,208
Razzmatazz           E3256B   227,37,107
Red                  EE204D   238,32,77
Red Orange           FF5349   255,83,73
Red Violet           c0448f   192,68,143
Robin Egg Blue       1FCECB   31,206,203
Royal Purple         7851A9   120,81,169
Salmon               FF9BAA   255,155,170
Scarlet              FC2847   242,40,71
Screamin Green       76FF7A   118,255,122
Sea Green            9FE2BF   159,226,191
Sepia                A5694F   165,105,79
Shadow               8A795D   138,121,93
Shamrock             45CEA2   69,206,162
Shocking Pink        FB7EFD   251,126,253
Silver               CDC5C2   205,197,194
Sky Blue             80DAEB   128,218,235
Spring Green         ECEABE   236,234,190
Sunglow              FFCF48   255,207,72
Sunset Orange        FD5E53   253,94,83
Tan                  FAA76C   250,167,108
Tickle Me Pink       FC89AC   252,137,172
Timberwolf           DBD7D2   219,215,210
Tropical Rain Forest 17806D   23,128,109
Tumbleweed           DEAA88   222,170,136
Turquoise Blue       77DDE7   119,221,231
Unmellow Yellow      FDFC74   253,252,116
Violet (Purple)      926EAE   146,110,174
Violet Red           F75394   247,83,148
Vivid Tangerine      FFA089   255,160,137
Vivid Violet         8F509D   143,80,157
White                EDEDED   237,237,237
Wild Blue Wonder     A2ADD0   162,173,208
Wild Strawberry      F664AF   255,67,164
Wild Watermelon      FC6C85   252,108,133
Wisteria             CDA4DE   205,164,222
Yellow               FCE883   252,232,131
Yellow Green         C5E384   197,227,132
Yellow Orange        FFB653   255,182,83

*/

#define kip_make_complete(macro) \
   macro( complete, Black,   black,     0,   0,   0 ) \
   macro( complete, White,   white,   255, 255, 255 )

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

// colors
kip_make_complete(kip_make_defn)



// -----------------------------------------------------------------------------
// read_value
// -----------------------------------------------------------------------------

// 2019-sep-18
// I think ISTREAM is always a kip::istream, but to make compilation
// work with the change, some reordering is apparently needed.
template<class ISTREAM, class derived>
bool read_value(ISTREAM &s, crayola::base<derived> &value)
{
   s.bail = false;
   const std::string label = value.description();

   std::string word;
   if (read_value(s,word)) {
      for (ulong n = value.table().size();  n-- ; )
         if (word == value.table()[n].first)
            return value = crayola::base<derived>(uchar(n)), !s.fail();

      std::ostringstream oss;
      oss << "Unknown " << label << " color \"" << word
          << "\"\nSetting to " << label << "(0) (rgb (0,0,0))";
      s.warning(oss);
      return value = crayola::base<derived>(0), !s.fail();
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
inline kip::ostream &operator<<(
   kip::ostream &k,
   const base<derived> &obj
) {
   const ulong size = obj.table().size();

   if (ulong(obj.id()) >= size) {
      const std::string label = obj.description();
      std::ostringstream oss;
      oss << "Index " << unsigned(obj.id()) << " for " << label << " color "
          << "is outside valid range [0,"
          <<  size-1 << "]\nWriting as " << label << "(0) (rgb (0,0,0))";
      warning(oss);
      return k << obj.table()[0].first;
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
