#ifndef PEPE_IO
#define PEPE_IO

#include "pepe_core.h"
#include <unistd.h>
typedef struct Pepe_File Pepe_File;
struct Pepe_File {
  void * FileDescriptor;
};

Pepe_File
Pepe_IO_OpenFile(const char *filePath, int flags)
{
  Pepe_File file;
  int fd;
  fd = open(filePath, flags, 0);
  file.FileDescriptor = (void *)fd;
  return file;
}

void
Pepe_IO_CloseFile(Pepe_File file)
{
  close((int)file.FileDescriptor);
}

// TODO: make error handling
int
Pepe_IO_Read(Pepe_File file, Pepe_Slice slice) {
  int fd, result;

  fd = (int)file.FileDescriptor;

  result = read(fd, slice.base, slice.length);

  return result;
}

Pepe_Slice
Pepe_IO_ReadEntireFileDebug(Pepe_Arena *arena, Pepe_File file)
{
  u32 prevSize, chunkSize;
  Pepe_Slice slice;

  chunkSize = 4 * 4096;
  prevSize = 0;
  slice.capacity = slice.length = chunkSize;
  slice.base = Pepe_ArenaAllocAlign(arena, slice.length, PEPE_DEFAULT_ALIGNMENT);
  assert(slice.base);
  for (;;) {
    int bytesReaded;
    bytesReaded = 0;

    bytesReaded = Pepe_IO_Read(file, Pepe_SliceRight(slice, prevSize));
    if (bytesReaded == 0) {
      slice = Pepe_SliceLeft(slice, prevSize);
      break;
    }
    if (bytesReaded < 0) {
      slice.base = nil;
      break;
    }
    prevSize = slice.capacity;
    slice.capacity = slice.length = prevSize + bytesReaded; 
  }

  return slice;
}

Pepe_Slice
Pepe_IO_ReadEntireFileFromPathDebug(Pepe_Arena *arena, const char *path)
{
  Pepe_File file;
  Pepe_Slice slice;

  file = Pepe_IO_OpenFile(path);
  slice = Pepe_IO_ReadEntireFileDebug(arena, file);

  Pepe_IO_CloseFile(file);

  return slice;
}

#endif
