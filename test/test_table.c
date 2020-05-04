#include "table.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
   int i;
   int c = atoi(argv[1]);
   int ret = 0;
   char buff[MAX_HANDLE];

   init_table();


   for(i = 0; i < c; i++){
      sprintf(buff, "Luke %d", i);
      ret = add_entry(buff, i);
   }

   for(i = 0; i < c; i++){
      sprintf(buff, "Luke %d", i);
      ret = remove_entry(buff);
      if(ret < 0){
         printf("Error removing %s\n", buff);
      }
   }
   reset_table();

}