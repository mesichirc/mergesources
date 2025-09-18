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
#define COLOR_11 Pepe_ColorMake(0xe5, 0xb5, 0x66, 0xFF)
#define COLOR_11_HIGHLIGHT Pepe_ColorMake(0xf6, 0xc6, 0x77, 0xFF)

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
  CGContextScaleCTM(context, 1.0 / scale, 1.0 / scale);
  CGImageRef image = CGBitmapContextCreateImage(context);

  // Release the color space and context
	CGColorSpaceRelease(colorSpace);
	CGContextRelease(context);

  return image;
}

bool isHot = false;

void 
AppRender(Pepe_Bitmap *bitmap)
{
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
}

void
Redraw(void)
{
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
    bitmap.memory = (u8 *)mmap(nil, bitmap.pitch * bitmap.height, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  }

  // clear
  for (u32 i = 0; i < bitmap.height * bitmap.width; i++) {
    bitmap.memory[i] = BACKGROUND_COLOR.value;
  }

  AppRender(&bitmap);

  CGImageRef imageRef = create_image_from_bytes(&bitmap, scale);
  NSView *view = NSWindow_contentView(win);
  CALayer *layer = objc_msgSend_id(view, sel_registerName("layer"));

  NSRect rect = NSMakeRect(0.0, 0.0, bitmap.width, bitmap.height);

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

void
UIHandle(Pepe_ThreadPool *pool)
{ 
  unused(pool);
  si_func_to_SEL_with_name(SI_DEFAULT, "windowShouldClose", (void *)windowWillClose);
  isRunning = true;

  NSApp = NSApplication_sharedApplication();
  bitmap.width = 0;
  bitmap.height = 0;
  bitmap.pitch = 0;
  bitmap.memory = nil;

  NSApplication_setActivationPolicy(NSApp, NSApplicationActivationPolicyRegular);

  win = NSWindow_init(
      NSMakeRect(200, 200, 640, 480),
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
    Redraw();
  }
}
