/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
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

   $RCSfile: symboltable.h,v $   
   $Revision: 1.8 $
   $Author: dhakim $
   $Date: 2003/07/07 00:28:04 $
 
==============================================================================*/
#ifndef __SYMBOLTABLE_H
#define __SYMBOLTABLE_H

#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "enumeration.h"

/*-----------------------------------------------------------------------------
        typedefs
-----------------------------------------------------------------------------*/

/* the bucket node struct - stores a chained list of node "collisions" */
typedef struct sBucketNode {
   void *name;
   void *value;
   struct sBucketNode *next;
} SBucketNode;

/* the bucket struct - stores the head of the chained list of bucket nodes */
typedef struct sBucket {
   int num_elements;	/* The number of nodes in this bucket */
   struct sBucketNode *elements;
} SBucket;

/* the symbol table struct */
typedef struct symbolTable {
   int num_buckets;	/* The number of buckets in use */
   int cap;				/* The total number of buckets */
   int size;			/* The number of n-v pairs in the symbol table */
   struct sBucket *buckets;
   unsigned int (*hash_fn)(void*); /* The hash function */
   int (*comp_fn)(void*,void*); /* The key comparator */
} SymbolTable;

/* the symbol table iterator struct */
typedef struct symbolTableIterator {
   SymbolTable *p;
   int cur_bucket;
   struct sBucketNode *cur_bucket_node;
} SymbolTableIterator;

/*-----------------------------------------------------------------------------
        allocation functions
-----------------------------------------------------------------------------*/

/**
 * Allocates a new symbol table with the given parameters.
 * @param cap the initial capacity of the symbol table.
 * @param hfn the hash function to use on names.
 * @param cfn the comparator function to use on names.
 * @return a pointer to the newly allocated symbol table.
 */
SymbolTable *stab_create (
   int cap, /* The initial number of buckets */
   unsigned int (*hfn)(void*), /* The hash function */
   int (*cfn)(void*,void*) /* The key comparator */
);

/**
 * Allocates a new symbol table with default parameters.
 * @return a pointer to the newly allocated symbol table.
 */
SymbolTable *stab_createDefault();

/**
 * Frees the memory associated with a symbol table.
 * @param table a pointer to a symbol table.
 */
void stab_free(SymbolTable *table);

/*-----------------------------------------------------------------------------
        data structure functions
-----------------------------------------------------------------------------*/

/**
 * Clears the symbol table so it contains no name-value mappings.
 * @param table a pointer to a symbol table.
 */
void stab_clear(SymbolTable *table);

/**
 * Returns the density of a symbol table. The density equals the number of
 * symbol table buckets that are in use divided by 100.
 * @param table a pointer to a symbol table.
 * @return the density of the given symbol table. 
 */
int stab_density(SymbolTable *table);

/**
 * Returns a pointer to the value mapped to the given name.
 * @param table a pointer to a symbol table.
 * @param name the name to whose value is to be retrieved.
 * @return a pointer to the value mapped to the given name, or NULL is the
 * name is not mapped to a value. 
 */
void *stab_get(SymbolTable *table, char *name);

int stab_contains(SymbolTable *table, char *name);

/** 
 * Returns true if this StringSet contains the same elements as <i>S</i> 
 */
int stab_equals(SymbolTable* p1, SymbolTable* p2);

int stab_notequals(SymbolTable* p1, SymbolTable* p2);

/**
 * Returns an enumeration of the keys in a symbol table.
 * @param table a pointer to a symbol table.
 * @return an enumeration of the elements in the given symbol table. 
 */
Enumeration *stab_getKeys(SymbolTable *table);

/**
 * Returns an enumeration of the values in a symbol table.
 * @param table a pointer to a symbol table.
 * @return an enumeration of the elements in the given symbol table.
 */
Enumeration *stab_getValues(SymbolTable *table);

/**
 * Puts a name-value mapping into the symbol table. Any previous value
 * that was mapped to the given name is overwritten.
 * @param table a pointer to a symbol table.
 * @param name the name to use to create the name-value mapping.
 * @param value the value to use to create the name-value mapping.
 */
void *stab_put(SymbolTable *table, char *name, void *value);

/**
 * Removes a name-value mapping from the symbol table.
 * @param table a pointer to a symbol table.
 * @param name the name to whose value is to be removed.
 */
void *stab_remove(SymbolTable *table, char *name);

/**
 * Returns the size of a symbol table. The size equals the number of
 * name-value mappings contained in the symbol table.
 * @param table a pointer to a symbol table.
 * @return the size of the given symbol table. 
 */
int stab_size(SymbolTable *table);

/**
 * Returns a string representation of a symbol table.
 * @param table a pointer to a symbol table.
 * @return a string representation of the given symbol table. 
 */
char *stab_toString(SymbolTable *table);

#endif

/*=============================================================================
// end of file: $RCSfile: symboltable.h,v $
==============================================================================*/

