
// c++
#include <map>

// gl, tk
extern "C" {
#include "gl.h"
#include "tk.h"
}

// kip
#include "kip.h"

// kip: types
using real  = double;
using base  = kip::rgb;
using color = kip::rgba;

// kip: objects
kip::model <real,base > model;
kip::view  <real      > view;
kip::light <real      > light;
kip::engine<real      > engine;
kip::image <real,color> image;

// misc
int downsize;



// -----------------------------------------------------------------------------
// variables from command-line arguments
// -----------------------------------------------------------------------------

namespace vars {
   // window: size
   int hwindow = 1200;
   int vwindow = 1200;

   // window: position
   int hpos = 50;
   int vpos = 0;

   // initial #renders prior to user control
   int nrender = 0;

   // if timing: no window; render, exit
   bool timing = false;

   // if Exit: window, then render, exit
   bool Exit = false;

   // print mode: print various parameters
   bool print = false;

   // debug mode: very chatty
   bool debug = false;
}



// -----------------------------------------------------------------------------
// initialize
// Sets up the major kip objects. All values already have reasonable
// defaults; the real purpose here is to illustrate what's available.
// -----------------------------------------------------------------------------

void initialize()
{
   if (vars::debug)
      std::cout << "initialize()" << std::endl;

   // model
   // Nothing in particular to set; model.append is set in main()

   // view
   view.target = kip::point<real>(0.0,0.0,0.0);
   view.d      =  5.0;
   view.fov    = 24.0;
   view.theta  = 60.0;
   view.phi    = 30.0;
   view.roll   =  0.0;

   // light
   // Nothing in particular to set; we'll keep light towards observer's left

   // engine
   // Know what you're doing if you change any of these
   engine.method    = kip::method_t::uniform;
   engine.hzone     = 50;
   engine.vzone     = 50;
   engine.hsub      = 4;
   engine.vsub      = 4;
   engine.sort_frac = 0.02;
   engine.sort_min  = 64;
   engine.lean      = true;

   // image
   image.background    = color(150,150,150);
   image.aspect        = 1.0;  // fixme Consider a/s for up/down
   image.anti          = 1;
   image.border.bin    = false;
   image.border.object = false;

   // misc
   downsize = 1;
}



// -----------------------------------------------------------------------------
// render
// putimage
// -----------------------------------------------------------------------------

// render
void render()
{
   if (vars::debug) {
      std::cout << "render()" << std::endl;
      std::cout << "   image.hpixel = " << image.hpixel << std::endl;
      std::cout << "   image.vpixel = " << image.vpixel << std::endl;
   }

   // light (moves w/viewer)
   light[0](view.target, view.d, view.theta+60, view.phi+10);

   // size: zero
   if (image.hpixel == 0 || image.vpixel == 0) {
      if (vars::debug)
         std::cout << "...image size = 0; returning" << std::endl;
      return;
   }

   // print
   if (vars::print)
      std::cout
         << "\nhpixel   = " << image .hpixel
         << "\nvpixel   = " << image .vpixel
         << "\ntarget   = " << view  .target
         << "\nd        = " << view  .d
         << "\nfov      = " << view  .fov
         << "\ntheta    = " << view  .theta
         << "\nphi      = " << view  .phi
         << "\nroll     = " << view  .roll
      // << "\naspect   = " << image .aspect
         << "\ndownsize = " << downsize
         << "\nanti     = " << image .anti
      // << "\nlean     = " << engine.lean
         << std::endl
      ;

   // trace
   if (vars::debug)
      std::cout << "trace()" << std::endl;
   kip::trace(model, view, light, engine, image);
}



// putimage
void putimage()
{
   if (vars::debug)
      std::cout << "putimage()" << std::endl;

   // size: zero
   if (image.hpixel == 0 || image.vpixel == 0) {
      if (vars::debug)
         std::cout << "...image size = 0; returning" << std::endl;
      return;
   }

   // size: same as window
   if (int(image.hpixel) == vars::hwindow &&
       int(image.vpixel) == vars::vwindow
   ) {
      if (vars::debug)
         std::cout << "...glDrawPixels() #1" << std::endl;
      glDrawPixels(
         GLsizei(vars::hwindow), GLsizei(vars::vwindow),
         GL_RGBA, GL_UNSIGNED_BYTE, image()
      );
      glFlush();
      return;
   }

   // size: different from window
   static kip::array<2,color> bitmap;
   bitmap.upsize(vars::hwindow,vars::vwindow);
   const real hfac = real(image.hpixel) / real(vars::hwindow);
   const real vfac = real(image.vpixel) / real(vars::vwindow);

   #ifdef _OPENMP
   #pragma omp parallel for
   #endif
   for (int n = 0;  n < vars::hwindow * vars::vwindow;  ++n)
      bitmap[n] = image(
         int(hfac*(real(n % vars::hwindow) + 0.5)),
         int(vfac*(real(n / vars::hwindow) + 0.5))
      );

   if (vars::debug)
      std::cout << "...glDrawPixels() #2" << std::endl;
   glDrawPixels(
      GLsizei(vars::hwindow), GLsizei(vars::vwindow),
      GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data()
   );
   glFlush();
}



// -----------------------------------------------------------------------------
// sample
// Return value: does image need to be re-rendered?
// -----------------------------------------------------------------------------

enum class sampling {
   coarser,
   finer
};



bool sample(const sampling samp)
{
   const int current_downsize = downsize;

   // change
   if (samp == sampling::coarser)
      downsize++;
   if (samp == sampling::finer)
      downsize--;

   // clip
   if (downsize < 1)
      downsize = 1;
   if (downsize > std::min(vars::hwindow,vars::vwindow))
      downsize = std::min(vars::hwindow,vars::vwindow);

   // different?
   return downsize != current_downsize;
}



// -----------------------------------------------------------------------------
// move
// Return value: does image need to be re-rendered?
// -----------------------------------------------------------------------------

bool move(const int key)
{
   if (vars::debug) {
      std::cout << "move()" << std::endl;
      std::cout << "   key = " << key << std::endl;
   }

   // parameters
   static const real delta_angle  = 3.0;    // change for angles
   static const real delta_target = 0.025;  // change for target x, y, z
   static const real times_inout  = 0.95;   // multiplier for in, out

   // key
   switch (key) {
      // bin/object border toggle
      case XK_bracketleft  : image.border.bin    = !image.border.bin   ;  break;
      case XK_bracketright : image.border.object = !image.border.object;  break;

      // in, out
      case XK_i : view.d *= times_inout;  break;
      case XK_o : view.d /= times_inout;  break;

      // antialiasing
      case XK_space     : return image.anti < 5 ? (image.anti++, true) : false;
      case XK_BackSpace : return image.anti > 1 ? (image.anti--, true) : false;

      // theta, phi, roll
      case XK_Right : view.theta = fmod(view.theta - delta_angle, 360);  break;
      case XK_Left  : view.theta = fmod(view.theta + delta_angle, 360);  break;
      case XK_Down  : view.phi   = fmod(view.phi   - delta_angle, 360);  break;
      case XK_Up    : view.phi   = fmod(view.phi   + delta_angle, 360);  break;
      case XK_0     : view.roll  = fmod(view.roll  - delta_angle, 360);  break;
      case XK_9     : view.roll  = fmod(view.roll  + delta_angle, 360);  break;

      // fov
      case XK_minus : return view.fov >=   1.1 ? (view.fov -= 1, true) : false;
      case XK_equal : return view.fov <= 178.9 ? (view.fov += 1, true) : false;

      // x-, x+, y-, y+, z-, z+
      case XK_KP_4      : view.target.x -= delta_target;  break;
      case XK_KP_6      : view.target.x += delta_target;  break;
      case XK_KP_2      : view.target.y -= delta_target;  break;
      case XK_KP_8      : view.target.y += delta_target;  break;
      case XK_Page_Down : view.target.z -= delta_target;  break;
      case XK_Page_Up   : view.target.z += delta_target;  break;

      // sampling
      case ',' : case '<' : return sample(sampling::coarser);
      case '.' : case '>' : return sample(sampling::finer);

      // toggle print, low(er)-memory mode
      case XK_p : vars::print = !vars::print;  break;
      //case XK_m : engine.lean = !engine.lean;  break;

      // home
      case XK_Home : initialize();  break;

      // default
      default : return false;
   }

   return true;
}



// -----------------------------------------------------------------------------
// Events
//    expose  - window was exposed
//    keydown - keyboard key was pressed
// -----------------------------------------------------------------------------

// diffsize: helper
inline bool diffsize(int &himage, int &vimage)
{
   himage = std::max(1, int(real(vars::hwindow)/real(downsize) + 0.5));
   vimage = std::max(1, int(real(vars::vwindow)/real(downsize) + 0.5));
   return
      int(image.hpixel) != himage ||
      int(image.vpixel) != vimage;
}



// expose
void expose(const int hsize, const int vsize)
{
   if (vars::debug) {
      std::cout << "expose()" << std::endl;
      std::cout << "   old hwindow = " << vars::hwindow << std::endl;
      std::cout << "   old vwindow = " << vars::vwindow << std::endl;
   }

   vars::hwindow = hsize;
   vars::vwindow = vsize;

   if (vars::debug) {
      std::cout << "   new hwindow = " << vars::hwindow << std::endl;
      std::cout << "   new vwindow = " << vars::vwindow << std::endl;
   }

   int himage, vimage;
   if (diffsize(himage,vimage)) {
      image.upsize(himage,vimage);
      render();
   }
   putimage();
}



// keydown
GLenum keydown(const int key, const GLenum /*state*/)
{
   if (vars::debug)
      std::cout << "keydown()" << std::endl;

   // quit?
   if (key == XK_Escape || key == XK_q || key == XK_Q)
      return TK_QUIT;

   // process key
   if (move(key)) {
      int himage, vimage;
      if (diffsize(himage,vimage))
         image.upsize(himage,vimage);
      render();
      putimage();
   }

   return GL_TRUE;
}



// -----------------------------------------------------------------------------
// interactive
// -----------------------------------------------------------------------------

int interactive(const std::string &title)
{
   if (vars::debug)
      std::cout << "interactive()" << std::endl;

   if (!vars::timing) {
      // window: set position and size
      tkInitPosition(vars::hpos, vars::vpos, vars::hwindow, vars::vwindow);

      // window: open
      // Note: it seems that tkInitWindow() is not const correct...
      char *const str = new char[title.size()+1];  // tkInitWindow wants char *
      const bool okay = tkInitWindow(strcpy(str,title.c_str())) != GL_FALSE;
      delete[] str;
      if (!okay) return 1;
   }

   if (vars::nrender > 0) {
      // Meaning: if the current image size (which in this context would have
      // been kip's default) is different from what we need, given the window
      // size and the down-sampling, then [hv]image will come back as what we
      // need; and thus we must upsize the image accordingly.
      // fixme diffsize() does two things, and is confusing.
      int himage, vimage;
      if (diffsize(himage,vimage))
         image.upsize(himage,vimage);
      // else image is already exactly the size we need, so we're good to go

      for (int n = 0;  n < vars::nrender;  ++n) {
         if (n) {
            view.theta += real(1);
            view.phi   += real(1);
         }
         render();
         if (!vars::timing)
            putimage();
      }
   }
   if (vars::Exit) return 0;

   if (!vars::timing) {
      // start
      tkExposeFunc (expose);
      tkKeyDownFunc(keydown);
      if (vars::nrender < 1)
         image.upsize(0,0); // ensure render()
      tkExec();

      // close
      tkCloseWindow();
   }

   // done
   return 0;
}



// -----------------------------------------------------------------------------
// command-line arguments
// shapes
// -----------------------------------------------------------------------------

namespace args {

// ------------------------
// generic
// ------------------------

// shape
template<class SHAPE>
inline bool shape(const int n)
{
   static SHAPE obj;
   for (int i = 0;  i < n;  ++i)
      model.push(kip::random(obj));
   return true;
}

// ------------------------
// specific
// ------------------------

inline bool bicylinder(const int n)
   { return shape<kip::bicylinder<real,base>>(n); }
inline bool biwasher(const int n)
   { return shape<kip::biwasher<real,base>>(n); }
inline bool box(const int n)
   { return shape<kip::box<real,base>>(n); }
inline bool circle(const int n)
   { return shape<kip::circle<real,base>>(n); }
inline bool cone(const int n)
   { return shape<kip::cone<real,base>>(n); }
inline bool cube(const int n)
   { return shape<kip::cube<real,base>>(n); }
inline bool cylinder(const int n)
   { return shape<kip::cylinder<real,base>>(n); }
inline bool ellipsoid(const int n)
   { return shape<kip::ellipsoid<real,base>>(n); }
inline bool paraboloid(const int n)
   { return shape<kip::paraboloid<real,base>>(n); }
inline bool pill(const int n)
   { return shape<kip::pill<real,base>>(n); }
inline bool polygon(const int n)
   { return shape<kip::polygon<real,base>>(n); }
inline bool silo(const int n)
   { return shape<kip::silo<real,base>>(n); }
inline bool sphere(const int n)
   { return shape<kip::sphere<real,base>>(n); }
inline bool spheroid(const int n)
   { return shape<kip::spheroid<real,base>>(n); }
inline bool tabular(const int n)
   { return shape<kip::tabular<real,base>>(n); }
inline bool triangle(const int n)
   { return shape<kip::triangle<real,base>>(n); }
inline bool washer(const int n)
   { return shape<kip::washer<real,base>>(n); }

/*
surf
tri
xplane
yplane
zplane
*/

} // namespace args



// -----------------------------------------------------------------------------
// command-line arguments
// non-shapes
// -----------------------------------------------------------------------------

namespace args {

// ------------------------
// window
// ------------------------

// hwindow
inline bool hwindow(const int n)
{
   vars::hwindow = std::max(100,std::abs(n));
   return true;
}

// vwindow
inline bool vwindow(const int n)
{
   vars::vwindow = std::max(100,std::abs(n));
   return true;
}

// window
inline bool window(const int n)
{
   vars::hwindow =
   vars::vwindow = std::max(100,std::abs(n));
   return true;
}

// hpos
inline bool hpos(const int n)
{
   vars::hpos = std::max(0,std::abs(n));
   return true;
}

// vpos
inline bool vpos(const int n)
{
   vars::vpos = std::max(0,std::abs(n));
   return true;
}



// ------------------------
// misc, threads
// ------------------------

// render
inline bool render(const int n)
{
   vars::nrender = std::abs(n);
   return true;
}

// timing
inline bool timing(const int)
{
   vars::timing = true;
   return true;
}

// exit
inline bool Exit(const int)
{
   vars::Exit = true;
   return true;
}

// print
inline bool print(const int)
{
   vars::print = true;
   return true;
}

// debug
inline bool debug(const int)
{
   vars::debug = true;
   return true;
}

// threads
inline bool threads(const int n)
{
   kip::threads = n;
   return true;
}

} // namespace args



// -----------------------------------------------------------------------------
// command-line arguments
// -----------------------------------------------------------------------------

namespace args {

#define make_shape(shape) { "-" #shape, { args::shape, true } }

// map
// last value == option has int parameter?
std::map<std::string, std::pair<bool (*)(const int), bool>> map = {
   // shapes
   make_shape(bicylinder),
   make_shape(biwasher),
   make_shape(box),
   make_shape(circle),
   make_shape(cone),
   make_shape(cube),
   make_shape(cylinder),
   make_shape(ellipsoid),
   make_shape(paraboloid),
   make_shape(pill),
   make_shape(polygon),
   make_shape(silo),
   make_shape(sphere),
   make_shape(spheroid),
   make_shape(tabular),
   make_shape(triangle),
   make_shape(washer),

   // window
   { "-hwindow", { args::hwindow, true  } },
   { "-vwindow", { args::vwindow, true  } },
   { "-window",  { args::window,  true  } },
   { "-hpos",    { args::hpos,    true  } },
   { "-vpos",    { args::vpos,    true  } },

   // misc
   { "-render",  { args::render,  true  } },
   { "-timing",  { args::timing,  false } },
   { "-exit",    { args::Exit,    false } },
   { "-print",   { args::print,   false } },
   { "-debug",   { args::debug,   false } },

   // threads
   { "-threads", { args::threads, true  } },
};

#undef make_shape



// getint
bool getint(
   const int argc, const char *const *const argv,
   const int i, int &n, std::string &title
) {
   if (!(i < argc)) {
      std::cout
         << "Error: " << argv[i-1]
         << " requires an int parameter (" << argv[i-1] << " #)" << std::endl;
      return false;
   }

   std::istringstream iss(argv[i]);
   if (!(iss >> n)) {
      std::cout
         << "Error: " << argv[i-1] << " parameter " << argv[i]
         << " could not be converted to int" << std::endl;
      return false;
   }

   title += std::string(" ") + argv[i];
   return true;
}



// read
bool read(
   const int argc, const char *const *const argv,
   std::string &title
) {
   bool okay = true;

   // process command-line arguments
   for (int i = 1;  i < argc;  ++i) {
      title += std::string(" ") + argv[i];

      // option?
      auto iter = map.find(argv[i]);
      if (iter != map.end()) {
         int n;
         auto p = iter->second; // the pair
         if ((!p.second || getint(argc,argv,++i,n,title)) &&
             (okay = p.first(n)))
            continue;
         else
            return false;
      }

      // file?
      kip::istream stream(argv[i]);
      if (stream) {
         model.append = true;
         stream >> model;
      } else {
         std::cout << "Could not open file \"" << argv[i] << '"' <<  std::endl;
         okay = false;
      }
   }

   // done
   return okay;
}

} // namespace args



// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

#define printval(val) std::cout << #val " = " << val << std::endl

int main(const int argc, const char *const *const argv)
{
   /*
   printval(sizeof(int));
   printval(sizeof(unsigned int));
   printval(sizeof(long));
   printval(sizeof(unsigned long));
   printval(sizeof(size_t));

   printval(sizeof(kip::bicylinder<real,base>));
   printval(sizeof(kip::biwasher  <real,base>));
   printval(sizeof(kip::box       <real,base>));
   printval(sizeof(kip::circle    <real,base>));
   printval(sizeof(kip::cone      <real,base>));
   printval(sizeof(kip::cube      <real,base>));
   printval(sizeof(kip::cylinder  <real,base>));
   printval(sizeof(kip::ellipsoid <real,base>));
   printval(sizeof(kip::paraboloid<real,base>));
   printval(sizeof(kip::pill      <real,base>));
   printval(sizeof(kip::polygon   <real,base>));
   printval(sizeof(kip::silo      <real,base>));
   printval(sizeof(kip::sphere    <real,base>));
   printval(sizeof(kip::spheroid  <real,base>));
   printval(sizeof(kip::tabular   <real,base>));
   printval(sizeof(kip::triangle  <real,base>));
   printval(sizeof(kip::washer    <real,base>));
   */

   kip::threads = 0;
   std::string title = argv[0];

   // command-line arguments
   if (argc < 2) {
      std::cout << "Usage: " << argv[0] << " <file> <option> ..." << std::endl;
      exit(1);
   }
   if (!args::read(argc,argv,title))
      exit(1);

   // initialize kip parameters
   initialize();

   // action!
   return interactive(title);
}
