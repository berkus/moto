#ifndef __MOTOBIO_H
#define __MOTOBIO_H

#include <openssl/bio.h>
#include <openssl/evp.h>
#include "mxarr.h"
#include "vector.h"

/* BIO types */
#define MB_BASE64    1
#define MB_BUFFER    2
#define MB_CIPHER    3
#define MB_DIGEST    4
#define MB_NULLF     5
#define MB_FSTREAM   6
#define MB_MEM       7
#define MB_NULLS     8

/* CIPHER types */
#define MB_DES       1
#define MB_DES2      2
#define MB_DES3      3
#define MB_RC4       4
#define MB_IDEA      5
#define MB_RC2       6
#define MB_BLOWFISH  7
#define MB_CAST5     8
#define MB_RC5       9

/* MD types */
#define MB_MD2       1
#define MB_MD5       2
#define MB_SHA       3
#define MB_SHA1      4
#define MB_DSS       5
#define MB_DSS1      6
#define MB_MDC2      7
#define MB_RIPE160   8

/* FSTREAM modes */
#define FS_READ      1
#define FS_WRITE     2
#define FS_APPEND    3
#define FS_RW        4

typedef struct {
  BIO *obj;
  EVP_CIPHER *cipher;
  EVP_MD *digest;
  char *file;
  char *mode;
  const unsigned char *key;
  unsigned char *md;
  int mdLen;
  int buffer;
  int bytesRead;
  Vector *chain;
} Bio;


void            motoBio_init(Bio *this, BIO_METHOD *type);
Bio*            motoBio_createDefault();
Bio*            motoBio_createNew(int type);
void            motoBio_free(Bio *bio);
void            motoBio_push(Bio *this, Bio *bio);
void            motoBio_pop(Bio *this, Bio *bio);
Bio*            motoBio_popLast(Bio *this);
Bio*            motoBio_popBioAt(Bio *this, int index);
Bio*            motoBio_getBioAt(Bio *this, int index);
void            motoBio_setCipher(Bio *this, int type);
void            motoBio_setDigest(Bio *this, int type);
void            motoBio_setFile(Bio *this, char *file, int mode);
void            motoBio_setBufferSize(Bio *this, int size);
int             motoBio_getBufferSize(Bio *this);
void            motoBio_setKey(Bio *this, const unsigned char *key);
unsigned char*  motoBio_getKey(Bio *this);
int             motoBio_write(Bio *this, const char *data);
int             motoBio_writeEnc(Bio *this, const char *data, const unsigned char *key);
UnArray*        motoBio_read(Bio *this);
int             motoBio_getBytesRead(Bio *this);
void            motoBio_flush(Bio *this);
void            motoBio_reset(Bio *this);
int             motoBio_eof(Bio *this);
int             motoBio_size(Bio *this);
void            motoBio_createChain(Bio *this, UnArray *types);

#endif

