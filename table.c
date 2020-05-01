/* Library to manipulate socket table for chat program
 * This is not efficient for large table 
 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "safemem.h"
#include "table.h"

static struct table_entry *table;
static int size;

void init_table(){
   struct table_entry *entry;
   int i;
   char *test;

   table = smalloc(sizeof(struct table_entry) * DEFAULT_TABLE_SIZE);
   size = DEFAULT_TABLE_SIZE;

   for(i = 0; i < DEFAULT_TABLE_SIZE; i++){
      entry = &table[i];
      test = entry->handle;
      entry->is_free = FREE;
      entry->socket = -1;
   }
}

void reset_table(){
   if(table != NULL){
      free(table);
   }
   table = NULL;
   size = 0;
}


void expand_size(){
   struct table_entry *temp;
   int i;

   /* Get memory */
   table = srealloc(table, sizeof(table) + TABLE_INCREMENT);

   /* Init new memory */
   
   for(i = 0; i < TABLE_INCREMENT; i++){
      temp = &table[size + i];
      temp->is_free = FREE;
      temp->socket = -1;
   }
   
   size = size + TABLE_INCREMENT;
}


int add_entry(char handle[MAX_HANDLE], int socket){
   int i;
   struct table_entry *entry;
   printf("Handle %s\n", handle);

   /* Check if handle/socket already exists */
   for(i = 0; i < size; i++){
      entry = &table[i];
      if(strcmp(handle, entry->handle) == 0){
         fprintf(stderr, "Handle already in table\n");
         return -1;
      }
      if(socket == entry->socket){
         fprintf(stderr, "Socket already in table\n");
         exit(-1);
      }
   }

   /* Find empty space */
   for(i = 0; i < size; i++){
      entry = &table[i];
      if(entry->is_free == FULL){
         continue;
      }
      sstrcpy(entry->handle, handle);
      entry->socket = socket;
      entry->is_free = FULL;
      return(0);
   }
   entry = &table[size];
   expand_size();
   sstrcpy(entry->handle, handle);

   entry->socket = socket;
   entry->is_free = FULL;
   return(0);
}


void print_table(){
   int i;
   struct table_entry *entry;

   printf("\n\nSocket Table:\n");
   for(i = 0; i < size; i++){
      entry = &table[i];
      printf("Entry %d\n", i);
      printf("\tSocket #: %d\n", entry->socket);
      printf("\tHandle: %s\n", entry->handle);
      printf("\tFree flag: %d\n\n", entry->is_free);
   }
}