#ifndef PEPE_GRAPHICS_H
#define PEPE_GRAPHICS_H

typedef struct Pepe_GColor Pepe_GColor;
struct Pepe_GColor {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
};

typedef f32 Pepe_GF32Vec2[2];
typedef f32 Pepe_GF32Vec3[3];

typedef struct Pepe_GPoint Pepe_GPoint;
struct Pepe_GPoint {
  f32 x;
  f32 y;
};

typedef struct Pepe_GSize Pepe_GSize;
struct Pepe_GSize {
  f32 width;
  f32 height;
};

typedef struct Pepe_GRect Pepe_GRect;
struct Pepe_GRect {
  Pepe_GPoint origin;
  Pepe_GSize  size;
};

Pepe_GRect
Pepe_GRectMake(f32 x, f32 y, f32 width, f32 height)
{
  Pepe_GRect rect;

  rect.origin.x = x;
  rect.origin.y = y;
  rect.size.width = width;
  rect.size.height = height;

  return rect;
}

#endif
