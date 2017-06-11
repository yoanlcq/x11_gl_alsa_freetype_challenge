#pragma once

#include <stdint.h>

#define Vec2(t) struct { t x, y; }
#define Uv(t) struct { t u, v; }
#define Extent2(t) struct { t w, h; }
#define Rgba(t) struct { t r, g, b, a; }
typedef Vec2(int32_t) Vec2i;
typedef Vec2(float) Vec2f;
typedef Extent2(uint32_t) Extent2u;
typedef Extent2(float) Extent2f;
typedef Rgba(uint8_t) Rgba32;
typedef Uv(float) TexUv;
_Static_assert(sizeof(Rgba32) == 4, "Rgba32 is not 4 bytes!");
