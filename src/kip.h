
#pragma once

// Users should #include only this file.

// Defines
//#define KIP_BLOCK  // zzz Faster without, and block.h is broken anyway

//#define KIP_SEGMENTING_DIAG
//#define KIP_SEGMENTING_QUAD
//#define KIP_SEGMENTING_3060
//#define KIP_SEGMENTING_1575

#define KIP_SEGMENTING_LINEAR
#define KIP_SEGMENTING_BINARY

// printval
#ifndef printval
#define printval(val) std::cout << #val " = " << val << std::endl
#endif



// -----------------------------------------------------------------------------
// General #includes
// -----------------------------------------------------------------------------

// C++
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <new>
#include <sstream>
#include <vector>

// OpenMP
#ifdef _OPENMP
#include <omp.h>
#endif



// -----------------------------------------------------------------------------
// Kip #includes
// -----------------------------------------------------------------------------

#define _kip_quote(arg) #arg
#define  kip_quote(arg) _kip_quote(arg)

#define _kip_paste(one,two)  one ## two
#define  kip_paste(one,two) _kip_paste(one,two)

#define _kip_description(one,two,three) one #two three
#define  kip_description(arg) _kip_description("logical-", arg, " operator")

namespace kip {

// ----------------
// declarations
// ----------------

class istream;
class ostream;

// ----------------
// misc
// ----------------

#include "kip-misc-basic.h"
#include "kip-misc-point.h"
#include "kip-misc-array.h"
#include "kip-misc-rotate.h"

#include "kip-color-rgb.h"
#include "kip-color-crayola.h"
#include "kip-color-marble.h"
#include "kip-color-colormap.h"

// ----------------
// i/o
// ----------------

#include "kip-io-stream.h"
#include "kip-io-read.h"

// ----------------
// shapes
// ----------------

// misc
#include "kip-shape-misc.h"
#include "kip-shape-bbox.h"
#include "kip-shape.h"

// parts of primitives
#include "kip-shape-tri.h"

// primitives
#include "kip-shape-bicylinder.h"
#include "kip-shape-biwasher.h"
#include "kip-shape-box.h"
#include "kip-shape-cone.h"
#include "kip-shape-cylinder.h"
#include "kip-shape-cube.h"
#include "kip-shape-ellipsoid.h"
#include "kip-shape-paraboloid.h"
#include "kip-shape-nothing.h"
#include "kip-shape-everything.h"
#include "kip-shape-half.h"
#include "kip-shape-circle.h"
#include "kip-shape-xplane.h"
#include "kip-shape-yplane.h"
#include "kip-shape-zplane.h"
#include "kip-shape-pill.h"
#include "kip-shape-silo.h"
#include "kip-shape-sphere.h"
#include "kip-shape-spheroid.h"
#include "kip-shape-triangle.h"
#include "kip-shape-polygon.h"
#include "kip-shape-washer.h"
#include "kip-shape-tabular.h"
#include "kip-shape-surf.h"

// operators
#include "kip-operator.h"

// operators: unary
#include "kip-operator-unary-not.h"

// operators: binary
#include "kip-operator-binary-and.h"
#include "kip-operator-binary-cut.h"
#include "kip-operator-binary-or.h"
#include "kip-operator-binary-xor.h"

// operators: nary
#include "kip-operator-nary-ands.h"
#include "kip-operator-nary-ors.h"
#include "kip-operator-nary-one.h"
#include "kip-operator-nary-some.h"
#include "kip-operator-nary-odd.h"
#include "kip-operator-nary-even.h"

// misc
#include "kip-shape-end.h"

// ----------------
// per-pixel info;
// coloring
// ----------------

#include "kip-type-ray.h"
#include "kip-color-texture.h"

// ----------------
// major classes
// ----------------

// for kip's use
#include "kip-type-vars.h"

// for users
#include "kip-type-model.h"
#include "kip-type-view.h"
#include "kip-type-light.h"
#include "kip-type-engine.h"
#include "kip-type-image.h"
#include "kip-type-scene.h"

// ----------------
// ray tracing
// ----------------

// helper
#include "kip-trace-dry.h"
#include "kip-trace-fill.h"
// #include "kip-trace-fill-triangle.h"
// #include "kip-trace-fill-tri.h"

// methods
#include "kip-trace-uniform.h"
#include "kip-trace-recursive.h"
#include "kip-trace-block.h"

// api
#include "kip-trace.h"

// ----------------
// cleanup
// ----------------

#undef _kip_quote
#undef  kip_quote
#undef _kip_paste
#undef  kip_paste
#undef _kip_description
#undef  kip_description
#undef  kip_expand
#undef  kip_extra

} // namespace kip
