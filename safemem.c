#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include "safemem.h"

void *smemcpy(void *dest, const void *src, size_t n){
   void *ret = NULL;
   ret = memcpy(dest, src, n);
   if(ret == NULL){
      perror("memcpy");
      exit(-1);
   }
   return ret;
}