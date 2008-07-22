#include "intset.h"

IntSet * iset_plus(IntSet * i1, IntSet * i2) {

   IntSet *p;

   p = iset_clone(i1);
   iset_addSet(p, i2);

   return p;
		
}

IntSet * iset_minus(IntSet * i1, IntSet * i2) {

   IntSet *p;

   p = iset_clone(i1);
   iset_removeSet(p, i2);

   return p;
		
}

IntSet * iset_pluseq(IntSet * lval, int i) {

   IntSet *p;

   p = iset_clone(lval);
   iset_add(p, i);

   return p;
		
}

IntSet * iset_pluseq2(IntSet * lval, IntSet * rval) {
	return iset_plus(lval, rval);
}

IntSet * iset_minuseq(IntSet * lval, int i) {

   IntSet *p;

   p = iset_clone(lval);
   iset_remove(p, i);

   return p;
		
}

IntSet * iset_minuseq2(IntSet * lval, IntSet * rval) {
	return iset_minus(lval, rval);
}

static int iset_comp2(void* p1, void* p2){

   IntSet* is1 = (IntSet*)p1;
   IntSet* is2 = (IntSet*)p2;

   if (iset_size(is1) > iset_size(is2)) {
      return 1;
   } else if (iset_size(is1) < iset_size(is2)) {
      return -1;
   }
  
   return 0;
		   
}

int iset_notequal(IntSet * i1, IntSet * i2) {
	return (iset_comp(i1, i2) == -1) ? 1 : 0;
}

int iset_equal(IntSet * i1, IntSet * i2) {
	return (iset_comp(i1, i2) == 0) ? 1 : 0;
}

int iset_gt(IntSet * i1, IntSet * i2) {
	return (iset_comp2(i1, i2) == 1) ? 1 : 0;
}

int iset_lt(IntSet * i1, IntSet * i2) {
	return (iset_comp2(i1, i2) == -1) ? 1 : 0;
}

int iset_gte(IntSet * i1, IntSet * i2) {

	int ret = iset_comp2(i1, i2);
	if ((ret == 1) || (ret == 0)) {
		return 1;
	} else {
		return 0;
	}
		
}

int iset_lte(IntSet * i1, IntSet * i2) {

	int ret = iset_comp2(i1, i2);
	if ((ret == -1) || (ret == 0)) {
		return 1;
	} else {
		return 0;
	}
		
}

