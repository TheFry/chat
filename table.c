#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "networks.h"
#include "safemem.h"

#define DEFAULT_SIZE 9
#define FREE 0
#define FULL 1

struct table_entry{
   int socket;
   int is_free;
   char handle[MAX_HANDLE];
} __attribute__((packed));


static struct table_entry *table;
static int size;

void init_table(){
   struct table_entry *entry;
   int i;

   table = smalloc(sizeof(struct table_entry) * DEFAULT_SIZE);
   size = DEFAULT_SIZE;

   for(i = 0; i < DEFAULT_SIZE; i++){
      entry = &table[i];
      entry->is_free = FREE;
      entry->socket = -1;
      smemset(entry->handle, '0', MAX_HANDLE);
   }
}

void reset_table(){
   if(table != NULL){
      free(table);
   }
   table = NULL;
   size=0;
}

int add_entry(char *handle){
   int i;
   struct table_entry *entry;
   
   /* Check if already exists */
   for(i = 0; i < size; i++){
      entry = &table[i];
      if(strcmp(handle, entry->handle) == 0){
         return -1;
      }
   }

   /* Check if space in table */
}