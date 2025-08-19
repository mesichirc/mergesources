#ifndef PEPE_CORE_H
#define PEPE_CORE_H

#define MULTILINE_STR(...) #__VA_ARGS__
#define PEPE_PACKED_ENUM enum __attribute__((__packed__))
#define max(A, B) ((A) > (B) ? (A) : (B))
#define min(A, B) ((A) > (B) ? (B) : (A))

typedef struct Pepe_Slice Pepe_Slice;
struct Pepe_Slice {
  void *base;
  u64  length;
  u64  capacity;
};

typedef struct Pepe_String Pepe_String;
struct Pepe_String {
  u8  *base;
  u64 length;
};

Pepe_Slice
Pepe_SliceInit(void *base, u64 size)
{
  Pepe_Slice slice;
  slice.capacity = size;
  slice.length = size;
  slice.base = base;

  return slice;
}

/* a runtime endian check */
const union {
    long l;
    char c;
} Pepe_Endian = { 1 };

#define PEPE_IS_BE() (Pepe_Endian.c == 0)

Pepe_Slice
Pepe_SliceLeft(Pepe_Slice slice, u64 count)
{
  Pepe_Slice result;
  result.length = count;
  result.capacity = result.length;
  result.base = slice.base;

  return result;
}

Pepe_Slice
Pepe_SliceRight(Pepe_Slice slice, u64 count)
{
  Pepe_Slice result;
  assert(count < slice.length);
  result.base = (u8*)slice.base + count;
  result.length = slice.length - count;
  result.capacity = slice.capacity - count;

  return result;
}


Pepe_String
Pepe_StringFromCString(char *cstr)
{
  Pepe_String str;
  str.base = (u8 *)cstr;
  str.length = strlen(cstr);

  return str;
}

Pepe_String
Pepe_StringInit(Pepe_Slice slice)
{
  Pepe_String str;
  str.base = (u8 *)slice.base;
  str.length = slice.length;

  return str;
}

typedef struct Pepe_Arena Pepe_Arena;
struct Pepe_Arena {
  u8    *buf;
  u64   size;
  u64   currentOffset;
  u64   previousOffset; 
};

void
Pepe_ArenaInit(Pepe_Arena *arena, Pepe_Slice slice)
{
  assert(arena);
  arena->previousOffset = 0;
  arena->currentOffset = 0;
  arena->size = slice.length;
  arena->buf  = (u8 *)slice.base;
}


#define IS_POWER_OF_TWO(x) (((x) & ((x) - 1)) == 0)

uptr 
Pepe_AlignForward(uptr ptr, u64 align)
{
  uptr p, a;

  assert(IS_POWER_OF_TWO(align));

  p = ptr;
  a = (uptr)align;

  return (p + (a - 1)) & ~(a - 1);
}

#ifndef PEPE_DEFAULT_ALIGNMENT
#define PEPE_DEFAULT_ALIGNMENT (2*sizeof(void *))
#endif

void*
Pepe_ArenaAllocAlign(Pepe_Arena *arena, u64 size, u64 align)
{
  uptr currentPointer = (uptr)arena->buf + (uptr)arena->currentOffset;
  uptr offset = Pepe_AlignForward(currentPointer, align);
  assert(arena->buf);
  offset -= (uptr)arena->buf;

  if ((offset + size) > arena->size) {
    return nil;
  }

  void *pointer = &arena->buf[offset];
  arena->previousOffset = offset;
  arena->currentOffset = offset + size;
  memset(pointer, 0, size);

  return pointer;
}

void*
Pepe_ArenaResizeAlign(
    Pepe_Arena *arena, 
    void *old, 
    u64 oldSize, 
    u64 newSize, 
    u64 align
) 
{
  u8 *oldMem = old;
  assert(IS_POWER_OF_TWO(align));

  if (oldMem == nil || oldSize == 0) {
		return Pepe_ArenaAllocAlign(arena, newSize, align);
	} else if (arena->buf <= oldMem && oldMem < arena->buf + arena->size) {
		if (arena->buf + arena->previousOffset == oldMem) {
			arena->currentOffset = arena->previousOffset + newSize;
			if (newSize > oldSize) {
				// Zero the new memory by default
				memset(&arena->buf[arena->currentOffset], 0, newSize-oldSize);
			}
			return old;
		} else {
			void *newMemory = Pepe_ArenaAllocAlign(arena, newSize, align);
			u64 copySize = oldSize < newSize ? oldSize : newSize;
			// Copy across old memory to the new memory
			memmove(newMemory, old, copySize);
			return newMemory;
		}
	} else {
		assert(0 && "Memory is out of bounds of the buffer in this arena");
		return NULL;
	}
}

#define PEPE_ARENA_ALLOC(arenaPointer, size) Pepe_ArenaAllocAlign(arenaPointer, size, PEPE_DEFAULT_ALIGNMENT)
#define PEPE_ARENA_RESIZE(arenaPointer, old, oldSize, newSize) (Pepe_ArenaResizeAlign(arenaPointer, old, oldSize, newSize, PEPE_DEFAULT_ALIGNMENT))
#define PEPE_NEW(arenaPointer, type) Pepe_ArenaAllocAlign(arenaPointer, sizeof(type), PEPE_DEFAULT_ALIGNMENT)
#define PEPE_ARENA_CLEAR(arenaPointer) do { \
  (arenaPointer)->previousOffset = 0;       \
  (arenaPointer)->currentOffset = 0;        \
} while(0)


byte
Pepe_ToLowerCase(byte c)
{
  if (c >= 'A' && c <= 'Z') {
    return c + 32;
  }

  return c;
}

bool
Pepe_IsStringsEqualCaseInsensitive(Pepe_String s1, Pepe_String s2)
{
  u64 i;
  if (s1.length != s2.length) {
    return false;
  }
  if (s1.base == s2.base) {
    return true;
  }

  for (i = 0; i < s1.length; i++) {
    if (Pepe_ToLowerCase(s1.base[i]) != Pepe_ToLowerCase(s2.base[i])) {
      return false;
    }
  }  
  return true;
}

u16
Pepe_U16SwapBytes(u16 n)
{
	return (n << 8) | ((n & 0xFF00) >> 8);
}

u64
Pepe_U64SwapBytes(u64 n)
{
    return (u64)n << 56 | (u64)n << 48 |
           (u64)n << 40 | (u64)n << 32 |
           (u64)n << 24 | (u64)n << 16 |
           (u64)n <<  8 | (u64)n <<  0;
}

#endif
