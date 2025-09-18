#ifndef PEPE_GRAPHICS_H
#define PEPE_GRAPHICS_H

typedef struct Pepe_Color Pepe_Color;
struct Pepe_Color {
  u32 value;
};

typedef struct Pepe_Bitmap Pepe_Bitmap;
struct Pepe_Bitmap {
  u8 *memory;
  u32 width;
  u32 height;
  u32 pitch;
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

void
Pepe_DrawRect(
    Pepe_Bitmap *canvas, 
    u32 x, u32 y,
    u32 width, u32 height, 
    Pepe_Color color // ABGR 
)
{
  u32 currentPixel;
  u32 endPixel;
  u32 row, rowWidth;
  u32 *memory;
  if (x > canvas->width || y > canvas->height || width == 0 || height == 0) {
    return;
  }

  currentPixel = canvas->width * y + x;
  endPixel = (canvas->width * (min(canvas->height, y + height) - 1)) + min(x + width, canvas->width);
  row = 0;
  rowWidth = x + width > canvas->width ? canvas->width - x : width;
  memory = (u32*)canvas->memory;
  while (currentPixel < endPixel) {
    memory[currentPixel] = color.value;
    currentPixel++;
    row++;
    if (row == rowWidth) {
      currentPixel += (canvas->width - rowWidth);
      row = 0;
    }
  }
}

#if 0
void
Pepe_DrawAlphaBitmap(
    Pepe_Bitmap *canvas,
    u32 x, u32 y,
    Pepe_Bitmap *bitmap,
    u32 color
) 
{
  u32 currentPixel;
  u32 endPixel;
  u32 row, rowWidth;
  u32 *memory;
  if (x > canvas->width || y > canvas->height || width == 0 || height == 0) {
    return;
  }

  currentPixel = canvas->width * y + x;
  endPixel = (canvas->width * (min(canvas->height, y + height) - 1)) + min(x + width, canvas->width);
  row = 0;
  rowWidth = x + width > canvas->width ? canvas->width - x : width;
  memory = (u32*)canvas->memory;
  while (currentPixel < endPixel) {
    memory[currentPixel] = color.value;
    currentPixel++;
    row++;
    if (row == rowWidth) {
      currentPixel += (canvas->width - rowWidth);
      row = 0;
    }
  }
}
#endif


#endif
