#ifndef PEPE_IO
#define PEPE_IO

#include <sys/stat.h>
#include "pepe_core.h"
#include <unistd.h>
typedef struct Pepe_File Pepe_File;
struct Pepe_File {
  int FileDescriptor;
};

Pepe_File
Pepe_IO_OpenFile(const char *filePath, int flags)
{
  Pepe_File file;
  int fd;
  fd = open(filePath, flags, 0);
  file.FileDescriptor = fd;
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
Pepe_IO_ReadEntireFileDebug(Pepe_File file)
{
  Pepe_Slice slice;

  struct stat fileStat;
  fstat(file.FileDescriptor, &fileStat);

  slice.capacity = fileStat.st_size;
  slice.length = fileStat.st_size;

  slice.base = mmap(0, slice.capacity, PROT_READ, MAP_PRIVATE, file.FileDescriptor, 0);

  return slice;
}

Pepe_Slice
Pepe_IO_ReadEntireFileFromPathDebug(const char *path)
{
  Pepe_File file;
  Pepe_Slice slice;

  file = Pepe_IO_OpenFile(path, 0);
  slice = Pepe_IO_ReadEntireFileDebug(file);

  Pepe_IO_CloseFile(file);

  return slice;
}

#endif
