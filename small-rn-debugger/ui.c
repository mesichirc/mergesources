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

  RGFW_setGLHint(RGFW_glMinor, 3);
  RGFW_setGLHint(RGFW_glMajor, 3);

  viewport = Pepe_GRectMake(0, 0, 800, 600);
	RGFW_window* window = RGFW_createWindow("LearnOpenGL", RGFW_RECT(viewport.origin.x, viewport.origin.y, viewport.size.width, viewport.size.height), RGFW_windowAllowDND | RGFW_windowScaleToMonitor);
  if (window == NULL)
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

  unused(glContext);
  if (!Pepe_GLContextInit(arena, &glContext, &glConextDeclaration)) {
    goto finish;
  }

  for (;;) {
  }

finish:
  RGFW_window_close(window);
}
