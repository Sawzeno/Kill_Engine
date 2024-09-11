#include  "filesystem.h"
#include  "core/logger.h"
#include  "core/kstring.h"

#include  <errno.h>
#include  <stdio.h>
#include  <string.h>
#include  <sys/stat.h>
#include  <unistd.h>

#define RETURN_DEFER(value) do { result = (value); goto defer; } while(0)

char* getExecutablePath() {
    static char path[MAX_STR_LEN];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';  
        return path;
    }
    return NULL;
}

b32
KDirExists(const char* path){
  TRACEFUNCTION;

  struct stat stats;
  stat(path , &stats);
  return S_ISDIR(stats.st_mode)? TRUE : FALSE;
}

b32
KMakeDir(const char* path){
  TRACEFUNCTION;

  b32 exists = KDirExists(path);
  if(exists){
    KINFO("'%s' ALREAD EXISTS",path);
    return TRUE;
  }else{
    KWARN("PATH NOT EXIST , MAKING '%s'",path);
    if(mkdir(path, S_IFDIR) < 0){
      KERROR("COULD NOT MAKE %s  : %s",path,strerror(errno));
      return FALSE;
    }else{
      KINFO("CREATED '%s'",path);
      return TRUE;
    }
  }
}

KFILE_TYPE
KGetFileType(const char* path){
  TRACEFUNCTION;

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

b32
KFileExists(const char* path){
  TRACEFUNCTION;

  if(access(path, F_OK)){
    return TRUE;
  }else{
    KERROR("FILE DOES NOT EXIST : %s",path);
    UERROR("FILE DOES NOT EXIST : %s",path);
    return FALSE;
  }
}

KFILE_RET
KFileOpen(const char* path, KFILE_MODE  mode, b32 binary, FileHandle* outHandle){
  TRACEFUNCTION;

  outHandle->path     = path;
  outHandle->handle   = NULL;
  outHandle->is_valid = FALSE;
  outHandle->size     = 0;
  char* modestr       = NULL;
  b32 result         = FALSE;

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
    return KFILE_RET_ERROR;
  }
  if(modestr == NULL){
    return KFILE_RET_ERROR;
  }
  FILE* file = fopen(path, modestr);

  if(file == NULL)                  RETURN_DEFER(FALSE);
  if(fseek(file, 0, SEEK_END) < 0)  RETURN_DEFER(FALSE);
  u64 filesize  = ftell(file);
  if(filesize < 0)                  RETURN_DEFER(FALSE);
  if(fseek(file, 0, SEEK_SET) < 0)  RETURN_DEFER(FALSE);
  result = TRUE;
  outHandle->handle   = file;
  outHandle->size     = filesize;
  outHandle->is_valid = TRUE;

defer:
  if(!result){
    KERROR("FAILED TO OPEN FILE %s : %s", path, strerror(errno));
    if(file) fclose(file);
    return KFILE_RET_ERROR;
  }
  return KFILE_RET_SUCCESS;
}

KFILE_RET
KFileClose(FileHandle* handle){
  TRACEFUNCTION;

  if(handle){
    if(fclose((FILE*)handle->handle) == EOF){
      KERROR("COULD NOT CLOSE FILE %s due to %s",handle->path, strerror(errno));
    }
    handle->path    = NULL;
    handle->size    = 0;
    handle->handle  = NULL;
    handle->is_valid= FALSE;
    return KFILE_RET_SUCCESS;
  }
  KERROR("FILE HANDLE PASSED AS NULL");
  return KFILE_RET_ERROR;
}

KFILE_RET
FileReadSizeBytes(FileHandle* handle, size_t dataSize, void* data){
  TRACEFUNCTION;

  if(data == NULL) return KFILE_RET_NULL_PARAM;
  if(handle == NULL || !handle->is_valid) return KFILE_RET_NULL_PARAM;
  FILE* h = (FILE*)handle->handle;

  if (dataSize > UINT16_MAX) {
    KERROR("REQUESTED DATA SIZE IS TOO LARGE: %llu", dataSize);
    return KFILE_RET_ERROR;
  }

  size_t bytesRead;
  bytesRead = fread(data, 1, dataSize, h);

  if(ferror(h)){
    KERROR("ERROR AT KfileReadLine : %s", strerror(errno));
    return KFILE_RET_ERROR;
  }
  if(feof(h) && bytesRead < dataSize){
    KTRACE("END OF FILE REACHED : %s", handle->path);
    return KFILE_RET_EOF;
  }
  if(bytesRead  != dataSize){
    KERROR("COULD NOT READ ALL DATA FOR FILE %s read %zu of %zu required",handle->path,bytesRead,dataSize);
    return KFILE_RET_ERROR;
  }

  return KFILE_RET_SUCCESS;
}

KFILE_RET
KFileReadAllBytes(FileHandle* handle, u8** outBytes){
  TRACEFUNCTION;

  if(outBytes == NULL) return KFILE_RET_NULL_PARAM;
  if(handle == NULL || !handle->is_valid) return KFILE_RET_NULL_PARAM;
  FILE* h = (FILE*)handle->handle;
  size_t bytesRead;

  bytesRead = fread(*outBytes, 1, handle->size, (FILE*)handle->handle);
  if(ferror(h)){
    KERROR("ERROR AT KfileReadLine : %s", strerror(errno));
    return KFILE_RET_ERROR;
  }

  if(feof(h)){
    KTRACE("END OF FILE REACHED : %s", handle->path);
    return KFILE_RET_EOF;
  }

  if(bytesRead  != handle->size){
    KERROR("COULD NOT READ ALL DATA FOR FILE %s read %zu of %zu required",handle->path,bytesRead,handle->size);
    return KFILE_RET_ERROR;
  }

  return KFILE_RET_SUCCESS;
}

KFILE_RET
KFileReadLine(FileHandle* handle, char** lineBuffer, u64 size){
  TRACEFUNCTION;

  b32 result = FALSE;
  if(lineBuffer == NULL) return KFILE_RET_NULL_PARAM;
  if(handle == NULL || !handle->is_valid) return KFILE_RET_NULL_PARAM;
  FILE* h = (FILE*)handle->handle;

  if (fgets(*lineBuffer, size, h) != NULL) {
    return KFILE_RET_SUCCESS;
  }

  if(feof(h)){
    KTRACE("END OF FILE REACHED : %s", handle->path);
    return KFILE_RET_EOF;
  }

  if(ferror(h)){
    KERROR("ERROR AT KfileReadLine : %s", strerror(errno));
    return KFILE_RET_ERROR;
  }
  return KFILE_RET_ERROR;
}

KFILE_RET
KFileWriteSize(FileHandle* handle, size_t dataSize, const void* data, u64* outBytesWritten){
  TRACEFUNCTION;

  if(data == NULL) return KFILE_RET_NULL_PARAM;
  if(handle == NULL || !handle->is_valid) return KFILE_RET_NULL_PARAM;
  FILE* h = (FILE*)handle->handle;

  if (dataSize > UINT16_MAX) {
    KERROR("rEQUESTED DATA SIZE IS TOO LARGE: %llu", dataSize);
    return KFILE_RET_ERROR;
  }

  size_t written = fwrite(data, 1, dataSize, h);
  if(ferror(h)){
    KERROR("ERROR AT KfileReadLine : %s", strerror(errno));
    return KFILE_RET_ERROR;
  }

  *outBytesWritten = written;
  handle->size += *outBytesWritten;

  if(written != dataSize){
    KERROR("COULD NOT WRITE DATA TO %s written %d of %d ",handle->path,*outBytesWritten,dataSize);
    return KFILE_RET_ERROR;
  }

  return KFILE_RET_SUCCESS;
}

