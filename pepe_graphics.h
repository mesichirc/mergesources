#ifndef PEPE_GRAPHICS_H
#define PEPE_GRAPHICS_H

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

typedef struct Pepe_GRect PepeGRect;
struct Pepe_GRect {
  Pepe_GPoint origin;
  Pepe_GSize  size;
};

#endif
