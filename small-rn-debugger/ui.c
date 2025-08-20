#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void UIHandle(Pepe_Arena *arena) {
  unused(arena);
  Pepe_GLContextDeclaration glConextDeclaration;
  Pepe_GRect viewport;
  Pepe_GLContext glContext;
  RGFW_monitor monitor;

  RGFW_setGLHint(RGFW_glMinor, 3);
  RGFW_setGLHint(RGFW_glMajor, 3);

  viewport = Pepe_GRectMake(0, 0, 800, 600);
	RGFW_window* window = RGFW_createWindow("LearnOpenGL", RGFW_RECT(0, 0, viewport.size.width, viewport.size.height), RGFW_windowAllowDND | RGFW_windowScaleToMonitor);
  if (window == nil)
  {
      printf("Failed to create RGFW window\n");
      return;
  }

  RGFW_window_makeCurrent(window);
  //RGFW_window_swapInterval(window, 60);

  if (RGL_loadGL3((RGLloadfunc)RGFW_getProcAddress))
  {
      printf("Failed to initialize GLAD\n");
      return;
  }

  Pepe_GLContextDeclarationInit(&glConextDeclaration, viewport); 

  if (!Pepe_GLContextInit(arena, &glContext, &glConextDeclaration)) {
    printf("failed to create gl context");
    goto finish;
  }

  monitor = RGFW_window_getMonitor(window);
  Pepe_GLContextSetScale(&glContext, monitor.scaleX, monitor.scaleY);
  glContext.viewport.origin.x = 0.0f;
  glContext.viewport.origin.y = 0.0f;
  glContext.viewport.size.width = window->r.w;
  glContext.viewport.size.height = window->r.h;
  glContext.pixelRatio = monitor.pixelRatio;

  for (;;) { 
    glContext.viewport.size.width = window->r.w;
    glContext.viewport.size.height = window->r.h;
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    Pepe_GLBeginDraw(&glContext);
    Pepe_GLDrawRectInternal(&glContext, 0.0f, 0.0f, 100.0f, 100.0f, 255, 255, 255, 255, nil);
    Pepe_GLEndDraw(&glContext);

    RGFW_window_swapBuffers(window);
    while (RGFW_window_checkEvent(window)) {
      if (window->event.type == RGFW_quit) {
        break;
      }
      if (window->event.type == RGFW_windowResized || window->event.type == RGFW_windowMoved) {
        monitor = RGFW_window_getMonitor(window);
        glContext.pixelRatio = monitor.pixelRatio;
      }
    }
  }

finish:
  RGFW_window_close(window);
}
