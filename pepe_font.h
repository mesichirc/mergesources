typedef struct Pepe_CharData Pepe_CharData;
struct Pepe_CharData {
  u8 x0, y0, x1, y1;
  f32 xoff, yoff, xadvance;
};

typedef struct Pepe_Glyphset Pepe_Glyphset;
struct Pepe_Glyphset {
  Pepe_Bitmap image;
  Pepe_CharData *chars;
};

