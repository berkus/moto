/*=============================================================================

	Copyright (C) 2000 Webcodex, Inc.
	This file is part of the Codex C Library.

	The Codex C Library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	The Codex C Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with the Codex C Library; see the file COPYING.	If not,
	write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	$RCSfile: sharedmem.c,v $	 
	$Revision: 1.53 $
	$Author: dhakim $
	$Date: 2003/02/11 23:09:15 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/file.h>
#include <string.h>

#include "mmmap.h"
#include "sharedmem.h"
#include "exception.h"
#include "excpfn.h"
#include "util.h"

#define SHARED_MEM_FLAGS 1

#ifndef SOLARIS_OS
#	 define BYTE_ALIGNMENT 4
#else
#	 define BYTE_ALIGNMENT 8
#endif

typedef struct sharedStats{
	int anodes;
	size_t amemory;
} SharedStats;

typedef struct fnode{
	size_t size;
	void* startAddress;
	struct fnode* parentRange;
	struct fnode* lowerRange;
	struct fnode* higherRange;
} FNode;

#define MMAN_INFO 1

typedef struct anode {
	struct anode* prev;
	struct anode* next;
	size_t size;
#ifdef MMAN_INFO
	void* pool;
	void(*freefn)(void *);
#endif
#ifdef SHARED_MEM_FLAGS
	char flags;
#endif
} ANode;

size_t SHARED_SEGMENT=32*1024*1024;
size_t ACCOUNTING_SEGMENT=1024*512;

FNode** SIZE_RANGE_ROOT;
ANode** ALIST_HEAD;
ANode** ALIST_TAIL;
void** LOW_MEMORY;
SharedStats* SHARED_STATS;

void* HIGH_MEMORY;
void* BASE_ADDRESS = 0;

inline FNode  * td_splay_on_size (size_t s, void* i, FNode * t);

inline void recoverNodeMemory(FNode* rtn);

inline FNode* createFNode( size_t size, void* startAddress);

int shared_initDefault(){
	return shared_init(32*1024*1024);
}

int shared_init(size_t size){
	int fd;

	size = size % BYTE_ALIGNMENT == 0 ? size :
			 size + BYTE_ALIGNMENT - size % BYTE_ALIGNMENT;

	if (BASE_ADDRESS != 0){
		/* fprintf(stderr,"\n### Hey, you already got some memory!\n"); */
		return -1;
	}

	if (size > 0) SHARED_SEGMENT = size;
	
	/* For now we will just make the accounting segment 1/5 of the shared segment */
	ACCOUNTING_SEGMENT = SHARED_SEGMENT * .2;
	
	if (( fd = open("/dev/zero", O_CREAT|O_RDWR)) < 0) {
		/* could not map /dev/zero */
		return -1;
	}
	
	/* The first four bytes of shared memory are going to record the
		offset of the last piece of memory allocated */
	BASE_ADDRESS = (caddr_t)mmmap(0, SHARED_SEGMENT + ACCOUNTING_SEGMENT, 
											PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	fflush(stdout);

	if(BASE_ADDRESS == (caddr_t)-1)	{
		/* could not get memory */
		return -1;
	}			
	  
	/*
	===================================================================================
	------shmem code------
	int shared_shmid; The shared memory attachment identifier
	int fd;

	shared_shmid = shmget(IPC_PRIVATE,SHARED_SEGMENT + ACCOUNTING_SEGMENT,SHM_R|SHM_W);

	if(shared_shmid == -1)
		return -1;

	BASE_ADDRESS = shmat(shared_shmid,0,0);
	------shmem code------
	===================================================================================
	*/

	HIGH_MEMORY = BASE_ADDRESS + SHARED_SEGMENT + 
					  ACCOUNTING_SEGMENT - BYTE_ALIGNMENT;

	SIZE_RANGE_ROOT = HIGH_MEMORY - 2*sizeof(void*);
	ALIST_HEAD = HIGH_MEMORY - 3*sizeof(void*);
	ALIST_TAIL = HIGH_MEMORY - 4*sizeof(void*);
	LOW_MEMORY = HIGH_MEMORY - 5*sizeof(void*);
	SHARED_STATS = HIGH_MEMORY - 5*sizeof(void*) - sizeof(SharedStats);

	*LOW_MEMORY = 
		HIGH_MEMORY - 5*sizeof(void*) - sizeof(SharedStats) - sizeof(FNode);

	*ALIST_HEAD = *ALIST_TAIL = NULL;
	*SIZE_RANGE_ROOT = createFNode(SHARED_SEGMENT,BASE_ADDRESS);

	SHARED_STATS->anodes=0;
	SHARED_STATS->amemory=0;

	return 1;
}

int MEM_LOCK = 0;

#define LOCK shared_lock()
#define UNLOCK shared_unlock()

#ifdef HAVE_FLOCK

inline void shared_lock(){
	if (MEM_LOCK){
		flock(MEM_LOCK,LOCK_EX);
	}
}

inline void shared_unlock(){
	if (MEM_LOCK){
		flock(MEM_LOCK,LOCK_UN);
	}
}

int shared_initLocking(char* globalLockDir){
	char lockPath[1024];
	char* lockDir = (globalLockDir != NULL) ? globalLockDir : "/tmp/";

	if(strlen(lockDir)+strlen("sharedmem.lock")+1 < 1024) {
	  sprintf(lockPath,"%s/sharedmem.lock",lockDir);

	  if (MEM_LOCK == 0){
		 if((MEM_LOCK = open(lockPath,O_RDWR | O_CREAT,S_IRWXU|S_IRWXO|S_IRWXG))<0){
			fprintf(stderr,"### Could not initialize shared memory locking !!!\n");
			return -1; /* could not create lock */
		 }
	  }
	  return 1;
	} else {
	  fprintf(stderr,"### Could not initialize shared memory locking: ");
	  fprintf(stderr,"Directory %s is too long.\n",lockDir);
	  return -1;
	}
}

#else

static struct flock lock_it;
static struct flock unlock_it;

int shared_initLocking(char* globalLockDir){

	 lock_it.l_whence = SEEK_SET;			 /* from current point */
	 lock_it.l_start = 0;					 /* -"- */
	 lock_it.l_len = 0;						 /* until end of file */
	 lock_it.l_type = F_WRLCK;				 /* set exclusive/write lock */
	 lock_it.l_pid = 0;						 /* pid not actually interesting */
	 unlock_it.l_whence = SEEK_SET;		 /* from current point */
	 unlock_it.l_start = 0;					 /* -"- */
	 unlock_it.l_len = 0;					 /* until end of file */
	 unlock_it.l_type = F_UNLCK;			 /* set exclusive/write lock */
	 unlock_it.l_pid = 0;					 /* pid not actually interesting */

	char lockPath[1024];
	char* lockDir = (globalLockDir != NULL) ? globalLockDir : "/tmp/";

	if(strlen(lockDir)+strlen("sharedmem.lock")+1 < 1024) {
	  sprintf(lockPath,"%s/sharedmem.lock",lockDir);

	  if (MEM_LOCK == 0){
		 if((MEM_LOCK = open(lockPath,O_RDWR | O_CREAT,S_IRWXU|S_IRWXO|S_IRWXG))<0) {
			fprintf(stderr,"### Could not initialize shared memory locking !!!\n");
			return -1; /* could not create lock */
		 }
	  }
	  return 1;
	} else {
	  fprintf(stderr,"### Could not initialize shared memory locking: ");
	  fprintf(stderr,"Directory %s is too long.\n",lockDir);
	  return -1;
	}
}

inline void shared_lock(){
	if (MEM_LOCK){
		int ret;

		while ((ret = fcntl(MEM_LOCK, F_SETLKW, &lock_it)) < 0 && errno == EINTR) {
			/* nop */
		}

		if (ret < 0) {fprintf(stderr,"Could not lock shared memory segment.\n"); }
	}
}

inline void shared_unlock(){
	if (MEM_LOCK){
		int ret;

		while ((ret = fcntl(MEM_LOCK, F_SETLKW, &unlock_it)) < 0 && errno == EINTR) {
			/* nop */
		}
		if (ret < 0) {fprintf(stderr,"Could not lock shared memory segment.\n"); }
	}
}

#endif

/*
 * Reporting functions 
 */

int shared_getTotalMemory(){
	return SHARED_SEGMENT;
}
int shared_getFreeMemory(){
	return SHARED_SEGMENT - SHARED_STATS->amemory - SHARED_STATS->anodes * sizeof(ANode);
}
int shared_getANodes(){
	return SHARED_STATS->anodes;
}
int shared_getFNodes(){
	return 
		(HIGH_MEMORY - (*LOW_MEMORY) - 5*sizeof(void*) - sizeof(SharedStats)) /
			(uint)sizeof(FNode);
}
int shared_getMaxFNodes(){
	return
		(ACCOUNTING_SEGMENT - 5*sizeof(void*) - sizeof(SharedStats)) /
			(uint)sizeof(FNode);
}

int shared_check(void* address){
	return (address >= BASE_ADDRESS ) && (address < BASE_ADDRESS + SHARED_SEGMENT);
}

void shared_cleanup() {
}

void debugTree(FNode* rtn, int depth)
{
	int i;

	if (rtn == NULL)
		return;

	debugTree(rtn->lowerRange,depth+1);

	for(i=0;i<depth;i++)
		fprintf(stderr," ");

	fprintf(stderr,"|[%p,%p)| = %u id=%u pid=%u\n",
		rtn->startAddress,
		rtn->startAddress + rtn->size,
		(unsigned int)(rtn->size),
		(HIGH_MEMORY - 4*sizeof(void*) - sizeof(FNode) - (void*)rtn),
		(HIGH_MEMORY - 4*sizeof(void*) - sizeof(FNode) - (void*)rtn->parentRange)
	);

	debugTree(rtn->higherRange,depth+1);

} 

void debug()
{
	ANode* cur;

	fprintf(stderr,"Size Range Tree : \n");
	debugTree(*SIZE_RANGE_ROOT,1);
	fprintf(stderr,"Allocated Range Tree : \n");

	cur = *ALIST_HEAD;
	while(cur){
		fprintf(stderr,"|[%p,%p)| = %u(%u) \n",
			(void*)cur + sizeof(ANode), 
			(void*)cur + sizeof(ANode) + cur->size, 
			(unsigned int)cur->size, 
			(unsigned int)(cur->size + sizeof(ANode))
		);
		cur = cur->next;
	}	
	
	fprintf(stderr,"According to lo-mem we have %u nodes allocated\n\n",
		(unsigned int)((HIGH_MEMORY - 4*sizeof(void*) - sizeof(FNode) - *LOW_MEMORY)/
		sizeof(FNode))) ;
}

unsigned int shared_nodesAllocated() {
	return (HIGH_MEMORY - 4 * sizeof(void*) - sizeof(FNode) - *LOW_MEMORY)/
	sizeof(FNode);
}

inline FNode* createFNode(
	size_t size,
	void* startAddress
)
{
	FNode* newNode = (FNode*)(*LOW_MEMORY);

	*LOW_MEMORY -= sizeof(FNode);

	if (*LOW_MEMORY < BASE_ADDRESS + SHARED_SEGMENT) {
		fprintf(stderr,"### Accounting segment overflow, too many objects!\n"); /* out of memory! */

		exit(0);
	}
	
	newNode->parentRange = NULL;
	newNode->lowerRange = NULL;
	newNode->higherRange = NULL;
	newNode->startAddress = startAddress;
	newNode->size = size;

	return newNode;
}

inline void insertFreeNode(void* startAddress,void* endAddress) {

	/* Insert i into the tree t, unless it's already there.	  */
	/* Return a pointer to the resulting tree.					  */

	FNode* t, *new;
	size_t size = endAddress - startAddress;

	t = *SIZE_RANGE_ROOT;

	if (t == NULL) {
		*SIZE_RANGE_ROOT = createFNode(size,startAddress);
		return;
	}

	t = td_splay_on_size(size, startAddress,t);

	if (size < t->size || (size == t->size && startAddress < t->startAddress)) {
		new = createFNode(endAddress - startAddress,startAddress);
		new->lowerRange = t->lowerRange;
		new->higherRange = t;
		if(t->lowerRange != NULL) {
			t->lowerRange->parentRange = new;
			t->lowerRange = NULL;
		}
		t->parentRange = new;
		t = new;
	} else if (size > t->size || (size == t->size && startAddress > t->startAddress)) {
		new = createFNode(endAddress - startAddress,startAddress);
		new->higherRange = t->higherRange;
		new->lowerRange = t;
		if(t->higherRange != NULL){
			t->higherRange->parentRange = new;
			t->higherRange = NULL;
		}
		t->parentRange = new;
		t = new;
	}

	*SIZE_RANGE_ROOT = t;
}

inline void deleteFreeNode(size_t size, void* startAddress) {

	/* Deletes i from the tree if it's there.					  */

	FNode **t = SIZE_RANGE_ROOT;
	FNode *x,*y;
	if (*t==NULL) return;

	*t = td_splay_on_size(size,startAddress,*t);
	if (size == (*t)->size && startAddress == (*t)->startAddress) {					/* found it */
		if ((*t)->lowerRange == NULL) {
			x = (*t)->higherRange;
			if(x!=NULL)
				x->parentRange = NULL;
		} else {
			x = td_splay_on_size(size,startAddress, (*t)->lowerRange);
			x->higherRange = (*t)->higherRange;
			x->parentRange = NULL;
			if(x->higherRange != NULL)
				x->higherRange->parentRange=x;
		}

		y=*t;
		*t=x;
		recoverNodeMemory(y);
	} else {
		fprintf(stderr,"### Trying to delete a free node that doesn't exist\n");
		exit(0); 
	}

}

/*
 * WARNING: This function has very dangerous side effects! Once you call it or call
 * a function that calls it you can not trust any RangeTreeNode* in your local scope
 * unless you re Locate() it!
 */
  
inline void recoverNodeMemory(FNode* rtn){
	
	/* If this node I'm deleting is not the */
	/* lowest node then move the lowest node */
	/* here */

	if((FNode*)(*LOW_MEMORY + sizeof(FNode)) != rtn) {
		FNode* lowrtn = (FNode*)(*LOW_MEMORY + sizeof(FNode));

		/* copy the low node over the */
		/* killed node */

		*rtn = *lowrtn;
							/* fix up the node's parent */
		if ( rtn->parentRange != NULL) {
			if (rtn->parentRange->lowerRange == lowrtn)
				rtn->parentRange->lowerRange = rtn;
			else /* if (rtn->parentRange->higherRange == lowrtn) */
				rtn->parentRange->higherRange = rtn;
		} else {
			*SIZE_RANGE_ROOT = (FNode*)rtn;
		}
							/* fix up the node's children */
		if (rtn->lowerRange != NULL)
			rtn->lowerRange->parentRange = rtn;
		if (rtn->higherRange != NULL)
			rtn->higherRange->parentRange = rtn;

	} 
	*LOW_MEMORY += sizeof(FNode);
}

void *shared_calloc(size_t nmemb,size_t size){
	void* address;

	size = size % BYTE_ALIGNMENT == 0 ? size :
			 size + BYTE_ALIGNMENT - size % BYTE_ALIGNMENT;

	address = shared_alloc(nmemb * size);

	if (address != NULL)
		memset(address,'\0',nmemb * size);
	return address;
}

int funkycount = 0;

inline FNode* locateLPFNode(FNode* root, size_t size) {
	FNode* best=NULL;
	FNode* cur=root;

	while(cur){
		if (cur->size >= size) {
			best = cur;
			cur = cur->lowerRange;
		} else cur=cur->higherRange;
	}
	return best;
}

inline FNode* locateBPFNode(FNode* root, size_t size) {
	FNode* best=NULL;
	FNode* cur=root;

	while(cur){
		if (cur->size >= size) best = cur;
		cur=cur->higherRange;
	}
	return best;
}

void *shared_alloc(size_t size){
	ANode *anode = NULL;
	FNode *fnode = NULL;
	void* returnAddress;

	size = size % BYTE_ALIGNMENT == 0 ? size : 
			 size + BYTE_ALIGNMENT - size % BYTE_ALIGNMENT;

	LOCK;

	if (*SIZE_RANGE_ROOT == NULL){							  /* No Memory Left */
		UNLOCK;
		return NULL;												  
	}

	if(size < 0){													  /* Sub-zero length allocation */
		UNLOCK;														  
		return NULL;
	}

	/* Locate the first range of the closest size */

	fnode = locateLPFNode(*SIZE_RANGE_ROOT,size + sizeof(ANode));

	if(fnode != NULL) {
		fnode = *SIZE_RANGE_ROOT = td_splay_on_size(fnode->size, fnode->startAddress, *SIZE_RANGE_ROOT);
	} else {													/* Not enough contiguous memory left */
		fprintf(stderr,"### Allocation failure!\n");
		UNLOCK;
		return NULL;
	}

	anode	 = fnode->startAddress;
	
	returnAddress = fnode->startAddress + sizeof(ANode);

	if (*ALIST_HEAD == NULL) {										/* Only Allocated Node */
		*ALIST_HEAD = *ALIST_TAIL = anode;
		anode->prev = anode->next = NULL;
	} else if (*ALIST_HEAD > anode) {							/* New AList Head */
		anode->prev = NULL;
		anode->next = *ALIST_HEAD;
		(*ALIST_HEAD)->prev = anode;
		*ALIST_HEAD = anode;
	} else if (*ALIST_TAIL < anode) {							/* New AList Tail */
		anode->next = NULL;
		anode->prev = *ALIST_TAIL;
		(*ALIST_TAIL)->next = anode;
		*ALIST_TAIL = anode;
	} else {																/* Interior Allocated Node */
		ANode* nextNode, *prevNode;
		nextNode = (ANode*)(fnode->startAddress + fnode->size);
		prevNode = nextNode->prev;

		prevNode->next = anode;
		nextNode->prev = anode;
		anode->prev = prevNode;
		anode->next = nextNode;
	}

	anode->size = size;

#ifdef MMAN_INFO
	anode->pool = NULL;
	anode->freefn = NULL;
#endif

	if(fnode->size - size - sizeof(ANode) != 0){
		void* startAddress, *endAddress;

		endAddress	 = fnode->startAddress + fnode->size;
		startAddress = fnode->startAddress + size + sizeof(ANode);

		{
			/* OPTIMIZATION! Only works because FNode is the root! */
			FNode* sbn = locateLPFNode(fnode->higherRange, endAddress - startAddress); /* Smallest bigger node */
			FNode* bsn = locateBPFNode(fnode->lowerRange, endAddress - startAddress); /* Biggest smaller node */

			if((sbn == NULL || sbn->size > endAddress - startAddress ) &&
				(bsn == NULL )) {

				fnode->size = endAddress - startAddress;
				fnode->startAddress = startAddress;
			} else {
				deleteFreeNode(fnode->size, fnode->startAddress);
				insertFreeNode(startAddress,endAddress);
			}
		}
	} else {	  
		deleteFreeNode(fnode->size, fnode->startAddress);
	}

	UNLOCK;

	SHARED_STATS->amemory += size;
	SHARED_STATS->anodes += 1;

	return returnAddress;
}

inline void 
shared_setPool(void* address, void* pool){
	ANode *anode = address - sizeof(ANode);
	anode->pool = pool;
}

inline void 
shared_setDestructor(void* address, void(*freefn)(void *)){
	ANode *anode = address - sizeof(ANode);
	anode->freefn = freefn;
}

inline void* 
shared_getPool(void* address){
	ANode *anode = address - sizeof(ANode);
	return anode->pool ;
}

inline void* 
shared_getDestructor(void* address){
	ANode *anode = address - sizeof(ANode);
	return anode->freefn ;
}

void shared_free(void* address){
	ANode *anode;
	void* startAddress, *endAddress;
	void	*lowerAddress, *higherAddress;

	int validAddress = 1;
	
	LOCK; 

	startAddress = anode = address - sizeof(ANode);
	endAddress = address + anode->size;

	/* Check if the address is actually allocated */
	
	if(anode->prev != NULL) {
		if(!shared_check(anode->prev)) validAddress = 0;
		else if(anode->prev->next != anode) validAddress = 0;
	} else if (*ALIST_HEAD != anode) validAddress = 0;
	
	if(anode->next != NULL) {
		if(!shared_check(anode->next)) validAddress = 0;
		else if(anode->next->prev != anode) validAddress = 0;
	} else if (*ALIST_TAIL != anode) validAddress = 0; 
	
	if(!validAddress){
		UNLOCK;
		THROW("DeAllocationFailureException","Attempt to free memory that was not allocated"); 
	}
			
	/* Delete the anode */

	if(anode->prev != NULL){
		anode->prev->next = anode->next;
		lowerAddress = (void*)anode->prev + anode->prev->size + sizeof(ANode);
	} else {
		*ALIST_HEAD = anode->next;
		lowerAddress = BASE_ADDRESS;
	}

	if(anode->next != NULL) {
		anode->next->prev = anode->prev;
		higherAddress = anode->next;
	} else {
		*ALIST_TAIL = anode->prev;
		higherAddress = BASE_ADDRESS+SHARED_SEGMENT;
	}

	/* Insert and Coalesce as free node*/

	if(higherAddress != endAddress)
		deleteFreeNode(higherAddress - endAddress,endAddress);

	if(lowerAddress != startAddress)
		deleteFreeNode(startAddress - lowerAddress,lowerAddress);

	insertFreeNode(lowerAddress,higherAddress);

	SHARED_STATS->amemory -= endAddress - address ;
	SHARED_STATS->anodes -= 1;

	/* Zero out the ANode */
	anode->next = NULL;
	anode->prev = NULL;
	
	UNLOCK;
}


void* shared_realloc(void* address, size_t size){
	ANode* anode;
	void* startAddress;
	void* endAddress;
	int difference;

	size = size % BYTE_ALIGNMENT == 0 ? size : 
			 size + BYTE_ALIGNMENT - size % BYTE_ALIGNMENT;

	LOCK;

	/* realloc from NULL = malloc */

	if (address == NULL){

		UNLOCK;

		return shared_alloc(size);
	}

	/* realloc to size 0 = free */

	if (size == 0){

		UNLOCK;

		shared_free (address);
		return NULL;
	}

	anode = (ANode*)(address - sizeof(ANode));

	/* What if this ptr was never allocated ? */

	/*
	if (rtn == NULL || rtn->startAddress != address){

		UNLOCK;

		return NULL;
	}
	*/

	difference = size - anode->size;

	if (difference < 0){
		void* higherAddress = address + anode->size;

		/* free and coalesce */

		startAddress = address + size;
		endAddress = address + anode->size;

		SHARED_STATS->amemory -= anode->size - size;
		
		anode->size = size;		  /* Fix up the allocated node */

		if( anode->next != NULL){
			higherAddress = anode->next;
		} else {
			higherAddress = BASE_ADDRESS + SHARED_SEGMENT;
		}

		if (higherAddress != endAddress)
			deleteFreeNode(higherAddress - endAddress,endAddress);

		insertFreeNode(startAddress,higherAddress);

		UNLOCK;

		return address;

	} else if (difference > 0) {

		void* higherAddress = BASE_ADDRESS + SHARED_SEGMENT;

		if(anode->next != NULL) {
			higherAddress = anode->next;
		}

		if (address + size > higherAddress) {

			void* newaddress;
			size_t oldSize = anode->size;

			UNLOCK;

			newaddress = shared_alloc(size);

			if(newaddress == NULL) {
				/* out of memory, could not complete re-alloc */
				return NULL;
			}

			shared_setDestructor(newaddress,shared_getDestructor(address));
			shared_setPool(newaddress,shared_getPool(address));

			memcpy(newaddress,address,oldSize);
			shared_free(address);

			return newaddress;

		} else /* address + size <= higherAddress */ {
			size_t origSize = higherAddress - address - anode->size;
			void* origStart = address + anode->size;

			SHARED_STATS->amemory += size - anode->size;
			 
			anode->size = size;

			deleteFreeNode( origSize, origStart);

			if(origStart + difference != higherAddress) {
				insertFreeNode( origStart + difference, higherAddress);
			}
			UNLOCK;

			return address;
		}
	} else /* no change in allocation */ {

		UNLOCK;

		return address;
	}
}

/*---------------------------------------------------------------------------
	Splay tree implementation
---------------------------------------------------------------------------*/

inline FNode * td_splay_on_size (
	size_t s, 
	void* i, 
	FNode * t
) {
	FNode N, *l, *r, *y;
	if (t == NULL) return t;
	N.lowerRange = N.higherRange = NULL;
	l = r = &N;

	for (;;) {
		if (s < t->size || (s == t->size && i < t->startAddress)) {
			if (t->lowerRange != NULL && 
					(s < t->lowerRange->size || 
						(s == t->lowerRange->size && i < t->lowerRange->startAddress) 
					)
				) {
				y = t->lowerRange;

				t->lowerRange = y->higherRange;
				if(y->higherRange != NULL)
					y->higherRange->parentRange=t;

				y->higherRange = t;
				y->parentRange=t->parentRange;
				t->parentRange=y;

				t = y;
			}
			if (t->lowerRange == NULL) break;
			r->lowerRange = t; t->parentRange=r; r = t; t = t->lowerRange;
		} else if (s > t->size || (s == t->size && i > t->startAddress)) {
			if (t->higherRange != NULL && 
					(s > t->higherRange->size ||
						(s == t->higherRange->size && i > t->higherRange->startAddress)
					)
				) {
				y = t->higherRange;

				t->higherRange = y->lowerRange;
				if(y->lowerRange != NULL)
					y->lowerRange->parentRange=t;

				y->lowerRange = t;
				y->parentRange=t->parentRange;
				t->parentRange=y; t = y;
			}
			if (t->higherRange == NULL) break;
			l->higherRange = t; t->parentRange=l; l = t; t = t->higherRange;
		} else break;
	}

	l->higherRange=t->lowerRange;
	if (t->lowerRange != NULL) t->lowerRange->parentRange=l;
	r->lowerRange=t->higherRange;
	if (t->higherRange != NULL) t->higherRange->parentRange=r;
	t->lowerRange=N.higherRange;
	t->higherRange=N.lowerRange;
	if (t->lowerRange != NULL) t->lowerRange->parentRange=t;
	if (t->higherRange != NULL) t->higherRange->parentRange=t;
	t->parentRange = NULL;
	return t;
}

/*=============================================================================
	end of file: $RCSfile: sharedmem.c,v $
==============================================================================*/

