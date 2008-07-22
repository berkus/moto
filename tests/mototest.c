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

   $RCSfile: mototest.c,v $   
   $Revision: 1.24 $
   $Author: dhakim $
   $Date: 2003/01/28 02:49:26 $
 
==============================================================================*/
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MOTO "../bin/moto"
#define MOTOFLAGS "-l"
#define MOTOCFLAGS "-c"
#define GCCLIBS "../lib/libcodex.a"
#define LIST "test.list"
#define BUFSIZE 1024
#define MAX_NAME_LEN 34
#define MOTO_ERR_LEN 27
#define CHK_ERR_LEN 26

int ok = 0;
int no = 0;
int okmem = 0;
int nomem = 0;

void ppad(int pad) {
   if (pad > 0) {
      while (--pad) {
         printf(".");
      }
   }
}

int getfile(const char *filename, char **text) {
   FILE *file = NULL;
   int total_read = 0;

   file = fopen(filename, "r");
   if (file == NULL) {
      fprintf(stderr, "io error: %s, %d\n", filename, __LINE__);
      return -1;
   }
   else {
      int char_size = sizeof(char);
      int index = 0;
      int cap = BUFSIZE;
      char *buf;

      *text = (char *)malloc(cap);
      if (*text == NULL) {
         fprintf(stderr, "memory allocation failure\n");
         exit(1);
      }

      buf = (char *)malloc(BUFSIZE);
      if (buf == NULL) {
         fprintf(stderr, "memory allocation failure\n");
         exit(1);
      }
   
      do {
         int read;
         read = fread(buf, char_size, BUFSIZE, file);
         if (ferror(file) != 0) {
            free(buf);
            free(*text);
            fclose(file);
            fprintf(stderr, "io error: %d\n", __LINE__);
            exit(1);
         }
         total_read += read;
         if (total_read > cap) {
            char *newmem = realloc(*text, (cap * 2));
            if(newmem == NULL) {
               free(buf);
               free(*text);
               fprintf(stderr, "io error: %d\n", __LINE__);
               exit(1);
            }
            *text = newmem;   
            cap = total_read;
         }
         memcpy((*text) + index, buf, read);
         index = total_read;
      }
      while (feof(file) == 0);
      free(buf);
      (*text)[total_read] = '\0';
      /* close file */
      if (fclose(file) != 0) {
         fprintf(stderr, "io error: %d\n", __LINE__);
         exit(1);
      }
   }
   return total_read;
}

int test_exec(char* outname, char* errname, char* cmd, ...){
   int pid; /* pid of the forked process */
   int fd,efd;  /* file descriptor for output of the forked process */
   int status; /* the status returned by the process */
 
   va_list args;

   /* Fork a child to run the test */
   pid = fork();
   if (pid == 0) {    /* child */

      /* Create a temporary file to catch output of test program */
      fd = open(outname, O_CREAT | O_WRONLY, S_IRWXU);
      if (fd == -1) {
         fprintf(stderr, "open of outfile failed\n");
      }

      /* Create a temporary file to catch stderr of test program */
      efd = open(errname, O_CREAT | O_WRONLY, S_IRWXU);
      if (efd == -1) {
         fprintf(stderr, "open of errfile failed\n");
      }

      else {
         char* execargs[20];
         int i;

         /* Redirect stdout to a temporary file */

         close(1);
         dup(fd);
         close(fd);

         close(2);
         dup(efd);
         close(efd);

         va_start(args, cmd);
         for(i=0;(execargs[i]=va_arg(args, char *))!=NULL;i++);
         va_end(args); 

         /* exec moto on test file */
         execvp(cmd, execargs);

         fprintf(stderr, "exec of %s failed\n",cmd);
         exit(1);
      }
   }
   else if (pid == -1) {
      fprintf(stderr, "fork failed\n");
      exit(1);
   }

   /* Wait for child to complete before returning */
   while (wait(&status) != pid) { printf("\nfoo");}  /* empty loop */
   return status;
}

int runtest(const char *test,int debug) {
   int pid;
   int fd;
   int chkfd;
   char motoname[64];
   char motocname[64];
   char chkname[64];
   char outname[64];
   char errname[64];
   int outlen, chklen, memlen;
   char compilationerrname[64];
   char *out;
   char *chk;
   char *end;
   struct stat buf;
   int mem = 0;
   int pad;
   int status;

   strcpy(motoname, test);
   strcat(motoname, ".moto");

   strcpy(chkname, test);
   strcat(chkname, ".txt");

   strcpy(outname, test);
   strcat(outname, ".out");

   strcpy(errname, test);
   strcat(errname, ".err");

   strcpy(compilationerrname, test);
   strcat(compilationerrname, ".cerr");

   strcpy(motocname, test);
   strcat(motocname, ".c");

   printf("%s", motoname);
   
   if(stat(motoname, &buf)) {
      pad = MOTO_ERR_LEN - strlen(motoname);
      ppad(pad);
      printf("moto file not found\n");
      return -1;   
   }
   if(stat(chkname, &buf)) {
      pad = CHK_ERR_LEN - strlen(motoname);
      ppad(pad);
      printf("check file not found\n");
      return -1;   
   }

   /* pad with dots */
   pad = MAX_NAME_LEN - strlen(motoname);
   ppad(pad);

   test_exec(outname,errname,MOTO,MOTO,"-X","../mx",MOTOFLAGS,motoname,NULL);

   /* collect result and check output from files */   
   outlen = getfile(outname, &out);
   chklen = getfile(chkname, &chk);
      
   /* look for memory output info */
   memlen = strlen("//mem-ok//\n");
   if(outlen >= memlen && strcmp(out+outlen-memlen,"//mem-ok//\n") == 0 ){
      mem = 1;
   } else {
      memlen = strlen ("//mem-leak//\n");
      mem = -1;
   }

   /* compare output with expected result */
   if (outlen == chklen + memlen && memcmp(out, chk, chklen) == 0) {
      ok++;
      if (mem == 1) {
         printf("ok.......ok.......");
         okmem++;
      }
      else if (mem == -1) {
         printf("ok.......fail.....");
         nomem++;
      }
      else {
         printf("ok\n");
      }
   }
   else {
      no++;
      if (mem == 1) {
         printf("fail.....ok.......");
         okmem++;
      }
      else if (mem == -1) {
         printf("fail.....fail.....");
         nomem++;
      }
      else {
         printf("fail\n");
      }
   }

   remove(outname);
   remove(errname);

   status = test_exec(motocname,errname,MOTO, MOTO, "-X","../mx",MOTOCFLAGS, motoname, NULL);
   if(status != 0) {
      printf("n/a......n/a......n/a\n");
      if (!debug)
         remove(motocname);
      remove(errname);
      return;
   }
   test_exec(outname,compilationerrname,"./compile.sh","./compile.sh", test,CC,NULL);
   test_exec(outname,errname,"./a.out", "./a.out","-l", NULL);

   /* collect result and check output from files */
   outlen = getfile(outname, &out);
   chklen = getfile(chkname, &chk);

   /* look for memory output info */
   memlen = strlen("//mem-ok//\n");
   if(outlen >= memlen && strcmp(out+outlen-memlen,"//mem-ok//\n") == 0 ){
      mem = 1;
   } else {         
      memlen = strlen ("//mem-leak//\n");
      mem = -1;    
   }

   /* compare output with expected result */
   if (outlen == chklen + memlen && memcmp(out, chk,chklen) == 0) {
      ok++;
      if (mem == 1) {
         printf("ok.......ok.......ok\n");
         okmem++;
      }
      else if (mem == -1) {
         printf("ok.......ok.......fail\n");
         nomem++;
      }
      else {
         printf("ok\n");
      }
   }
   else {
      no++;
      if (mem == 1) {
         printf("ok.......fail.....ok\n");
         okmem++;
      }
      else if (mem == -1) {
         printf("ok.......fail.....fail\n");
         nomem++;
      }
      else {
         printf("fail\n");
      }
   }

   remove(outname);
   remove(errname);
   if(!debug)
      remove(compilationerrname);

   free(out);
   free(chk);

   /* clean up the temporary file */
   remove(outname);
   if(!debug)
      remove(motocname);

   return 0;
}

int main(int argc, char **argv) {
   
   char **suites;
   char *list;
   char *line;
   int num;
   int t;
   int i;
   int head;
   int debug=1;

   printf("Mototest begin\n");
   printf("==========================================================================\n");

   if (argc < 2) {
      suites = malloc(sizeof(char *));
      suites[0] = LIST;
      num = 1;
      debug =0;
   }
   else {
      num = argc - 1;
      suites = malloc(num * sizeof(char *));
      for (i = 0; i < num; i++) {
         suites[i] = argv[i + 1];
      }
      debug =1;
   }

   head = 1;
   for (i = 0; i < num; i++) {
      getfile(suites[i], &list);
      line = strtok(list, "\n");
      while (line != NULL) {
         if (line[0] == '#') {
            head = 0;
            printf("%s\n", line);
         }
         else {
            if (head == 0) {
               printf("Test Name                      Output   Memory   Compile   Output   Memory\n");
               printf("--------------------------------------------------------------------------\n");
            }
            head = 1;
            runtest(line,debug);
         }
         line = strtok(NULL, "\n");
      }
      free(list);
   }
   free(suites);

   t = ok + no;
   printf("--------------------------------------------------------------------------\n");
   printf("Total Tests Run:  %4d\n", t);
   printf("--------------------------------------------------------------------------\n");
   printf("Summary         Output              Memory\n", t);
   printf("Passed:     %4d (%6.2f%%)      %4d (%6.2f%%)\n", 
          ok, ((float)ok / (float)t * 100),
          okmem, ((float)okmem / (float)t * 100)
          );
   printf("Failed:     %4d (%6.2f%%)      %4d (%6.2f%%)\n", 
          no, ((float)no / (float)t * 100),
          nomem, ((float)nomem / (float)t * 100)
          );
   printf("==========================================================================\n");
   printf("Mototest done\n");

   return no + nomem;

}

/*=============================================================================
// end of file: $RCSfile: mototest.c,v $
==============================================================================*/

