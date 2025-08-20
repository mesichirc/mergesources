#ifndef PEPE_GL_H
#define PEPE_GL_H

typedef enum {
  PEPE_GL_ERROR_NONE = 0,
  PEPE_GL_ERROR_DEFAULT_SHADER_NOT_COMPILED,
} Pepe_GLErrorType;

typedef enum {
  PEPE_VBO_POSITIONS = 0,
  PEPE_VBO_TEXCOORDS,
  PEPE_VBO_COLORS,
  PEPE_VBO_INDICIES,
  PEPE_VBO_LENGTH
} Pepe_VBO_Type;

typedef struct Pepe_GLImage Pepe_GLImage;
struct Pepe_GLImage {
  byte               *data; 
  u32                width;
  u32                height;
  u32                pitch;
};

typedef struct Pepe_GLUVRect Pepe_GLUVRect;
struct Pepe_GLUVRect {
  Pepe_GF32Vec2 topLeft;
  Pepe_GF32Vec2 bottomLeft;
  Pepe_GF32Vec2 topRight;
  Pepe_GF32Vec2 bottomRight;
};

typedef struct Pepe_GLDrawCall Pepe_GLDrawCall;
struct Pepe_GLDrawCall {
  u32 start;
  u32 length;
  u32 textureID;
  u32 shaderID;
};

typedef struct Pepe_GLIndiciesArray Pepe_GLIndiciesArray;
struct Pepe_GLIndiciesArray {
  u32 *data;
  u32 length;
  u32 capacity;
};

typedef struct Pepe_GLDrawCallArray Pepe_GLDrawCallArray;
struct Pepe_GLDrawCallArray {
  Pepe_GLDrawCall *data;
  u32             length;
  u32             capacity;
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
  u32 capacity;
  u32 length;
};

typedef struct Pepe_GLContextDeclaration Pepe_GLContextDeclaration;
struct Pepe_GLContextDeclaration {
  Pepe_GRect viewport;
  u32        vertexesCount;
  u32        drawcallsCount;
  u32        texturesCount;
};

void
Pepe_GLContextDeclarationInit(
    Pepe_GLContextDeclaration *declaration,
    Pepe_GRect viewport
)
{
  assert(declaration);
  declaration->viewport = viewport;
  declaration->vertexesCount = 1024;
  declaration->drawcallsCount = 16;
  declaration->texturesCount = 256;
}

typedef struct Pepe_GLContext Pepe_GLContext;
struct Pepe_GLContext {
  Pepe_Arena              *arena;
  Pepe_GLDrawCallArray    drawcalls;
  Pepe_GLVertexes         vertexes;
  Pepe_GLIndiciesArray    indicies;
  u32                     VAO;
  u32                     defaultTextureID;
  Pepe_GLShaderLocations  defaultShaderLocations;
  u32                     defaultShaderID;
  u32                     VBO[PEPE_VBO_LENGTH];
};


char* PEPE_GLDefaultVertexShader =
  "#version 330 core                                  \n"
  "in vec3 vertexPosition;                            \n"
  "in vec2 vertexTexCoord;                            \n"
  "in vec4 vertexColor;                               \n"
  "out vec2 fragTexCoord;                             \n"
  "out vec4 fragColor;                                \n"
  "uniform mat4 mpv;                                  \n"
  "void main()                                        \n"
  "{                                                  \n"
  "   fragTexCoord  = vertexTexCoord;                 \n"
  "   fragColor     = vertexColor;                    \n"
  "   gl_Position   = mpv*vec4(vertexPosition, 1.0);  \n" 
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
  *shader = glCreateShader(type);
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

bool
Pepe_GLBuildDefaultShader(u32 *program)
{
  bool success;
  u32 vertexShader;
  u32 fragmentShader;
  if (!program) {
    printf("could not build default shader because program id is nil");
    return false;
  }

  vertexShader = 0;
  fragmentShader = 0;
  success = false;

  success = Pepe_GLCompileShader(Pepe_StringFromCString(PEPE_GLDefaultVertexShader), GL_VERTEX_SHADER, &vertexShader);
  success = Pepe_GLCompileShader(Pepe_StringFromCString(PEPE_GLDefaultFragmentShader), GL_FRAGMENT_SHADER, &fragmentShader);

  if (!success) {
    return success;
  }

  return Pepe_GLLinkShader(vertexShader, fragmentShader, program); 
}

u32
Pepe_GLLoadTexture(Pepe_GLImage image)
{
  u32 id;

  unused(image);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
  glBindTexture(GL_TEXTURE_2D, 0);

  return id;
}


bool
Pepe_GLContextInit(Pepe_Arena *arena, Pepe_GLContext *context, Pepe_GLContextDeclaration *declaration)
{
  assert(arena);
  assert(declaration);
  u32 indiciesSize;
  u32 textureCoordinatesSize;
  u32 positionsSize;
  u32 colorsSize;
  u8  whitepixels[4];
  Pepe_GLImage whitepixelImage;

  unused(indiciesSize);
  unused(textureCoordinatesSize);
  unused(positionsSize);
  unused(colorsSize);

  whitepixels[0] = 255;
  whitepixels[1] = 255;
  whitepixels[2] = 255;
  whitepixels[3] = 255;

  context->arena = arena;
  indiciesSize = sizeof(u32) * declaration->vertexesCount * 2;
  textureCoordinatesSize = sizeof(f32) * declaration->vertexesCount * 2;
  positionsSize = sizeof(f32) * declaration->vertexesCount * 3;
  colorsSize = sizeof(u8) * declaration->vertexesCount * 4;

  unused(whitepixelImage);
  // load default white texture
  whitepixelImage.data = whitepixels;
  whitepixelImage.width = 1;
  whitepixelImage.height = 1;
  whitepixelImage.pitch = 1;

  context->defaultTextureID = Pepe_GLLoadTexture(whitepixelImage);

  glGenVertexArrays(1, &context->VAO);
  context->drawcalls.data = Pepe_ArenaAllocAlign(
      context->arena,
      sizeof(Pepe_GLDrawCall) * declaration->drawcallsCount,
      PEPE_DEFAULT_ALIGNMENT 
  );

  context->drawcalls.length = 0;
  context->drawcalls.capacity = declaration->drawcallsCount;
  assert(context->drawcalls.data);
  if (context->drawcalls.data == nil) {
    return false;
  }

  context->vertexes.length = declaration->vertexesCount;
  context->vertexes.textureCoordinates = Pepe_ArenaAllocAlign(
      context->arena,
      textureCoordinatesSize,
      PEPE_DEFAULT_ALIGNMENT
  ); 
  if (context->vertexes.textureCoordinates == nil) {
    return false;
  }

  context->vertexes.positions = Pepe_ArenaAllocAlign(
      context->arena,
      positionsSize,
      PEPE_DEFAULT_ALIGNMENT
  );
  if (context->vertexes.positions == nil) {
    return false;
  }
  
  context->vertexes.colors = Pepe_ArenaAllocAlign(
      context->arena,
      colorsSize,
      PEPE_DEFAULT_ALIGNMENT
  );
  if (context->vertexes.colors == nil) {
    return false;
  }

  context->indicies.length = 0;
  context->indicies.capacity = declaration->vertexesCount;
  context->indicies.data = Pepe_ArenaAllocAlign(
      context->arena,
      indiciesSize,
      PEPE_DEFAULT_ALIGNMENT
  );
  if (context->indicies.data == nil) {
    return false;
  }

  if (!Pepe_GLBuildDefaultShader(&context->defaultShaderID)) {
    return false;
  }

  glBindVertexArray(context->VAO);
  glGenBuffers(PEPE_VBO_LENGTH, context->VBO);

  context->defaultShaderLocations.position                = 0;
  context->defaultShaderLocations.textureCoordinates      = 0;
  context->defaultShaderLocations.color                   = 0;
  context->defaultShaderLocations.mpv                     = 0;

  context->defaultShaderLocations.position = glGetAttribLocation(context->defaultShaderID, "vertexPosition");
  if (context->defaultShaderLocations.position < 0) {
    printf("position location not found\n");
    return false;
  }
  context->defaultShaderLocations.textureCoordinates = glGetAttribLocation(context->defaultShaderID, "vertexTexCoord");
  if (context->defaultShaderLocations.textureCoordinates < 0) {
    printf("textureCoordinates location not found\n");
    return false;
  }
  context->defaultShaderLocations.color = glGetAttribLocation(context->defaultShaderID, "vertexColor");
  if (context->defaultShaderLocations.color < 0) {
    printf("color location not found\n");
    return false;
  }
  context->defaultShaderLocations.mpv = glGetUniformLocation(context->defaultShaderID, "mpv");
  if (context->defaultShaderLocations.mpv < 0) {
    printf("mpv location not found\n");
    return false;
  }

  glBindBuffer(GL_ARRAY_BUFFER, context->VBO[PEPE_VBO_POSITIONS]);
  glBufferData(GL_ARRAY_BUFFER, positionsSize, context->vertexes.positions, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(context->defaultShaderLocations.position);
  glVertexAttribPointer(context->defaultShaderLocations.position, 3, GL_FLOAT, 0, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, context->VBO[PEPE_VBO_TEXCOORDS]);
  glBufferData(GL_ARRAY_BUFFER, textureCoordinatesSize, context->vertexes.textureCoordinates, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(context->defaultShaderLocations.textureCoordinates);
  glVertexAttribPointer(context->defaultShaderLocations.textureCoordinates, 2, GL_FLOAT, 0, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, context->VBO[PEPE_VBO_COLORS]);
  glBufferData(GL_ARRAY_BUFFER, textureCoordinatesSize, context->vertexes.colors, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(context->defaultShaderLocations.color);
  glVertexAttribPointer(context->defaultShaderLocations.color, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context->VBO[PEPE_VBO_INDICIES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiciesSize, context->indicies.data, GL_DYNAMIC_DRAW);
  
  return true;
}

void
Pepe_GLDrawCallArrayAppend(Pepe_GLDrawCallArray *drawcalls, Pepe_GLDrawCall drawcall) 
{
  assert(drawcalls.length < drawcalls.capacity);
  *(drawcalls->data + drawcalls->length) = drawcall;
  drawcalls->length++;
}

void
Pepe_GlBeginDraw(Pepe_GLContext *context)
{
  Pepe_GLDrawCall drawcall;

  context->vertexes.length = 0;
  context->indicies.length = 0;
  context->drawcalls.length = 0;
  drawcall.start  = 0;
  drawcall.length = 0;
  drawcall.textureID = context->defaultTextureID;
  drawcall.shaderID = context->defaultShaderID;

  Pepe_GlLDrawCallArrayAppend(&context->drawcalls, drawcall);
}

u32
Pepe_GLPushVertex(
    Pepe_GLContext *context, 
    f32 x,
    f32 y,
    f32 z,
    f32 u,
    f32 v,
    Pepe_GColor color
) 
{
  u32 index;

  index = context->vertexes.length;
  assert(index < context->vertexes.capacity);
  context->vertexes.positions[index * 3 + 0] = x;
  context->vertexes.positions[index * 3 + 1] = y;
  context->vertexes.positions[index * 3 + 2] = z;

  context->vertexes.textureCoordinates[index * 2 + 0] = u;
  context->vertexes.textureCoordinates[index * 2 + 1] = v;

  context->vertexes.colors[index * 4 + 0] = color.r;
  context->vertexes.colors[index * 4 + 1] = color.g;
  context->vertexes.colors[index * 4 + 2] = color.b;
  context->vertexes.colors[index * 4 + 3] = color.a;

  context->vertexes.length += 1;
  return index;
}

void
Pepe_GLFlush(Pepe_GLContext *context)
{
  u32 i;
  u32 *indicies;
  Pepe_GLDrawcall *drawcall;
  f32 f1, f2;
  Pepe_GPoint origin;
  Pepe_GSize  size;

  origin = context->viewport.origin;
  size   = context->viewport.size;

  f1 = 2.0f / size.width;
  f2 = 2.0f / size.height;

  f32 Proj[] = 
  {
       a, 0.0f, 0.0f, 0.0f,
    0.0f,    b, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
   -1.0f, 1.0f, 0.0f, 1.0f
  };

  glViewport((u32)origin.x, (u32)origin.y, (u32)size.width, (u32)size.height);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  glUseProgram(context->defaultShaderID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, context->defaultTextureID);
  glBindBuffer(GL_ARRAY_BUFFER, context->VBO[PEPE_VBO_POSITIONS]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, context->vertexes.length * 3 * sizeof(f32), context->vertexes.positions);

  glBindBuffer(GL_ARRAY_BUFFER, context->VBO[PEPE_VBO_TEXCOORDS]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, context->vertexes.index * 2 * sizeof(f32), ctx->vertexes.texcoords);

  glBindBuffer(GL_ARRAY_BUFFER, context->VBO[PEPE_VBO_COLORS]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, context->vertexes.index * 4 * sizeof(u8), ctx->vertexes.colors); 

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, context->VBO[SCV_VBO_INDICIES]);
  glBindVertexArray(context->VAO);
  glUniformMatrix4fv(context->defaultShaderLocations.mvp, 1, false, Proj);

  for (i = 0; i < context->drawcalls.length; i++) {
    
    drawcall = context->drawcalls.data + i;
    if (drawcall->length == 0) {
      continue;
    }
    indicies = context->indicies.data + drawcall->start;
    glUseProgram(drawcall->shaderID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, drawcall->textureID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, drawcall->length * sizeof(u32), indicies);
    glDrawElements(GL_TRIANGLES, drawcall->length, GL_UNSIGNED_INT, (void *)0); 
  }

  glUseProgram(0);
  context->vertexes.length = 0;
  context->indicies.length = 0;

  context->drawcalls.length = 1;
  drawcall = context->drawcalls.data;

  drawcall.start  = 0;
  drawcall.length = 0;
  drawcall.textureID = context->defaultTextureID;
  drawcall.shaderID = context->defaultShaderID;
}

void
Pepe_GLPushIndex(Pepe_GLContext *context, u32 index)
{
  u32 length;

  Pepe_GLDrawcall *drawcall;

  length   = context->indicies.length;
  drawcall = context->drawcalls.data + (context->drawcalls.length - 1);

  assert(drawcall);
  assert(length < context->indicies.capacity);

  context->indicies.data[length] = index;
  context->indicies.length += 1;
  drawcall->length += 1;
}

void
Pepe_GLDrawRectInternal(Pepe_GLContext *context, Pepe_GRect rect, Pepe_GColor color, Pepe_GLUVRect *uvs)
{
  u32 i1, i2, i3, i4;
  f32 x, y, width, height;
  Pepe_GF32Vec3 position;
  Pepe_GF32Vec2 textureCoordinate;

  if (context->vertexes.length >= context->vertexes.capacity - 5) {
    Pepe_GLFlush(Pepe_GLContext *context);
  }
  x = rect.origin.x;
  y = rect.origin.y;
  width = rect.size.width;
  height = rect.size.height;

  // top left
  i1 = Pepe_GLPushVertex(context, x, y, 1.0f, uvs ? uvs->topLeft[0] : 0.0f, uvs ? uvs->topLeft[1] : 0.0f);
  // top right
  i2 = Pepe_GLPushVertex(context, x + width, y, 1.0f, uvs ? uvs->topRight[0] : 0.0f, uvs ? uvs->topRight[1] : 0.0f);
  // bottom right
  i3 = Pepe_GLPushVertex(context, x + width, y + height, 1.0f, uvs ? uvs->bottomRight[0] : 0.0f, uvs ? uvs->bottomRight[1] : 0.0f);
  // bottom left
  i4 = Pepe_GLPushVertex(context, x, y + height, 1.0f, uvs ? uvs->bottomLeft[0] : 0.0f, uvs ? uvs->bottomLeft[1] : 0.0f);


  // clockwise push indicies
  Pepe_GLPushIndex(context, i1);  
  Pepe_GLPushIndex(context, i2);  
  Pepe_GLPushIndex(context, i3);  

  Pepe_GLPushIndex(context, i1);  
  Pepe_GLPushIndex(context, i3);  
  Pepe_GLPushIndex(context, i4);  
}

void
Pepe_GLEndDraw(Pepe_GLContext *context) 
{
  Pepe_GLFlush(context);
}


#endif
