
// Kip
#include "kip.h"
using namespace kip;

// Types
typedef double  real;
typedef crayola base;
typedef rgba    out;

// Visualization
extern "C" {
#include "gl.h"
#include "tk.h"
}



namespace {

// Some cylinders, for use as operands to the nary operators
cylinder<real,base>
   cyl0(-20+1,-3,-20+0, -20+1,3,-20+0, 2),
   cyl1(-20+1,-3,-20+1, -20+1,3,-20+1, 2),
   cyl2(-20+0,-3,-20+1, -20+0,3,-20+1, 2),
   cyl3(-20-1,-3,-20+1, -20-1,3,-20+1, 2),
   cyl4(-20-1,-3,-20+0, -20-1,3,-20+0, 2),
   cyl5(-20-1,-3,-20-1, -20-1,3,-20-1, 2),
   cyl6(-20+0,-3,-20-1, -20+0,3,-20-1, 2),
   cyl7(-20+1,-3,-20-1, -20+1,3,-20-1, 2),
   mask(-20,-2,-20, -20,2,-20, 4);

// Perform a certain modification to the above cylinders
void move_cylinders()
{
   cyl0.a.x += 8;  cyl0.b.x += 8;  cyl1.a.x += 8;  cyl1.b.x += 8;
   cyl2.a.x += 8;  cyl2.b.x += 8;  cyl3.a.x += 8;  cyl3.b.x += 8;
   cyl4.a.x += 8;  cyl4.b.x += 8;  cyl5.a.x += 8;  cyl5.b.x += 8;
   cyl6.a.x += 8;  cyl6.b.x += 8;  cyl7.a.x += 8;  cyl7.b.x += 8;
   mask.a.x += 8;  mask.b.x += 8;
}

}



// Main
int main(const int, const char *const *const argv)
{
   kip::threads = 0;


   // ------------------------
   // Initialize
   // ------------------------

   // Scene
   scene<real,base,out> s;
   s.resize(1200,1024);
   s.resize(960,800);
   ///s.anti = 3;
   s.theta=75, s.phi=15, s.d=250;
   s.background = crayola::black;


   // ------------------------
   // Primitives
   // ------------------------

   // bicylinder(a.x,a.y,a.z, b.x,b.y,b.z, r.a,r.b, color)
   s.push(bicylinder<real,base>(-23,0,20, -17,0,20, 3,2, crayola::red));

   // biwasher(a.x,a.y,a.z, b.x,b.y,b.z, i.a,i.b, o.a,o.b, color)
   s.push(biwasher<real,base>(-15,0,20, -9,0,20, 2,1, 3,2, crayola::red));

   // box(c.x,c.y,c.z, a.x,a.y,a.z, r.x,r.y,r.z, color)
   s.push(box<real,base>(-4,0,20, 0,0,0, 3,1,2, crayola::red));

   // circle(c.x,c.y,c.z, n.x,n.y,n.z, r, color)
   s.push(circle<real,base>(-20,0,-10, 0,1,0, 4, crayola::red));

   // cone(a.x,a.y,a.z, b.x,b.y,b.z, r, color)
   s.push(cone<real,base>(1,0,20, 7,0,20, 2, crayola::red));

   // cylinder(a.x,a.y,a.z, b.x,b.y,b.z, r, color)
   s.push(cylinder<real,base>(9,0,20, 15,0,20, 2, crayola::red));

   // ellipsoid(c.x,c.y,c.z, a.x,a.y,a.z, r.x,r.y,r.z, color)
   s.push(ellipsoid<real,base>(20,0,20, 0,0,0, 3,1,2, crayola::red));

   // half(p.x,p.y,p.z, n.x,n.y,n.z, color)
   s.push(half<real,base>(-550,2750,-950, 1,-1,1, crayola::gray));

   // paraboloid(a.x,a.y,a.z, b.x,b.y,b.z, r, color)
   s.push(paraboloid<real,base>(-23,0,10, -17,0,10, 3, crayola::red));

   // pill(a.x,a.y,a.z, b.x,b.y,b.z, r, color)
   s.push(pill<real,base>(-13,0,10, -11,0,10, 2, crayola::red));

   // polygon(
   //    n,
   //    table[ 0 ].x, table[ 0 ].y, table[ 0 ].z,
   //    ...,
   //    table[n-1].x, table[n-1].y, table[n-1].z,
   //    color
   // )
   // Create, then populate with (x,y,z) points
   s.push(polygon<real,base>(crayola::red));
   {
      s.polygon.back().push(-7, 0,  6);
      s.polygon.back().push(-7, 0, 14);
      s.polygon.back().push(-4, 0, 10);
      s.polygon.back().push(-1, 0, 14);
      s.polygon.back().push(-1, 0,  6);
   }

   // silo(a.x,a.y,a.z, b.x,b.y,b.z, r, color)
   s.push(silo<real,base>(3,0,10, 7,0,10, 2, crayola::red));

   // sphere(c.x,c.y,c.z, r, color)
   s.push(sphere<real,base>(12,0,10, 2, crayola::red));

   // spheroid(a.x,a.y,a.z, b.x,b.y,b.z, r, color)
   s.push(spheroid<real,base>(17,0,10, 23,0,10, 2, crayola::red));

   // triangle(u.x,u.y,u.z, v.x,v.y,v.z, w.x,w.y,w.z, color)
   s.push(triangle<real,base>(-23,0,-4, -20,0,4, -17,0,-4, crayola::red));

   // washer(a.x,a.y,a.z, b.x,b.y,b.z, i,o, color)
   s.push(washer<real,base>(-7,0,0, -1,0,0, 1,2, crayola::red));

   // tabular(
   //    a.x,a.y,a.z, b.x,b.y,b.z,
   //    n,
   //    table[ 0 ].x, table[ 0 ].r,
   //    ...,
   //    table[n-1].x, table[n-1].r,
   //    color
   // )
   // Create, then populate with (x,r) points
   s.push(tabular<real,base>(0,0,0, 1,0,0, crayola::red));
   {
      s.tabular.back().push(1, 0);
      s.tabular.back().push(2, 2);
      s.tabular.back().push(3, 2.75);
      s.tabular.back().push(4, 3);
      s.tabular.back().push(5, 2.75);
      s.tabular.back().push(6, 2);
      s.tabular.back().push(7, 0);
   }

   // xplane(x, size.y,size.z, color,second-color)
   // Logical-and with a sphere, for viz purposes
   s.push(kipand<real,base>(
      xplane<real,base>(-12,1,2, crayola::red, crayola::gray),
      sphere<real,base>(-12,0,0, 4)
   ));

   // yplane(y, size.x,size.z, color,second-color)
   // Logical-and with a sphere, for viz purposes
   s.push(kipand<real,base>(
      yplane<real,base>(0,1,2, crayola::green, crayola::gray),
      sphere<real,base>(12,0,0, 4)
   ));

   // zplane(z, size.x,size.y, color,second-color)
   // Logical-and with a sphere, for viz purposes
   s.push(kipand<real,base>(
      zplane<real,base>(0,1,2, crayola::blue, crayola::gray),
      sphere<real,base>(20,0,0, 4)
   ));


   // ------------------------
   // Operators: unary
   // ------------------------

   // not(a, color)
   // Write "kipnot", as this is C++
   // Visually, this not(box) gives us the "room" we're in
   s.push(kipnot<real,base>(
      box<real,base>(0,-200,0, 0,0,0, 600,3000,1000, crayola::gray)
   ));


   // ------------------------
   // Operators: binary
   // ------------------------

   // Note: each is logical-anded with a cylinder, for viz purposes

   // and(a,b, color)
   // Write "kipand", as this is C++
   s.push(
      kipand<real,base>(
         kipand<real,base>(  // <-- illustrating this
            cylinder<real,base>(-5,-3,-10, -5,3,-10, 3),
            cylinder<real,base>(-3,-3,-10, -3,3,-10, 3)
         ),
         cylinder<real,base>(-4,-2,-10, -4,2,-10, 5),
         crayola::green
      )
   );

   // cut(a,b, color)
   // Write "kipcut", as this is C++
   s.push(
      kipand<real,base>(
         kipcut<real,base>(  // <-- illustrating this
            cylinder<real,base>(3,-3,-10,  3,3,-10, 3),
            cylinder<real,base>(5,-3,-10,  5,3,-10, 3)
         ),
         cylinder<real,base>(4,-2,-10,  4,2,-10, 5),
         crayola::green
      )
   );

   // or(a,b, color)
   // Write "kipor", as this is C++
   s.push(
      kipand<real,base>(
         kipor<real,base>(  // <-- illustrating this
            cylinder<real,base>(11,-3,-10, 11,3,-10, 3),
            cylinder<real,base>(13,-3,-10, 13,3,-10, 3)
         ),
         cylinder<real,base>(12,-2,-10, 12,2,-10, 5),
         crayola::green
      )
   );

   // xor(a,b, color)
   // Write "kipxor", as this is C++
   s.push(
      kipand<real,base>(
         kipxor<real,base>(  // <-- illustrating this
            cylinder<real,base>(19,-3,-10, 19,3,-10, 3),
            cylinder<real,base>(21,-3,-10, 21,3,-10, 3)
         ),
         cylinder<real,base>(20,-2,-10, 20,2,-10, 5),
         crayola::green
      )
   );


   // ------------------------
   // Operators: nary
   // ------------------------

   // Note: each is logical-anded with the "mask" cylinder, for viz purposes

   // ands([a, ...] [; color])
   ands<real,base> ANDS;  // create, then populate...
   ANDS.push(cyl0);  ANDS.push(cyl1);  ANDS.push(cyl2);  ANDS.push(cyl3);
   ANDS.push(cyl4);  ANDS.push(cyl5);  ANDS.push(cyl6);  ANDS.push(cyl7);
   s.push(kipand<real,base>(ANDS, mask, crayola::blue));
   move_cylinders();

   // even([a, ...] [; color])
   even<real,base> EVEN;  // create, then populate...
   EVEN.push(cyl0);  EVEN.push(cyl1);  EVEN.push(cyl2);  EVEN.push(cyl3);
   EVEN.push(cyl4);  EVEN.push(cyl5);  EVEN.push(cyl6);  EVEN.push(cyl7);
   s.push(kipand<real,base>(EVEN, mask, crayola::blue));
   move_cylinders();

   // odd([a, ...] [; color])
   odd<real,base> ODD;  // create, then populate...
   ODD.push(cyl0);  ODD.push(cyl1);  ODD.push(cyl2);  ODD.push(cyl3);
   ODD.push(cyl4);  ODD.push(cyl5);  ODD.push(cyl6);  ODD.push(cyl7);
   s.push(kipand<real,base>(ODD, mask, crayola::blue));
   move_cylinders();

   // one([a, ...] [; color])
   one<real,base> ONE;  // create, then populate...
   ONE.push(cyl0);  ONE.push(cyl1);  ONE.push(cyl2);  ONE.push(cyl3);
   ONE.push(cyl4);  ONE.push(cyl5);  ONE.push(cyl6);  ONE.push(cyl7);
   s.push(kipand<real,base>(ONE, mask, crayola::blue));
   move_cylinders();

   // ors([a, ...] [; color])
   ors<real,base> ORS;  // create, then populate...
   ORS.push(cyl0);  ORS.push(cyl1);  ORS.push(cyl2);  ORS.push(cyl3);
   ORS.push(cyl4);  ORS.push(cyl5);  ORS.push(cyl6);  ORS.push(cyl7);
   s.push(kipand<real,base>(ORS, mask, crayola::blue));
   move_cylinders();

   // some([a, ...] [; color])
   some<real,base> SOME;  // create, then populate...
   SOME.push(cyl0);  SOME.push(cyl1);  SOME.push(cyl2);  SOME.push(cyl3);
   SOME.push(cyl4);  SOME.push(cyl5);  SOME.push(cyl6);  SOME.push(cyl7);
   s.push(kipand<real,base>(SOME, mask, crayola::blue));
   move_cylinders();


   // ------------------------
   // Finish
   // ------------------------

   // Ray trace
   s.trace();

   // Open window
   tkInitPosition(150,100, int(s.hpixel), int(s.vpixel));

   // Initialize window
   // Argh; tkInitWindow lacks const correctness
   const std::size_t len = 100;
   char name[len];
   assert(strlen(argv[0]) < len);
   strcpy(name, argv[0]);
   assert(tkInitWindow(name) != GL_FALSE);

   // Show kip's bitmap
   glViewport(0,0, GLint(s.hpixel),GLint(s.vpixel));
   glDrawPixels(
      int(s.hpixel),
      int(s.vpixel),
      GL_RGBA, GL_UNSIGNED_BYTE,
      s()
   );
   glFlush();

   // Press <enter> to continue
   key();

   // If you wish to print the model, as a kip file
   // std::cout << s << std::endl;
}
