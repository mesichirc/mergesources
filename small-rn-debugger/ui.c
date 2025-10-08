#include <CoreGraphics/CGContext.h>
#include <CoreGraphics/CGImage.h>
#include <objc/objc.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

// global
NSApplication *NSApp;
NSWindow *win;
Pepe_Bitmap bitmap;
bool isRunning;
u32 mouseX;
u32 mouseY;
u32 lastMouseDownX;
u32 lastMouseDownY;
bool mouseDown;

#define BACKGROUND_COLOR Pepe_ColorMake(0x20, 0x20, 0x20, 0xFF)
#define WHITE_COLOR Pepe_ColorMake(0xFF, 0xFF, 0xFF, 0xFF)
#define COLOR_11 Pepe_ColorMake(0xe5, 0xb5, 0x66, 0xFF)
#define COLOR_11_HIGHLIGHT Pepe_ColorMake(0xf6, 0xc6, 0x77, 0xFF)

u8 *chessBuffer = nil;
u8 *alphaBuffer = nil;

Pepe_Bitmap
initChessBuffer(void)
{
  Pepe_Bitmap bitmap;
  u32 x, y, row, col, pixel;
  u32 *memory;
  Pepe_Color color;
  memory = (u32 *)chessBuffer;
  bitmap.memory = chessBuffer;
  bitmap.width = 50;
  bitmap.height = 50;
  bitmap.pitch = 4 * bitmap.width;
  color = BACKGROUND_COLOR;
 
  for (row = 0; row < 5; row++) {
    for (col = 0; col < 5; col++) {
      color = color.value == BACKGROUND_COLOR.value ? WHITE_COLOR : BACKGROUND_COLOR;
      for (y = row * 10; y < row * 10 + 10; y++) {
        for (x = col * 10; x < col * 10 + 10; x++) {
          pixel = bitmap.width * y + x;
          memory[pixel] = Pepe_ColorSetAlpha(color, 100).value;
        }
      }
    }
  }
  return bitmap;
}


bool
windowWillClose(void *self)
{
  unused(self);
  isRunning = false;
  return true;
}

static void *mouseLocationOutsideOfEventStreamFunc;
static u32 menuBarHeight;

NSPoint 
getMouseLocation(NSWindow *win)
{
  if (!mouseLocationOutsideOfEventStreamFunc) {
    mouseLocationOutsideOfEventStreamFunc = sel_registerName("mouseLocationOutsideOfEventStream");
  }

  return ((NSPoint (*)(id, SEL)) objc_msgSend)(win, mouseLocationOutsideOfEventStreamFunc);
}



CGImageRef 
create_image_from_bytes(Pepe_Bitmap *bitmap, f32 scale)
{
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(
			(u8*)bitmap->memory, 
			bitmap->width, bitmap->height,
			8,
			bitmap->pitch, 
			colorSpace,
			kCGImageAlphaPremultipliedLast
  );

  // Scale context so that the 1x buffer is drawn in high-res
  unused(scale);
  CGImageRef image = CGBitmapContextCreateImage(context);

  // Release the color space and context
	CGColorSpaceRelease(colorSpace);
	CGContextRelease(context);

  return image;
}

bool isHot = false;

void 
AppRender(Pepe_Bitmap *bitmap, Pepe_Bitmap fontBitmap)
{
  Pepe_Bitmap chessBitmap = initChessBuffer();
  Pepe_DrawOpaqueRect(bitmap, 0, 0, bitmap->width, bitmap->height, BACKGROUND_COLOR);
  Pepe_Color rectColor = COLOR_11;
  u32 buttonX = 40;
  u32 buttonY = 60 + menuBarHeight;
  u32 buttonWidth = 50;
  u32 buttonHeight = 50;


  if (mouseX >= 40 && mouseX <= (buttonX + buttonWidth) && mouseY >= buttonY && mouseY <= (buttonY + buttonHeight)) {
    rectColor = COLOR_11_HIGHLIGHT;
    if (mouseDown) {
      isHot = true;
    } else if (isHot) {
      printf("BUTTON CLICKED\n");
      isHot = false;
    }
  } else {
    isHot = false;
  }
  Pepe_DrawRect(bitmap, buttonX, buttonY, buttonWidth, buttonHeight, rectColor);
  Pepe_Draw4ChannelBitmap(bitmap, 0, 30 + menuBarHeight, chessBitmap);
  Pepe_Bitmap alphaBitmap;
  alphaBitmap.width = 50;
  alphaBitmap.height = 50;
  alphaBitmap.pitch = 1;
  alphaBitmap.memory = alphaBuffer;
  Pepe_Draw1ChannelBitmap(bitmap, 50, 30 + menuBarHeight, COLOR_11, alphaBitmap);
  Pepe_Draw1ChannelBitmap(bitmap, 0, 100 + menuBarHeight, COLOR_11, fontBitmap);
}

void
Redraw(Pepe_Bitmap fontBitmap)
{
  if (chessBuffer == nil) {
    chessBuffer =(u8 *)mmap(nil, 50 * 50 * sizeof(u32), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0); 
  }
  if (alphaBuffer == nil) {
    alphaBuffer =(u8 *)mmap(nil, 50 * 50, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0); 
    for (u32 i = 0; i < 50 * 50; i++) {
      alphaBuffer[i] = 100;
    }
  }
	NSScreen *nsscreen =  NSScreen_mainScreen();

  f32 scale = objc_msgSend_float(nsscreen, sel_registerName("backingScaleFactor"));
  NSRect viewFrame = NSWindow_frame(win);

  if (viewFrame.size.width != bitmap.width || viewFrame.size.height != bitmap.height) {
    if (bitmap.memory != nil) {
      munmap((void *)bitmap.memory, (u64)bitmap.height * (u64)bitmap.pitch);
    }
    bitmap.width = viewFrame.size.width;
    bitmap.height = viewFrame.size.height;
    bitmap.pitch = bitmap.width * sizeof(u32);
    bitmap.memory = (u8 *)mmap(nil, bitmap.pitch * bitmap.height * sizeof(u32), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  }

  AppRender(&bitmap, fontBitmap);

  CGImageRef imageRef = create_image_from_bytes(&bitmap, scale);
  NSView *view = NSWindow_contentView(win);
  CALayer *layer = objc_msgSend_id(view, sel_registerName("layer"));

  NSRect rect = NSMakeRect(0.0, 0.0, (f32)bitmap.width, (f32)bitmap.height);

  ((void(*)(id, SEL, NSRect))objc_msgSend)(layer,
				sel_registerName("setFrame:"),
				rect);


  id graphicsContext = NSGraphicsContext_graphicsContextWithWindow(win);

  // Get the CGContext from the current NSGraphicsContext
  id cgContext = objc_msgSend_id(graphicsContext, sel_registerName("graphicsPort"));

  // Draw the image in the context
  CGContextDrawImage((void*)cgContext, *(CGRect*)&rect, imageRef);

  // Flush the graphics context to ensure the drawing is displayed
  objc_msgSend_id(graphicsContext, sel_registerName("flushGraphics"));
            
  objc_msgSend_void_id(layer, sel_registerName("setContents:"), (id)imageRef);
  objc_msgSend_id(layer, sel_registerName("setNeedsDisplay"));
            
  CGImageRelease(imageRef);
  NSApplication_updateWindows(NSApp);
}
stbtt_bakedchar cdata[186];
Pepe_Bitmap
LoadDefaultFont(Pepe_Arena *arena)
{
  Pepe_Slice fontMemory;
  u32 bitmapSize = 512 * 512;
  Pepe_Bitmap result;

  fontMemory = Pepe_IO_ReadEntireFileFromPathDebug("../fonts/ProggyVector-Regular.ttf");
  assert(fontMemory.base);
  printf("font readed %llu\n", fontMemory.length);
  result.memory = (u8 *)PEPE_ARENA_ALLOC(arena, bitmapSize);
  result.width = 512;
  result.height = 512;
  result.pitch = 1;
  assert(result.memory);
  i32 status = stbtt_BakeFontBitmap(fontMemory.base, 0, 12.0 * 2.0, (void*)result.memory, 512, 512, 32, 96, cdata);

  printf("stbtt_BakeFontBitmap result = %d\n", status);

  return result; 
}

void
UIHandle(Pepe_Arena *arena)
{ 
  si_func_to_SEL_with_name(SI_DEFAULT, "windowShouldClose", (void *)windowWillClose);
  isRunning = true;

  NSApp = NSApplication_sharedApplication();
  bitmap.width = 0;
  bitmap.height = 0;
  bitmap.pitch = 0;
  bitmap.memory = nil;

  NSApplication_setActivationPolicy(NSApp, NSApplicationActivationPolicyRegular);

  win = NSWindow_init(
      NSMakeRect(200, 200, 1200, 800),
      NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable, NSBackingStoreBuffered,
      false
  );

  NSWindow_setTitle(win, "Basic Example");
  NSWindow_makeKeyAndOrderFront(win, nil);
  NSWindow_setIsVisible(win, true);
  NSWindow_orderFront(win, nil);
  NSApplication_activateIgnoringOtherApps(NSApp, true);
  NSApplication_setActivationPolicy(NSApp, NSApplicationActivationPolicyRegular);
  mouseDown = false;

  NSApplication_finishLaunching(NSApp);
  Pepe_Bitmap fontBitmap = LoadDefaultFont(arena);
  for (;isRunning;) {
    NSEvent* event = NSApplication_nextEventMatchingMask(NSApp, NSEventMaskAny, NSDate_distantFuture(), NSDefaultRunLoopMode, true);
    if (NSEvent_type(event) == NSEventTypeKeyDown && NSEvent_keyCode(event) == 12)
			break;

      NSRect viewFrame = NSWindow_frame(win);
      NSPoint mousePoint = getMouseLocation(win);
      mouseX = (u32)mousePoint.x;
      mouseY = (u32)(viewFrame.size.height - mousePoint.y);
    if (NSEvent_type(event) == NSEventTypeLeftMouseDown) {
      mouseDown = true;
    }
    if (NSEvent_type(event) == NSEventTypeLeftMouseUp) {
      mouseDown = false;
    }
    if (mouseDown) {
      lastMouseDownX = mouseX;
      lastMouseDownY = mouseY;
    }
    NSApplication_sendEvent(NSApp, event);
    NSApplication_updateWindows(NSApp);
    Redraw(fontBitmap);
  }
}
