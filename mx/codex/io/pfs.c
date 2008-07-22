#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "pfs.h"
#include "excpfn.h"
#include "exception.h"
#include "vector.h"
#include "memsw.h"

#ifdef MAXPATHLEN
   static const int MAX_PATH_LENGTH = MAXPATHLEN;
#else
   static const int MAX_PATH_LENGTH = PATH_MAX;
#endif

char *erealpath(const char *path, char *resolved_path){
   if((resolved_path = realpath(path,resolved_path)) == NULL){
		switch (errno) {
			case ENOENT:
			case ENOTDIR:
			case ELOOP:
			case ENAMETOOLONG:
			case EINVAL:
				THROW("FileNotFoundException", "%s", path);
				break;
			case EACCES:
				THROW("SecurityException", "Cannot access %s", path);
				break;
		}
   }
   return resolved_path;
}

static void elstat(const char* path, struct stat* sb){
   if(lstat(path,sb) == -1){
		switch (errno) {
			case ENOENT:
			case ENOTDIR:
			case ELOOP:
			case EFAULT:
			case ENAMETOOLONG:
				THROW("FileNotFoundException", "%s", path);
				break;
			case EACCES:
				THROW("SecurityException", "Cannot access %s", path);
				break;
			case ENOMEM:
				THROW("AllocationFailureException", "%s", path);
				break;
		}
   }
}

static void echmod(const char* path, int mode){
   if(chmod(path,mode) == -1){
		switch (errno) {
			case ENOTDIR:
			case ENAMETOOLONG:
			case ENOENT:
			case ELOOP:
			case EFAULT:
				THROW("FileNotFoundException", "%s", path);
				break;
			case EPERM:
			case EROFS:
				THROW("SecurityException", "Cannot chmod %s", path);
				break;
			case EIO:
				THROW("IOException", "%s", path);
				break;
		}
   }
}

static void echown(const char* path, int uid, int gid){
   if(chown(path,uid,gid) == -1){
		switch (errno) {
			case ENOENT:
			case ENOTDIR:
			case ELOOP:
			case EFAULT:
			case ENAMETOOLONG:
				THROW("FileNotFoundException", "%s", path);
				break;
			case EACCES:
			case EPERM:
			case EROFS:
				THROW("SecurityException", "Cannot chown %s", path);
				break;
			case EIO:
				THROW("IOException", "%s", path);
				break;
		}
   }
}

static void emkdir(const char* path, int mode){
   if(mkdir(path,mode) == -1){
		switch (errno) {
			case ENOTDIR:
			case ENAMETOOLONG:
			case ENOENT:
			case ELOOP:
			case EFAULT:
				THROW("FileNotFoundException", "Parent of %s not found", path);
				break;
			case EACCES:
			case EROFS:
			case EDQUOT:
				THROW("SecurityException", "Cannot mkdir %s", path);
				break;
			case ENOSPC:
			case EEXIST:
			case EIO:
				THROW("IOException", "%s", path);
				break;
		}
   }
}

static void esymlink(const char* from, const char* at){
   if(symlink(from,at) == -1){
		switch (errno) {
			case ENOTDIR :
			case ENAMETOOLONG :
			case ENOENT :
			case ELOOP :
			case EFAULT :
				THROW("FileNotFoundException", "%s not found", from);
				break;
			case EACCES:
			case EROFS:
			case EDQUOT:
				THROW("SecurityException", "Cannot symlink %s", from);
				break;
			case ENOSPC:
			case EEXIST:
			case EIO:
				THROW("IOException", "%s", from);
				break;
		}
   }
}

static void ermdir(const char* path){
   if(rmdir(path) == -1){
		switch (errno) {
			case ENOTDIR:
			case ENAMETOOLONG:
			case ENOENT:
			case ELOOP:
			case EFAULT:
				THROW("FileNotFoundException", "%s not found", path);
				break;
			case EACCES:
			case EROFS:
			case EPERM:
			case ENOTEMPTY:
				THROW("SecurityException", "Cannot remove directory %s", path);
				break;
			case EIO:
			case EBUSY:
				THROW("IOException", "%s", path);
				break;
		}
   }
}

static void eremove(const char* path){
   if(remove(path) == -1){
		switch (errno) {
			case ENOTDIR:
			case ENAMETOOLONG:
			case ENOENT:
			case ELOOP:
			case EFAULT:
				THROW("FileNotFoundException", "%s not found", path);
				break;
			case EACCES:
			case EROFS:
			case EPERM:
				THROW("SecurityException", "Cannot remove %s", path);
				break;
			case EBUSY:
			case EIO:
				THROW("IOException", "%s", path);
				break;
		}
   }
}

static void ereadlink(const char* path, char *buf, int bufsiz){
   if(readlink(path,buf,bufsiz) == -1){
		switch (errno) {
			case ENOTDIR:
			case ENAMETOOLONG:
			case ENOENT:
			case ELOOP:
			case EFAULT:
			case EINVAL:
				THROW("FileNotFoundException", "Symbolic link %s not found", path);
				break;
			case EACCES:
				THROW("SecurityException", "Cannot read %s", path);
				break;
			case EIO:
				THROW("IOException", "%s", path);
				break;
		}
   }
}

static DIR* eopendir(const char* path){
   DIR* dirp;
   if((dirp=opendir(path)) == NULL){
		switch (errno) {
			case ENOENT:
			case ENOTDIR:
				THROW("FileNotFoundException", "Directory %s not found", path);
				break;
			case EACCES:
			case EMFILE:
			case ENFILE:
				THROW("SecurityException", "Cannot open %s", path);
				break;
			case ENOMEM:
				THROW("IOException", "%s", path);
				break;
		}
   }
   return dirp;
}

static int pfs;

PosixFileSystem* 
localFileSystem(){
   return (void*)&pfs;
}

int 
pfs_getMode(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return (int)sb.st_mode; }   
int 
pfs_getGroupOwner(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return (int)sb.st_gid; } 
int 
pfs_getOwner(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return (int)sb.st_uid; } 
int 
pfs_getSize(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return (int)sb.st_size; }

struct tm* 
pfs_getLastAccessTime(PosixFileSystem* pfs, char* path){
   struct tm *tmp,* d = (struct tm*)emalloc(sizeof(struct tm));
   struct stat sb; 
   elstat(path,&sb); 
   tmp=localtime(&sb.st_atime);
   
   memcpy(d,tmp,sizeof(struct tm));
   return d; 
}
 
struct tm* 
pfs_getLastModificationTime(PosixFileSystem* pfs, char* path){
   struct tm *tmp,* d = (struct tm*)emalloc(sizeof(struct tm));
   struct stat sb; 
   elstat(path,&sb); 
   tmp=localtime(&sb.st_mtime);
   
   memcpy(d,tmp,sizeof(struct tm));
   return d; 
}
 
struct tm* 
pfs_getLastChangeTime(PosixFileSystem* pfs, char* path){
   struct tm *tmp,* d = (struct tm*)emalloc(sizeof(struct tm));
   struct stat sb; 
   elstat(path,&sb); 
   tmp=localtime(&sb.st_ctime);
   
   memcpy(d,tmp,sizeof(struct tm));
   return d; 
}

char 
pfs_isSymbolicLink(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return S_ISLNK(sb.st_mode); }
char 
pfs_isRegularFile(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return S_ISREG(sb.st_mode); }
char 
pfs_isDirectory(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return S_ISDIR(sb.st_mode); }
char 
pfs_isCharacterDevice(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return S_ISCHR(sb.st_mode); }
char 
pfs_isBlockDevice(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return S_ISBLK(sb.st_mode); }
char 
pfs_isFirstInFirstOut(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return S_ISFIFO(sb.st_mode); }
char 
pfs_isSocket(PosixFileSystem* pfs, char* path){struct stat sb; elstat(path,&sb); return S_ISSOCK(sb.st_mode); }

void 
pfs_setMode(PosixFileSystem* pfs, char* path,int mode){echmod(path, mode); }   
void 
pfs_setGroupOwner(PosixFileSystem* pfs, char* path, int group){ echown(path, -1, group); } 
void 
pfs_setOwner(PosixFileSystem* pfs, char* path,int owner){ echown(path, owner, -1); } 

void
pfs_createDirectory(PosixFileSystem* pfs, char* path){emkdir(path,64*7+32*7+7);}
void
pfs_createSymbolicLink(PosixFileSystem* pfs, char* from,char* at){esymlink(from,at);}

void
pfs_removeDirectory(PosixFileSystem* pfs, char* path){ermdir(path);}
void
pfs_removeFile(PosixFileSystem* pfs, char* path){eremove(path);}

char*
pfs_pathForSymbolicLink(PosixFileSystem* pfs, char* linkpath){
   char* p;
   int path_max;
   
#ifdef PATH_MAX
	path_max = PATH_MAX;
#else
	path_max = pathconf (path, _PC_PATH_MAX);
	if (path_max <= 0)
	  path_max = 4096;
#endif

   p= (char*)ecalloc(path_max+1,1);
   ereadlink(linkpath,p,path_max);
   
   return p;
}

char*
pfs_canonicalPathForPath(PosixFileSystem* pfs, char* path){
   char* p;
   int path_max;
   
#ifdef PATH_MAX
	path_max = PATH_MAX;
#else
	path_max = pathconf (path, _PC_PATH_MAX);
	if (path_max <= 0)
	  path_max = 4096;
#endif

    p= (char*)ecalloc(path_max+1,1);
    p=erealpath(path,p);
    return p;
}

Enumeration*
pfs_getEntries(PosixFileSystem* pfs, char* path){
   DIR* dirp = eopendir(path);
   struct dirent *dp;
   
   Vector* v= vec_createDefault();
   if (dirp) {
      while ((dp = readdir(dirp)) != NULL) {
         vec_add(v,estrdup(dp->d_name)); 
         
      }
      closedir(dirp);
   }
   return vec_elements(v);
}
