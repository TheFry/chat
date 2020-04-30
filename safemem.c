#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include "safemem.h"

void *smemcpy(void *dest, const void *src, size_t n){
   void *ptr = NULL;
   ptr = memcpy(dest, src, n);
   
   if(ptr == NULL){
      perror("memcpy");
      exit(-1);
   }

   return ptr;
}

void *smalloc(size_t size){
   void *ptr = NULL;
   ptr = malloc(size);
   
   if(ptr == NULL){
      perror("malloc");
      exit(-1);
   }

   return ptr;
}

void *smemset(void *s, int c, size_t n){
   void *ptr = NULL;
   ptr = memset(s, c, n);

   if(ptr == NULL){
      perror("memset");
      exit(-1);
   }

   return ptr;
}