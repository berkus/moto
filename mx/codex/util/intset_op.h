#include "intset.h"

IntSet * iset_pluseq(IntSet * lval, int i);
IntSet * iset_pluseq2(IntSet * lval, IntSet * rval);
IntSet * iset_minuseq(IntSet * lval, int i);
IntSet * iset_minuseq2(IntSet * lval, IntSet * rval);
int iset_notequal(IntSet * i1, IntSet * i2);
int iset_equal(IntSet * i1, IntSet * i2);
int iset_gt(IntSet * i1, IntSet * i2);
int iset_gte(IntSet * i1, IntSet * i2);
int iset_lt(IntSet * i1, IntSet * i2);
int iset_lte(IntSet * i1, IntSet * i2);
IntSet * iset_plus(IntSet * i1, IntSet * i2);
IntSet * iset_minus(IntSet * i1, IntSet * i2);
