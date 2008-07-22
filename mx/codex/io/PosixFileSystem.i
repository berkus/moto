Extension: PosixFileSystem

Include: "pfs.h"

Interface:

PosixFileSystem PosixFileSystem::PosixFileSystem() =>
	PosixFileSystem* localFileSystem();

/** Gets a reference to the 'local file system' **/	
PosixFileSystem localFileSystem() =>
	PosixFileSystem* localFileSystem();
	
/** Get the permissions on the file specified by path **/
int PosixFileSystem::getMode(String path) =>
    int pfs_getMode(PosixFileSystem* this,char* path);
    
/** Get the owner of the file specified by path **/
int PosixFileSystem::getOwner(String path) =>
    int pfs_getOwner(PosixFileSystem* this,char* path);

/** Get the group owner on the file specified by path **/
int PosixFileSystem::getGroupOwner(String path) =>
    int pfs_getGroupOwner(PosixFileSystem* this,char* path);

/** Get the size in bytes of the file specified by path **/
int PosixFileSystem::getSize(String path) =>
    int pfs_getSize(PosixFileSystem* this,char* path);
    
/** Get the permissions on the file specified by path **/
Date PosixFileSystem::getLastAccessTime(String path) =>
    struct tm* pfs_getLastAccessTime(PosixFileSystem* this,char* path);
    
/** Get the owner of the file specified by path **/
Date PosixFileSystem::getLastModificationTime(String path) =>
    struct tm* pfs_getLastModificationTime(PosixFileSystem* this,char* path);

/** Get the group owner on the file specified by path **/
Date PosixFileSystem::getLastChangeTime(String path) =>
    struct tm* pfs_getLastChangeTime(PosixFileSystem* this,char* path);
    
/** Set the permissions on the file specified by path **/ 
void PosixFileSystem::setMode(String path,int mode) =>
	void pfs_setMode(PosixFileSystem* this,char* path,int mode); 

/** Set the group owner on the file specified by path **/ 
void PosixFileSystem::setGroupOwner(String path, int group) =>	  
	void pfs_setGroupOwner(PosixFileSystem* this,char* path, int group);

/** Set the owner on the file specified by path **/
void PosixFileSystem::setOwner(String path,int owner) =>
	void pfs_setOwner(PosixFileSystem* this,char* path,int owner);

/** Is the file at path a symbolic link ? **/
boolean PosixFileSystem::isSymbolicLink(String path) =>
	char pfs_isSymbolicLink(PosixFileSystem* this,char* path);

/** Is the file at path a regular file ? **/
boolean PosixFileSystem::isRegularFile(String path) =>
	char pfs_isRegularFile(PosixFileSystem* this,char* path);

/** Is the file at path a directory ? **/	
boolean PosixFileSystem::isDirectory(String path) =>
	char pfs_isDirectory(PosixFileSystem* this,char* path);

/** Is the file at path a character device ? **/
boolean PosixFileSystem::isCharacterDevice(String path) =>
	char pfs_isCharacterDevice(PosixFileSystem* this,char* path);

/** Is the file at path a block device ? **/
boolean PosixFileSystem::isBlockDevice(String path) =>
	char pfs_isBlockDevice(PosixFileSystem* this,char* path);

/** Is the file at path first in first out ? **/
boolean PosixFileSystem::isFirstInFirstOut(String path) =>
	char pfs_isFirstInFirstOut(PosixFileSystem* this,char* path);

/** Is the file at path a socket ? **/
boolean PosixFileSystem::isSocket(String path) =>
	char pfs_isSocket(PosixFileSystem* this,char* path);

/** Create a directory named path **/
void PosixFileSystem::createDirectory(String path) =>
	void pfs_createDirectory(PosixFileSystem* this,char* path);

/** Create a symbolic link to the file named 'file' at 'at' **/
void PosixFileSystem::createSymbolicLink(String file,String at) =>
	void pfs_createSymbolicLink(PosixFileSystem* this,char* from,char* at);

/** Removes the directory at path. Only empty directories can be removed **/	
void PosixFileSystem::removeDirectory(String path) =>
	void pfs_removeDirectory(PosixFileSystem* this,char* path);

/** Remove the file at path. This file cannot be a directory **/	
void PosixFileSystem::removeFile(String path) =>
	void pfs_removeFile(PosixFileSystem* this,char* path);

/** Get the path to the file pointed to by the symbolic link at 'linkpath' **/
String PosixFileSystem::pathForSymbolicLink(String linkpath) =>
	char* pfs_pathForSymbolicLink(PosixFileSystem* this,char* linkpath);

/** Get the canonical path for the specified path **/
String PosixFileSystem::canonicalPathForPath(String path) =>
    char* pfs_canonicalPathForPath(PosixFileSystem* this,char* path);
    
/** Get an enumeration of the filenames inside the specified directory **/
Enumeration PosixFileSystem::getEntries(String path) =>
	Enumeration* pfs_getEntries(PosixFileSystem* this,char* path);
