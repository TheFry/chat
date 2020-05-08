#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include "/home/luke/chat/headers/safemem.h"
#include "/home/luke/chat/headers/networks.h"

#define NUM_CLIENTS 9
int break_msg(char *msg, char messages[NUM_MSGS][MAX_MSG]);
int main(int argc, char**argv){
   char messages[NUM_MSGS][MAX_MSG];
   char message1[610] = "";
   char *message = message1;

   smemset(message, 'a', 250);
   smemset(message + 250, 'b', 250);
   smemset(message + 500, 'c', 109);

   break_msg(message, messages);
}


int break_msg(char *msg, char messages[NUM_MSGS][MAX_MSG]){
   int breakc;
   int leftover;
   int msg_len;
   int num_messages;

   int i;

   /* Clear out arrays, guarenteed to have a null byte for string */
   for(i = 0; i < NUM_MSGS; i++){
      smemset(messages[i], '\0', sizeof(messages[i]));
   }

   /* Message will fit into one packet */
   if((msg_len = sstrlen(msg)) < MAX_MSG){
      sstrcpy(messages[0], msg);
      return 1;
   }

   i = 0;
   while(msg_len > 0){
      
      if(msg_len < MAX_MSG){
         smemcpy(messages[i], msg, msg_len);
         break;
      }

      smemcpy(messages[i], msg, MAX_MSG - 1);
      msg += (MAX_MSG - 1);
      msg_len -= (MAX_MSG - 1);
      i++;
   }

   num_messages = i;

   for(i = 0; i < num_messages; i++){
      printf("Len: %ld\n%s\n", strlen(messages[i]), messages[i]);
   }

}