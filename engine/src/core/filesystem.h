#pragma once

#include "defines.h"
#include <stdio.h>

typedef struct FileHandle {
  const char* path;
  FILE*   handle;
  bool is_valid;
  size_t   size;
} FileHandle;

typedef enum {
  FILE_REGULAR = 0,
  FILE_DIRECTORY,
  FILE_SYMLINK,
  FILE_OTHER,
} KFILE_TYPE;

typedef enum {
  FILE_MODE_READ = 0x1,
  FILE_MODE_WRITE = 0x2
} KFILE_MODES;

typedef enum {
  KFILE_RET_SUCCESS = 0,
  KFILE_RET_EOF = 1,
  KFILE_RET_NULL_HANDLE = 2,
  KFILE_RET_NULL_BUFFER = 3,
  KFILE_RET_ERROR = 4,
}KFILE_RET;

KFILE_TYPE KGetFileType  (const char* path);

char* getAbsolutePath(const char* relativePath);

bool  KDirExsits    (const char* path);
bool  KMakeDir      (const char* path);
bool  KFileExists   (const char* path);

KFILE_RET KFileOpen     (const char* path  , KFILE_MODES  mode, bool binary, FileHandle* outHandle);
KFILE_RET KFileClose    (FileHandle* handle);

KFILE_RET KFileReadLine     (FileHandle* handle, char** lineBuffer, u64 size);
KFILE_RET KFileReadSizeBytes(FileHandle* handle, size_t dataSize, void* data);
KFILE_RET KFileReadAllBytes (FileHandle* handle, u8** outBytes);

KFILE_RET KFileWriteSize(FileHandle* handle, size_t dataSize, const void* data, u64* outBytesWritten);

