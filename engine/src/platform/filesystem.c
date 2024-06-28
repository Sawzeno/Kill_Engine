#include "filesystem.h"

#include "core/logger.h"
#include  "core/kmemory.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define RETURN_DEFER(value) do { result = (value); goto defer; } while(0)

bool
KDirExists(const char* path){
TRACEFUNCTION;
KDEBUG("path = %s",path);
  struct stat stats;
  stat(path , &stats);

  return S_ISDIR(stats.st_mode)? true : false;
}

bool
KMakeDir(const char* path){
TRACEFUNCTION;
KDEBUG("path = %s",path);
  bool exists = KDirExists(path);
  if(exists){
    KINFO("'%s' ALREAD EXISTS",path);
    return true;
  }else{
    KWARN("PATH NOT EXIST , MAKING '%s'",path);
    if(mkdir(path, S_IFDIR) < 0){
      KERROR("COULD NOT MAKE %s  : %s",path,strerror(errno));
      return false;
    }else{
      KINFO("CREATED '%s'",path);
      return true;
    }
  }
}

FILE_TYPE
KGetFileType(const char* path){
TRACEFUNCTION;
KDEBUG("path  = %s",path);
  struct stat stats;
  if(stat(path, &stats) < 0){
    KERROR("COULD NOT GET STATS OF %s : %s",path, strerror(errno));
    return -1;
  }

  switch(stats.st_mode & S_IFMT){
    case  S_IFDIR : return FILE_DIRECTORY;
    case  S_IFREG : return FILE_REGULAR;
    case  S_IFLNK : return FILE_SYMLINK;
    default       : return FILE_OTHER;
  }
}

bool
KFileExists(const char* path){
TRACEFUNCTION;
KDEBUG("path = %s",path);
  if(access(path, F_OK)){
    return true;
  }else{
    KERROR("FILE DOES NOT EXIST : %s",path);
    return false;
  }
}

bool 
KFileOpen(const char* path, FILE_MODES  mode, bool binary, FileHandle* outHandle){
TRACEFUNCTION;
KDEBUG("path : %s, mode : %llu, binary : %d, FileHandle* : %p",path,mode,binary,outHandle);
  outHandle->path     = path;
  outHandle->handle   = NULL;
  outHandle->is_valid = false;
  outHandle->size     = 0;
  char* modestr       = NULL;
  bool result         = false;
  if        ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0){
    // UDEBUG("FILE MODE READ & WRITE!");
    modestr = binary ? "w+b" : "w+";
  }else if  ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0){
    // UDEBUG("FILE MODE READ!");
    modestr = binary ? "rb"  : "r";
  }else if  ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0){
    // UDEBUG("FILE MODE WRITE!");
    modestr = binary ? "wb"  : "w";
  }else{
    KERROR("INVALID FILE MODE PASSED !");
    return false;
  }
  if(modestr == NULL){
    return false;
  }
  FILE* file = fopen(path, modestr);

  if(file == NULL)                  RETURN_DEFER(false);
  if(fseek(file, 0, SEEK_END) < 0)  RETURN_DEFER(false);
  u64 filesize  = ftell(file);
  if(filesize < 0)                  RETURN_DEFER(false);
  if(fseek(file, 0, SEEK_SET) < 0)  RETURN_DEFER(false);
  result = true;
  outHandle->handle   = file;
  outHandle->size     = filesize;
  outHandle->is_valid = true;

defer:
  if(!result){
    KERROR("FAILED TO OPEN FILE %s : %s", path, strerror(errno));
    if(file) fclose(file);
    return false;
  }
  return true;
}

bool KFileClose(FileHandle* handle){
TRACEFUNCTION;
KDEBUG("%handle : %p",handle);
  if(handle){
    if(fclose((FILE*)handle->handle) == EOF){
      KERROR("COULD NOT CLOSE FILE %s due to %s",handle->path, strerror(errno));
    }
    handle->path    = NULL;
    handle->size    = 0;
    handle->handle  = NULL;
    handle->is_valid= false;
    return true;
  }
  KERROR("FILE HANDLE PASSED AS NULL");
  return false;
}

bool
KFileReadSizeBytes(FileHandle* handle, u64 dataSize, void* data){
TRACEFUNCTION;
KDEBUG("handle : %p, dataSize : %"PRIu64" data : %p",handle, dataSize, data);
  if(handle && handle->is_valid){
    u64 bytesRead;
    bytesRead = fread(data, 1, dataSize, handle->handle);
    if(bytesRead  != dataSize){
      KERROR("COULD NOT READ ALL DATA FOR FILE %s read %d of %d required",handle->path,bytesRead,dataSize);
      return false;
    }
    return true;
  }else{
    KERROR("FILE HANDLE NULL OR NOT VALID");
    return false;
  }
}

bool
KFileReadAllBytes(FileHandle* handle, u8** outBytes){
TRACEFUNCTION;
KDEBUG("handle : %p outBytes : %p",handle, outBytes);
  if(handle && handle->is_valid){

    u64 bytesRead;
    *outBytes  = kallocate(sizeof(u8) * handle->size, MEMORY_TAG_STRING);
    bytesRead = fread(*outBytes, 1, handle->size, (FILE*)handle->handle);
    if(bytesRead  != handle->size){
      KERROR("COULD NOT READ ALL DATA FOR FILE %s read %d of %d required",handle->path,bytesRead,handle->size);
      return false;
    }
    return true;
  }else{
    KERROR("FILE HANDLE NULL OR NOT VALID");
    return false;
  }
}

bool
KFileReadLine(FileHandle* handle, char** lineBuffer, u64 size){
TRACEFUNCTION;
KDEBUG("handle : %p, lineBuffer : %p, size : %"PRIu64"");
  if (handle && handle->is_valid) {
    char buffer[size];
    //this should be changed , can jist allocate for strings arbritarrily
    if (fgets(buffer, size, (FILE*)handle->handle) != 0) {
      *lineBuffer = kallocate(size * sizeof(char), MEMORY_TAG_STRING);
      strcpy(*lineBuffer, buffer);
      return true;
    }else{
      KERROR("COULD NOT READ LINE FROM FILE %s",handle->path);
      return false;
    }
  }else{
    KERROR("FILE HANDLE NULL OR NOT VAILD");
    return false;
  }
}

bool
KFileWriteSize(FileHandle* handle, u64 dataSize, const void* data, u64* outBytesWritten){
TRACEFUNCTION;
KDEBUG("handle : %p dataSize : %"PRIu64" data : %p outBytesWritten : %"PRIu64"",handle,dataSize,data,outBytesWritten);
  if(handle && handle->is_valid){
    *outBytesWritten  = fwrite(data, 1, dataSize, handle->handle);
    if(*outBytesWritten != dataSize){
      KERROR("COULD NOT WRITE DATA TO %s written %d of %d ",handle->path,*outBytesWritten,dataSize);
      handle->size += *outBytesWritten;
      return false;
    }
    handle->size += *outBytesWritten;
    return true;
  }else{
    KERROR("FILE HANDLE NULL OR NOT VAILED");
    return false;
  }
}
