#include <stdint.h> // stdint's
#include <stdbool.h> // bool
#include <sys/mman.h> // for mmap
#include <assert.h> // for assert
#include <stdio.h> // printf
#include <string.h> // memset
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <wait.h>
#include "../u.h"
#include "../pepe_core.h"
#include "../pepe_memcmp.h"
#include "../pepe_encoding.h"
#include "../pepe_http.h"
#define RGFW_IMPLEMENTATION
#include "../external/RGFW.h"

#define BUF_SIZE 131072

bool
UpgradeToWebsocketProtocol(Pepe_HttpRequest *request, Pepe_HttpResponse *response)
{
  Pepe_String wsKeyHeader;
  Pepe_String wsResponseKey;

  if (!Pepe_HttpHeadersHasValueCString(&request->headers, "Upgrade", "websocket")) {
    return false;
  }
  if (!Pepe_HttpHeadersHasValueCString(&request->headers, "Connection", "Upgrade")) {
    return false;
  }

  wsKeyHeader = Pepe_HttpHeadersGet(&request->headers, Pepe_StringFromCString("Sec-Websocket-Key"));
  if (wsKeyHeader.length == 0) {
    return false;
  }

  wsResponseKey = Pepe_WebsocketSecWebsocketAcceptHeader(response->arena, wsKeyHeader);

  Pepe_HttpResponseWriteCString(response, "HTTP/1.1 101 Switching Protocols\r\n"); 
  Pepe_HttpResponseWriteCString(response, "Upgrade: websocket\r\n");
  Pepe_HttpResponseWriteCString(response, "Connection: Upgrade\r\n");
  Pepe_HttpResponseWriteCString(response, "Sec-WebSocket-Accept: ");
  Pepe_HttpResponseWrite(response, wsResponseKey);
  Pepe_HttpResponseWriteCString(response, "\r\n\r\n"); 

  return true;
}

void
WebsocketClose(Pepe_HttpResponse *response, u16 status)
{
  Pepe_String msg;
  byte buf[4];
  buf[0] = 0;

  buf[0] |= 8;
  buf[0] |= 0x80;
  buf[1] = 2;

  printf("closing connection\n");

  status = PEPE_IS_BE() ? status : Pepe_U16SwapBytes(status);
  *(u16 *)(buf + 2) = status;
  
  msg.base = buf;
  msg.length = 4;

  Pepe_HttpResponseWrite(response, msg);
  Pepe_HttpResponseClose(response);
}

void
HandleWebsocketMessage(Pepe_HttpRequest *request, Pepe_HttpResponse *response)
{
  u8 fin;
  u8 isMasked;
  u8 opcode;
  u64 size;
  u64 extra;
  byte *mask;
  Pepe_Slice memory;
  Pepe_Slice buf;
  Pepe_Slice payloadBuf;
  Pepe_Slice messageBuf;
  i64 rcvd;
  i32 fptr;
  
  fptr = open("logs", O_APPEND | O_CREAT | O_RDWR);
  memory = Pepe_SliceInit(PEPE_ARENA_ALLOC(response->arena, 14), 14);
  messageBuf = Pepe_SliceInit(PEPE_ARENA_ALLOC(response->arena, 1024), 1024);
  messageBuf.length = 0;

  for (;;) {
    memset(memory.base, 0, memory.length);
    extra = 0;
    fin = 0;
    isMasked = 0;
    opcode = 0;

    buf = Pepe_SliceLeft(memory, 2);
    if (Pepe_ReadFromConnection(request, buf) != (i64)buf.length) {
      perror("recv() call failed0");
      break;
    }
    fin = ((u8*)buf.base)[0] >> 7;
    opcode = ((u8*)buf.base)[0] & 0xf;
    isMasked = ((u8*)buf.base)[1] >> 7;
    
    if (isMasked == 1) {
      extra += 4;
    }
    size = (u64)(((u8*)buf.base)[1] & 0x7f);
    if (size == 126) {
      extra += 2;
    } else if (size == 127) {
      extra += 8;
    }

    if (extra > 0) {
      buf = Pepe_SliceLeft(Pepe_SliceRight(memory, 2), extra);
      rcvd = Pepe_ReadFromConnection(request, buf);
      if (rcvd != (i64)buf.length) {

        printf("recv() call failed1 %ld\n", rcvd);
        break; 
      }
      if (size == 126) {
        size = (u64)(*(u16 *)buf.base);
        size = PEPE_IS_BE() ? size : Pepe_U16SwapBytes(size);
        buf = Pepe_SliceRight(buf, 2);
      } else if (size == 127) {
        size = *(u64 *)buf.base;
        size = PEPE_IS_BE() ? size : Pepe_U64SwapBytes(size);
        buf = Pepe_SliceRight(buf, 8);
      }
    }

    mask = (u8 *)buf.base;

    if (size + messageBuf.length > messageBuf.capacity) {
      u64 oldCapacity;

      oldCapacity = messageBuf.capacity;
      while (payloadBuf.length + messageBuf.length > messageBuf.capacity) {
        messageBuf.capacity *= 2;
      }

      messageBuf.base = PEPE_ARENA_RESIZE(response->arena, messageBuf.base, oldCapacity, messageBuf.capacity);
    } 
 
    payloadBuf.base = (u8*)messageBuf.base + messageBuf.length;
    payloadBuf.length = size;
    payloadBuf.capacity = size;
    rcvd = Pepe_ReadFromConnection(request, payloadBuf);
    if (rcvd != (i64)payloadBuf.length) {
      perror("recv() call failed2");
      break;
    }
    messageBuf.length += size;

    if (isMasked) {
      u64 i;
      byte *payload;

      payload = (byte *)payloadBuf.base;
      for (i = 0; i < payloadBuf.length; i++) {
        payload[i] ^= mask[i%4];
      }
    }

    if (opcode == 8) {
      break;
    } else if (fin == 1) {
      write(fptr, (u8 *)messageBuf.base, (u32)messageBuf.length);
      printf("message: %.*s", (u32)messageBuf.length, (u8 *)messageBuf.base);
      fsync(fptr);
      messageBuf.length = 0;
    }  
  }
  close(fptr);
}

void
HandleHttpRequest(Pepe_HttpRequest *request, Pepe_HttpResponse *response, void *userdata)
{
  unused(userdata);
  Pepe_HttpRequestPrint(request);

  if (UpgradeToWebsocketProtocol(request, response)) {
    printf("Upgrade to websocket success\n");
    HandleWebsocketMessage(request, response);
    WebsocketClose(response, 1000);
  } else {
    printf("Upgrade to websocket failed\n");
    WebsocketClose(response, 1000);
  }
}

void keyfunc(RGFW_window* win, u8 key, u8 keyChar, u8 keyMod, u8 pressed) {
    unused(keyChar);
    unused(keyMod);
    if (key == RGFW_escape && pressed) {
        RGFW_window_setShouldClose(win, true);
    }
}

void
UIHandle(void) {
  RGFW_window* win = RGFW_createWindow("a window", RGFW_RECT(0, 0, 800, 600), RGFW_windowCenter | RGFW_windowNoResize);

  RGFW_setKeyCallback(keyfunc); // you can use callbacks like this if you want

  while (RGFW_window_shouldClose(win) == false) {
      while (RGFW_window_checkEvent(win)) {  // or RGFW_window_checkEvents(); if you only want callbacks
          // you can either check the current event yourself
          if (win->event.type == RGFW_quit) break;
          
          if (win->event.type == RGFW_mouseButtonPressed && win->event.button == RGFW_mouseLeft) {
              printf("You clicked at x: %d, y: %d\n", win->event.point.x, win->event.point.y);
          }

          // or use the existing functions
          if (RGFW_isMousePressed(win, RGFW_mouseRight)) {
              printf("The right mouse button was clicked at x: %d, y: %d\n", win->event.point.x, win->event.point.y);
          }
      }
      
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // You can use modern OpenGL techniques, but this method is more straightforward for drawing just one triangle.
      glBegin(GL_TRIANGLES);
      glColor3f(1, 0, 0); glVertex2f(-0.6, -0.75);
      glColor3f(0, 1, 0); glVertex2f(0.6, -0.75);
      glColor3f(0, 0, 1); glVertex2f(0, 0.75);
      glEnd();

      RGFW_window_swapBuffers(win);
  }
  printf("window closing\n");

  RGFW_window_close(win);
}

i32
main(i32 argc, char **argv)
{
  unused(argc);
  unused(argv);
  u64 requredMemory;
  Pepe_Slice arenaMemory;
  Pepe_Arena arena;
  Pepe_HttpHandler handler;
  Pepe_HttpWorkers httpWorkers;
  // i32 pidIndex;
  i32 workersData[4]; 
  httpWorkers.length = 4;
  httpWorkers.data = workersData;
  requredMemory = BUF_SIZE;
  arenaMemory = Pepe_SliceInit(mmap(nil, requredMemory, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0), requredMemory);

  Pepe_ArenaInit(&arena, arenaMemory);
  handler.userdata = nil;
  handler.handle = HandleHttpRequest;

  Pepe_HttpListenAndServe(arena, handler, 8080, httpWorkers);
  UIHandle(); 
  for (u32 i = 0; i < httpWorkers.length; i++) {
    kill(httpWorkers.data[i], SIGTERM);
  }
}
