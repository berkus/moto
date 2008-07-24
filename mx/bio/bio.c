/*=============================================================================

   Copyright (C) 2000 David Hakim.
   This file is part of the Codex C Library.

   The Codex C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Codex C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Codex C Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

==============================================================================*/

#include "motoBio.h"
#include <string.h>
#include "memsw.h"
#include "exception.h"
#include "excpfn.h"
#include "mxstring.h"
#include <openssl/buffer.h>


void
motoBio_init(Bio *this, BIO_METHOD *type)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  this->obj    = BIO_new(type);
  this->buffer = 1024;
  this->cipher = EVP_bf_cfb();
  this->digest = EVP_sha1();
  this->file   = NULL;
  this->mode   = NULL;
  this->key    = NULL;
  this->md     = NULL;
  this->chain  = vec_create(0, NULL);
}


Bio *
motoBio_createDefault()
{
  Bio *this = (Bio*)malloc(sizeof(Bio));
  motoBio_init(this, BIO_s_null());
  return this; 
}


Bio *
motoBio_createNew(int type)
{
  Bio *this = (Bio*)malloc(sizeof(Bio));

  switch(type){
    case MB_BASE64:
      motoBio_init(this, BIO_f_base64());
      BIO_set_flags(this->obj, BIO_FLAGS_BASE64_NO_NL);
      break;

    case MB_BUFFER:
      motoBio_init(this, BIO_f_buffer());
      break;

    case MB_CIPHER:
      motoBio_init(this, BIO_f_cipher());
      break;

    case MB_DIGEST:
      motoBio_init(this, BIO_f_md());
      break;

    case MB_NULLF:
      motoBio_init(this, BIO_f_null());
      break;

    case MB_MEM:
      motoBio_init(this, BIO_s_mem());
      BIO_set_close(this->obj, BIO_CLOSE);
      break;

    case MB_FSTREAM:
      motoBio_init(this, BIO_s_file());
      BIO_set_close(this->obj, BIO_CLOSE);
      break;

    case MB_NULLS:
    default:
      motoBio_init(this, BIO_s_null()); 
  }

  return this;
}


void
motoBio_free(Bio *this)
{
  if (this != NULL){
    BIO_vfree(this->obj);
    if (this->md != NULL)
      free(this->md);

    free(this);
  }
}


void
motoBio_push(Bio *this, Bio *bio)
{
  if ((this == NULL) || (bio == NULL))
    THROW_D("NullPointerException");

  this->obj = BIO_push(this->obj, bio->obj);
  vec_add(this->chain, bio);
}


void
motoBio_pop(Bio *this, Bio *bio)
{
  if ((this == NULL) || (bio == NULL))
    THROW_D("NullPointerException");

  this->obj = BIO_pop(bio->obj);
  vec_remove(this->chain, bio);
}


Bio *
motoBio_popLast(Bio *this)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  int size = vec_size(this->chain)-1;
  Bio *tmp = (Bio*)vec_get(this->chain, size);

  this->obj = BIO_pop(tmp->obj);
  if (vec_removeAt(this->chain, size) == NULL){
    THROW("ArrayBoundsException", "No Bio chain exists");
  }

  return tmp;
}


Bio *
motoBio_popBioAt(Bio *this, int index)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  Bio * tmp = (Bio*)vec_get(this->chain, index);

  if (tmp == NULL){
    THROW_D("NoSuchElementException");
  }

  this->obj = BIO_pop(tmp->obj);
  vec_removeAt(this->chain, index);

  return tmp;
}


Bio *
motoBio_getBioAt(Bio *this, int index)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  Bio * tmp = (Bio*)vec_get(this->chain, index);

  if (tmp == NULL){
    THROW_D("NoSuchElementException");
  }

  return tmp;
}


void
motoBio_setCipher(Bio *this, int type)
{
  EVP_CIPHER *cipher = NULL;

  if (this == NULL)
    THROW_D("NullPointerException");

  if (BIO_method_type(this->obj) == BIO_TYPE_CIPHER){
    switch (type){
      case MB_DES:
        cipher = EVP_des_cfb();
        break;

      case MB_DES2:
        cipher = EVP_des_ede_cfb();
        break;

      case MB_DES3:
        cipher = EVP_des_ede3_cfb();
        break;

      case MB_RC4:
        cipher = EVP_rc4();
        break;

      case MB_IDEA:
        cipher = EVP_idea_cfb();
        break;

      case MB_RC2:
        cipher = EVP_rc2_cfb();
        break;

      case MB_BLOWFISH:
        cipher =  EVP_bf_cfb();
        break;

      case MB_CAST5:
        cipher = EVP_cast5_cfb();
        break;

      case MB_RC5:
        cipher = EVP_rc5_32_12_16_cfb();
        break;

      default:
        THROW("IllegalArgumentException", "Invalid CIPHER type");
        break;
    }

    this->cipher = cipher;
  }
  else{
    THROW("UnsupportedOperationException", "Bio is not a CIPHER bio");
  }
}


void
motoBio_setDigest(Bio *this, int type)
{
  EVP_MD *digest = NULL;

  if (this == NULL)
    THROW_D("NullPointerException");

  if (BIO_method_type(this->obj) == BIO_TYPE_MD){
    switch (type){
      case MB_MD2:
        digest = EVP_md2();
        break;

      case MB_MD5:
        digest = EVP_md5();
        break;

      case MB_SHA:
        digest = EVP_sha();
        break;

      case MB_SHA1:
        digest = EVP_sha1();
        break;

      case MB_DSS:
        digest = EVP_dss();
        break;

      case MB_DSS1:
        digest = EVP_dss1();
        break;

      case MB_MDC2:
        digest = EVP_mdc2();
        break;

      case MB_RIPE160:
        digest = EVP_ripemd160();
        break;

      default:
        THROW("IllegalArgumentException", "Invalid DIGEST type"); 
        break;
    }

    this->digest = digest;
  }
  else{
    THROW("UnsupportedOperationException", "Bio is not a DIGEST bio");
  }
}


void
motoBio_setFile(Bio *this, char *file, int mode)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  if (BIO_method_type(this->obj) == BIO_TYPE_FILE){
    this->file = file;

    switch(mode){
      case FS_READ:
        this->mode = "r";
        break;

      case FS_WRITE:
        this->mode = "w";
        break;

      case FS_APPEND:
        this->mode = "a+";
        break;

      case FS_RW:
        this->mode = "r+";
        break;

      default:
        this->mode = "r+"; 
    }
  }
  else{
    THROW("UnsupportedOperationException", "Bio is not a FILE bio");
  }
}


void
motoBio_setBufferSize(Bio *this, int size)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  if (size > 1024){
    this->buffer = size;
  }
}


int
motoBio_getBufferSize(Bio *this)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  return this->buffer;
}


void
motoBio_setKey(Bio *this, const unsigned char *key)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  this->key = key;
}


unsigned char *
motoBio_getKey(Bio *this)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  return (unsigned char*)this->key;
}


int 
motoBio_write(Bio *this, const char *data)
{
  int t = 0;

  if (this == NULL)
    THROW_D("NullPointerException");

  if (data == NULL)
    THROW("NullPointerException", "Cannot write a null value");

  switch (BIO_method_type(this->obj)){
    case BIO_TYPE_BUFFER:
    case BIO_TYPE_MEM:
      this->bytesRead = BIO_puts(this->obj, data);
      break;

    /* Can't get Digest Bio to work, so we will use the 
       EVP API directly and circumvent the BIO API */
    case BIO_TYPE_MD:
      {
        EVP_MD_CTX ctx;
        this->md = (unsigned char*)malloc(EVP_MAX_MD_SIZE);
        EVP_DigestInit(&ctx, this->digest);
        EVP_DigestUpdate(&ctx, (char*)data, strlen(data));
        EVP_DigestFinal(&ctx, this->md, &this->bytesRead);
        this->mdLen = this->bytesRead;
      }

      break;

    case BIO_TYPE_FILE:
      if (this->file == NULL)
        THROW("NullPointerException", "You must call setFile(file, mode) before using an FSTREAM Bio");

      if (strcmp(this->mode, "w") == 0){
        if (BIO_write_filename(this->obj, this->file) == 0)
          THROW_D("IOException");
      }
      else if (strcmp(this->mode, "a+") == 0){
        if (BIO_append_filename(this->obj, this->file) == 0)
          THROW_D("IOException");
      }
      else if (strcmp(this->mode, "r") == 0){
        THROW("IOException", "Cannot write to file in read-only mode");
      }
      else{
        if (BIO_rw_filename(this->obj, this->file) == 0)
          THROW("IOException", "File %s does not exist", this->file);
      }

      // Fall through

    case BIO_TYPE_BASE64:
    case BIO_TYPE_CIPHER:
    default:
      {
        /* Make sure we have a source/sink in the chain or 
           else we will probably cause a core dump */
        BIO *btmp = this->obj;
        do {
          btmp = BIO_find_type(btmp, BIO_TYPE_SOURCE_SINK);

          if (btmp == NULL) {
            THROW("IOException", "You must chain a source/sink Bio to write to a filter Bio");
          }

          btmp = BIO_next(btmp);
        } while(btmp);

        this->bytesRead = BIO_write(this->obj, data, strlen(data));
        t = 1;
      }
  }

  if (this->bytesRead <= 0){
    /* This determines if we got an error or not */
    if (!BIO_should_retry(this->obj)){
      this->bytesRead = -1;
    }
  }

  if (this->bytesRead == -2){
    THROW("UnsupportedOperationException", "This Bio type does not support <%s>", t==0?"puts":"write");
  }

  return this->bytesRead;
}


int
motoBio_writeEnc(Bio *this, const char *data, const unsigned char *key)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  if (BIO_method_type(this->obj) == BIO_TYPE_CIPHER){
      unsigned char iv[EVP_MAX_IV_LENGTH], k[EVP_MAX_KEY_LENGTH];
      memset(k, '0', EVP_MAX_KEY_LENGTH);
      memmove(iv, key, sizeof(iv));

      this->key = iv;
      BIO_set_cipher(this->obj, this->cipher, k, iv, 1);
      this->bytesRead = motoBio_write(this, data);
  }
  else{
    THROW("UnsupportedOperationException", "Only CIPHER bios can encrypt data");
  }

  return this->bytesRead;
}


UnArray *
motoBio_read(Bio *this)
{
  UnArray *arr;
  int t = 0;
  unsigned char data[this->buffer];

  if (this == NULL)
    THROW_D("NullPointerException");

  memset(data, '\0', this->buffer);

  switch (BIO_method_type(this->obj)){
    case BIO_TYPE_BUFFER:
      /* Buffering BIOs implement BIO_gets() by calling BIO_read() on the 
         next BIO in the chain. If there is no next BIO, of course the 
         OpenSSL libraries don't catch it so we need to */

      if ((BIO*)this->obj->next_bio == NULL)
        THROW_D("IOException");

      this->bytesRead = BIO_gets(this->obj, data, BIO_pending(this->obj));
      break;

    case BIO_TYPE_MEM:
      /* Create binary safe return of data.
         BIO_gets for MEM BIOs is not binary 
         safe */
      {
        BUF_MEM *bm = (BUF_MEM*)this->obj->ptr;
        this->bytesRead = BIO_pending(this->obj);
        memcpy(data, (unsigned char*)bm->data, this->bytesRead);
      }

      break;

    case BIO_TYPE_CIPHER:
      if (this->key != NULL){
        unsigned char iv[EVP_MAX_IV_LENGTH], k[EVP_MAX_KEY_LENGTH];
        memset(k, '0', EVP_MAX_KEY_LENGTH);
        memmove(iv, this->key, sizeof(iv));

        BIO_set_cipher(this->obj, this->cipher, k, iv, 0);
      }
      else{
        THROW("NullPointerException", "You must first encrypt data before decrypting");
      }

      this->bytesRead = BIO_read(this->obj, data, BIO_pending(this->obj));
      t = 1;
      break;

    case BIO_TYPE_MD:
      {
        char t[3] = {'\0'};
        int i;

        if (this->mdLen == 0){
          int pending     = BIO_pending((BIO*)this->obj->prev_bio);
          this->bytesRead = BIO_read(this->obj, data, pending);

          /* No data was passed from BIO chain. Some BIOs
             do nothing on flush so we need to go get the 
             data directly */
          if (this->bytesRead <= 0 && pending > 0){
            this->bytesRead = BIO_read((BIO*)this->obj->prev_bio, data, pending);
            motoBio_write(this, data);
            memset(data, '\0', this->buffer);
          }
        }

        for (i = 0; i < this->mdLen; i++){
          sprintf(t, "%02x", this->md[i]);
          strncat(data, t, 2);
        }

        this->bytesRead = strlen(data);
        break;
      }

    case BIO_TYPE_FILE:
      if (BIO_rw_filename(this->obj, this->file) == 0)
        THROW("IOException", "File %s does not exist", this->file);

      this->bytesRead = BIO_gets(this->obj, data, this->buffer-1);
      break;

    case BIO_TYPE_BASE64:
    default:
      this->bytesRead = BIO_read(this->obj, data, BIO_pending(this->obj));
      t = 1;
  }

  if (this->bytesRead <= 0){
    /* This determines if we got an error or not */
    if (!BIO_should_retry(this->obj)){
      THROW("IOException", "Error occurred reading from Bio"); 
    }
  }

  if (this->bytesRead == -2){
    THROW("UnsupportedOperationException", "This Bio type does not support <%s>", t==0?"gets":"read");
  }

  arr = arr_create(1, 0, 0, (int*)&this->bytesRead, ARR_BYTE_TYPE);
  memcpy(&arr->ya.data[0], data, this->bytesRead);
  return arr;
}


int
motoBio_getBytesRead(Bio *this)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  return this->bytesRead;
}


void
motoBio_flush(Bio *this)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  if (BIO_method_type(this->obj) == BIO_TYPE_MD){
    if (motoBio_size(this) > 0){
      Bio *bio = motoBio_getBioAt(this, 0);

      if (BIO_method_type(bio->obj) == BIO_TYPE_CIPHER){
        motoBio_writeEnc(bio, yarr_toString(motoBio_read(this)), bio->key);
      }
      else{
        motoBio_write(bio, yarr_toString(motoBio_read(this)));
      }
    }
  }
  else{
    BIO_flush(this->obj);
  }
}


void
motoBio_reset(Bio *this)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  BIO_reset(this->obj);
}


int
motoBio_eof(Bio *this)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  return BIO_eof(this->obj);
}


int
motoBio_size(Bio *this)
{
  if (this == NULL)
    THROW_D("NullPointerException");

  return vec_size(this->chain);
}


void
motoBio_createChain(Bio *this, UnArray *types)
{
  int x;

  if ((this == NULL) || (types == NULL))
    THROW_D("NullPointerException");

  TRY {
    for (x = 0; x < arr_length(types); x++){
      Bio *bio = motoBio_createNew(*isub(types, x));
      motoBio_push(this, bio);
    }
  }
  CATCH_ALL {
  }
  END_TRY
}


