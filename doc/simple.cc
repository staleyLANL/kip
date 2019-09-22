
#include "kip.h"

extern "C" {
#include "gl.h"
#include "tk.h"
}

int main(const int, const char *const *const argv)
{
   kip::threads = 0;
   kip::scene<> s;

   // View information
   s.theta = 60;
   s.phi = 30;
   s.d = 20;

   // Miscellaneous information
   s.background = kip::rgba(127,127,127);  // gray background
   s.resize(1024,768);  // window size
   // Set other values, as desired, in scene s

   // Sphere
   s.sphere.push_back(kip::sphere<>(0,0,0, 1, kip::rgb(255,0,0)));
   // Or: s.push(kip::sphere<>(0,0,0, 1, kip::rgb(255,0,0)));

   // Ray trace
   kip::trace(s);
   // Or: s.trace();

   // Open window
   tkInitPosition(150, 100, int(s.hpixel), int(s.vpixel));

   // Initialize window
   // Argh; tkInitWindow lacks const correctness
   const std::size_t len = 100;
   char name[len];
   assert(strlen(argv[0]) < len);
   strcpy(name, argv[0]);
   assert(tkInitWindow(name) != GL_FALSE);

   // Display bitmap from kip
   glViewport(0, 0, GLint(s.hpixel), GLint(s.vpixel));
   glDrawPixels(
      int(s.hpixel), int(s.vpixel),
      GL_RGBA, GL_UNSIGNED_BYTE,
      s()  // kip::scene::operator() gives address of bitmap(0,0)
   );
   glFlush();

   // Press <enter> key to finish
   kip::key();
}
