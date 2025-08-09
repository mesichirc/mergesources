#ifndef PEPE_LAYOUT_H
#define PEPE_LAYOUT_H

typedef struct Pepe_Color Pepe_Color;
struct Pepe_Color {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
};

typedef struct Pepe_Point Pepe_Point;
struct Pepe_Point {
  u16 x;
  u16 y;
};

typedef struct Pepe_CornerRadius Pepe_CornerRadius;
struct Pepe_CornerRadius {
  u16 topLeft;
  u16 topRight;
  u16 bottomLeft;
  u16 bottomRight;
};

// TODO:(sichirc) add more settings for text elements
typedef struct Pepe_TextElementConfig Pepe_TextElementConfig;
struct Pepe_TextElementConfig {
  u16 fontId;
  u16 fontSize;
  u16 letterSpacing;
  u16 lineHeight;
};

typedef struct Pepe_Size Pepe_Size;
struct Pepe_Size {
  u16 width;
  u16 height;
};

typedef struct Pepe_Frame Pepe_Frame;
struct Pepe_Frame {
  Pepe_Point origin;
  Pepe_Size  size;
};

typedef PEPE_PACKED_ENUM {
  PEPE_ELEMENT_TYPE_ROW,
  PEPE_ELEMENT_TYPE_COLUMN,
  PEPE_ELEMENT_TYPE_TEXT,
  PEPE_ELEMENT_TYPE_CUSTOM
} Pepe_ElementType;

typedef PEPE_PACKED_ENUM {
  PEPE_FRAME_TYPE_FIXED,  // known size 0
  PEPE_FRAME_TYPE_FIT,    // based on children sizes and layout direction 1
  PEPE_FRAME_TYPE_GROW    // occupy free space 2
} Pepe_FrameType;

typedef PEPE_PACKED_ENUM {
  PEPE_NONE,
  PEPE_RECT,
  PEPE_TEXT
} Pepe_CommandType;

typedef struct Pepe_TextConfigRef Pepe_TextConfigRef;
struct Pepe_TextConfigRef { u16 id; };

typedef struct Pepe_Command Pepe_Command;
struct Pepe_Command {
  u32                 id;
  Pepe_CommandType    type;
  Pepe_Frame          frame;
  Pepe_Color          backgroundColor;
  Pepe_TextConfigRef  textConfigRef;
  Pepe_String         text;
};

typedef struct Pepe_ElementRef Pepe_ElementRef;
struct Pepe_ElementRef { u32 id; };

typedef struct Pepe_ElementRefArray Pepe_ElementRefArray;
struct Pepe_ElementRefArray {
  Pepe_ElementRef *data;
  u32             length;
  u32             capacity;
};

#define PEPE_CONSTRAINT_INFINITY 65535 // just max u16

typedef struct Pepe_Constraints Pepe_Constraints;
struct Pepe_Constraints {
  u16 minWidth;
  u16 maxWidth;
  u16 minHeight;
  u16 maxHeight;
};

typedef struct Pepe_ElementId Pepe_ElementId;
struct Pepe_ElementId { u32 id; };

typedef PEPE_PACKED_ENUM {
  PEPE_AXIS_TYPE_X,
  PEPE_AXIS_TYPE_Y,
} Pepe_AxisType;

typedef struct Pepe_ElementOptions Pepe_ElementOptions;
struct Pepe_ElementOptions {
  Pepe_AxisType   mainAxis; // 8
  Pepe_FrameType  frameType; // 8
  Pepe_ElementType type;  // 8
};

// TODO: ADD frame type and axis in one 32 bit field
typedef struct Pepe_Element Pepe_Element;
struct Pepe_Element {
  Pepe_ElementId        id;
  Pepe_ElementRef       self;
  Pepe_Frame            frame;
  Pepe_Color            backgroundColor;
  Pepe_CornerRadius     corners;
  Pepe_String           text; 
  Pepe_ElementRef       parentRef;
  Pepe_Constraints      constraints;
  Pepe_Constraints      parentConstraints;
  Pepe_ElementRef       firstChildRef;
  Pepe_ElementRef       nextRef;
  Pepe_ElementRefArray  arrangedChildren;
  Pepe_ElementOptions   options;
};

typedef struct Pepe_ElementRefQueue Pepe_ElementRefQueue;
struct Pepe_ElementRefQueue {
  Pepe_ElementRef *data;
  u32             head;
  u32             tail;
  u32             length;
};

typedef struct Pepe_ElementArray Pepe_ElementArray;
struct Pepe_ElementArray {
  Pepe_Element  *data;
  u32           length;
  u32           capacity;
};


typedef struct Pepe_CommandArray Pepe_CommandArray;
struct Pepe_CommandArray {
  Pepe_Command *data;
  u32          length;
  u32          capacity;
};

typedef struct Pepe_TextElementConfifgArray Pepe_TextElementConfifgArray;
struct Pepe_TextElementConfifgArray {
  Pepe_TextElementConfig  *data;
  u32                     length;
  u32                     capacity;
};


typedef struct Pepe_Context Pepe_Context;
struct Pepe_Context {
  Pepe_Frame                    windowFrame;
  Pepe_Arena                    arena; 
  Pepe_CommandArray             commands;
  Pepe_TextElementConfifgArray  textConfigs;
  Pepe_ElementArray             elements;
  Pepe_ElementRefArray          openedElements; 
  Pepe_ElementId                previousId;
  Pepe_ElementRef               lastClosedRef;
  Pepe_ElementRefArray          refsArray;
};

typedef struct Pepe_U32Array Pepe_U32Array;
struct Pepe_U32Array {
  u32 *data;
  u32 capacity;
  u32 length;
};

typedef struct Pepe_MeasureIterator Pepe_MeasureIterator;
struct Pepe_MeasureIterator {
  Pepe_Context  *context;
  Pepe_ElementRefDoubleStack stack;
};

Pepe_MeasureIterator
Pepe_MeasureIteratorInit(Pepe_Context *context)
{
  Pepe_MeasureIterator iterator;
  Pepe_Slice memory;
  Pepe_ElementRef root;

  root.id = 1;
  memory.length = memory.capacity = context->elements.capacity * sizeof(Pepe_ElementRef);
  memory.base = PEPE_ARENA_ALLOC(&context->arena, memory.length);

  iterator.depth = 0;
  iterator.context = context;
  iterator.stack = Pepe_ElementRefDoubleStackInit(memory);
  Pepe_ElementRefDoubleStackPushFromStart(&iterator.stack, ref);
 
  return iterator;
}

bool
Pepe_MeasureIteratorHasNext(Pepe_MeasureIterator *iterator)
{
  return iterator->stack.lengthFromEnd != 0 || iterator->stack.lengthFromStart != 0;
}

Pepe_ElementRef
Pepe_ElementRefDoubleStackPeekFromStart(Pepe_ElementRefDoubleStack *stack)
{
  Pepe_ElementRef ref;
  ref.id = 0;
  if (stack->lengthFromStart > 0) {
    ref.id = stack->data[stack->lengthFromStack - 1].id;
  }

  return ref;
}

// START From to top (root node) and go 
Pepe_ElementRef
Pepe_MeasureIteratorGetNext(Pepe_MeasureIterator *iterator)
{
  Pepe_ElementRef ref;
  Pepe_Element *element;
  Pepe_Element *parent;
  Pepe_ElementRef latestFromStartRef;
  u32 i;

  if (iterator->stack.lengthFromEnd > 0) {
    ref = Pepe_ElementRefDoubleStackPopFromEnd(&iterator->stack);
    element = Pepe_GetElementByRef(iterator->context, ref);
    if (element->parent.id != 0) {
      parent = Pepe_GetElementByRef(iterator->context, ref->parent);
      latestFromStartRef = Pepe_ElementRefDoubleStackPeekFromStart(&iterator->stack);
      if (element->arrangedChildren.length == 0 && element->parent.id != 0) {
        Pepe_ElementRefDoubleStackPushFromEnd(&iterator->stack, element->parent);
      } else if (latestFromStartRef.id == 0 || Pepe_GetElementByRef(iterator->context, latestFromStartRef)->parent.id != parent->self.id) {
        Pepe_ElementRefDoubleStackPushFromEnd(&iterator->stack, element->parent);
      } 
    }
  } else {
    ref = Pepe_ElementRefDoubleStackPopFromStart(&iterator->stack);
    element = Pepe_GetElementByRef(iterator->context, ref);
    for (i = element->arrangedChildren.length; i > 0; i--) {
      Pepe_ElementRefDoubleStackPushFromStart(&iterator->stack, element->arrangedChildren.data[i - 1]);
    }
    if (element->arrangedChildren.length == 0 && element->parent.id != 0) {
      Pepe_ElementRefDoubleStackPushFromEnd(&iterator->stack, element->parent);
    }
  }


  return ref;
}

typedef struct Pepe_ElementBreadthFirstIterator Pepe_ElementBreadthFirstIterator;
struct Pepe_ElementBreadthFirstIterator {
  Pepe_Context          *context;
  Pepe_ElementRefQueue  queue;
};

/*
 * Stack looks like this:  
 * |x|x|x|x|x| | | | |.....| | | | |x|x|x|x|
 *           ^                     ^
 *           lengthFromStart        lengthFromEnd
 */
typedef struct Pepe_ElementRefDoubleStack Pepe_ElementRefDoubleStack;
struct Pepe_ElementRefDoubleStack {
  Pepe_ElementRef *data;
  u32             lengthFromEnd;
  u32             lengthFromStart;
  u32             capacity;
};

typedef struct Pepe_ElementDepthFirstIterator Pepe_ElementDepthFirstIterator;
struct Pepe_ElementDepthFirstIterator {
  Pepe_Context *context;
  Pepe_ElementRefDoubleStack stack;
};

u32 Pepe_GetOpenedElementIndex(Pepe_Context *context);
Pepe_ElementBreadthFirstIterator Pepe_ElementBreadthFirstIteratorInit(Pepe_Context *context, Pepe_Slice memory);
bool Pepe_ElementBreadthFirstIteratorHasNext(Pepe_ElementBreadthFirstIterator *iterator);
Pepe_ElementRef Pepe_ElementBreadthFirstIteratorGetNext(Pepe_ElementBreadthFirstIterator *iterator);
Pepe_Element *Pepe_GetOpenedElement(Pepe_Context *context);
Pepe_Element *Pepe_GetElementByRef(Pepe_Context *context, Pepe_ElementRef ref);
void Pepe_MeasureElement(Pepe_Context *context, Pepe_Element *element, Pepe_Constraints constraints);
void Pepe_PlaceElement(Pepe_Context *context, Pepe_Element *element);

// DECLARATIONS END


bool
Pepe_ElementIsLeaf(Pepe_Element *element)
{
  return element->firstChildRef.id == 0;
} 

bool
Pepe_ElementIsLastChild(Pepe_Element *element)
{
  return element->nextRef.id == 0;
}

Pepe_ElementRefDoubleStack
Pepe_ElementRefDoubleStackInit(Pepe_Slice memory)
{
  Pepe_ElementRefDoubleStack stack;
  stack.lengthFromEnd = 0;
  stack.lengthFromStart = 0;
  stack.capacity = memory.length / sizeof(Pepe_ElementRef);
  stack.data = (Pepe_ElementRef *)memory.base;

  return stack;
}

void
Pepe_ElementRefDoubleStackPushFromEnd(Pepe_ElementRefDoubleStack *stack, Pepe_ElementRef ref)
{
  assert(stack->lengthFromEnd + stack->lengthFromStart < stack->capacity);
  stack->data[stack->capacity - 1 - stack->lengthFromEnd] = ref;
  stack->lengthFromEnd++;
}

Pepe_ElementRef
Pepe_ElementRefDoubleStackPopFromEnd(Pepe_ElementRefDoubleStack *stack)
{
  Pepe_ElementRef ref;
  assert(stack->lengthFromEnd > 0);

  ref = stack->data[stack->capacity - stack->lengthFromEnd];
  stack->lengthFromEnd--;

  return ref;
}

void
Pepe_ElementRefDoubleStackPushFromStart(Pepe_ElementRefDoubleStack *stack, Pepe_ElementRef ref)
{
  assert(stack->lengthFromEnd + stack->lengthFromStart < stack->capacity);

  stack->data[stack->lengthFromStart] = ref;
  stack->lengthFromStart++;
}

Pepe_ElementRef
Pepe_ElementRefDoubleStackPopFromStart(Pepe_ElementRefDoubleStack *stack)
{
  Pepe_ElementRef ref;
  assert(stack->lengthFromStart > 0);

  stack->lengthFromStart--;
  ref = stack->data[stack->lengthFromStart];

  return ref;
}

void
Pepe_ElementRefDoubleStackPush(Pepe_ElementRefDoubleStack *stack, Pepe_ElementRef ref, Pepe_FrameType frameType)
{
  if (frameType == PEPE_FRAME_TYPE_GROW) {
    Pepe_ElementRefDoubleStackPushFromEnd(stack, ref);
  } else {
    Pepe_ElementRefDoubleStackPushFromStart(stack, ref);
  }
}

Pepe_ElementRef
Pepe_ElementRefDoubleStackPop(Pepe_ElementRefDoubleStack *stack)
{
  if (stack->lengthFromStart > 0) {
    return Pepe_ElementRefDoubleStackPopFromStart(stack);
  } else {
    return Pepe_ElementRefDoubleStackPopFromEnd(stack);
  }
}

Pepe_ElementDepthFirstIterator
Pepe_ElementDepthFirstIteratorInit(Pepe_Context *context, Pepe_Slice memory) 
{
  Pepe_ElementDepthFirstIterator iterator;
  Pepe_ElementRef rootRef;
  Pepe_Element *rootElement;

  assert(context && memory.base && memory.length >= sizeof(Pepe_ElementRef));
  
  rootRef.id = 1;
  rootElement = Pepe_GetElementByRef(context, rootRef);
  iterator.stack = Pepe_ElementRefDoubleStackInit(memory);
  iterator.context = context;

  if (context->elements.length > 0) {
    Pepe_ElementRefDoubleStackPush(&iterator.stack, rootRef, rootElement->options.frameType);
  }

  return iterator;
}

bool
Pepe_ElementDepthFirstIteratorHasNext(Pepe_ElementDepthFirstIterator *iterator)
{
  assert(iterator);

  return (iterator->stack.lengthFromEnd > 0 || iterator->stack.lengthFromStart > 0);
}

Pepe_ElementRef
Pepe_ElementDepthFirstIteratorGetNext(Pepe_ElementDepthFirstIterator *iterator)
{
  Pepe_Element *element;
  Pepe_Element *child;
  Pepe_ElementRef ref;
  Pepe_ElementRef childRef;
  assert(iterator && iterator->context);

  ref = Pepe_ElementRefDoubleStackPop(&iterator->stack);
  element = Pepe_GetElementByRef(iterator->context, ref);
  childRef.id = element->firstChildRef.id;
  while (childRef.id != 0) {
    child = Pepe_GetElementByRef(iterator->context, childRef);
    Pepe_ElementRefDoubleStackPush(&iterator->stack, child->self, child->options.frameType);
    childRef.id = child->nextRef.id;
  }

  return ref;
}

Pepe_ElementRefQueue
Pepe_ElementRefQueueInit(Pepe_Slice memory)
{
  Pepe_ElementRefQueue queue;
  queue.head = 0;
  queue.tail = 0;
  queue.length = memory.length / sizeof(Pepe_ElementRef);
  queue.data   = (Pepe_ElementRef *)memory.base;

  return queue;
}

void
Pepe_DebugPrintChildren(Pepe_Context *context) 
{
  Pepe_ElementRefArray refsArray;
  u32 i;

  assert(context && context->refsArray.data);
  refsArray = context->refsArray;
  printf("[");
  for (i = 0; i < refsArray.length; i++) {
    printf("%d", refsArray.data[i].id);
    if (i != refsArray.length - 1) {
      printf(", ");
    }
  }
  printf("]\n");
}

bool
Pepe_ElementRefQueueIsEmpty(Pepe_ElementRefQueue *queue)
{
  return (queue->head == queue->tail);
}

bool
Pepe_ElementRefQueueIsFull(Pepe_ElementRefQueue *queue)
{
  return ((queue->tail + 1) % queue->length == queue->head);
}

void
Pepe_ElementRefQueuePush(Pepe_ElementRefQueue *queue, Pepe_ElementRef ref)
{
  if (!Pepe_ElementRefQueueIsFull(queue)) {
    queue->data[queue->tail] = ref;
    queue->tail = (queue->tail + 1) % queue->length;
  }
}

Pepe_ElementRef
Pepe_ElementRefQueuePop(Pepe_ElementRefQueue *queue)
{
  Pepe_ElementRef ref;
  ref.id = 0;
  if (!Pepe_ElementRefQueueIsEmpty(queue)) {
    ref = queue->data[queue->head];
    queue->head = (queue->head + 1) % queue->length;
  }

  return ref;
}

u64
Pepe_Strlen(char * str)
{
  char *s;
  for (s = (char *)str; *s; ++s)
    ;

  return (u64)(s - str);
}


#define PEPE_STRING_CONST(strliteral) ((Pepe_String) {.base = (u8 *)strliteral, .length = sizeof(strliteral) - 1})

inline Pepe_String
Pepe_UnsafeCString(char *str)
{
  Pepe_String result;

  result.base         = (u8 *)str;
  result.length       = Pepe_Strlen(str);
  
  return result;
}

Pepe_Slice
Pepe_SliceInit(void *ptr, u64 length)
{
  assert(ptr);
  Pepe_Slice slice = {0};

  slice.base = ptr;
  slice.length = slice.capacity = length;

  return slice;
}

// https://ru.wikipedia.org/wiki/MurmurHash2
#define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

u32
Pepe_Murmur2AHash(Pepe_Slice slice, u32 seed)
{
  u32 m = 0x5bd1e995;
  i32 r = 24;
  u32 len = (u32)slice.length;
  u32 l = len;
  u8 *data = (u8 *)slice.base;
  u32 h = seed;
  u32 k;

  while (len >= 4) {
    k = *(u32 *)data;
    mmix(h, k);

    data += 4;
    len -= 4;
  }

  u32 t = 0;

  switch (len) {
    case 3: t ^= data[2] << 16;
    case 2: t ^= data[1] << 8;
    case 1: t ^= data[0];
  }

  mmix(h,t);
	mmix(h,l);

  h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

  return h;
}

u32
Pepe_Murmur2AHashU32(u32 item, u32 seed)
{
  Pepe_Slice slice = Pepe_SliceInit(&item, sizeof(u32));

  return Pepe_Murmur2AHash(slice, seed);
}

u32
Pepe_Murmur2AHashString(Pepe_String str, u32 seed)
{
  Pepe_Slice slice = Pepe_SliceInit(str.base, str.length);
  return Pepe_Murmur2AHash(slice, seed);
}

typedef PEPE_PACKED_ENUM Pepe_ErrorTypes {
  PEPE_ERROR_ARENA_MEMORY_NOT_ENOUGH,
} Pepe_ErrorTypes;

Pepe_Element *
Pepe_GetElementByRef(Pepe_Context *context, Pepe_ElementRef ref)
{
  //Pepe_Element *element;
  assert(context && context->elements.data);
  assert(context->elements.length > ref.id);

  return context->elements.data + ref.id;
}

void
Pepe_DebugGraphvizPrintLayoutTree(Pepe_Context *context)
{
  Pepe_Element *element;
  Pepe_ElementRef ref;
  Pepe_Slice memory;
  u64 size;
  assert(context && context->elements.data);
  Pepe_ElementBreadthFirstIterator iterator;
  Pepe_ElementDepthFirstIterator depthFirstIterator;

  size = (u64)context->elements.capacity * sizeof(Pepe_ElementRef);
  memory = Pepe_SliceInit(PEPE_ARENA_ALLOC(&context->arena, size), size);

  iterator = Pepe_ElementBreadthFirstIteratorInit(context, memory);

  printf("BreadthFIRST\n");
  printf("digraph G {\n");

  while (Pepe_ElementBreadthFirstIteratorHasNext(&iterator)) {
    ref = Pepe_ElementBreadthFirstIteratorGetNext(&iterator);
    element = Pepe_GetElementByRef(context, ref);
    //printf("%d\n", ref.id);
    if (element->parentRef.id != 0) {
      printf("%d -> %d;\n", element->parentRef.id, ref.id);
    }
  }

  printf("}\n");
  printf("\n");

  memset(memory.base, 0, memory.length);
  depthFirstIterator = Pepe_ElementDepthFirstIteratorInit(context, memory);


  printf("DepthFIRST\n");
  printf("digraph G {\n");

  while (Pepe_ElementDepthFirstIteratorHasNext(&depthFirstIterator)) {
    ref = Pepe_ElementDepthFirstIteratorGetNext(&depthFirstIterator);
    element = Pepe_GetElementByRef(context, ref);
/*
    printf("%d [label=\"id = %d\\n", ref.id, ref.id);
    printf("frame:\\n");
    printf("x=%d, y=%d\\n", element->frame.origin.x, element->frame.origin.y);
    printf("w=%d, h=%d\\n", element->frame.size.width, element->frame.size.height);
    printf("constraints:\\n");
    printf("minWidth=%d, maxWidth=%d\\n", element->constraints.minWidth, element->constraints.maxWidth);
    printf("minHeight=%d, maxHeight=%d\\n", element->constraints.minHeight, element->constraints.maxHeight);
    printf("\"]\n");
*/
    if (element->parentRef.id != 0) {
     printf("%d -> %d;\n", element->parentRef.id, ref.id);
    }
  }

  printf("}\n");


}

#define PEPE_ELEMENT_IS_LEAF(elementPtr) ((elementPtr)->childRef.id == 0)


/*
// 2 - FIT
// 3 - GROW
// 4 - FIT
// 5 - GROW
// 6 - GROW
// 7 - GROW 
// 8 - FIT
// 2 firstChildRef = 4
// 2 lastChildRef  = 7
// 4 jumpRef = 8
// 8 jumpRef = 3
// 2 [3, 4, 5, 6, 7, 8] lastMeasuredChild - 1, measuredChildrenCount - 1
// 2 [3, 4, 5, 6, 7, 8] lastMeasuredChild - 5, measuredChildrenCount - 2
// 2 [3, 4, 5, 6, 7, 8] lastMeasuredChild > 0
//
//            |1| - (window size as constraints) ROW
//           /   \
//          2     6
//         /|\    | \ 
//        3 4 5   7  8
//          |     |   \
//          11    9   10

*/



#define PEPE_MAX_ELEMENTS_COUNT       8192
#define PEPE_MAX_COMMANDS_COUNT       8192
#define PEPE_MAX_TEXT_CONFIG_COUNT    4096
#define PEPE_REST_BUFFER              4096 * 3

#define PEPE_REQUERED_MEMORY() (PEPE_MAX_ELEMENTS_COUNT * sizeof(Pepe_Command) + PEPE_MAX_COMMANDS_COUNT * sizeof(Pepe_Command) + PEPE_MAX_TEXT_CONFIG_COUNT * sizeof(Pepe_TextElementConfig) + PEPE_MAX_ELEMENTS_COUNT * sizeof(u32) + PEPE_MAX_ELEMENTS_COUNT * sizeof(Pepe_ElementRef) + PEPE_REST_BUFFER + PEPE_MAX_ELEMENTS_COUNT * sizeof(Pepe_ElementRef))

typedef struct Pepe_ContextDeclaration Pepe_ContextDeclaration;
struct Pepe_ContextDeclaration {
  Pepe_Slice memory;
  u32 maxElementsCount;
  u32 maxCommandsCount;
  u32 maxTextConfigCount;
};

void
Pepe_ContextDeclarationDefault(Pepe_ContextDeclaration *declaration)
{
  assert(declaration);

  declaration->maxElementsCount = declaration->maxElementsCount == 0 ? PEPE_MAX_ELEMENTS_COUNT : declaration->maxElementsCount;
  declaration->maxCommandsCount = declaration->maxCommandsCount == 0 ? PEPE_MAX_COMMANDS_COUNT : declaration->maxCommandsCount;
  declaration->maxTextConfigCount = declaration->maxTextConfigCount == 0 ? PEPE_MAX_ELEMENTS_COUNT : declaration->maxTextConfigCount;

}


Pepe_Context
Pepe_ContextInit(Pepe_ContextDeclaration declaration)
{
  assert(declaration.memory.base);
  Pepe_Context context = {0};
  Pepe_ArenaInit(&context.arena, declaration.memory);
  assert(context.arena.buf);

  Pepe_ContextDeclarationDefault(&declaration);

  context.commands.data = nil;
  context.commands.capacity = declaration.maxCommandsCount;

  context.textConfigs.data = nil;
  context.textConfigs.capacity = declaration.maxTextConfigCount;

  context.elements.data = nil;
  context.elements.capacity = declaration.maxElementsCount;

  context.openedElements.data = nil;
  context.openedElements.capacity = declaration.maxElementsCount;

  context.refsArray.data = nil;
  context.refsArray.capacity = declaration.maxElementsCount;

  return context;
}

typedef struct Pepe_CustomLayout Pepe_CustomLayout;
struct Pepe_CustomLayout {
  void *userdata;
  // for each child provide constraints
  Pepe_Constraints (* produceConstraintsForChild)(Pepe_Context *context, Pepe_Element *element, Pepe_Element *child, void *userdata);
  // after measuring each children set itself sizes
  void (* setFrame)(Pepe_Context *context, Pepe_Element *element, void *userdata);
  // place children
  void (* placeChildren)(Pepe_Context *context, Pepe_Element *element, void *userdata);
};

void
Pepe_BeginLayout(Pepe_Context *context)
{
  PEPE_ARENA_CLEAR(&context->arena);
  assert(context->arena.buf);

  context->commands.data = PEPE_ARENA_ALLOC(&context->arena, context->commands.capacity);
  assert(context->commands.data);
  context->commands.length = 1;

  context->elements.data = Pepe_ArenaAllocAlign(&context->arena, context->elements.capacity, PEPE_DEFAULT_ALIGNMENT);
  assert(context->elements.data);
  context->elements.length = 1;

  context->textConfigs.data = PEPE_ARENA_ALLOC(&context->arena, context->textConfigs.capacity);
  assert(context->textConfigs.data);
  context->textConfigs.length = 1;

  context->openedElements.data = PEPE_ARENA_ALLOC(&context->arena, context->openedElements.capacity);
  assert(context->openedElements.data);
  context->openedElements.length = 1;

  context->refsArray.data = PEPE_ARENA_ALLOC(&context->arena, context->openedElements.capacity);
  assert(context->refsArray.data);
  context->refsArray.length = 0;

  context->lastClosedRef.id = 0;
}

typedef struct Pepe_CommandsIterator Pepe_CommandsIterator;
struct Pepe_CommandsIterator {
  Pepe_Command *ptr;
  u32 index;
  u32 length;
};

#define PEPE_COMMANDS_HAS_NEXT(iterator) ((iterator).index <= (iterator).length)

#define PEPE_COMMANDS_GET_NEXT(iterator) ((iterator).ptr[(iterator).index++])

// iterator pattern
// 1. init element variable
// 2. init iterator
// 3. pass to while loop ITERATOR_HAS_NEXT macro
// 3. assign elemnt to ITERATOR_GET_NEXT value 
// example:
// Pepe_Command el;
// Pepe_CommandsIterator iterator = Pepe_EndLayout(ctx);
// while (PEPE_COMMANDS_HAS_NEXT(iterator)) {
//  el = PEPE_COMMANDS_GET_NEXT(iterator);
//  ...do something...
// }
//
// or
//
// Pepe_Commands el;
// Pepe_CommandsIterator iterator = Pepe_EndLayout(ctx);
// PEPE_PROCESS_COMMANDS(iterator, el) {
//   ...do something...
// }

#define PEPE_PROCESS_COMMANDS(iterator, el) \
  for (el = PEPE_COMMANDS_GET_NEXT(iterator); PEPE_COMMANDS_HAS_NEXT(iterator); el = PEPE_COMMANDS_GET_NEXT(iterator))


Pepe_Size
Pepe_ElementSize(Pepe_Element *element, Pepe_Constraints constraints)
{
  Pepe_Size size;
  size.width = 0;
  size.height = 0;

  switch (element->options.frameType) {
    case PEPE_FRAME_TYPE_GROW: {
      size.height = constraints.maxHeight;
      size.width = constraints.maxWidth;
      break;
    }
    case PEPE_FRAME_TYPE_FIXED: {
      size.width = min(constraints.maxWidth, max(constraints.minWidth, element->frame.size.width));
      size.height = min(constraints.maxHeight, max(constraints.minHeight, element->frame.size.height));
      break;
    }
    case PEPE_FRAME_TYPE_FIT: {
      size.height = constraints.minHeight;
      size.width = constraints.minWidth;
      break;
    }
  }

  return size;
}

// Pepe_ElementCalculateConstraints
// Pepe_ElementMeasure
// Pepe_ElementPlaceChildren



Pepe_CommandsIterator
Pepe_EndLayout(Pepe_Context *context) 
{
  Pepe_CommandsIterator iterator = {0};

  iterator.length = context->commands.length;
  iterator.ptr    = context->commands.data;
  iterator.index  = 0;

  //Pepe_DebugGraphvizPrintLayoutTree(context);
  Pepe_DebugPrintChildren(context);

  return iterator;
}

Pepe_ElementId
Pepe_GenerateId(Pepe_Context *ctx, Pepe_String str)
{
  Pepe_ElementId elementId;
  elementId.id = Pepe_Murmur2AHashString(str, ctx->previousId.id);
  ctx->previousId.id = elementId.id;
  return elementId;
}


void
Pepe_PushCommand(Pepe_Context *ctx, Pepe_Command cmd)
{
  assert(ctx->commands.length < ctx->commands.capacity);
  assert(ctx->commands.data);
  ctx->commands.data[ctx->commands.length++] = cmd;
}

void
Pepe_AddIndexToOpenedElements(Pepe_Context *context, u32 index)
{
  assert(context->openedElements.length < context->openedElements.capacity);
  context->openedElements.data[context->openedElements.length].id = index;
  context->openedElements.length += 1;
}

/* when elements opens we dont know
 * is it growing elemnt or not so we add elements in draw order
 * on close we will order in layout order too,
 *  layout order - FIT and FIXED first, GROW last
 */
void 
Pepe_OpenElement(Pepe_Context *context)
{ 
  Pepe_Element  *element;
  Pepe_Element  *parent;
  u32           currentIndex;
  Pepe_Element  *lastClosedElement;

  assert(context->elements.data);
  assert(context->elements.length < context->commands.capacity);
  assert(context);

  currentIndex = context->elements.length;
  context->elements.length++;
  element = &context->elements.data[currentIndex];

  element->firstChildRef.id = 0;
  element->nextRef.id = 0;
  element->self.id = currentIndex;

  parent = Pepe_GetOpenedElement(context);
  if (parent) {
    element->parentRef.id = parent->self.id;
    parent->arrangedChildren.length += 1;
    if (parent->firstChildRef.id == 0) {
      parent->firstChildRef.id = currentIndex;
    } else {
      if (context->lastClosedRef.id != 0) {
        lastClosedElement = Pepe_GetElementByRef(context, context->lastClosedRef);
        if (lastClosedElement->parentRef.id == element->parentRef.id) {
          lastClosedElement->nextRef.id = currentIndex;
        }
      }
    }
  } else {
    element->constraints.minWidth = 0;
    element->constraints.maxWidth = context->windowFrame.size.width;
    element->constraints.minHeight = 0;
    element->constraints.maxHeight = context->windowFrame.size.height;
  }


  context->openedElements.data[context->openedElements.length].id = currentIndex;
  context->openedElements.length += 1;
}

u32
Pepe_GetOpenedElementIndex(Pepe_Context *context)
{
  u32 lastOpenedIndex;
  assert(context);
  assert(context->openedElements.data);
  if (context->openedElements.length == 1) {
    return 0;
  }

  lastOpenedIndex = context->openedElements.data[context->openedElements.length - 1].id;
  assert(context->elements.data);
  assert(context->elements.length > lastOpenedIndex);

  return lastOpenedIndex;
}

Pepe_Element *
Pepe_GetOpenedElement(Pepe_Context *context)
{
  u32 lastOpenedIndex;
  lastOpenedIndex = Pepe_GetOpenedElementIndex(context);
  if (lastOpenedIndex == 0) {
    return nil;
  }
  return &context->elements.data[lastOpenedIndex];
}

void
Pepe_OpenedElementAttachId(Pepe_Context *context, Pepe_ElementId id)
{
  Pepe_Element *element;
  element = Pepe_GetOpenedElement(context);

  if (element != nil) {
    element->id.id = id.id;
  }
}

void
Pepe_OpenedElementSetStringId(Pepe_Context *context, Pepe_String label)
{
  Pepe_OpenedElementAttachId(context, Pepe_GenerateId(context, label));
}

Pepe_ElementBreadthFirstIterator
Pepe_ElementBreadthFirstIteratorInit(Pepe_Context *context, Pepe_Slice memory)
{
  Pepe_ElementBreadthFirstIterator iterator;

  assert(context && memory.base && memory.length >= sizeof(Pepe_ElementRef));

  iterator.queue = Pepe_ElementRefQueueInit(memory);
  iterator.context = context;
  if (context->elements.length > 0) {
    Pepe_ElementRefQueuePush(&iterator.queue, context->elements.data[1].self);
  }

  return iterator;
}

bool
Pepe_ElementBreadthFirstIteratorHasNext(Pepe_ElementBreadthFirstIterator *iterator)
{
  return !Pepe_ElementRefQueueIsEmpty(&iterator->queue);
}

Pepe_ElementRef
Pepe_ElementBreadthFirstIteratorGetNext(Pepe_ElementBreadthFirstIterator *iterator)
{
  Pepe_Element *element;
  Pepe_ElementRef ref;
  Pepe_ElementRef childRef;
  Pepe_Element *child;
  ref = Pepe_ElementRefQueuePop(&iterator->queue);
  element = Pepe_GetElementByRef(iterator->context, ref);
  childRef.id = element->firstChildRef.id;
  while (childRef.id != 0) {
    Pepe_ElementRefQueuePush(&iterator->queue, childRef);
    child = Pepe_GetElementByRef(iterator->context, childRef);
    childRef.id = child->nextRef.id;
  }

  return ref;
}

void
Pepe_MeasureLayout(Pepe_Context *context)
{
  Pepe_Element *element;
  Pepe_Element *previousElement;
  Pepe_Element *parent;
  Pepe_ElementRef ref;
  u32 i;

  for (i = 0; i < context->refsArray.length; i++) {
    ref = context->refsArray.data[i];
    element = Pepe_GetElementByRef(context, ref);
    if (element->parentRef.id == previousElement->parentRef.id) {
      // [FIXED]->[FIT]->[GROW]
      if (element->options.frameType == PEPE_FRAME_TYPE_GROW && (!previousElement || previousElement->options.frameType != PEPE_FRAME_TYPE_GROW)) {
          
      }
    }

    previousElement = element;
  }
}

void
Pepe_CloseElement(Pepe_Context *context)
{
  Pepe_Element *element;
  Pepe_ElementRef childRef;
  Pepe_Element *child;
  Pepe_ElementRefDoubleStack stack;

  element = Pepe_GetOpenedElement(context);
  assert(element);
  context->lastClosedRef.id = element->self.id;
  assert(context->refsArray.capacity > context->refsArray.length);
  if (element->arrangedChildren.length > 0) {
    stack.data = context->refsArray.data + context->refsArray.length;
    stack.lengthFromEnd = 0;
    stack.lengthFromStart = 0;
    stack.capacity = element->arrangedChildren.length;
    context->refsArray.length += stack.capacity;
    element->arrangedChildren.data = stack.data;

    childRef = element->firstChildRef;
    while (childRef.id != 0) {
      child = Pepe_GetElementByRef(context, childRef);
      Pepe_ElementRefDoubleStackPush(&stack, child->self, child->options.frameType);
      childRef.id = child->nextRef.id;
    }
  }
  
  context->openedElements.data[context->openedElements.length - 1].id = 0;
  context->openedElements.length -= 1;  
}

#endif
