#include "mx.h"
#include "err.h"

#include "primitives.h"

Integer *inum_create(int32_t n) {
   Integer *p = malloc(sizeof(Integer));
   p->value = n;
   return p;
}
void inum_free(Integer *p) {
   if (p) {
      free(p);
   }
}
int32_t inum_getValue(Integer *p) {
   return p->value;
}
void inum_setValue(Integer *p,int32_t n){
   p->value = n;
}
int char_toInt(char c) {
   int r = 0;
   if (c >= 48 && c <= 57) {
      r = (int)c - 48;
   }
   return r;
}

Long *lnum_create(int64_t n) {
   Long *p = malloc(sizeof(Long));
   err_assert(p != NULL, err_e("AllocationFailure"));
   p->value = n;
   return p;
}
void lnum_free(Long *p) {
   free(p);
}
int64_t lnum_getValue(Long *p) {
   return p->value;
}

void lnum_setValue(Long *p,int64_t n){
   p->value = n;
}

Float *fnum_create(float n) {
   Float *p = malloc(sizeof(Float));
   err_assert(p != NULL, err_e("AllocationFailure"));
   p->value = n;
   return p;
}
void fnum_free(Float *p) {
   free(p);
}
float fnum_getValue(Float *p) {
   return p->value;
}
void fnum_setValue(Float *p,float n){
   p->value = n;
}
Double *dnum_create(double n) {
   Double *p = malloc(sizeof(Double));
   err_assert(p != NULL, err_e("AllocationFailure"));
   p->value = n;
   return p;
}
void dnum_free(Double *p) {
   free(p);
}
double dnum_getValue(Double *p) {
   return p->value;
}
void dnum_setValue(Double* p, double n) {
   p->value = n;
}

Boolean *bool_create(char n) {
   Boolean *p = malloc(sizeof(Boolean));
   err_assert(p != NULL, err_e("AllocationFailure"));
   p->value = n;
   return p;
}
void bool_free(Boolean *p) {
   free(p);
}
char bool_getValue(Boolean *p) {
   return p->value;
}

void bool_setValue(Boolean *p,char n){
   p->value=n;
}
   
Character *char_create(char n) {
   Character *p = malloc(sizeof(Character));
   err_assert(p != NULL, err_e("AllocationFailure"));
   p->value = n;
   return p;
}
void char_free(Character *p) {
   free(p);
}
char char_getValue(Character *p) {
   return p->value;
}
void char_setValue(Character *p, char n){
   p->value=n;
}  
