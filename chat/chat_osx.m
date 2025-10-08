#import <AppKit/AppKit.h>
#import <IOKit/hid/IOHIDLib.h>
#include <mach/mach_init.h>
#include <mach/mach_time.h>
#include "../u.h"
#include "../pepe_graphics.h"
#include "chat.h"
#include "chat.c"
#include <stdint.h>

mach_timebase_info_data_t globalPerfCountFrequency;
int bytesPerPixel = 4;
bool running = true;

void 
macosRedrawBuffer(NSWindow *window, Pepe_Bitmap *bitmap)
{
  @autoreleasepool {
    u8* plane = bitmap->memory;
    NSBitmapImageRep *rep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes: &plane 
                                  pixelsWide: bitmap->width
                                  pixelsHigh: bitmap->height
                                  bitsPerSample: 8
                                  samplesPerPixel: 4
                                  hasAlpha: YES
                                  isPlanar: NO
                                  colorSpaceName: NSDeviceRGBColorSpace
                                  bytesPerRow: bitmap->pitch
                                  bitsPerPixel: bytesPerPixel * 8] autorelease];
    NSSize imageSize = NSMakeSize(bitmap->width, bitmap->height);
    NSImage *image = [[[NSImage alloc] initWithSize: imageSize] autorelease];
    [image addRepresentation: rep];
    window.contentView.layer.contents = image;
  }
}

@interface ChatMainWindowDelegate: NSObject<NSWindowDelegate>
@end

@implementation ChatMainWindowDelegate 
- (void)windowWillClose:(id)sender {
    running = false;  
}
@end

@interface ChatMainWindow : NSWindow
@end

@implementation ChatMainWindow 
@end

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

void
ChatInputClear(ChatInput *input)
{
  // only drop flag and characters length 
  input->flag = 0;
  input->charactersLength = 0;
}

ChatKeyCode
macosGetKeyCode(u16 keyCode)
{
  switch (keyCode) {
  case 0x1D: return ChatKeyCode0;
	case 0x12: return ChatKeyCode1;
	case 0x13: return ChatKeyCode2;
	case 0x14: return ChatKeyCode3;
	case 0x15: return ChatKeyCode4;
	case 0x17: return ChatKeyCode5;
	case 0x16: return ChatKeyCode6;
	case 0x1A: return ChatKeyCode7;
	case 0x1C: return ChatKeyCode8;
	case 0x19: return ChatKeyCode9;
	case 0x00: return ChatKeyCodeA;
	case 0x0B: return ChatKeyCodeB;
	case 0x08: return ChatKeyCodeC;
	case 0x02: return ChatKeyCodeD;
	case 0x0E: return ChatKeyCodeE;
	case 0x03: return ChatKeyCodeF;
	case 0x05: return ChatKeyCodeG;
	case 0x04: return ChatKeyCodeH;
	case 0x22: return ChatKeyCodeI;
	case 0x26: return ChatKeyCodeJ;
	case 0x28: return ChatKeyCodeK;
	case 0x25: return ChatKeyCodeL;
	case 0x2E: return ChatKeyCodeM;
	case 0x2D: return ChatKeyCodeN;
	case 0x1F: return ChatKeyCodeO;
	case 0x23: return ChatKeyCodeP;
	case 0x0C: return ChatKeyCodeQ;
	case 0x0F: return ChatKeyCodeR;
	case 0x01: return ChatKeyCodeS;
	case 0x11: return ChatKeyCodeT;
	case 0x20: return ChatKeyCodeU;
	case 0x09: return ChatKeyCodeV;
	case 0x0D: return ChatKeyCodeW;
	case 0x07: return ChatKeyCodeX;
	case 0x10: return ChatKeyCodeY;
	case 0x06: return ChatKeyCodeZ;
	case 0x27: return ChatKeyCodeApostrophe;
	case 0x2A: return ChatKeyCodeBackSlash;
	case 0x2B: return ChatKeyCodeComma;
	case 0x18: return ChatKeyCodeEquals;
	case 0x32: return ChatKeyCodeBacktick;
	case 0x21: return ChatKeyCodeBracket;
	case 0x1B: return ChatKeyCodeMinus;
	case 0x2F: return ChatKeyCodePeriod;
	case 0x1E: return ChatKeyCodeCloseBracket;
	case 0x29: return ChatKeyCodeSemicolon;
	case 0x2C: return ChatKeyCodeSlash;
	case 0x0A: return ChatKeyCodeWorld1;
	case 0x33: return ChatKeyCodeBackSpace;
	case 0x39: return ChatKeyCodeCapsLock;
	case 0x75: return ChatKeyCodeDelete;
	case 0x7D: return ChatKeyCodeDown;
	case 0x77: return ChatKeyCodeEnd;
	case 0x24: return ChatKeyCodeEnter;
	case 0x35: return ChatKeyCodeEscape;
	case 0x7A: return ChatKeyCodeF1;
	case 0x78: return ChatKeyCodeF2;
	case 0x63: return ChatKeyCodeF3;
	case 0x76: return ChatKeyCodeF4;
	case 0x60: return ChatKeyCodeF5;
	case 0x61: return ChatKeyCodeF6;
	case 0x62: return ChatKeyCodeF7;
	case 0x64: return ChatKeyCodeF8;
	case 0x65: return ChatKeyCodeF9;
	case 0x6D: return ChatKeyCodeF10;
	case 0x67: return ChatKeyCodeF11;
	case 0x6F: return ChatKeyCodeF12;
	case 0x69: return ChatKeyCodePrintScreen;
	case 0x6B: return ChatKeyCodeF14;
	case 0x71: return ChatKeyCodeF15;
	case 0x6A: return ChatKeyCodeF16;
	case 0x40: return ChatKeyCodeF17;
	case 0x4F: return ChatKeyCodeF18;
	case 0x50: return ChatKeyCodeF19;
	case 0x5A: return ChatKeyCodeF20;
	case 0x73: return ChatKeyCodeHome;
	case 0x72: return ChatKeyCodeInsert;
	case 0x7B: return ChatKeyCodeLeft;
	case 0x3A: return ChatKeyCodeAltL;
	case 0x3B: return ChatKeyCodeControlL;
	case 0x38: return ChatKeyCodeShiftL;
	case 0x37: return ChatKeyCodeSuperL;
	case 0x6E: return ChatKeyCodeMenu;
	case 0x47: return ChatKeyCodeNumLock;
	case 0x79: return ChatKeyCodePageDown;
	case 0x74: return ChatKeyCodePageUp;
	case 0x7C: return ChatKeyCodeRight;
	case 0x3D: return ChatKeyCodeAltR;
	case 0x3E: return ChatKeyCodeControlR;
	case 0x3C: return ChatKeyCodeShiftR;
	case 0x36: return ChatKeyCodeSuperR;
	case 0x31: return ChatKeyCodeSpace;
	case 0x30: return ChatKeyCodeTab;
	case 0x7E: return ChatKeyCodeUp;
	case 0x52: return ChatKeyCodeKp0;
	case 0x53: return ChatKeyCodeKp1;
	case 0x54: return ChatKeyCodeKp2;
	case 0x55: return ChatKeyCodeKp3;
	case 0x56: return ChatKeyCodeKp4;
	case 0x57: return ChatKeyCodeKp5;
	case 0x58: return ChatKeyCodeKp6;
	case 0x59: return ChatKeyCodeKp7;
	case 0x5B: return ChatKeyCodeKp8;
	case 0x5C: return ChatKeyCodeKp9;
	case 0x45: return ChatKeyCodeKpSlash;
	case 0x41: return ChatKeyCodeKpPeriod;
	case 0x4B: return ChatKeyCodeKpSlash;
	case 0x4C: return ChatKeyCodeKpReturn;
	case 0x51: return ChatKeyCodeKpEqual;
	case 0x43: return ChatKeyCodeKpMultiply;
	case 0x4E: return ChatKeyCodeKpMinus;
  default: return ChatKeyCodeNull;
  }
}

int 
main(int argc, const char * argv[])
{
  unused(argc);
  unused(argv);
  ChatInputKeyCodesDebugInit();
  mach_timebase_info(&globalPerfCountFrequency);
  ChatMainWindowDelegate *mainWindowDelegate = [[ChatMainWindowDelegate alloc] init];

  // NSRect screenRect = [[NSScreen mainScreen] frame];

  ChatInput chatInputs[2];
  memset(chatInputs, 0, sizeof(ChatInput) * 2);
  u32 prevInput = 0;
  u32 input = 1;

  f32 width = 1024;
  f32 height = 768;
  NSRect initialFrame = NSMakeRect(0, 0, width, height);
  NSWindow *window = [[ChatMainWindow alloc]
                      initWithContentRect: initialFrame
                      styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                      backing: NSBackingStoreBuffered
                      defer:NO];

  [[NSApplication sharedApplication] activateIgnoringOtherApps:true];
  [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyRegular];
  [window setBackgroundColor: NSColor.blackColor];
  [window setTitle: @"Chatto"];
  [window makeKeyAndOrderFront: nil];
  [window setIsVisible: true];
  [window orderFront:nil];
  [window setDelegate: mainWindowDelegate];
  [[NSApplication sharedApplication] finishLaunching];
  // 1 frame keycodes [0, 0, 1]
  // 2 frame keycodes [0, 0, 0]
  // 3 frame keycodes [0, 0, 1]
  ChatInput *chatInput;
  ChatInputHandle inputHandle;
  window.contentView.wantsLayer = YES;
  for (;running;) {
        inputHandle.newInput = chatInputs + input;
        inputHandle.prevInput = chatInputs + prevInput;
        ChatUpdateAndRender(nil, inputHandle);
        prevInput = input;
        input = (input + 1) & 1;
        chatInput = chatInputs + input;
        memcpy(&chatInput->keyCodes, (chatInputs + prevInput)->keyCodes, sizeof(chatInput->keyCodes));
        ChatInputClear(chatInput);
        NSEvent* event;
        do {
            event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                       untilDate: nil
                                          inMode: NSDefaultRunLoopMode
                                         dequeue: YES];
           
            switch ([event type]) {
              case NSEventTypeKeyDown:
                chatInput->flag |= ChatKeyPressedFlag; 
                chatInput->keyCodes[macosGetKeyCode(event.keyCode)] = 1;
                assert(chatInput->charactersLength + event.characters.cStringLength < ARRAY_LENGTH(chatInput->characters));
                memcpy(chatInput->characters + chatInput->charactersLength, event.characters.UTF8String, strlen(event.characters.UTF8String)); 
                chatInput->charactersLength += strlen(event.characters.UTF8String);
                break;
              case NSEventTypeKeyUp:
                chatInput->flag |= ChatKeyReleasedFlag;
                chatInput->keyCodes[macosGetKeyCode(event.keyCode)] = 0;
                printf("key released = %d\n", macosGetKeyCode(event.keyCode));
                break;
              case NSEventTypeLeftMouseDown:
                chatInput->flag |= ChatMouseBtnPressedFlag;
                chatInput->mouseButtonsState[0] = 1;
                break;
              case NSEventTypeLeftMouseUp:
                chatInput->flag |= ChatMouseBtnReleasedFlag;
                chatInput->mouseButtonsState[0] = 0;
                break;
              case NSEventTypeRightMouseDown:
                chatInput->flag |= ChatMouseBtnPressedFlag;
                chatInput->mouseButtonsState[1] = 1;
                break;
              case NSEventTypeRightMouseUp:
                chatInput->flag |= ChatMouseBtnReleasedFlag;
                chatInput->mouseButtonsState[1] = 0;
                break;
              case NSEventTypeOtherMouseDown:
                chatInput->flag |= ChatMouseBtnPressedFlag;
                chatInput->mouseButtonsState[event.buttonNumber] = 1;
                break;
              case NSEventTypeOtherMouseUp:
                chatInput->flag |= ChatMouseBtnReleasedFlag;
                chatInput->mouseButtonsState[event.buttonNumber] = 0;
                break;
              case NSEventTypeScrollWheel:
                chatInput->flag |= ChatMouseScrollFlag;
                chatInput->deltaX = event.deltaX;
                chatInput->deltaY = event.deltaY;
                break;
              case NSEventTypeMouseMoved:
                chatInput->flag |= ChatMousePositionChangedFlag;
                chatInput->mouseX = (int)(window.mouseLocationOutsideOfEventStream.x + 0.5);
                chatInput->mouseY = (int)(window.frame.size.height - window.mouseLocationOutsideOfEventStream.y + 0.5);
                break;
              default:
                break;
            }

            switch ([event type]) {
              default:         
                [NSApp sendEvent: event];
            }
        } while (event != nil);
  }
}

void 
ChatInputPrint(ChatInput *input)
{
  if (input->flag == 0) {
    return;
  }
  printf("ChatInput:\n");
  if (input->charactersLength > 0) {
    printf("characters= %.*s\n", input->charactersLength, (const char *)input->characters);
  }
  if (input->flag & ChatMousePositionChangedFlag) {
    printf("mouseX= %d, mouseY= %d\n", input->mouseX, input->mouseY);
  }
  for (int i = 0; i < (int)ARRAY_LENGTH(input->mouseButtonsState); i++) {
    if (input->mouseButtonsState[i]) {
      if (i == 0) {
        printf("left mouseButton %d pressed\n", i);
      } else if (i == 1) {
        printf("right mouseButton %d pressed\n", i);
      } else {
        printf("right mouseButton %d pressed\n", i);
      }
    }
  }
  if (input->flag & ChatMouseScrollFlag) {
    printf("deltaX = %f, deltaY = %f\n", input->deltaY, input->deltaY);
  }
  if (input->flag & ChatKeyPressedFlag) {
    printf("ChatKeyPressed");
  }
  if (input->flag & ChatKeyReleasedFlag) {
    printf("ChatKeyReleased\n");
  }
  if (input->flag & ChatMouseScrollFlag) {
    printf("ChatMouseScroll\n");
  }
  if (input->flag & ChatMouseBtnPressedFlag) {
    printf("ChatMouseBtnPressed\n");
  }
  if (input->flag & ChatMouseBtnReleasedFlag) {
    printf("ChatMouseBtnReleased\n");
  }
  if (input->flag & ChatMousePositionChangedFlag) {
    printf("ChatMousePositionChanged\n");
  }

  for (int i = 0; i < (int)ARRAY_LENGTH(input->keyCodes); i++) {
    if (input->keyCodes[i]) {
      printf("ChatKeyCodePressed %s = %d\n", keyCodesStrings[i], i);
    }
  }
}
