#pragma once

#define CONSOLE false

#if CONSOLE == true
#include <iostream>
#endif

constexpr float float_1div60 = 1.0f / 60.0f;

#define MODULO(x, a) ((((x) % (a)) + (a)) % (a))
#define SLIDE_INT(x1, x2, t, ds) ((x1) + ((x2) - (x1)) * (t) / (ds))
#define SLIDE_INT_EQ(x1, x2, t, ds) (x1) = SLIDE_INT(x1, x2, t, ds)
#define SLIDE_FLOAT(x1, x2, t) ((x1) + ((x2) - (x1)) * (t))
#define GET_SIGN(x) ((x) < 0 ? -1 : 1)

using uint = unsigned int;

#define SURF const surface& surf
#define SRC_DEST const surface& src, const surface& dest
#define DEST_SRC const surface& dest, const surface& src
using cpr = const point&;
#define COLOR const uint& color
