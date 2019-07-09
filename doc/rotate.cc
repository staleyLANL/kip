
#define noTIMING

// Kip
#include "kip.h"

// Visualization
#ifndef TIMING
extern "C" {
#include "gl.h"
#include "tk.h"
}
#endif

const std::size_t hsize = 1024;
const std::size_t vsize = 1024;


// Main
int main(const int, const char *const *const argv)
{
   (void)argv;
   kip::threads = 0;

   // ------------------------
   // Scene constituents
   // ------------------------

   // We'll illustrate these components of a "scene" individually.
   // Alternatively, you could define a single scene<real,base,color>,
   // set the same member data, and call the same member functions.

   // Template parameters
   typedef double       real_t;
   typedef kip::crayola base_t;  // <-- each shape will contain one of these
   typedef kip::rgba   color_t;  // <-- the output bitmap will consist of these

   // Scene constituents
   kip::model  <real_t,base_t >  model ;  // CSG model
   kip::view   <real_t        >  view  ;  // View information
   kip::light  <real_t        >  light ;  // Light source
   kip::engine <real_t        >  engine;  // Ray tracing parameters
   kip::image  <real_t,color_t>  image ;  // Output bitmap


   // ------------------------
   // Model
   // ------------------------

   // push()
   model.push(kip::sphere<real_t,base_t>(0,0,0, 1, kip::crayola::black));

   // size()
   assert(model.size() == 1);

   // clear()
   model.clear();  // clears the model (deletes all of its shapes)

   // Stream input
   {
      std::istringstream
         positive(
            // +x
            "silo(0,0,0, 0.9,0,0, 0.01, brick_red)"
            "cone(1,0,0, 0.9,0,0, 0.02, brick_red)"
            // +y
            "silo(0,0,0, 0,0.9,0, 0.01, jungle_green)"
            "cone(0,1,0, 0,0.9,0, 0.02, jungle_green)"
            // +z
            "silo(0,0,0, 0,0,0.9, 0.01, denim)"
            "cone(0,0,1, 0,0,0.9, 0.02, denim)"
         ),
         negative(
            // -x
            "silo(-0,0,0, -0.9,0,0, 0.01, wild_watermelon)"
            "cone(-1,0,0, -0.9,0,0, 0.02, wild_watermelon)"
            // -y
            "silo(0,-0,0, 0,-0.9,0, 0.01, granny_smith_apple)"
            "cone(0,-1,0, 0,-0.9,0, 0.02, granny_smith_apple)"
            // -z
            "silo(0,0,-0, 0,0,-0.9, 0.01, cornflower)"
            "cone(0,0,-1, 0,0,-0.9, 0.02, cornflower)"
         ),
         origin(
            // origin
            "sphere(0,0,0, 0.02, black)"
         );

      // set append = true so model's operator>> appends rather than replaces
      model.append = true;

      origin   >> model;
      positive >> model;
      negative >> model;

      model.append = false;
   }


   // ------------------------
   // View
   // ------------------------

   // target - where we're looking
   view.target(0,0,0);

   // distance from target
   view.d = 10;

   // field-of-view (degrees)
   view.fov = 12;

   // (theta,phi) angles (degrees)
   view.theta = 45;
   view.phi   = 45;

   // roll (degrees)
   view.roll  = 0;


   // ------------------------
   // Light
   // ------------------------

   // light-source position
   light[0](0,-10,5);


   // ------------------------
   // Engine
   // ------------------------

   // Most users shouldn't change these

   engine.method = kip::method_t::uniform;

   engine.hzone = 50;
   engine.vzone = 50;

   engine.min_area  = 800;
   engine.hdivision = 2;
   engine.vdivision = 2;

   engine.sort_min  = 64;
   engine.sort_frac = 0.02;

   engine.xzone = 26;
   engine.yzone = 26;
   engine.zzone = 26;

   engine.fudge = 0.99999;


   // ------------------------
   // Image
   // ------------------------

   // background color
   // Note: this is a color_t (an rgba), not a base_t (a crayola)
   image.background(127,127,127);

   // pixel v/h aspect ratio
   image.aspect = 1.0;

   // antialiasing level
   image.anti = 1;

   // resize()
   image.resize(hsize,vsize);

   // upsize()
   image.upsize(hsize,vsize);


   // ------------------------
   // Make a simple animation
   // ------------------------

#ifndef TIMING
   // Open window
   tkInitPosition(150,100, int(image.hpixel),int(image.vpixel));

   // Initialize window
   // Argh; tkInitWindow lacks const correctness
   const std::size_t len = 100;
   char name[len];
   assert(strlen(argv[0]) < len);
   strcpy(name, argv[0]);
   assert(tkInitWindow(name) != GL_FALSE);

   // Some GL initialization
   glViewport(0,0, GLint(image.hpixel),GLint(image.vpixel));
#endif

   // Simple animation...
   for (unsigned n = 0;  n < 10*360;  ++n) {
      // ...ray trace
      kip::trace(model, view, light, engine, image);

#ifndef TIMING
      // ...display bitmap
      glDrawPixels(
         int(image.hpixel),
         int(image.vpixel),
         GL_RGBA, GL_UNSIGNED_BYTE,
         image()
      );
      glFlush();
#endif

      // ...modify some viewing parameters
      view.theta += 1.0;
      view.phi   += 1.0;
      view.roll  += 1.0;
   }

   /*
   // Press <enter> to continue
   kip::key();
   */
}
