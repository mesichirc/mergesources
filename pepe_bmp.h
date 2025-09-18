#ifndef PEPE_BMP
#define PEPE_BMP

#include "pepe_core.h"
#define PEPE_BI_ALPHABITFIELDS 108
#define PEPE_BMP_FILE_HEADER   14
#define PEPE_BMP_HEADER_SIZE (PEPE_BI_ALPHABITFIELDS + PEPE_BMP_FILE_HEADER)
#define PEPE_BMP_BI_RGB 0
#define PEPE_BMP_BI_BITFIELDS 3

typedef struct Pepe_BMP Pepe_BMP;
struct Pepe_BMP {
  Pepe_Slice bytes;
  u32 width;
  u32 height;
  int valid;
};

u32
Pepe_BMP_PutU16ToSlice(Pepe_Slice slice, u16 i)
{
  u16 *base = (u16 *)slice.base;

  *base = PEPE_IS_BE() ? Pepe_U16SwapBytes(i) : i;
  return 2;
}

u32
Pepe_BMP_PutI32ToSlice(Pepe_Slice slice, i32 i)
{
  i32 *base = (i32 *)slice.base;
  *base = PEPE_IS_BE() ? (i32)Pepe_U32SwapBytes((u32)i) : i;
  return 4;  
}

u32
Pepe_BMP_Width(Pepe_BMP *bitmap)
{
  u32 width;
  width = *(u32 *)Pepe_SliceRight(bitmap->bytes, PEPE_BMP_FILE_HEADER + 4).base;

  return PEPE_IS_BE() ? Pepe_U32SwapBytes(width) : width;
}

u32
Pepe_BMP_Height(Pepe_BMP *bitmap)
{
  u32 height;
  height = *(u32 *)Pepe_SliceRight(bitmap->bytes, PEPE_BMP_FILE_HEADER + 8).base;

  return PEPE_IS_BE() ? Pepe_U32SwapBytes(height) : height;
}

bool
Pepe_BMP_ColorPixel(Pepe_BMP *bitmap, u32 x, u32 y, u8 r, u8 g, u8 b, u8 a)
{
  bool result;
  u32 width, height, pos;
  Pepe_Slice imageSlice;
  u8 *buf;

  result = false;

  if (!bitmap->valid) {
    return result;
  }

  width = Pepe_BMP_Width(bitmap);
  height = Pepe_BMP_Height(bitmap);
  if (width < x || height < y) {
    return result;
  }

  imageSlice = Pepe_SliceRight(bitmap->bytes, PEPE_BMP_HEADER_SIZE);
  buf = (u8*)imageSlice.base;
  pos = y * width * 4 + x * 4;

  buf[pos + 0] = a;
  buf[pos + 1] = r;
  buf[pos + 2] = g;
  buf[pos + 3] = b;
  
  result = true;
    
  return result;
}

Pepe_BMP
Pepe_BMP_BitmapCreate(Pepe_Arena *arena, u32 width, u32 height)
{
  Pepe_BMP bitmap;
  u8 *base;
  u32 size, offset, imgSize, pitch;

  offset = 0;
  pitch = width * 4;
  imgSize = pitch * height;
  size = imgSize + PEPE_BMP_HEADER_SIZE;
  bitmap.valid = false;
  bitmap.bytes.base = Pepe_ArenaAllocAlign(arena, size, PEPE_DEFAULT_ALIGNMENT);

  if (bitmap.bytes.base != nil) {
    bitmap.bytes.capacity = size;
    bitmap.bytes.length = size;

    base = (u8 *)bitmap.bytes.base;
    // write file header
    base[offset++] = 'B';
    base[offset++] = 'M';
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), size);
    offset += 4;
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), PEPE_BMP_HEADER_SIZE);

    // write DIB header
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), PEPE_BI_ALPHABITFIELDS);
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), width);
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), height);
    offset += Pepe_BMP_PutU16ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 1);
    offset += Pepe_BMP_PutU16ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 32);
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), PEPE_BMP_BI_BITFIELDS);
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), imgSize);
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 2835); // 72 DPI pritn resolution
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 2835); // 72 DPI pritn resolution
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 0); // numbers of colors in palette
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 0); // 0 means all colors are important
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 0x0000FF00); // red channel mask
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 0x00FF0000); // green channel mask
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 0xFF000000); // blue channel mask
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 0x000000FF); // alpha channel mask
    offset += Pepe_BMP_PutI32ToSlice(Pepe_SliceRight(bitmap.bytes, offset), 0x206E6957); // 'Win ' in hex
    offset += 36; // CIEXYZTRIPLE Color Space endpoints, unused so just skip
  } 


  return bitmap;
}




#endif
