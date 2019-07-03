
// c++
#include <map>

// kip
#include "kip.h"

// gl, tk
extern "C" {
#include "gl.h"
#include "tk.h"
}

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

// window: current size, initial position
// Note: the rendered-image size is initially set to [hv]window,
// but may become smaller if the undersampling feature is used.
size_t hwindow = 1200;
size_t vwindow = 1200;
const size_t hpos = 50;
const size_t vpos = 0;

// downsize
size_t downsize = 1;

// print useful information per-render
bool print = false;

// for debugging
const bool diag = false;
const size_t ntimes = 0;



// -----------------------------------------------------------------------------
// initialize
// Sets up the major kip objects. All values already have reasonable
// defaults; the real purpose here is to illustrate what's available.
// -----------------------------------------------------------------------------

void initialize()
{
   // model
   // Nothing in particular to set; model.append is set in main()

   // view
   view.target = kip::point<real>(0.0,0.0,0.0);
   view.d      = 10.0;
   view.fov    = 20.0;
   view.theta  = 60.0;
   view.phi    = 30.0;
   view.roll   =  0.0;

   // light
   // Nothing in particular to set; we'll keep light towards observer's left

   // engine
   // Know what you're doing if you change any of these
   engine.method    = kip::uniform;
   engine.hzone     = 50;
   engine.vzone     = 50;
   engine.hsub      = 4;
   engine.vsub      = 4;
   engine.sort_frac = 0.02;
   engine.sort_min  = 64;
   engine.lean      = true;

   // image
   image.background    = color(150,150,150);
   image.aspect        = 1.0;  // qqq Consider a/s for up/down
   image.anti          = 1;
   image.border.bin    = false;
   image.border.object = false;

   // other
   downsize = 1;
}



// -----------------------------------------------------------------------------
// render
// putimage
// -----------------------------------------------------------------------------

// render
inline void render()
{
   // diagnostics
   if (diag) {
      std::cout << "   render" << std::endl;
      std::cout << "      image.hpixel = " << image.hpixel << std::endl;
      std::cout << "      image.vpixel = " << image.vpixel << std::endl;
      std::cout << "      rendering..." << std::endl;
   }

   // light (moves w/viewer)
   light[0](view.target, view.d, view.theta+60, view.phi+10);

   // anything to do?
   if (image.hpixel == 0 || image.vpixel == 0)
      return;

   // print
   if (print)
      std::cout
         << "\nhpixel   = " << image .hpixel
         << "\nvpixel   = " << image .vpixel
         << "\ntarget   = " << view  .target
         << "\nd        = " << view  .d
         << "\nfov      = " << view  .fov
         << "\ntheta    = " << view  .theta
         << "\nphi      = " << view  .phi
         << "\nroll     = " << view  .roll
//         << "\naspect   = " << image .aspect
         << "\ndownsize = " <<        downsize
         << "\nanti     = " << image .anti
//         << "\nlean     = " << engine.lean
         << "\n"
      ;

   // trace
   kip::trace(model, view, light, engine, image);
}



// putimage
void putimage()
{
   if (diag)
      std::cout << "   putimage" << std::endl;

   // size: zero
   if (image.hpixel == 0 &&
       image.vpixel == 0) {
      if (diag) {
         std::cout << "      size: zero" << std::endl;
         std::cout << "      do nothing" << std::endl;
      }
      return;
   }

   // size: same as window
   if (image.hpixel == hwindow &&
       image.vpixel == vwindow
   ) {
      if (diag)
         std::cout << "      glDrawPixels #1" << std::endl;
      glDrawPixels(
         GLsizei(hwindow), GLsizei(vwindow),
         GL_RGBA, GL_UNSIGNED_BYTE, image()
      );
      glFlush();
      return;
   }

   // size: different from window
   static kip::array<2,color> bitmap;
   bitmap.upsize(hwindow,vwindow);
   const real hfac = real(image.hpixel)/real(hwindow);
   const real vfac = real(image.vpixel)/real(vwindow);

   #ifdef _OPENMP
   #pragma omp parallel for
   #endif
   for (size_t n = 0;  n < hwindow*vwindow;  ++n)
      bitmap[n] = image(
         size_t(hfac*(real(n % hwindow) + 0.5)),
         size_t(vfac*(real(n / hwindow) + 0.5))
      );

   if (diag)
      std::cout << "      glDrawPixels #2" << std::endl;
   glDrawPixels(
      GLsizei(hwindow), GLsizei(vwindow),
      GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data()
   );
   glFlush();
}



// -----------------------------------------------------------------------------
// sample
// Return value: does image need to be re-rendered?
// -----------------------------------------------------------------------------

enum sampling_t {
   sampling_coarser,
   sampling_finer
};

bool sample(const sampling_t samp)
{
   const size_t current_downsize = downsize;

   // change
   if (samp == sampling_coarser)
      downsize++;
   if (samp == sampling_finer)
      downsize--;

   // clip
   if (downsize < 1)
      downsize = 1;
   if (downsize > std::min(hwindow,vwindow))
      downsize = std::min(hwindow,vwindow);

   // different?
   return downsize != current_downsize;
}



// -----------------------------------------------------------------------------
// move
// Return value: does image need to be re-rendered?
// -----------------------------------------------------------------------------

bool move(const int key)
{
   if (diag)
      std::cout << "   move: key == " << key << std::endl;

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
      case ',' : case '<' : return sample(sampling_coarser);
      case '.' : case '>' : return sample(sampling_finer);

      // toggle print, low(er)-memory mode
      case XK_p : print = !print;  break;
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
bool diffsize(size_t &hpix, size_t &vpix)
{
   hpix = std::max(size_t(1), size_t(real(hwindow)/real(downsize) + 0.5));
   vpix = std::max(size_t(1), size_t(real(vwindow)/real(downsize) + 0.5));
   return
      image.hpixel != hpix ||
      image.vpixel != vpix;
}



// expose
void expose(const int hsize, const int vsize)
{
   if (diag) {
      std::cout << "EXPOSE" << std::endl;
      std::cout << "   old hwindow = " << hwindow << std::endl;
      std::cout << "   old vwindow = " << vwindow << std::endl;
   }

   hwindow = size_t(hsize);
   vwindow = size_t(vsize);

   if (diag) {
      std::cout << "   new hwindow = " << hwindow << std::endl;
      std::cout << "   new vwindow = " << vwindow << std::endl;
   }

   size_t hpix, vpix;
   if (diffsize(hpix,vpix)) {
      image.upsize(hpix,vpix);
      render();
   }
   putimage();
}



// keydown
GLenum keydown(const int key, const GLenum /*state*/)
{
   if (diag)
      std::cout << "KEYDOWN" << std::endl;

   // quit?
   if (key == XK_Escape || key == XK_q || key == XK_Q)
      return TK_QUIT;

   // process key
   if (move(key)) {
      size_t hpix, vpix;
      if (diffsize(hpix,vpix))
         image.upsize(hpix,vpix);
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
   // set window position and size
   tkInitPosition(int(hpos), int(vpos), int(hwindow), int(vwindow));

   // title
   // Note: it seems that tkInitWindow() is not const correct
   char *const str = new char[title.size()+1];  // tkInitWindow wants char *
   const bool okay = tkInitWindow(strcpy(str,title.c_str())) != GL_FALSE;
   delete[] str;
   if (!okay) return 1;

   if (ntimes > 0) {
      // Below, std::max(size_t(1),ntimes) is equivalent to ntimes; it just
      // silences a g++ warning about comparison with zero when ntimes == 0.
      expose(int(hwindow), int(vwindow));
      for (size_t count = 1;  count < std::max(size_t(1),ntimes);  ++count) {
         view.theta += real(1);
         view.phi   += real(1);
         render();
         putimage();
      }
   } else {
      tkExposeFunc (expose);
      tkKeyDownFunc(keydown);
      image.resize(0,0);
      tkExec();
   }

   // done
   tkCloseWindow();
   if (diag)
      std::cout << "\nReturning from interactive()" << std::endl;
   return 0;
}



// -----------------------------------------------------------------------------
// command arguments
// -----------------------------------------------------------------------------

/*
-shapes
-threads
-rotate [#]
-exit
what else

(void) to () change
*/



namespace command {

// shape
template<class SHAPE>
inline bool shape(const int n)
{
   std::cout << "      shape()" << std::endl;
   (void)n;
   return true;
}



// sphere
static inline bool sphere(const int n)
{
   std::cout << "   sphere()" << std::endl;
   return shape<kip::sphere<real,base>>(n);
}

// cone
static inline bool cone(const int n)
{
   std::cout << "   cone()" << std::endl;
   return shape<kip::cone<real,base>>(n);
}

// threads
static inline bool threads(const int n)
{
   std::cout << "   threads()" << std::endl;
   kip::threads = n;
   return true;
}

// rotate
static int nrotate = -1;
static inline bool rotate(const int n)
{
   std::cout << "   rotate()" << std::endl;
   nrotate = std::abs(n);
   return true;
}

// exit
static bool do_exit = false;
static inline bool exitfun(const int)
{
   std::cout << "   exitfun()" << std::endl;
   do_exit = true;
   return true;
}



// map
static std::map<std::string, std::pair<bool (*)(const int), bool>> map = {
   { "-sphere",  { command::sphere,  true  } },
   { "-cone",    { command::cone,    true  } },
   { "-threads", { command::threads, true  } },
   { "-rotate",  { command::rotate,  true  } },
   { "-exit",    { command::exitfun, false } }
};



// getint
static inline bool getint(
   const int argc, const char *const *const argv,
   const int i, int &n, std::string &title
) {
   if (!(i < argc)) {
      std::cout
         << "Error: " << argv[i-1]
         << " requires an argument" << std::endl;
      return false;
   }

   std::istringstream iss(argv[i]);
   if (!(iss >> n)) {
      std::cout
         << "Error: " << argv[i-1] << " argument " << argv[i]
         << " could not be converted to int" << std::endl;
      return false;
   }

   ///std::cout << "   n == " << n << std::endl;
   title += std::string(" ") + argv[i];
   return true;
}



// arguments
static bool arguments(
   const int argc, const char *const *const argv,
   std::string &title
) {
   bool okay = true;

   // process command arguments
   for (int i = 1;  i < argc;  ++i) {
      title += std::string(" ") + argv[i];

      // option?
      auto iter = map.find(argv[i]);
      if (iter != map.end()) {
         std::cout << "Found argument " << iter->first << std::endl;
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
         std::cout << "Could not open file \"" << argv[i] << '"' << std::endl;
         okay = false;
      }
   }

   // done
   return okay;
}

} // namespace command



// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(const int argc, const char *const *const argv)
{
   kip::threads = 0;
   std::string title = argv[0];

   // arguments
   if (argc < 2) {
      std::cout << "Usage: " << argv[0] << " <file> <option> ..." << std::endl;
      exit(1);
   }
   if (!command::arguments(argc,argv,title))
      exit(1);

   // initialize kip parameters
   initialize();

   // run
   return interactive(title);
}
