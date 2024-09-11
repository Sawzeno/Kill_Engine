#pragma once

#include "defines.h"
#include  <stdio.h>

typedef struct FileHandle {
  const char* path;
  FILE*   handle;
  b32 is_valid;
  size_t   size;
} FileHandle;

typedef enum {
  FILE_REGULAR    = 0,
  FILE_DIRECTORY  = 1,
  FILE_SYMLINK    = 3,
  FILE_OTHER      = 4,
} KFILE_TYPE;

typedef enum {
  FILE_MODE_READ  = 0x1,
  FILE_MODE_WRITE = 0x2
} KFILE_MODE;

typedef enum {
  KFILE_RET_SUCCESS     = 0,
  KFILE_RET_EOF         = 1,
  KFILE_RET_NULL_PARAM  = 2,
  KFILE_RET_ERROR       = 3,
}KFILE_RET;

KFILE_TYPE KGetFileType     (const char* path);

b32 KDirExsits  (const char* path);
b32 KMakeDir    (const char* path);
b32 KFileExists (const char* path);

KFILE_RET KFileOpen         (const char* path  , KFILE_MODE  mode, b32 binary, FileHandle* outHandle);
KFILE_RET KFileClose        (FileHandle* handle);

KFILE_RET KFileReadLine     (FileHandle* handle, char** lineBuffer, u64 size);
KFILE_RET KFileReadSizeBytes(FileHandle* handle, size_t dataSize, void* data);
KFILE_RET KFileReadAllBytes (FileHandle* handle, u8** outBytes);

KFILE_RET KFileWriteSize    (FileHandle* handle, size_t dataSize, const void* data, u64* outBytesWritten);

