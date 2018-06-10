// $Id: oclib.c,v 1.75 2018-05-18 16:24:12-07 - - $

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __OCLIB_C__
#include "oclib.h"

void* xcalloc (int nelem, int size) {
   void* result = calloc (nelem, size);
   assert (result != NULL);
   return result;
}

char* scan (int (*skipover) (int), int (*stopat) (int)) {
   int byte;
   do {
      byte = getchar();
      if (byte == EOF) return NULL;
   } while (skipover (byte));
   static char buffer[0x10000];
   char* end = buffer;
   do {
      *end++ = byte;
      assert (end < &buffer[sizeof buffer]);
      *end = '\0';
      byte = getchar();
   }while (byte != EOF && ! stopat (byte));
   char* result = strdup (buffer);
   assert (result != NULL);
   return result;
}


static int isfalse (int byte) { (void) byte; return 0; } 
static int isnl (int byte)    { return byte == '\n'; }
void putint (int val)         { printf ("%d", val); }
void putstr (const char* s)   { printf ("%s", s); }
char* getword (void)          { return scan (isspace, isspace); }
char* getln (void)            { return scan (isfalse, isnl); } 
void endl()                   { putchar ('\n'); }

