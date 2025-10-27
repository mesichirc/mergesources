#ifndef PEPE_GRAPHICS_H
#define PEPE_GRAPHICS_H

#include "u.h"
typedef struct Pepe_Color Pepe_Color;
struct Pepe_Color {
  u32 value;
};


typedef struct Pepe_Rect Pepe_Rect;
struct Pepe_Rect {
  i32 x;
  i32 y;
  u32 w;
  u32 h;
};


typedef struct Pepe_Bitmap Pepe_Bitmap;
struct Pepe_Bitmap {
  u8 *memory;
  u32 width;
  u32 height;
  u32 pitch;
  u16 bytesPerPixel;
  u16 scale;
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

Pepe_Color  
Pepe_ColorMake(u8 r, u8 g, u8 b, u8 a)
{
  Pepe_Color color;

  color.value = (((u32)(a) << 24) & (0xFF000000)) |
                (((u32)(b) << 16) & (0x00FF0000)) |
                (((u32)(g) << 8 ) & (0x0000FF00)) |
                (((u32)(r) << 0 ) & (0x000000FF));

  return color;
}

#define PEPE_COLOR_R 0
#define PEPE_COLOR_G 1
#define PEPE_COLOR_B 2
#define PEPE_COLOR_A 3
#define PEPE_COLOR_COMPS 4

void
Pepe_ColorUnpack(Pepe_Color color, u8 buf[PEPE_COLOR_COMPS])
{
  buf[PEPE_COLOR_A] = (u8)((color.value >> 24) & 0xFF);
  buf[PEPE_COLOR_B] = (u8)((color.value >> 16) & 0xFF);
  buf[PEPE_COLOR_G] = (u8)((color.value >> 8) & 0xFF);
  buf[PEPE_COLOR_R] = (u8)((color.value >> 0) & 0xFF);
}

Pepe_Color 
Pepe_ColorSetRed(Pepe_Color color, u8 r)
{
  Pepe_Color result;

  result.value = color.value & (0xFFFFFF00);
  result.value = result.value | (((u32)(r) << 0) & (0x000000FF));
  return result;
}

Pepe_Color 
Pepe_ColorSetAlpha(Pepe_Color color, u8 a)
{
  Pepe_Color result;

  result.value = color.value & (0x00FFFFFF);
  result.value = result.value | (((u32)(a) << 24) & (0xFF000000));
  return result;
}

#define PEPE_COLOR_RED Pepe_ColorMake(255, 0, 0, 255)
#define PEPE_COLOR_BLUE Pepe_ColorMake(0, 0, 255, 255)
#define PEPE_COLOR_GREEN Pepe_ColorMake(0, 255, 0, 255)
#define PEPE_COLOR_BLACK Pepe_ColorMake(0, 0, 0, 255)
#define PEPE_COLOR_WHITE Pepe_ColorMake(255, 255, 255, 255)

u8 
Pepe_MixComps(u16 c1, u16 c2, u16 a)
{
  return c1 + (c2 - c1) * a / 255;
}

Pepe_Color Pepe_MixColors(Pepe_Color c1, Pepe_Color c2)
{
  Pepe_Color result;
  u8 comp1[PEPE_COLOR_COMPS];
  Pepe_ColorUnpack(c1, comp1);

  u8 comp2[PEPE_COLOR_COMPS]; 
  Pepe_ColorUnpack(c2, comp2);

  result = Pepe_ColorMake(
      Pepe_MixComps(comp1[PEPE_COLOR_R], comp2[PEPE_COLOR_R], comp2[PEPE_COLOR_A]), 
      Pepe_MixComps(comp1[PEPE_COLOR_G], comp2[PEPE_COLOR_G], comp2[PEPE_COLOR_A]), 
      Pepe_MixComps(comp1[PEPE_COLOR_B], comp2[PEPE_COLOR_B], comp2[PEPE_COLOR_A]), 
      comp1[PEPE_COLOR_A]
  );

  return result;
}

u32 
Pepe_MixColorsU32(u32 c1, u32 c2)
{
  Pepe_Color a, b;
  a.value = c1;
  b.value = c2;

  b = Pepe_MixColors(a, b);

  return b.value;
}

u32
Pepe_ClipU32(u32 c, u32 l, u32 r)
{
  return (c < l) ? l : (c > r ? r : c);
}

i32
Pepe_ClipI32(i32 c, i32 l, i32 r)
{
  return (c < l) ? l : (c > r ? r : c);
}

i32
Pepe_Smoothstep(i32 e0, i32 e1, i32 value)
{
  i32 t;
  t = Pepe_ClipI32((value - e0) / (e1 - e0), 0, 1);

  return t * t * (3 - 2 * t);
}

void
Pepe_ClipPoint(i32 point[2], i32 leftX, i32 leftY, i32 rightX, i32 rightY)
{
  point[0] = Pepe_ClipU32(point[0], leftX, rightX);
  point[1] = Pepe_ClipU32(point[1], leftY, rightY);
}

void
Pepe_ClipRect(
    Pepe_Rect *rect,
    Pepe_Rect clip
)
{  
  i32 origin[2];
  i32 bottomRight[2];
  origin[0] = rect->x;
  origin[1] = rect->y;

  bottomRight[0] = rect->x + rect->w;
  bottomRight[1] = rect->y + rect->h;

  Pepe_ClipPoint(origin, clip.x, clip.y, clip.x + clip.w, clip.y + clip.h);
  Pepe_ClipPoint(bottomRight, clip.x, clip.y, clip.x + clip.w, clip.y + clip.h);
  
  rect->x = origin[0];
  rect->y = origin[1];
  rect->w = bottomRight[0] - origin[0];
  rect->h = bottomRight[1] - origin[1];
}

inline u32
Pepe_AbsoluteValue(i32 v)
{
  u32 result;
  i32 mask = (v >> sizeof(i32)) - 1;
  result = (v + mask) ^ mask;

  return result;
}

Pepe_Rect
Pepe_ClipByCanvas(Pepe_Bitmap *canvas, i32 x, i32 y, u32 w, u32 h)
{
  Pepe_Rect rect, clip;
  rect.x = (x < 0 ? canvas->width + x : x);
  rect.y = (y < 0 ? canvas->height + y : y);
  rect.w = w;
  rect.h = h;

  clip.x = clip.y = 0;
  clip.w = canvas->width;
  clip.h = canvas->height;
  
  Pepe_ClipRect(&rect, clip);

  return rect;
}

inline u32
Pepe_GetPixel(u32 x, u32 y, u32 pitch, u32 scale)
{
  return (x + y * pitch * scale);
}

void
Pepe_DrawOpaqueRect(
    Pepe_Bitmap *canvas, 
    u32 x, u32 y,
    u32 width, u32 height, 
    Pepe_Color color // ABGR 
)
{
  Pepe_Rect rect;
  u32 *memory;

  rect = Pepe_ClipByCanvas(canvas, x, y, width, height);

  if (rect.w == 0 || rect.h == 0) {
    return;
  }

  memory = (u32*)canvas->memory;
  for (u32 y = rect.y * canvas->scale; y < (rect.y + rect.h) * canvas->scale; y ++) {
    for (u32 x = rect.x * canvas->scale; x < (rect.x + rect.w) * canvas->scale; x++) {
      memory[Pepe_GetPixel(x, y, canvas->pitch, canvas->scale)] = color.value;
    }
  }
}

void
Pepe_DrawRect(
    Pepe_Bitmap *canvas, 
    u32 x, u32 y,
    u32 width, u32 height, 
    Pepe_Color color // ABGR 
)
{
  u32 *memory;
  Pepe_Color c1;
  Pepe_Rect rect;

  rect = Pepe_ClipByCanvas(canvas, x, y, width, height);

  if (rect.w == 0 || rect.h == 0) {
    return;
  }
  memory = (u32*)canvas->memory;
  for (u32 y = rect.y * canvas->scale; y < (rect.y + rect.h) * canvas->scale; y ++) {
    for (u32 x = rect.x * canvas->scale; x < (rect.x + rect.w) * canvas->scale; x++) {
      u32 pixel = Pepe_GetPixel(x, y, canvas->pitch, canvas->scale);
      c1.value = memory[pixel];
      memory[pixel] = Pepe_MixColors(c1, color).value;
    }
  }
}

void
Pepe_DrawRectWrapped(
    Pepe_Bitmap *canvas, 
    i32 x, i32 y,
    u32 width, u32 height, 
    Pepe_Color color // ABGR 
)
{
  Pepe_Rect rect;

  rect = Pepe_ClipByCanvas(canvas, x, y, width, height);

  Pepe_DrawRect(canvas, x, y, width, height, color);
  if (rect.w == width && rect.h == height) {
    return;
  }

  if (y + height > canvas->height && x + width > canvas->width) {
    Pepe_DrawRect(canvas, 0, 0, width - rect.w, height - rect.h, color);
    return;
  }

  if (y + height > canvas->height) {
    Pepe_DrawRect(canvas, x, 0, width, height - rect.h, color);
  }
  if (x + width > canvas->width) {
    Pepe_DrawRect(canvas, 0, y, width - rect.w, height, color);
  }
}

#define PEPE_CANVAS_GET_PIXEL(canvas, x, y) \
  (canvas->width * y + x)

#define PEPE_ABS_MASK(x) (x >> 31)
i32
Pepe_ABS(i32 x)
{
  return (x + PEPE_ABS_MASK(x)) ^ PEPE_ABS_MASK(x);
}

void
Pepe_DrawCircle(
    Pepe_Bitmap *canvas,
    u32 x, u32 y, u32 r,
    Pepe_Color color
)
{
  u32 x0, y0, pixel, sqR;
  Pepe_Color c1, c2;
  i32 length;
  u32 *pixels;
  Pepe_Rect rect;
  rect.x = x >= r ? x - r : 0;
  rect.y = y >= r ? y - r : 0;
  rect.w = r + x - rect.x;
  rect.h = r + y - rect.y;
  sqR = r * r;
  unused(sqR);
  unused(length);
  unused(color);

  pixels = (u32 *)canvas->memory;

  for (x0 = rect.x; x0 < rect.x + rect.w; ++x0) {
    for (y0 = rect.y; y0 < rect.y + rect.h; ++y0) {
      length = ((i32)x0 - (i32)x) * ((i32)x0 - (i32)x) + ((i32)y0 - (i32)y) * ((i32)y0 - (i32)y) - sqR;
      length = -length;
      pixel = PEPE_CANVAS_GET_PIXEL(canvas, x0, y0);
    
      c1.value = pixels[pixel];
      c2 = Pepe_ColorSetAlpha(color, Pepe_ClipI32(sqrt(length), 0, 0xFF));
      pixels[pixel] = Pepe_MixColors(c1, c2).value;
    }
  }
}

void
Pepe_Draw4ChannelBitmap(
    Pepe_Bitmap *canvas,
    u32 x0, u32 y0,
    Pepe_Bitmap bitmap
)
{
  u32 x, y;
  Pepe_Rect rect;
  u32 *pixels;
  u32 *sourceDest;
  u32 *bitmapPixels;
  rect = Pepe_ClipByCanvas(canvas, x0, y0, bitmap.width, bitmap.height);
  pixels = (u32 *)canvas->memory;
  bitmapPixels = (u32 *)bitmap.memory;

  pixels += canvas->width * y0 + x0;
  sourceDest = pixels; 
  
  for (y = 0; y < rect.h; y++) {
    for (x = 0; x < rect.w; x++) {
      *sourceDest = Pepe_MixColorsU32(*sourceDest, *bitmapPixels);
      sourceDest++;
      bitmapPixels++;
    }
    pixels += canvas->width;
    sourceDest = pixels;
  } 
}

void
Pepe_Draw1ChannelBitmap(
    Pepe_Bitmap *canvas,
    u32 x0, u32 y0,
    Pepe_Color color,
    Pepe_Bitmap bitmap
)
{
  u32 x, y;
  Pepe_Rect rect;
  u32 *pixels;
  u8 *bitmapPixels;
  rect = Pepe_ClipByCanvas(canvas, x0, y0, bitmap.width, bitmap.height);
  pixels = (u32 *)canvas->memory;
  bitmapPixels = (u8 *)bitmap.memory;
  
  for (y = 0; y < rect.h; y++) {
    for (x = 0; x < rect.w; x++) {
      u32 sourcePixel = Pepe_GetPixel(x0 + x, y0 + y, canvas->pitch, canvas->scale);
      u32 destPixel = Pepe_GetPixel(x, y, bitmap.pitch, bitmap.scale);
      pixels[sourcePixel] = Pepe_MixColorsU32(
          pixels[sourcePixel],
          Pepe_ColorSetAlpha(color, bitmapPixels[destPixel]).value
      );
    }
  } 
}

#endif
