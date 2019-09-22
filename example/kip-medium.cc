
#include "kip.h"

// for display of the result
extern "C" {
#include "gl.h"
#include "tk.h"
}

// main program
int main(const int argc, char *const argv[])
{
   // open and read from specified file
   if (argc != 2) {
      kip::cerr << "Usage: kip <file>" << std::endl;
      exit(1);
   }
   std::ifstream infile(argv[1]);
   assert(infile);  // assertion failure iff couldn't open file
   kip::scene<> scene;
   infile >> scene;  // read kip scene from file

   // set some scene parameters
   scene.theta = 60;
   scene.phi = 30;
   scene.d = 10;
   scene.background = kip::rgba(150,150,150);
   scene.resize(800,800);  // window size

   // ray trace
   kip::trace(scene);

   // a contiguous memory block of red-green-blue-alpha values, with alpha set
   // to 255, is now in scene(), and can be displayed in whatever way you wish
   tkInitPosition(150,100, int(scene.hpixel), int(scene.vpixel));
   assert(tkInitWindow(argv[1]) != GL_FALSE);
   glViewport(0, 0, GLint(scene.hpixel), GLint(scene.vpixel));
   glDrawPixels(
      int(scene.hpixel), int(scene.vpixel),
      GL_RGBA, GL_UNSIGNED_BYTE,
      scene()
   );
   glFlush();

   // press <enter> key to finish
   kip::key();
}
