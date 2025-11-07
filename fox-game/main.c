#define RGFW_IMPLEMENTATION
#include "../external/RGFW.h"
#include <stdio.h>
#include "../u.h"
#include "../pepe_core.h"

void keyfunc(RGFW_window* win, RGFW_key key, u8 keyChar, RGFW_keymod keyMod, RGFW_bool repeat, RGFW_bool pressed) {
    RGFW_UNUSED(repeat);
    if (key == RGFW_escape && pressed) {
        RGFW_window_setShouldClose(win, 1);
    }
}

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BYTES_PER_PIXEL 4
#define BUFFER_SIZE_INT (WINDOW_WIDTH * WINDOW_HEIGHT)
#define BUFFER_SIZE (BUFFER_SIZE_INT * BYTES_PER_PIXEL)

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

/**
 * Colors defined in Big Endian for sake of simplicity,
 * so if we running on Little Endian we need to check and convert all colors
 */
void
initColors(void)
{
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
}

/**
 * color must be converted to right byte order
 **/
void
Foxy_ClearBitmap(u32 color, u8 *buffer, u32 width, u32 height)
{
  u32 *intBuf = (u32 *)buffer;
  u32 bufferSize = width * height;
  for (int i = 0; i < bufferSize; i += 1) {
    intBuf[i] = color;
  }
}

int main() {
  initColors();
    RGFW_window *win = RGFW_createWindow("a window", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, RGFW_windowCenter | RGFW_windowNoResize);

    RGFW_setKeyCallback(keyfunc); // you can use callbacks like this if you want RGBA <-> ABGR

    u8 *buffer = (u8*)RGFW_ALLOC(BUFFER_SIZE);
    RGFW_surface *surface = RGFW_createSurface(buffer, WINDOW_WIDTH, WINDOW_HEIGHT, RGFW_formatRGBA8);

    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        RGFW_event event;
        
        Foxy_ClearBitmap(Foxy_Colors[Foxy_ColorText], buffer, WINDOW_WIDTH, WINDOW_HEIGHT);
        while (RGFW_window_checkEvent(win, &event)) {  // or RGFW_pollEvents(); if you only want callbacks
            // you can either check the current event yourself
            if (event.type == RGFW_quit) break;

            i32 mouseX, mouseY;
            RGFW_window_getMouse(win, &mouseX, &mouseY);

            if (event.type == RGFW_mouseButtonPressed && event.button.value == RGFW_mouseLeft) {
                printf("You clicked at x: %d, y: %d\n", mouseX, mouseY);
            }

            // or use the existing functions
            if (RGFW_isMousePressed(RGFW_mouseRight)) {
                printf("The right mouse button was clicked at x: %d, y: %d\n", mouseX, mouseY);
            }
        }

        RGFW_window_blitSurface(win, surface);
    }

    RGFW_window_close(win);
    return 0;
}
