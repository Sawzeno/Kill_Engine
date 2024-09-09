#pragma once

#include "defines.h"

typedef struct FileHandle {
  const char* path;
  void* handle;
  bool is_valid;
  u64   size;
} FileHandle;

typedef enum {
  FILE_REGULAR = 0,
  FILE_DIRECTORY,
  FILE_SYMLINK,
  FILE_OTHER,
} FILE_TYPE;

typedef enum  FILE_MODES{
  FILE_MODE_READ = 0x1,
  FILE_MODE_WRITE = 0x2
} FILE_MODES;

FILE_TYPE KGetFileType  (const char* path);
bool  KDirExsits    (const char* path);
bool  KMakeDir      (const char* path);

bool  KFileExists   (const char* path);
bool  KFileOpen     (const char* path  , FILE_MODES  mode, bool binary, FileHandle* outHandle);
bool  KFileClose    (FileHandle* handle);

bool  KFileReadLine (FileHandle* handle, char** lineBuffer, u64 size);
bool  KFileReadSizeBytes(FileHandle* handle, u64 dataSize, void* data);
bool  KFileReadAllBytes (FileHandle* handle, u8** outBytes);

bool  KFileWriteSize(FileHandle* handle, u64 dataSize, const void* data, u64* outBytesWritten);

