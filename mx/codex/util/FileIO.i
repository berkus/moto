Extension: FileIO

Include: "fileio.h"
Include: "fiowrap.h"

Interface:

/** Returns true if the specified path exists **/
boolean existsFile(String path) =>
	int fio_exists(const char *filename);

/** Returns the contents of the file at the specified path as a String **/
tracked String getFile(String path) =>
	char *fiowrap_get(char* path);

/** Returns the contents of the file at the specified path as a byte[] **/
tracked byte[] getFileBytes(String path) =>
	UnArray *fiowrap_get_yarr(char* path);
	
/** Writes the String <i>contents</i> to a file at the specified path **/
void putFile(String path,String contents) =>
	void fio_put(char* path, char* contents);

/** Appends the String <i>contents</i> to a file at the specified path **/
void appendFile(String path, String contents) =>
	int fio_append(const char *filename, const char *text);

/** Writes the byte[] <i>contents</i> to a file at the specified path **/
void putFile(String path,byte[] contents) =>
	void fio_put_yarr(char* path, UnArray* contents);

/** Appends the byte[] <i>contents</i> to a file at the specified path **/
void appendFile(String path, byte[] contents) =>
	int fio_append_yarr( char *path, UnArray* contents);
	
/** <b>depricated</b> Writes out the <i>length</i> bytes of the Object <i>contents</i> to the file
at the specified path **/
int putFileBytes(String path, byte[] data, int length) =>
	int fio_put_yarr_d( char *path, UnArray* contents, int length);

/** <b>depricated</b> Appends <i>length</i> bytes of the Object <i>contents</i> to the file
at the specified path **/
int appendFileBytes(String path, byte[] contents, int length) =>
	int fio_append_yarr_d( char *path, UnArray* contents, int length);

