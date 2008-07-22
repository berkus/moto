#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include "enumeration.h"

typedef void* PosixFileSystem;

/*
static void estat(const char* path, struct stat* sb);
static void echmod(const char* path, int mode);
static void echown(const char* path, int uid, int gid);
static void emkdir(const char* path, int mode);
static void esymlink(const char* from, const char* at);
static void ermdir(const char* path);
static void eremove(const char* path);
static void ereadlink(const char* path, char *buf, int bufsiz);
static DIR* eopendir(const char* path);
*/


PosixFileSystem* localFileSystem();

int pfs_getMode(PosixFileSystem* pfs, char* path);   
int pfs_getGroupOwner(PosixFileSystem* pfs, char* path); 
int pfs_getOwner(PosixFileSystem* pfs, char* path); 
int pfs_getSize(PosixFileSystem* pfs, char* path);

struct tm* pfs_getLastAccessTime(PosixFileSystem* pfs, char* path);
struct tm* pfs_getLastModificationTime(PosixFileSystem* pfs, char* path);
struct tm* pfs_getLastChangeTime(PosixFileSystem* pfs, char* path);

char pfs_isSymbolicLink(PosixFileSystem* pfs, char* path);
char pfs_isRegularFile(PosixFileSystem* pfs, char* path);
char pfs_isDirectory(PosixFileSystem* pfs, char* path);
char pfs_isCharacterDevice(PosixFileSystem* pfs, char* path);
char pfs_isBlockDevice(PosixFileSystem* pfs, char* path);
char pfs_isFirstInFirstOut(PosixFileSystem* pfs, char* path);
char pfs_isSocket(PosixFileSystem* pfs, char* path);

void pfs_setMode(PosixFileSystem* pfs, char* path,int mode);  
void pfs_setGroupOwner(PosixFileSystem* pfs, char* path, int group); 
void pfs_setOwner(PosixFileSystem* pfs, char* path,int owner);

void pfs_createDirectory(PosixFileSystem* pfs, char* path);
void pfs_createSymbolicLink(PosixFileSystem* pfs, char* from,char* at);

void pfs_removeDirectory(PosixFileSystem* pfs, char* path);
void pfs_removeFile(PosixFileSystem* pfs, char* path);

char* pfs_pathForSymbolicLink(PosixFileSystem* pfs, char* linkpath);
char* pfs_canonicalPathForPath(PosixFileSystem* pfs, char* path);

Enumeration* pfs_getEntries(PosixFileSystem* pfs, char* path);
