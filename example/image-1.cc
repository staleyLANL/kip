
#include "kip.h"

int main()
{
   kip::image<> image;

   // Set image background color to a medium gray.
   image.background = kip::rgba(127,127,127);

   // Pixel height/width aspect ratio. 1.0 for square pixels.
   image.aspect = 1.0;

   // Yes, Kip can do antialiasing! What was smooth, becomes
   // smoooooooooooother. anti*anti rays/pixel are computed
   // and averaged. Smooooooooth, but slooooooow. Don't use
   // more than about 3 or 4, or you'll regret it. Above that,
   // the difference isn't visually noticeable anyway.
   image.anti = 3;
}
