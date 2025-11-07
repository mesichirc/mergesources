/**
 *
 * 1. Render just in time without cache
 * 2. Cache codepoint info
 * 3. Cache kerning info
 * 4. Cache bitmap in texture
 *
 */

typedef struct Pepe_Font Pepe_Font;
struct Pepe_Font {
  stbtt_fontinfo fontInfo;
  f32 scale;
  int ascent;
  int baseline;
};

void
Pepe_FontInit(Pepe_Font *font, const char *path, f32 height)
{
  Pepe_Slice fontfile = Pepe_IO_ReadEntireFileFromPathDebug(path);
  stbtt_InitFont(&font->fontInfo, fontfile->base, 0);
  font->scale = stbtt_ScaleForPixelHeight(&font->fontInfo, height);
  stbtt_GetFontVMetrics(&font->fontInfo, &ascent, 0, 0); 
  font->baseline = (int) (ascent * scale);
}
