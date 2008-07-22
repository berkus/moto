/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
   This file is part of the Moto Programming Language.

   Moto Programming Language is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   Moto Programming Language is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Codex C Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   $RCSfile: ftwmain.c,v $   
   $Revision: 1.2 $
   $Author: dhakim $
   $Date: 2002/12/11 23:31:38 $
 
==============================================================================*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ftw.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define DEPTH 5

void output();

extern int optind;
extern int errno;

int pflags = 0;
int list_mode = 0;
int rel_mode = 0;
char *ext = NULL;
char *arg = NULL;
struct list *files;

typedef struct listNode {
   char *file;
   struct listNode *next;
} ListNode;

typedef struct list {
   ListNode *head;
   ListNode *tail;
} List;

static void usage(int code) {
   printf("Usage: ftw [options] <dir>\n");
   exit(code);
}

static void add_file(const char *file) {
   ListNode *node = files->head;
   node = malloc(sizeof(ListNode));
   node->file = strdup(file);
   node->next = NULL;
   if (files->head == NULL) {
      files->head = files->tail = node;
   }
   else {
      files->tail->next = node;
      files->tail = node;
   }
}

static int ftw_fn(const char *file, const struct stat *sb, int flag) {
   if (pflags == FTW_F && flag == FTW_F){
      if (ext == NULL) {
         add_file(file);
      }
      else {
         int flen = strlen(file);
         int elen = strlen(ext);
         if (strncmp(file + flen - elen, ext, elen) == 0) {
            add_file(file);
         }
      }
   }
   else if (pflags == FTW_D && flag == FTW_D) {
      if (strcmp(file, ".") != 0) {
         add_file(file);
      }
   }
   return 0;
}

int main(int argc, char **argv) {
   int result = 0;
   int c;
   struct stat sb;

   pflags = FTW_F;

   if (argc < 2) {
      usage(1);
   }

   while (1) {
      c = getopt(argc, argv, "1de:fhr");
      if (c == -1) {
         break;
      }
      switch (c) {
         case '1':
            list_mode = 1;
            break;
         case 'h':
            usage(0);
            break;
         case 'd':
            pflags |= FTW_D;
            pflags &= ~FTW_DNR;
            pflags &= ~FTW_F;
            pflags &= ~FTW_SL;
            break;
         case 'e':
            pflags &= ~FTW_D;
            pflags &= ~FTW_DNR;
            pflags |= FTW_F;
            pflags &= ~FTW_SL;
            ext = malloc(strlen(optarg) + 1);
            strcpy(ext, optarg);
            break;
         case 'f':
            pflags &= ~FTW_D;
            pflags &= ~FTW_DNR;
            pflags |= FTW_F;
            pflags &= ~FTW_SL;
            break;
         case 'r':
            rel_mode = 1;
            break;
      }
   }
   
   argc -= optind;
   if (argc < 1) {
      usage(1);
   }

   arg = argv[optind];
   result = stat(argv[optind], &sb);
   
   if (result == 0) {
      /* must be a directory */
      if (!S_ISDIR(sb.st_mode)) {
         printf("ftw: Name is not a directory: %s\n", arg);
         result = 1;
      }
      else {
         files = malloc(sizeof(List));
         files->head = NULL;
         ftw(argv[optind], ftw_fn, DEPTH);
         output();
      }
   }
   else {
      result = errno;
      switch (result) {
         case EACCES:
            printf("ftw: Read or search permission was denied "
                   "for a component of the path prefix: %s\n", arg);
            break;
         case ENOMEM:
            printf("ftw: Out of memory: %s\n", arg);
            break;
         case ELOOP:
            printf("ftw: Too many symbolic links were encountered "
                   "in translating the pathname: %s\n", arg);
            break;                                                            
         case ENAMETOOLONG:
            printf("ftw: Name or path is too long: %s\n", arg);
            break;
         case ENOENT:
            printf("ftw: The named file does not exist: %s\n", arg);
            break;
         case ENOTDIR:
            printf("ftw: A component of the path prefix "
                   "is not a directory: %s\n", arg);
            break;
      }
   }
   
   return result;
}

void output() {
   ListNode *node = files->head;
   char *fmt;
   int arglen = 0;
   if (rel_mode == 1) {
      arglen = strlen(arg);   
   }
   if (list_mode == 1) {
      fmt = "%s\n";
   }
   else {
      fmt = "%s ";
   }
   while (node) {      
      char *f = (node->file + arglen);
      if (f[0] == '/') {
         f++;
      }
      printf(fmt, f);
      node = node->next;
   }
   if (strcmp(fmt, "%s ") == 0 && files->head) {
      printf("\n");
   }
}

/*=============================================================================
// end of file: $RCSfile: ftwmain.c,v $
==============================================================================*/

