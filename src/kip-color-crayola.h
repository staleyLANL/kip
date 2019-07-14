
#pragma once

// This file defines the crayola class, which supports Crayola crayon colors.

class crayola;
using crayola_id_t = unsigned char;
using crayola_rgb_t = unsigned char;



// -----------------------------------------------------------------------------
// crayola_base
// Effectively hides crayola's description string and specific colors inside
// a template, to avoid multiply-defined symbols when multiple-source linking.
// -----------------------------------------------------------------------------

namespace internal {

template<class>
class crayola_base {

protected:
   crayola_id_t index;

public:
   inline explicit crayola_base() : index(0) { }
   inline explicit crayola_base(const crayola_id_t &_index) : index(_index) { }
   static const char *const description;

   // specific colors
   static const crayola
      unknown,

      almond, antique_brass, apricot, aquamarine,
      asparagus, atomic_tangerine, banana_mania, beaver,
      bittersweet, black, blizzard_blue, blue,

      blue_bell, /*blue_gray,*/ blue_green, blue_violet,
      brick_red, brink_pink, brown, burnt_orange,
      burnt_sienna, cadet_blue, canary, caribbean_green,

      carnation_pink, cerise, cerulean, chartreuse,
      chestnut, copper, cornflower, cotton_candy,
      cranberry, dandelion, denim, desert_sand,

      eggplant, electric_lime, fern, flesh,
      forest_green, fuchsia, fuzzy_wuzzy_brown, gold,
      goldenrod, granny_smith_apple, gray, green,

      /*green_blue,*/ green_yellow, hot_magenta, inch_worm,
      indian_red, indigo, jazzberry_jam, jungle_green,
      laser_lemon, lavender, /*lemon_yellow,*/ macaroni_and_cheese,

      magenta, magic_mint, mahogany, /*maize,*/
      manatee, mango_tango, maroon, mauvelous,
      melon, midnight_blue, mountain_meadow, mulberry,

      navy_blue, neon_carrot, olive_green, orange,
      /*orange_red,*/ /*orange_yellow,*/ orchid, outer_space,
      outrageous_orange, pacific_blue, peach, periwinkle,

      pig_pink, pine_green, pink_flamingo, plum,
      prussian_blue, purple_heart, purple_mountains_majesty, purple_pizzazz,
      radical_red, raw_sienna, /*raw_umber,*/ razzle_dazzle_rose,

      razzmatazz, red, red_orange, red_violet,
      robins_egg_blue, royal_purple, salmon, scarlet,
      screamin_green, sea_green, sepia, shadow,

      shamrock, shocking_pink, silver, sky_blue,
      spring_green, sunglow, sunset_orange, tan,
      /*teal_blue,*/ /*thistle,*/ tickle_me_pink, timberwolf,

      torch_red, tropical_rain_forest, tumbleweed, turquoise_blue,
      /*ultra_blue,*/ ultra_green, ultra_orange, ultra_pink,
      ultra_red, ultra_yellow, unmellow_yellow, violet,

      /*violet_blue,*/ violet_red, vivid_tangerine, vivid_violet,
      white, wild_blue_yonder, wild_strawberry, wild_watermelon,
      wisteria, yellow, yellow_green, yellow_orange,

      // the following are kip-specific; they aren't from crayola
      gray_dark, gray_medium, gray_light
   ;
};

// description
template<class unused>
const char *const crayola_base<unused>::description = "crayola";

} // namespace internal



// -----------------------------------------------------------------------------
// crayola
// -----------------------------------------------------------------------------

class crayola : public internal::crayola_base<char> {
public:

   // color_table()
   static inline std::vector<std::pair<RGB<crayola_rgb_t>, std::string>> &
      color_table();

   // crayola(), crayola(crayola_id_t)
   inline explicit crayola() { }
   inline explicit crayola(const crayola_id_t &_index) :
      internal::crayola_base<char>(_index)
   { }

   // crayola(crayola)
   constexpr crayola(const crayola &) = default;

   // id()
   inline crayola_id_t id() const { return index; }

   // operator=
   inline crayola &operator=(const crayola &from)
      { index = from.index;  return *this; }

   /*
   // conversion to RGB<crayola_rgb_t>
   inline operator const RGB<crayola_rgb_t> &() const
      { return color_table()[index].first; }

   // conversion to RGBA<crayola_rgb_t>
   inline operator RGBA<crayola_rgb_t>() const
      { return color_table()[index].first; }
   */
};



// randomize
inline crayola &randomize(crayola &obj)
{
   // doesn't include "unknown"
   return obj = crayola(
      crayola_id_t(
         1 +
         random_unit<double>() * double(crayola::color_table().size() - 1)
      )
   );
}



// -----------------------------------------------------------------------------
// color_table
// -----------------------------------------------------------------------------

namespace internal {

// color_table_initialize
template<class>
void color_table_initialize(
   std::vector<std::pair<RGB<crayola_rgb_t>, std::string>> &table
) {
   assert(table.size() == 0);  // should visit this function only once

   using rgb_t = RGB<crayola_rgb_t>;
   using pair = std::pair<rgb_t,std::string>;

   table.push_back(pair( rgb_t(0x00, 0x00, 0x00), "unknown"));
   table.push_back(pair( rgb_t(0xee, 0xd9, 0xc4), "almond"));
   table.push_back(pair( rgb_t(0xc8, 0x8a, 0x65), "antique_brass"));
   table.push_back(pair( rgb_t(0xfd, 0xd5, 0xb1), "apricot"));
   table.push_back(pair( rgb_t(0x71, 0xd9, 0xe2), "aquamarine"));
   table.push_back(pair( rgb_t(0x7b, 0xa0, 0x5b), "asparagus"));
   table.push_back(pair( rgb_t(0xff, 0x99, 0x66), "atomic_tangerine"));
   table.push_back(pair( rgb_t(0xfb, 0xe7, 0xb2), "banana_mania"));
   table.push_back(pair( rgb_t(0x92, 0x6f, 0x5b), "beaver"));
   table.push_back(pair( rgb_t(0xfe, 0x6f, 0x5e), "bittersweet"));
   table.push_back(pair( rgb_t(0x00, 0x00, 0x00), "black"));
   table.push_back(pair( rgb_t(0xa3, 0xe3, 0xed), "blizzard_blue"));
   table.push_back(pair( rgb_t(0x00, 0x66, 0xff), "blue"));
   table.push_back(pair( rgb_t(0x99, 0x99, 0xcc), "blue_bell"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "blue_gray"));
   table.push_back(pair( rgb_t(0x00, 0x95, 0xb6), "blue_green"));
   table.push_back(pair( rgb_t(0x64, 0x56, 0xb7), "blue_violet"));
   table.push_back(pair( rgb_t(0xc6, 0x2d, 0x42), "brick_red"));
   table.push_back(pair( rgb_t(0xfb, 0x60, 0x7f), "brink_pink"));
   table.push_back(pair( rgb_t(0xaf, 0x59, 0x3e), "brown"));
   table.push_back(pair( rgb_t(0xff, 0x70, 0x34), "burnt_orange"));
   table.push_back(pair( rgb_t(0xe9, 0x74, 0x51), "burnt_sienna"));
   table.push_back(pair( rgb_t(0xa9, 0xb2, 0xc3), "cadet_blue"));
   table.push_back(pair( rgb_t(0xff, 0xff, 0x99), "canary"));
   table.push_back(pair( rgb_t(0x00, 0xcc, 0x99), "caribbean_green"));
   table.push_back(pair( rgb_t(0xff, 0xa6, 0xc9), "carnation_pink"));
   table.push_back(pair( rgb_t(0xda, 0x32, 0x87), "cerise"));
   table.push_back(pair( rgb_t(0x02, 0xa4, 0xd3), "cerulean"));
   table.push_back(pair( rgb_t(0xff, 0x99, 0x66), "chartreuse"));
   table.push_back(pair( rgb_t(0xb9, 0x4e, 0x48), "chestnut"));
   table.push_back(pair( rgb_t(0xda, 0x8a, 0x67), "copper"));
   table.push_back(pair( rgb_t(0x93, 0xcc, 0xea), "cornflower"));
   table.push_back(pair( rgb_t(0xff, 0xb7, 0xd5), "cotton_candy"));
   table.push_back(pair( rgb_t(0xdb, 0x50, 0x79), "cranberry"));
   table.push_back(pair( rgb_t(0xfe, 0xd8, 0x5d), "dandelion"));
   table.push_back(pair( rgb_t(0x15, 0x60, 0xbd), "denim"));
   table.push_back(pair( rgb_t(0xed, 0xc9, 0xaf), "desert_sand"));
   table.push_back(pair( rgb_t(0x61, 0x40, 0x51), "eggplant"));
   table.push_back(pair( rgb_t(0xcc, 0xff, 0x00), "electric_lime"));
   table.push_back(pair( rgb_t(0x63, 0xb7, 0x6c), "fern"));
   table.push_back(pair( rgb_t(0xff, 0xcb, 0xa4), "flesh"));
   table.push_back(pair( rgb_t(0x5f, 0xa7, 0x77), "forest_green"));
   table.push_back(pair( rgb_t(0xc1, 0x54, 0xc1), "fuchsia"));
   table.push_back(pair( rgb_t(0xc4, 0x56, 0x55), "fuzzy_wuzzy_brown"));
   table.push_back(pair( rgb_t(0xe6, 0xbe, 0x8a), "gold"));
   table.push_back(pair( rgb_t(0xfc, 0xd6, 0x67), "goldenrod"));
   table.push_back(pair( rgb_t(0x9d, 0xe0, 0x93), "granny_smith_apple"));
   table.push_back(pair( rgb_t(0x8b, 0x86, 0x80), "gray"));
   table.push_back(pair( rgb_t(0x01, 0xa3, 0x68), "green"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "green_blue"));
   table.push_back(pair( rgb_t(0xf1, 0xe7, 0x88), "green_yellow"));
   table.push_back(pair( rgb_t(0xff, 0x00, 0xcc), "hot_magenta"));
   table.push_back(pair( rgb_t(0xb0, 0xe3, 0x13), "inch_worm"));
   table.push_back(pair( rgb_t(0xb9, 0x4e, 0x48), "indian_red"));
   table.push_back(pair( rgb_t(0x4f, 0x69, 0xc6), "indigo"));
   table.push_back(pair( rgb_t(0xa5, 0x0b, 0x5e), "jazzberry_jam"));
   table.push_back(pair( rgb_t(0x29, 0xab, 0x87), "jungle_green"));
   table.push_back(pair( rgb_t(0xff, 0xff, 0x66), "laser_lemon"));
   table.push_back(pair( rgb_t(0xfb, 0xae, 0xd2), "lavender"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "lemon_yellow"));
   table.push_back(pair( rgb_t(0xff, 0xb9, 0x7b), "macaroni_and_cheese"));
   table.push_back(pair( rgb_t(0xf6, 0x53, 0xa6), "magenta"));
   table.push_back(pair( rgb_t(0xaa, 0xf0, 0xd1), "magic_mint"));
   table.push_back(pair( rgb_t(0xca, 0x34, 0x35), "mahogany"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "maize"));
   table.push_back(pair( rgb_t(0x8d, 0x90, 0xa1), "manatee"));
   table.push_back(pair( rgb_t(0xe7, 0x72, 0x00), "mango_tango"));
   table.push_back(pair( rgb_t(0xc3, 0x21, 0x48), "maroon"));
   table.push_back(pair( rgb_t(0xf0, 0x91, 0xa9), "mauvelous"));
   table.push_back(pair( rgb_t(0xfe, 0xba, 0xad), "melon"));
   table.push_back(pair( rgb_t(0x00, 0x33, 0x66), "midnight_blue"));
   table.push_back(pair( rgb_t(0x1a, 0xb3, 0x85), "mountain_meadow"));
   table.push_back(pair( rgb_t(0xc5, 0x4b, 0x8c), "mulberry"));
   table.push_back(pair( rgb_t(0x00, 0x66, 0xcc), "navy_blue"));
   table.push_back(pair( rgb_t(0xff, 0x99, 0x33), "neon_carrot"));
   table.push_back(pair( rgb_t(0xb5, 0xb3, 0x5c), "olive_green"));
   table.push_back(pair( rgb_t(0xff, 0x68, 0x1f), "orange"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "orange_red"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "orange_yellow"));
   table.push_back(pair( rgb_t(0xe2, 0x9c, 0xd2), "orchid"));
   table.push_back(pair( rgb_t(0x2d, 0x38, 0x3a), "outer_space"));
   table.push_back(pair( rgb_t(0xff, 0x60, 0x37), "outrageous_orange"));
   table.push_back(pair( rgb_t(0x00, 0x9d, 0xc4), "pacific_blue"));
   table.push_back(pair( rgb_t(0xff, 0xcb, 0xa4), "peach"));
   table.push_back(pair( rgb_t(0xc3, 0xcd, 0xe6), "periwinkle"));
   table.push_back(pair( rgb_t(0xfd, 0xd7, 0xe4), "pig_pink"));
   table.push_back(pair( rgb_t(0x01, 0x79, 0x6f), "pine_green"));
   table.push_back(pair( rgb_t(0xff, 0x66, 0xff), "pink_flamingo"));
   table.push_back(pair( rgb_t(0x84, 0x31, 0x79), "plum"));
   table.push_back(pair( rgb_t(0x00, 0x33, 0x66), "prussian_blue"));
   table.push_back(pair( rgb_t(0x65, 0x2d, 0xc1), "purple_heart"));
   table.push_back(pair( rgb_t(0x96, 0x78, 0xb6), "purple_mountains_majesty"));
   table.push_back(pair( rgb_t(0xff, 0x00, 0xcc), "purple_pizzazz"));
   table.push_back(pair( rgb_t(0xff, 0x35, 0x5e), "radical_red"));
   table.push_back(pair( rgb_t(0xd2, 0x7d, 0x46), "raw_sienna"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "raw_umber"));
   table.push_back(pair( rgb_t(0xff, 0x33, 0xcc), "razzle_dazzle_rose"));
   table.push_back(pair( rgb_t(0xe3, 0x0b, 0x5c), "razzmatazz"));
   table.push_back(pair( rgb_t(0xed, 0x0a, 0x3f), "red"));
   table.push_back(pair( rgb_t(0xff, 0x3f, 0x34), "red_orange"));
   table.push_back(pair( rgb_t(0xbb, 0x33, 0x85), "red_violet"));
   table.push_back(pair( rgb_t(0x00, 0xcc, 0xcc), "robins_egg_blue"));
   table.push_back(pair( rgb_t(0x6b, 0x3f, 0xa0), "royal_purple"));
   table.push_back(pair( rgb_t(0xff, 0x91, 0xa4), "salmon"));
   table.push_back(pair( rgb_t(0xfd, 0x0e, 0x35), "scarlet"));
   table.push_back(pair( rgb_t(0x66, 0xff, 0x66), "screamin_green"));
   table.push_back(pair( rgb_t(0x93, 0xdf, 0xb8), "sea_green"));
   table.push_back(pair( rgb_t(0x9e, 0x5b, 0x40), "sepia"));
   table.push_back(pair( rgb_t(0x83, 0x70, 0x50), "shadow"));
   table.push_back(pair( rgb_t(0x33, 0xcc, 0x99), "shamrock"));
   table.push_back(pair( rgb_t(0xff, 0x6f, 0xff), "shocking_pink"));
   table.push_back(pair( rgb_t(0xc9, 0xc0, 0xbb), "silver"));
   table.push_back(pair( rgb_t(0x76, 0xd7, 0xea), "sky_blue"));
   table.push_back(pair( rgb_t(0xec, 0xeb, 0xbd), "spring_green"));
   table.push_back(pair( rgb_t(0xff, 0xcc, 0x33), "sunglow"));
   table.push_back(pair( rgb_t(0xfe, 0x4c, 0x40), "sunset_orange"));
   table.push_back(pair( rgb_t(0xfa, 0x9d, 0x5a), "tan"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "teal_blue"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "thistle"));
   table.push_back(pair( rgb_t(0xfc, 0x80, 0xa5), "tickle_me_pink"));
   table.push_back(pair( rgb_t(0xd9, 0xd6, 0xcf), "timberwolf"));
   table.push_back(pair( rgb_t(0xfd, 0x0e, 0x35), "torch_red"));
   table.push_back(pair( rgb_t(0x00, 0x75, 0x5e), "tropical_rain_forest"));
   table.push_back(pair( rgb_t(0xde, 0xa6, 0x81), "tumbleweed"));
   table.push_back(pair( rgb_t(0x6c, 0xda, 0xe7), "turquoise_blue"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "ultra_blue"));
   table.push_back(pair( rgb_t(0x66, 0xff, 0x66), "ultra_green"));
   table.push_back(pair( rgb_t(0xff, 0x60, 0x37), "ultra_orange"));
   table.push_back(pair( rgb_t(0xff, 0x6f, 0xff), "ultra_pink"));
   table.push_back(pair( rgb_t(0xfd, 0x5b, 0x78), "ultra_red"));
   table.push_back(pair( rgb_t(0xff, 0xff, 0x66), "ultra_yellow"));
   table.push_back(pair( rgb_t(0xff, 0xff, 0x66), "unmellow_yellow"));
   table.push_back(pair( rgb_t(0x83, 0x59, 0xa3), "violet"));
// table.push_back(pair( rgb_t(0x??, 0x??, 0x??), "violet_blue"));
   table.push_back(pair( rgb_t(0xf7, 0x46, 0x8a), "violet_red"));
   table.push_back(pair( rgb_t(0xff, 0x99, 0x80), "vivid_tangerine"));
   table.push_back(pair( rgb_t(0x80, 0x37, 0x90), "vivid_violet"));
   table.push_back(pair( rgb_t(0xff, 0xff, 0xff), "white"));
   table.push_back(pair( rgb_t(0x7a, 0x89, 0xb8), "wild_blue_yonder"));
   table.push_back(pair( rgb_t(0xff, 0x33, 0x99), "wild_strawberry"));
   table.push_back(pair( rgb_t(0xfd, 0x5b, 0x78), "wild_watermelon"));
   table.push_back(pair( rgb_t(0xc9, 0xa0, 0xdc), "wisteria"));
   table.push_back(pair( rgb_t(0xfb, 0xe8, 0x70), "yellow"));
   table.push_back(pair( rgb_t(0xc5, 0xe1, 0x7a), "yellow_green"));
   table.push_back(pair( rgb_t(0xff, 0xae, 0x42), "yellow_orange"));

   // kip-specific
   table.push_back(pair( rgb_t( 64,  64,  64), "gray_dark"));
   table.push_back(pair( rgb_t(128, 128, 128), "gray_medium"));
   table.push_back(pair( rgb_t(192, 192, 192), "gray_light"));
}

} // namespace internal



// crayola::color_table
inline std::vector<std::pair<RGB<crayola_rgb_t>, std::string>> &
crayola::color_table()
{
   static std::vector<std::pair<RGB<crayola_rgb_t>, std::string>> table;
   static bool first = true;

   if (first) {
      first = false;
      internal::color_table_initialize<char>(table);
   }
   return table;
}



// -----------------------------------------------------------------------------
// Specific colors
// -----------------------------------------------------------------------------

namespace internal {
   // crayola_lookup
   inline crayola crayola_lookup(const char *const name)
   {
      const crayola_id_t size = crayola_id_t(crayola::color_table().size());
      for (crayola_id_t c = 0;  c < size;  ++c)
         if (name == crayola::color_table()[c].second)
            return crayola(c);

      ///assert(false);
      return crayola::unknown;
   }
}



#ifdef KIP_GLOBAL_CRAYOLA
   // Make crayola colors available in kip::, too
   #define kip_make_color(name)\
      namespace internal {\
         template<class unused>\
         const crayola crayola_base<unused>::name = crayola_lookup(#name);\
      }\
      inline const crayola &name = crayola::name;
#else
   #define kip_make_color(name)\
      namespace internal {\
         template<class unused>\
         const crayola crayola_base<unused>::name = crayola_lookup(#name);\
      }
#endif

   kip_make_color( unknown )
   kip_make_color( almond )
   kip_make_color( antique_brass )
   kip_make_color( apricot )
   kip_make_color( aquamarine )
   kip_make_color( asparagus )
   kip_make_color( atomic_tangerine )
   kip_make_color( banana_mania )
   kip_make_color( beaver )
   kip_make_color( bittersweet )
   kip_make_color( black )
   kip_make_color( blizzard_blue )
   kip_make_color( blue )
   kip_make_color( blue_bell )
// kip_make_color( blue_gray )
   kip_make_color( blue_green )
   kip_make_color( blue_violet )
   kip_make_color( brick_red )
   kip_make_color( brink_pink )
   kip_make_color( brown )
   kip_make_color( burnt_orange )
   kip_make_color( burnt_sienna )
   kip_make_color( cadet_blue )
   kip_make_color( canary )
   kip_make_color( caribbean_green )
   kip_make_color( carnation_pink )
   kip_make_color( cerise )
   kip_make_color( cerulean )
   kip_make_color( chartreuse )
   kip_make_color( chestnut )
   kip_make_color( copper )
   kip_make_color( cornflower )
   kip_make_color( cotton_candy )
   kip_make_color( cranberry )
   kip_make_color( dandelion )
   kip_make_color( denim )
   kip_make_color( desert_sand )
   kip_make_color( eggplant )
   kip_make_color( electric_lime )
   kip_make_color( fern )
   kip_make_color( flesh )
   kip_make_color( forest_green )
   kip_make_color( fuchsia )
   kip_make_color( fuzzy_wuzzy_brown )
   kip_make_color( gold )
   kip_make_color( goldenrod )
   kip_make_color( granny_smith_apple )
   kip_make_color( gray )
   kip_make_color( green )
// kip_make_color( green_blue )
   kip_make_color( green_yellow )
   kip_make_color( hot_magenta )
   kip_make_color( inch_worm )
   kip_make_color( indian_red )
   kip_make_color( indigo )
   kip_make_color( jazzberry_jam )
   kip_make_color( jungle_green )
   kip_make_color( laser_lemon )
   kip_make_color( lavender )
// kip_make_color( lemon_yellow )
   kip_make_color( macaroni_and_cheese )
   kip_make_color( magenta )
   kip_make_color( magic_mint )
   kip_make_color( mahogany )
// kip_make_color( maize )
   kip_make_color( manatee )
   kip_make_color( mango_tango )
   kip_make_color( maroon )
   kip_make_color( mauvelous )
   kip_make_color( melon )
   kip_make_color( midnight_blue )
   kip_make_color( mountain_meadow )
   kip_make_color( mulberry )
   kip_make_color( navy_blue )
   kip_make_color( neon_carrot )
   kip_make_color( olive_green )
   kip_make_color( orange )
// kip_make_color( orange_red )
// kip_make_color( orange_yellow )
   kip_make_color( orchid )
   kip_make_color( outer_space )
   kip_make_color( outrageous_orange )
   kip_make_color( pacific_blue )
   kip_make_color( peach )
   kip_make_color( periwinkle )
   kip_make_color( pig_pink )
   kip_make_color( pine_green )
   kip_make_color( pink_flamingo )
   kip_make_color( plum )
   kip_make_color( prussian_blue )
   kip_make_color( purple_heart )
   kip_make_color( purple_mountains_majesty )
   kip_make_color( purple_pizzazz )
   kip_make_color( radical_red )
   kip_make_color( raw_sienna )
// kip_make_color( raw_umber )
   kip_make_color( razzle_dazzle_rose )
   kip_make_color( razzmatazz )
   kip_make_color( red )
   kip_make_color( red_orange )
   kip_make_color( red_violet )
   kip_make_color( robins_egg_blue )
   kip_make_color( royal_purple )
   kip_make_color( salmon )
   kip_make_color( scarlet )
   kip_make_color( screamin_green )
   kip_make_color( sea_green )
   kip_make_color( sepia )
   kip_make_color( shadow )
   kip_make_color( shamrock )
   kip_make_color( shocking_pink )
   kip_make_color( silver )
   kip_make_color( sky_blue )
   kip_make_color( spring_green )
   kip_make_color( sunglow )
   kip_make_color( sunset_orange )
   kip_make_color( tan )
// kip_make_color( teal_blue )
// kip_make_color( thistle )
   kip_make_color( tickle_me_pink )
   kip_make_color( timberwolf )
   kip_make_color( torch_red )
   kip_make_color( tropical_rain_forest )
   kip_make_color( tumbleweed )
   kip_make_color( turquoise_blue )
// kip_make_color( ultra_blue )
   kip_make_color( ultra_green )
   kip_make_color( ultra_orange )
   kip_make_color( ultra_pink )
   kip_make_color( ultra_red )
   kip_make_color( ultra_yellow )
   kip_make_color( unmellow_yellow )
   kip_make_color( violet )
// kip_make_color( violet_blue )
   kip_make_color( violet_red )
   kip_make_color( vivid_tangerine )
   kip_make_color( vivid_violet )
   kip_make_color( white )
   kip_make_color( wild_blue_yonder )
   kip_make_color( wild_strawberry )
   kip_make_color( wild_watermelon )
   kip_make_color( wisteria )
   kip_make_color( yellow )
   kip_make_color( yellow_green )
   kip_make_color( yellow_orange )

   // kip-specific
   kip_make_color( gray_dark )
   kip_make_color( gray_medium )
   kip_make_color( gray_light )

#undef kip_make_color



// -----------------------------------------------------------------------------
// read_value(crayola)
// -----------------------------------------------------------------------------

template<class ISTREAM>
bool read_value(
   ISTREAM &s, crayola &value,
   const std::string &description = "crayola"
) {
   s.bail = false;
   std::string word;

   if (read_value(s,word)) {
      bool found = false;

      const crayola_id_t size = crayola_id_t(crayola::color_table().size());
      for (crayola_id_t c = 0;  c < size;  ++c)
         if (word == crayola::color_table()[c].second) {
            value = crayola(c);
            found = true;  break;
         }
      if (!found) {
         std::ostringstream oss;
         oss << "Unknown crayola color \"" << word
             << "\"\nSetting to \"unknown\" (is (0,0,0))";
         s.warning(oss);
         value = crayola(0);
      }
   } else {
      s.add(std::ios::failbit);
      addendum("Detected while reading "+description, diagnostic_t::diagnostic_error);
   }
   return !s.fail();
}



// -----------------------------------------------------------------------------
// crayola::operator==
// -----------------------------------------------------------------------------

inline bool operator==(const crayola &a, const crayola &b)
{
   return a.id() == b.id();
}



// -----------------------------------------------------------------------------
// RGB(crayola)
// RGBA(crayola)
// -----------------------------------------------------------------------------

// RGB(crayola)
template<class rgb_t>
inline RGB<rgb_t>::RGB(const crayola &from)
{
   const RGB value = crayola::color_table()[from.id()].first;
   r = value.r;
   g = value.g;
   b = value.b;
}

// RGBA(crayola)
template<class rgb_t>
inline RGBA<rgb_t>::RGBA(const crayola &from)
{
   const RGB<rgb_t> value(from);
   r = value.r;
   g = value.g;
   b = value.b;
   a = opaque<rgb_t>();
}
