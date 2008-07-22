#ifndef __PRIMITIVES_H
#define __PRIMITIVES_H

#include "integer.h"

typedef struct inum {
   int32_t value;
} Integer;

typedef struct lnum {
   int64_t value;
} Long;

typedef struct fnum {
   float value;
} Float;

typedef struct dnum {
   double value;
} Double;

typedef struct booltype {
   char value;
} Boolean;

typedef struct chartype {
   char value;
} Character;

Integer *inum_create(int32_t n);
void inum_free(Integer *p);
int32_t inum_getValue(Integer *p);
void inum_setValue(Integer *p,int32_t n);
int char_toInt(char c);

Long *lnum_create(int64_t n);
void lnum_free(Long *p);
int64_t lnum_getValue(Long *p);
void lnum_setValue(Long *p,int64_t n);

Float *fnum_create(float n);
void fnum_free(Float *p);
float fnum_getValue(Float *p);
void fnum_setValue(Float *p,float n);

Double *dnum_create(double n);
void dnum_free(Double *p);
double dnum_getValue(Double *p);
void dnum_setValue(Double* p, double n);

Boolean *bool_create(char n);
void bool_free(Boolean *p);
char bool_getValue(Boolean *p);
void bool_setValue(Boolean *p,char n);

Character *char_create(char n);
void char_free(Character *p);
char char_getValue(Character *p);
void char_setValue(Character *p, char n);

#endif
