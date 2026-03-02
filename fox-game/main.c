#define RGFW_IMPLEMENTATION
#include <math.h>
#include "../external/RGFW.h"
#include <stdio.h>
#include "../u.h"
#include "../pepe_core.h"
#include "../pepe_graphics.h"
#include "./debug_font.h"
#include "./fox-game.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BYTES_PER_PIXEL 4

u8 *buffer = nil;
u32 windowWidth = WINDOW_WIDTH;
u32 windowHeight = WINDOW_HEIGHT;

FoxKeyCode
rgfwGetKeyCode(u16 keyCode)
{
  switch (keyCode) {
  case RGFW_keyNULL: return FoxKeyCodeNull;
  case RGFW_escape: return FoxKeyCodeEscape;
  case RGFW_backtick: return FoxKeyCodeBacktick;
  case RGFW_0: return FoxKeyCode0;
  case RGFW_1: return FoxKeyCode1;
  case RGFW_2: return FoxKeyCode2;
  case RGFW_3: return FoxKeyCode3;
  case RGFW_4: return FoxKeyCode4;
  case RGFW_5: return FoxKeyCode5;
  case RGFW_6: return FoxKeyCode6;
  case RGFW_7: return FoxKeyCode7;
  case RGFW_8: return FoxKeyCode8;
  case RGFW_9: return FoxKeyCode9;
  case RGFW_minus: return FoxKeyCodeMinus;
  case RGFW_equals: return FoxKeyCodeEquals;
  case RGFW_backSpace: return FoxKeyCodeBackSpace;
  case RGFW_tab: return FoxKeyCodeTab;
  case RGFW_space: return FoxKeyCodeSpace;
  case RGFW_a: return FoxKeyCodeA;
  case RGFW_b: return FoxKeyCodeB;
  case RGFW_c: return FoxKeyCodeC;
  case RGFW_d: return FoxKeyCodeD;
  case RGFW_e: return FoxKeyCodeE;
  case RGFW_f: return FoxKeyCodeF;
  case RGFW_g: return FoxKeyCodeG;
  case RGFW_h: return FoxKeyCodeH;
  case RGFW_i: return FoxKeyCodeI;
  case RGFW_j: return FoxKeyCodeJ;
  case RGFW_k: return FoxKeyCodeK;
  case RGFW_l: return FoxKeyCodeL;
  case RGFW_m: return FoxKeyCodeM;
  case RGFW_n: return FoxKeyCodeN;
  case RGFW_o: return FoxKeyCodeO;
  case RGFW_p: return FoxKeyCodeP;
  case RGFW_q: return FoxKeyCodeQ;
  case RGFW_r: return FoxKeyCodeR;
  case RGFW_s: return FoxKeyCodeS;
  case RGFW_t: return FoxKeyCodeT;
  case RGFW_u: return FoxKeyCodeU;
  case RGFW_v: return FoxKeyCodeV;
  case RGFW_w: return FoxKeyCodeW;
  case RGFW_x: return FoxKeyCodeX;
  case RGFW_y: return FoxKeyCodeY;
  case RGFW_z: return FoxKeyCodeZ;
  case RGFW_period: return FoxKeyCodePeriod;
  case RGFW_comma: return FoxKeyCodeComma;
  case RGFW_slash: return FoxKeyCodeSlash;
  case RGFW_bracket: return FoxKeyCodeBracket;
  case RGFW_closeBracket: return FoxKeyCodeCloseBracket;
  case RGFW_semicolon: return FoxKeyCodeSemicolon;
  case RGFW_apostrophe: return FoxKeyCodeApostrophe;
  case RGFW_backSlash: return FoxKeyCodeBackSlash;
  case RGFW_return: return FoxKeyCodeReturn;
  case RGFW_delete: return FoxKeyCodeDelete;
  case RGFW_F1: return FoxKeyCodeF1;
  case RGFW_F2: return FoxKeyCodeF2;
  case RGFW_F3: return FoxKeyCodeF3;
  case RGFW_F4: return FoxKeyCodeF4;
  case RGFW_F5: return FoxKeyCodeF5;
  case RGFW_F6: return FoxKeyCodeF6;
  case RGFW_F7: return FoxKeyCodeF7;
  case RGFW_F8: return FoxKeyCodeF8;
  case RGFW_F9: return FoxKeyCodeF9;
  case RGFW_F10: return FoxKeyCodeF10;
  case RGFW_F11: return FoxKeyCodeF11;
  case RGFW_F12: return FoxKeyCodeF12;
  case RGFW_F13: return FoxKeyCodeF13;
  case RGFW_F14: return FoxKeyCodeF14;
  case RGFW_F15: return FoxKeyCodeF15;
  case RGFW_F16: return FoxKeyCodeF16;
  case RGFW_F17: return FoxKeyCodeF17;
  case RGFW_F18: return FoxKeyCodeF18;
  case RGFW_F19: return FoxKeyCodeF19;
  case RGFW_F20: return FoxKeyCodeF20;
  case RGFW_F21: return FoxKeyCodeF21;
  case RGFW_F22: return FoxKeyCodeF22;
  case RGFW_F23: return FoxKeyCodeF23;
  case RGFW_F24: return FoxKeyCodeF24;
  case RGFW_F25: return FoxKeyCodeF25;
  case RGFW_capsLock: return FoxKeyCodeCapsLock;
  case RGFW_shiftL: return FoxKeyCodeShiftL;
  case RGFW_controlL: return FoxKeyCodeControlL;
  case RGFW_altL: return FoxKeyCodeAltL;
  case RGFW_superL: return FoxKeyCodeSuperL;
  case RGFW_shiftR: return FoxKeyCodeShiftR;
  case RGFW_controlR: return FoxKeyCodeControlR;
  case RGFW_altR: return FoxKeyCodeAltR;
  case RGFW_superR: return FoxKeyCodeSuperR;
  case RGFW_up: return FoxKeyCodeUp;
  case RGFW_down: return FoxKeyCodeDown;
  case RGFW_left: return FoxKeyCodeLeft;
  case RGFW_right: return FoxKeyCodeRight;
  case RGFW_insert: return FoxKeyCodeInsert;
  case RGFW_menu: return FoxKeyCodeMenu;
  case RGFW_end: return FoxKeyCodeEnd;
  case RGFW_home: return FoxKeyCodeHome;
  case RGFW_pageUp: return FoxKeyCodePageUp;
  case RGFW_pageDown: return FoxKeyCodePageDown;
  case RGFW_numLock: return FoxKeyCodeNumLock;
  case RGFW_kpSlash: return FoxKeyCodeKpSlash;
  case RGFW_kpMultiply: return FoxKeyCodeKpMultiply;
  case RGFW_kpPlus: return FoxKeyCodeKpPlus;
  case RGFW_kpMinus: return FoxKeyCodeKpMinus;
  case RGFW_kpEqual: return FoxKeyCodeKpEqual;
  case RGFW_kp1: return FoxKeyCodeKp1;
  case RGFW_kp2: return FoxKeyCodeKp2;
  case RGFW_kp3: return FoxKeyCodeKp3;
  case RGFW_kp4: return FoxKeyCodeKp4;
  case RGFW_kp5: return FoxKeyCodeKp5;
  case RGFW_kp6: return FoxKeyCodeKp6;
  case RGFW_kp7: return FoxKeyCodeKp7;
  case RGFW_kp8: return FoxKeyCodeKp8;
  case RGFW_kp9: return FoxKeyCodeKp9;
  case RGFW_kp0: return FoxKeyCodeKp0;
  case RGFW_kpPeriod: return FoxKeyCodeKpPeriod;
  case RGFW_kpReturn: return FoxKeyCodeKpReturn;
  case RGFW_scrollLock: return FoxKeyCodeScrollLock;
  case RGFW_printScreen: return FoxKeyCodePrintScreen;
  case RGFW_pause: return FoxKeyCodePause;
  case RGFW_world1: return FoxKeyCodeWorld1;
  case RGFW_world2: return FoxKeyCodeWorld2;
  case RGFW_keyLast: return FoxKeyCodeLast;
  }
}

#include <time.h>

static f64
currenttimems(void)
{
  struct timespec tp;
  clockid_t clk_id;

  clk_id = CLOCK_REALTIME;
  clock_gettime(clk_id, &tp);

  return (f64)(tp.tv_sec * 1000000000 + tp.tv_nsec) / 1000000;
}

static f64
monotonictimens(void)
{
  struct timespec tp;
  clockid_t clk_id;

  clk_id = CLOCK_BOOTTIME;
  clock_gettime(clk_id, &tp);

  return (f64)tp.tv_sec * 1000000000 + (f64)tp.tv_nsec;
}

static void
sleep_nanosec(u64 nanosecs)
{
  struct timespec ts;
  ts.tv_sec = nanosecs / 1000000000;
  ts.tv_nsec = nanosecs % 1000000000; 
  nanosleep(&ts, nil);
}

int main() {
  u32 windowWidth = WINDOW_WIDTH;
  u32 windowHeight = WINDOW_HEIGHT;
  RGFW_window *win = RGFW_createWindow("a window", 0, 0, windowWidth, windowHeight, RGFW_windowCenter | RGFW_windowNoResize);
  FoxInput foxInputs[2];
  memset(foxInputs, 0, sizeof(foxInputs));
  u32 prevInput = 0;
  u32 input = 1;
  FoxInputHandle inputHandle;

  RGFW_window_getSize(win, &windowWidth, &windowHeight);
  u8 *buffer = (u8*)RGFW_ALLOC(windowWidth * windowHeight * 4);
  RGFW_surface *surface = RGFW_createSurface(buffer, windowWidth, windowHeight, RGFW_formatRGBA8);
  RGFW_monitor monitor = RGFW_window_getMonitor(win);
  f64 target_fps = 60.0;
  f64 ns_per_frame = 1000000000.0 / target_fps;

  Foxy_Init(currenttimems(), (f64)monitor.mode.refreshRate, target_fps);
  f64 fps = 60.0;

  while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
    f64 startframe = monotonictimens();
    RGFW_event event;

    inputHandle.newInput = foxInputs + input;
    inputHandle.prevInput = foxInputs + prevInput;
    prevInput = input;
    input = (input + 1) & 1;
        
    while (RGFW_window_checkEvent(win, &event)) {  // or RGFW_pollEvents(); if you only want callbacks
      // you can either check the current event yourself
      if (event.type == RGFW_quit) break;

      i32 mouseX, mouseY;
      RGFW_window_getMouse(win, &mouseX, &mouseY);
      if (event.type == RGFW_windowResized) {
        RGFW_window_getSize(win, &windowWidth, &windowHeight);
        RGFW_FREE(buffer);
        buffer = (u8*)RGFW_ALLOC(windowWidth * windowHeight * 4);
        RGFW_surface_free(surface);
        surface = RGFW_createSurface(buffer, windowWidth, windowHeight, RGFW_formatRGBA8);
      }
    }
    FoxUpdateAndRender((u8*)buffer, windowWidth, windowHeight, inputHandle, currenttimems(), fps);
    f64 elapsedns = monotonictimens() - startframe;
    assert(elapsedns <= ns_per_frame);
    while (ns_per_frame > elapsedns) {
      sleep_nanosec(ns_per_frame - elapsedns);
      elapsedns = monotonictimens() - startframe;
    }
    
    fps = 1000000000.0 / elapsedns;
    RGFW_window_blitSurface(win, surface);
  }

  RGFW_window_close(win);
  return 0;
}
