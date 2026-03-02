#ifndef FOX_GAME
#define FOX_GAME

typedef struct vec2 vec2;
struct vec2 {
  f32 x;
  f32 y;
};

vec2 vec2_constant_mul(vec2 v, f32 c)
{
  return (vec2){ v.x * c, v.y * c };
}

Pepe_Color 
color_interpolate(Pepe_Color from, Pepe_Color to, f32 v)
{
  u8 fromBuf[PEPE_COLOR_COMPS];
  u8 toBuf[PEPE_COLOR_COMPS];
  Pepe_ColorUnpack(from, fromBuf);
  Pepe_ColorUnpack(to, toBuf);
  
  fromBuf[0] = (u8)((f32)fromBuf[0] + ((f32)toBuf[0] - (f32)fromBuf[0]) * v);
  fromBuf[1] = (u8)((f32)fromBuf[1] + ((f32)toBuf[1] - (f32)fromBuf[1]) * v);
  fromBuf[2] = (u8)((f32)fromBuf[2] + ((f32)toBuf[2] - (f32)fromBuf[2]) * v);
  fromBuf[3] = (u8)((f32)fromBuf[3] + ((f32)toBuf[3] - (f32)fromBuf[3]) * v);
  return Pepe_ColorMake(fromBuf[0], fromBuf[1], fromBuf[2], fromBuf[3]);
  
}

typedef u32 FoxEventType;
enum {
  FoxEventNone = 0,
  FoxEventKeyPressed,
  FoxEventKeyReleased,
  FoxEventMouseBtnPressed,
  FoxEventMouseBtnReleased,
  FoxEventMouseScroll,
  FoxEventMousePositionChanged
};

typedef u32 FoxKeyCode;
enum {
  FoxKeyCodeNull = 0,
  FoxKeyCodeEscape = '\033',
  FoxKeyCodeBacktick = '`',
  FoxKeyCode0 = '0',
  FoxKeyCode1 = '1',
  FoxKeyCode2 = '2',
  FoxKeyCode3 = '3',
  FoxKeyCode4 = '4',
  FoxKeyCode5 = '5',
  FoxKeyCode6 = '6',
  FoxKeyCode7 = '7',
  FoxKeyCode8 = '8',
  FoxKeyCode9 = '9',
  FoxKeyCodeMinus = '-',
	FoxKeyCodeEquals = '=',
	FoxKeyCodeBackSpace = '\b',
	FoxKeyCodeTab = '\t',
	FoxKeyCodeSpace = ' ',
  FoxKeyCodeA = 'a',
	FoxKeyCodeB = 'b',
	FoxKeyCodeC = 'c',
	FoxKeyCodeD = 'd',
	FoxKeyCodeE = 'e',
	FoxKeyCodeF = 'f',
	FoxKeyCodeG = 'g',
	FoxKeyCodeH = 'h',
	FoxKeyCodeI = 'i',
	FoxKeyCodeJ = 'j',
	FoxKeyCodeK = 'k',
	FoxKeyCodeL = 'l',
	FoxKeyCodeM = 'm',
	FoxKeyCodeN = 'n',
	FoxKeyCodeO = 'o',
	FoxKeyCodeP = 'p',
	FoxKeyCodeQ = 'q',
	FoxKeyCodeR = 'r',
	FoxKeyCodeS = 's',
	FoxKeyCodeT = 't',
	FoxKeyCodeU = 'u',
	FoxKeyCodeV = 'v',
	FoxKeyCodeW = 'w',
	FoxKeyCodeX = 'x',
	FoxKeyCodeY = 'y',
	FoxKeyCodeZ = 'z',
  FoxKeyCodePeriod = '.',
	FoxKeyCodeComma = ',',
	FoxKeyCodeSlash = '/',
	FoxKeyCodeBracket = '[',
  FoxKeyCodeCloseBracket = ']',
  FoxKeyCodeSemicolon = ';',
	FoxKeyCodeApostrophe = '\'',
	FoxKeyCodeBackSlash = '\\',
	FoxKeyCodeReturn = '\n',
	FoxKeyCodeEnter = FoxKeyCodeReturn,
	FoxKeyCodeDelete = '\177', /* 127 */
	FoxKeyCodeF1,
	FoxKeyCodeF2,
	FoxKeyCodeF3,
	FoxKeyCodeF4,
	FoxKeyCodeF5,
	FoxKeyCodeF6,
	FoxKeyCodeF7,
	FoxKeyCodeF8,
	FoxKeyCodeF9,
	FoxKeyCodeF10,
	FoxKeyCodeF11,
	FoxKeyCodeF12,
  FoxKeyCodeF13,
  FoxKeyCodeF14,
  FoxKeyCodeF15,
  FoxKeyCodeF16,
  FoxKeyCodeF17,
  FoxKeyCodeF18,
  FoxKeyCodeF19,
  FoxKeyCodeF20,
  FoxKeyCodeF21,
  FoxKeyCodeF22,
  FoxKeyCodeF23,
  FoxKeyCodeF24,
  FoxKeyCodeF25,
	FoxKeyCodeCapsLock,
	FoxKeyCodeShiftL,
	FoxKeyCodeControlL,
	FoxKeyCodeAltL,
	FoxKeyCodeSuperL,
	FoxKeyCodeShiftR,
	FoxKeyCodeControlR,
	FoxKeyCodeAltR,
	FoxKeyCodeSuperR,
	FoxKeyCodeUp,
	FoxKeyCodeDown,
	FoxKeyCodeLeft,
	FoxKeyCodeRight,
	FoxKeyCodeInsert,
	FoxKeyCodeMenu,
	FoxKeyCodeEnd,
	FoxKeyCodeHome,
	FoxKeyCodePageUp,
	FoxKeyCodePageDown,
	FoxKeyCodeNumLock,
	FoxKeyCodeKpSlash,
	FoxKeyCodeKpMultiply,
	FoxKeyCodeKpPlus,
	FoxKeyCodeKpMinus,
	FoxKeyCodeKpEqual,
	FoxKeyCodeKp1,
	FoxKeyCodeKp2,
	FoxKeyCodeKp3,
	FoxKeyCodeKp4,
	FoxKeyCodeKp5,
	FoxKeyCodeKp6,
	FoxKeyCodeKp7,
	FoxKeyCodeKp8,
	FoxKeyCodeKp9,
	FoxKeyCodeKp0,
	FoxKeyCodeKpPeriod,
	FoxKeyCodeKpReturn,
	FoxKeyCodeScrollLock,
  FoxKeyCodePrintScreen,
  FoxKeyCodePause,
	FoxKeyCodeWorld1,
  FoxKeyCodeWorld2,
  FoxKeyCodeLast = 256
};

typedef u32 FoxEventFlag;
enum {
  FoxKeyPressedFlag            = (1 << FoxEventKeyPressed),
  FoxKeyReleasedFlag           = (1 << FoxEventKeyReleased),
  FoxMouseScrollFlag           = (1 << FoxEventMouseScroll),
  FoxMouseBtnPressedFlag       = (1 << FoxEventMouseBtnPressed),
  FoxMouseBtnReleasedFlag      = (1 << FoxEventMouseBtnReleased),
  FoxMousePositionChangedFlag  = (1 << FoxEventMousePositionChanged)

};

const char *Fox_KeyCodesStrings[FoxKeyCodeLast];

typedef struct FoxInput FoxInput;
struct FoxInput {
  u8            characters[512];
  FoxKeyCode    keyCodes[FoxKeyCodeLast];
  int           mouseX;
  int           mouseY;
  u16           charactersLength;
  FoxEventFlag  flag;
  u8            mouseButtonsState[64];
  f32           deltaX;
  f32           deltaY;
  Pepe_Bitmap   *fonts; 
};

typedef struct FoxInputHandle FoxInputHandle;
struct FoxInputHandle {
  FoxInput *newInput;
  FoxInput *prevInput;
};


enum Foxy_ColorTokens {
  Foxy_ColorText,
  Foxy_ColorBase,
  Foxy_ColorLove,
  Foxy_ColorGold,
  Foxy_ColorRose,
  Foxy_ColorPine,
  Foxy_ColorFoam,
  Foxy_ColorIris,
  Foxy_ColorSize
};

static u32 Foxy_Colors[Foxy_ColorSize] = {
  [Foxy_ColorText] = 0x575279ff,
  [Foxy_ColorBase] = 0xfaf4edff,
  [Foxy_ColorLove] = 0xb4637aff,
  [Foxy_ColorGold] = 0xea9d34ff,
  [Foxy_ColorRose] = 0xd7827eff,
  [Foxy_ColorPine] = 0x286983ff,
  [Foxy_ColorFoam] = 0x56949fff,
  [Foxy_ColorIris] = 0x907aa9ff,
};

typedef struct State State;
struct State {
  vec2 from;
  vec2 to;
  Pepe_Color cfrom;
  Pepe_Color cto;
  vec2 current;
  f64 frametime;
  f64 refreshrate;
  f64 timeelapsed;
  f64 animtime;
  f32 k;
  f32 anim_val;
};

static State state;
#define RECT_TOP 128.0
#define RECT_BOTTOM 512.0
#define RECT_LEFT 16.0
#define RECT_RIGHT 128.0

f32 
f32_interpolate(f32 from, f32 to, f32 v)
{
  return from + (to - from) * v;
}

u32
u32_interpolate(u32 from, u32 to, f32 v)
{
  return (u32)f32_interpolate((f32)from, (f32)to, v);
}

vec2
vec2_interpolate(vec2 from, vec2 to, f32 v)
{
  return (vec2) { f32_interpolate(from.x, to.x, v), f32_interpolate(from.y, to.y, v) };
}
/**
 * Colors defined in Big Endian for sake of simplicity,
 * so if we running on Little Endian we need to check and convert all colors
 */
void
Foxy_Init(f64 frametimems, f64 refreshrate, f64 targetfps)
{
  state.current = (vec2){ RECT_LEFT, RECT_BOTTOM };
  state.refreshrate = refreshrate;
  state.k = 0.0001;
  state.animtime = 1000.0;
  state.timeelapsed = 0.0;
  state.frametime = frametimems;
  state.anim_val = 0.0;
  state.from = (vec2){ RECT_RIGHT, RECT_TOP };
  state.to = (vec2){ RECT_LEFT, RECT_BOTTOM };
  static bool isColorInitialized;
  if (isColorInitialized) {
    return;
  }
  isColorInitialized = true;
  if (!PEPE_IS_BE()) {
    for (int i = 0; i < Foxy_ColorSize; i += 1) {
      Foxy_Colors[i] = Pepe_U32SwapBytes(Foxy_Colors[i]);
    }
  }
  state.cfrom.value = Foxy_Colors[Foxy_ColorRose];
  state.cto.value = Foxy_Colors[Foxy_ColorGold];
}

/**
 * color must be converted to right byte order
 */
void
Foxy_ClearBitmap(u32 color, u8 *buffer, u32 width, u32 height)
{
  u32 *intBuf = (u32 *)buffer;
  u32 bufferSize = width * height;
  for (int i = 0; i < bufferSize; i += 1) {
    intBuf[i] = color;
  }
}

u8
checkbit(u8* glyph, u32 position) 
{
  u8 b = *(glyph + (u32)(position / 8));
  return (b >> (u8)(position % 8)) & 1;
}

void
renderglyph(Pepe_Bitmap *canvas, int x, int y, u8 scale, u8 *glyph, u16 w, u16 h, Pepe_Color clr)
{
  int cx, cy, st, pos;
  cx = x;
  cy = y;
  pos = 0;

  for (int ly = 0; ly < h; ly += 1) {
    for (int lx = 0; lx < w; lx += 1) {
      if (checkbit(glyph, pos)) {
        Pepe_DrawRect(canvas, cx + lx * scale, cy + ly * scale, scale, scale, clr);
      }
      pos += 1;
    }
  }
}

char renderstring1[] = "#abcdefghjklmnopqrstuvwyz\0";
char renderstring2[] = "\"$%&\\()*+,-./0123456789:;<=>?@";


void
draw_rect_subpixel(Pepe_Bitmap *canvas, f32 x, f32 y, u32 w, u32 h, Pepe_Color color)
{
  u32 h1, w1, x1, y1;
  f32 xshift, yshift;
  f32 t = 0.001;
  x1 = (u32)x;
  y1 = (u32)y;
  h1 = h;
  w1 = w;

  xshift = (x - (f32)x1);
  if ((xshift > t) && (w > 2)) {
    w1 = w - 1;
    x1 += 1;
  }

  yshift = (y - (f32)y1);
  if (yshift > t && (h > 2)) {
    h1 = h - 1;
    y1 += 1;
  } 

  Pepe_DrawRect(canvas, x1, y1, w1, h1, color);
  f32 alpha = (f32)Pepe_ColorGetAlpha(color);
  Pepe_Color top = Pepe_ColorSetAlpha(color, (u8)((1.0 - yshift) * alpha));
  Pepe_Color bottom = Pepe_ColorSetAlpha(color, (u8)(yshift) * alpha);
  Pepe_Color left = Pepe_ColorSetAlpha(color, (u8)((1.0 - xshift) * alpha));
  Pepe_Color right = Pepe_ColorSetAlpha(color, (u8)((xshift) * alpha));
  if (xshift > t && yshift > t) {   
    Pepe_Color tl = color;
    Pepe_Color tr = color;
    Pepe_Color bl = color;
    Pepe_Color br = color;
    br = Pepe_ColorSetAlpha(color, (u8)(min(xshift, yshift) * alpha));
    tr = Pepe_ColorSetAlpha(color, (u8)(min(xshift, 1.0 - yshift) * alpha));
    bl = Pepe_ColorSetAlpha(color, (u8)(min(1.0 - xshift, yshift) * alpha));
    tl = Pepe_ColorSetAlpha(color, (u8)(min(1.0 - xshift, 1.0 - yshift) * alpha));
    // Top left
    Pepe_DrawRect(canvas, (u32)x, (u32)y, 1, 1, tl);
    // Top right
    Pepe_DrawRect(canvas, (u32)x + w, (u32)y, 1, 1, tr);
    // Bottom right
    Pepe_DrawRect(canvas, (u32)x + w, (u32)y + h, 1, 1, br);
    // Bottom left
    Pepe_DrawRect(canvas, (u32)x, (u32)y + h, 1, 1, bl);

    // Top horizontal
    Pepe_DrawRect(canvas, x1, (u32)y, w1, 1, top);
    // Left vertical
    Pepe_DrawRect(canvas, (u32)x, y1, 1, h1, left);
    // Bottom horizontal
    Pepe_DrawRect(canvas, x1, (u32)y + h, w1, 1, bottom);
    // Right vertical
    Pepe_DrawRect(canvas, x1 + w1, y1, 1, h1, right);
  } else if (xshift > t) {
    // Left vertical
    Pepe_DrawRect(canvas, (u32)x, (u32)y, 1, h, left);
    // Right vertical
    Pepe_DrawRect(canvas, (u32)x + w, (u32)y, 1, h, right);
  } else if (yshift > t) {
    // Top horizontal
    Pepe_DrawRect(canvas, (u32)x, (u32)y, w, 1, top);
    // Bottom horizontal
    Pepe_DrawRect(canvas, (u32)y + h, (u32)x, w, 1, bottom);
  } 
}


#define STR8_LIT(s) (string8) { (u8*)(s), sizeof((s)) - 1 }

typedef struct string8 string8;
struct string8 {
  u8* base;
  u64 size;
};

#define MAX_U32_LENGTH 10
string8
u32_to_string8(string8 str, u32 n)
{
  i32 offset = MAX_U32_LENGTH - 1;
  u8 maxbuf[MAX_U32_LENGTH] = {0};
  u64 length = 0;
  u32 rest = n;
  
  if (n == 0) {
    str.base[0] = '0';
    str.size = 1;
    return str;
  } 

  while (rest > 0) {
    maxbuf[offset] = '0' + (u32)(rest % 10);
    rest /= 10;
    offset -= 1;
  }

  str.size = min((MAX_U32_LENGTH - offset - 1), str.size);
  memcpy(str.base, maxbuf + offset + 1, str.size); 
}

u32
renderstring(Pepe_Bitmap *canvas, u32 x, u32 y, string8 str, Pepe_Color color)
{
  u8 scale = 3;
  for (int i = 0; i < str.size; i += 1) {
    renderglyph(canvas, x, y, scale, getglyph((int)str.base[i]), LETTER_WIDTH, LETTER_HEIGHT, color);
    x += LETTER_WIDTH * scale + scale;
  }
  return x;
}

void
FoxUpdateAndRender(u8* buffer, u32 width, u32 height, FoxInputHandle input, f64 timestampms, f64 fps)
{
  Pepe_Color letterColor;
  Pepe_Bitmap canvas;
  canvas.memory = buffer;
  canvas.width  = width;
  canvas.height = height;
  canvas.bytesPerPixel = 4;
  canvas.pitch  = width;
  canvas.scale = 1;

  letterColor.value = Foxy_Colors[Foxy_ColorText];

  Foxy_ClearBitmap(Foxy_Colors[Foxy_ColorBase], buffer, width, height);
  u32 x = 0;
  x = renderstring(&canvas, 8, 8, STR8_LIT("FPS:"), letterColor);
  u8 numbuf[10];
  string8 numstr = (string8) { numbuf, 10 };
  numstr = u32_to_string8(numstr, (u32)fps);
  x = renderstring(&canvas, x, 8, numstr, letterColor);

  x = renderstring(&canvas, x, 8, STR8_LIT("."), letterColor);
  u32 precition = 6;
  for (u32 i = 0; i < precition; i += 1) {
    fps = (fps - (f64)((u64)(fps))) * 10.0;
    numstr = u32_to_string8(numstr, (u32)fps);
    x = renderstring(&canvas, x, 8, numstr, letterColor);
  }
  

  Pepe_Color rectclr = {0};
  rectclr = color_interpolate(state.cfrom, state.cto, state.anim_val);
  draw_rect_subpixel(&canvas, state.current.x, state.current.y, 48, 48, rectclr);
  f64 dt = (timestampms - state.frametime);
  state.frametime = timestampms;
  state.anim_val = (state.timeelapsed / state.animtime);
  state.anim_val = max(min(state.anim_val, 1.0), 0.0);
  state.timeelapsed += dt;
  state.current = vec2_interpolate(state.from, state.to, state.anim_val);
  if ((1.0 - state.anim_val) <= 0.00001) {
    state.anim_val = 0;
    vec2 to = state.to;
    state.timeelapsed = 0;
    state.to = state.from;
    state.from = to;
    Pepe_Color cto = state.cto;
    state.cto = state.cfrom;
    state.cfrom = cto;
  } 
}

#endif
