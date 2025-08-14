#ifndef PEPE_GL_H
#define PEPE_GL_H

typedef enum {
  PEPE_GL_ERROR_NONE = 0,
  PEPE_GL_ERROR_DEFAULT_SHADER_NOT_COMPILED,
} Pepe_GLErrorType;

typedef struct Pepe_GLImage Pepe_GLImage;
struct Pepe_GLImage {
  byte               *data; 
  u32                width;
  u32                height;
  u32                pitch;
};

typedef struct Pepe_GLDrawCall Pepe_GLDrawCall;
struct Pepe_GLDrawCall {
  u32 start;
  u32 end;
  u32 textureID;
  u32 shaderID;
};

typedef struct Pepe_GLIndiciesArray Pepe_GLIndiciesArray;
struct Pepe_GLIndiciesArray {
  u32 *index;
  u32 length;
};

typedef struct Pepe_GLDrawCallArray Pepe_GLDrawCallArray;
struct Pepe_GLDrawCallArray {
  Pepe_GLDrawCall *data;
  u32             length;
};

typedef struct Pepe_GLShaderLocations Pepe_GLShaderLocations;
struct Pepe_GLShaderLocations {
  i32 position;
  i32 textureCoordinates;
  i32 color;
  i32 mpv;
};

typedef struct Pepe_GLVertexes Pepe_GLVertexes;
struct Pepe_GLVertexes {
  f32 *positions;
  f32 *textureCoordinates;
  u8  *colors;
  u32 length;
  u32 index;
};

typedef struct Pepe_GLContextDeclaration Pepe_GLContextDeclaration;
struct Pepe_GLContextDeclaration {
  Pepe_GRect viewport;
  Pepe_Arena *arena;
  u32        vertexesCount;
  u32        drawcallsCount;
  u32        texturesCount;
};

void
Pepe_GLContextDeclarationInit(
    Pepe_GLContextDeclaration *declaration,
    Pepe_GLRect viewport,
    Pepe_Arena  *arena
)
{
  assert(declaration);
  assert(arena);

  declaration->arena = arena;
  declaration->viewport = viewport;
  declaration->vertexesCount = 1024;
  declaration->drawcallsCount = 16;
  declaration->textursCount = 256;
}

typedef struct Pepe_GLContext Pepe_GLContext;
struct Pepe_GLContext {
  Pepe_GLDrawCallArray    drawcalls;
  Pepe_GLVertexes         vertexes;
  Pepe_GLIndiciesArray    indicies;
  u32                     VAO;
  u32                     defaultTextureID;
  Pepe_GLShaderLocations  defaultShaderLocations;
  u32                     defaultShaderID;
  u32                     VBO;
};


char* PEPE_GLDefaultVertexShader =
  "#version 330 core                                  \n"
  "in vec3 vertexPosition;                            \n"
  "in vec2 vertexTexCoord;                            \n"
  "in vec4 vertexColor;                               \n"
  "out vec2 fragTexCoord;                             \n"
  "out vec4 fragColor;                                \n"
  "uniform mat4 mvp;                                  \n"
  "void main()                                        \n"
  "{                                                  \n"
  "   fragTexCoord  = vertexTexCoord;                 \n"
  "   fragColor     = vertexColor;                    \n"
  "   gl_Position   = mvp*vec4(vertexPosition, 1.0);  \n" 
  "}                                                  \n";


char* PEPE_GLDefaultFragmentShader =
  "#version 330 core                                      \n"
  "in vec2 fragTexCoord;                                  \n"
  "in vec4 fragColor;                                     \n"
  "out vec4 finalColor;                                   \n"
  "uniform sampler2D texture0;                            \n"
  "void main()                                            \n"
  "{                                                      \n"
  "   vec4 texelColor = texture(texture0, fragTexCoord);  \n"
  "   finalColor      = texelColor*fragColor;             \n"
  "}                                                      \n";


bool
Pepe_GLCompileShader(Pepe_String source, i32 type, u32 *shader)
{
  i32 success;
  i32 len;
  char errorBuffer[1024];

  if (shader == nil) {
    printf("could not compile shader need to provide valid pointer to shader id");
    return 0;
  }

  len = (i32)source.length;
  *shader = glShaderCreate(type);
  glShaderSource(*shader, 1, (const char* const*)&source.base, &len);
  glCompileShader(*shader);

  glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    len = 0;
    glGetShaderInfoLog(*shader, 1024, &len, errorBuffer);
    printf("could not compile shader %.*s", len, errorBuffer);
  }

  return (bool)success;
}

bool
Pepe_GLLinkShader(u32 vertexShader, u32 fragmentShader, u32 *program)
{
  i32 success;
  i32 logsize;
  char errorBuffer[1024];

  if (program == nil) {
    printf("could not link shader need to provide valid pointer to program id");
    return 0;
  }
 
  *program = glCreateProgram();
  glAttachShader(*program, vertexShader);
  glAttachShader(*program, fragmentShader);
  glLinkProgram(*program);

  glGetProgramiv(*program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(*program, sizeof(errorBuffer), &logsize, errorBuffer);
    printf("could not link shader %.*s", logsize, errorBuffer);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return success;
}

Pepe_GLErrorType
Pepe_GLBuildDefaultShader(u32 *program)
{
  bool success;
  u32 vertexShader;
  u32 fragmentShader;
  if (!program) {
    printf("could not build default shader because program id is nil");
    return PEPE_GL_ERROR_DEFAULT_SHADER_NOT_COMPILED;
  }

  vertexShader = 0;
  fragmentShader = 0;
  success = false;

  success = Pepe_GLCompileShader(Pepe_StringFromCString(PEPE_GLDefaultVertexShader), GL_VERTEX_SHADER, &vertexShader);
  success = Pepe_GLCompileShader(Pepe_StringFromCString(PEPE_GLDefaultFragmentShader), GL_FRAGMENT_SHADER, &fragmentShader);

  if (!success) {
    return success;
  }

  return Pepe_GLLinkShader(vertexShader, fragmentShader, *program); 
}

u32
Pepe_GLLoadTexture(Pepe_GLImage image)
{
  u32 id;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glPixelStorei(GL_UNPACK_ALIGMENT, 1);
  glGenTextures(GL_TEXTURE_2D, &id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

  glBindTexture(GL_TEXTURE_2D, 0);

  return id;
}


bool
Pepe_GLContextInit(Pepe_GLContext *context, Pepe_GLContextDeclaration *declaration)
{
  assert(declaration);
  u32 indiciesSize;
  u32 textureCoordinatesSize;
  u32 positionsSize;
  u32 colorsSize;
  u8  whitepixels[4];
  Pepe_GLImage whitepixelImage;


  whitepixels = {255, 255, 255, 255};
  indiciesSize = sizeof(u32) * declaration->vertexesCount * 2;
  textureCoordinatesSize = sizeof(f32) * declaration->vertexesCount * 2;
  positionsSize = sizeof(f32) * declaration->vertexesCount * 3;
  colorsSize = sizeof(u8) * declaration->vertexesCount * 4;

  // load default white texture
  whitepixelImage.data = whitepixels;
  whitepixelImage.width = 1;
  whitepixelImage.height = 1;
  whitepixelImage.pitch = 1;
  context->defaultTextureID = Pepe_GLLoadTexture(whitepixelImage);

  glGenVertexArrays(1, &context->VAO);
  context->drawcalls.data = Pepe_ArenaAllocAlign(
      context->arena,
      sizeof(Pepe_GLDrawCall) * description->drawcallsCount,
      PEPE_DEFAULT_ALIGMENT
  );
  context->drawcalls.length = description->drawcallsCount;
  assert(context->drawcalls.data);

  context->vertexes.length = description->vertexesCount;
  context->vertexes.textureCoordinates = Pepe_ArenaAllocAlign(
      context->arena,
      textureCoordinatesSize,
      PEPE_DEFAULT_ALIGMENT,
  ); 

  context->vertexes.positions = Pepe_ArenaAllocAlign(
      context->arena,
      positionsSize,
      PEPE_DEFAULT_ALIGMENT,
  );
  
  context->vertexes.colors = Pepe_ArenaAllocAlign(
      context->arena,
      colorsSize,
      PEPE_DEFAULT_ALIGMENT,
  );
  context->indicies.length = declaration->vertexCount;
  context->indicies.data = Pepe_ArenaAllocAlign(
      context->arena,
      colorsSize,
      PEPE_DEFAULT_ALIGMENT,
  );
 

}



#endif
