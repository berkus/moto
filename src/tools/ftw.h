#ifndef FTWIMPL
#define FTWIMPL

/*
This is from netlib, package stree
http://elib.zib.de/netlib/access/

autoconf awared caolan@skynet.ie

Original Header follows...
*/

/*
Last changed Sun Aug  9 18:59:40 EDT 1987.
This was written at AT&T Bell Laboratories by David Gay and Eric Grosse for
personal use.  It is not copyrighted, and people may copy it at will
if they retain this notice.  This program is not guaranteed either!  But 
since we depend on it for our own backups, we would welcome bug reports
(to research!dmg or research!ehg).
*/


#include <sys/types.h>
#include <sys/stat.h>

/*
 *	Codes for the third argument to the user-supplied function
 *	which is passed as the second argument to ftw...
 */

#define	FTW_F	0	/* file */
#define	FTW_D	1	/* directory */
#define	FTW_DNR	2	/* directory without read permission */
#define	FTW_NS	3	/* unknown type, stat failed */
#define FTW_DP	4	/* directory, postorder visit */
#define FTW_SL	5	/* symbolic link */
#define FTW_NSL 6	/* stat failed (errno = ENOENT) on symbolic link */

/*	Values the user-supplied function may wish to assign to
	component quit of struct FTW...
 */

#define FTW_SKD 1	/* skip this directory (2nd par = FTW_D) */
#define FTW_SKR 2	/* skip rest of current directory */
#define FTW_FOLLOW 3	/* follow symbolic link */

struct FTW { int quit, base, level;
#ifndef FTW_more_to_come
	};
#endif

int ftw (const char *path, int (*fn)(const char *file,const struct stat *sb,
int flag), int depth);
#endif 
