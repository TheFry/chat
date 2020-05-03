#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "safemem.h"
#include "networks.h"
#include "table.h"
#include "packet.h"

void server_parse_packet(uint8_t *buff, int socket){
   uint8_t flag = get_type(buff);

   switch(flag){
      case 1:
         parse_flag1(buff, socket);
         break;
      default:
         printf("Not defined\n");
         exit(-1);
   }
}


uint8_t get_type(uint8_t *buff){
   struct packet_header *header;

   header = (struct packet_header *)buff;
   return(header->flag);
}


/* Try to add handle to table.
 * Respond with flag 2 on success
 * Respond with flag 3 on error (handle exists)
 */
void parse_flag1(uint8_t *buff, int socket){
   char handle[MAX_HANDLE];
   struct packet_header *header;
   struct packet_header response;

   memset(handle, '0', MAX_HANDLE);
   get_handle(buff + HEADER_LEN, handle);

   if(add_entry(handle, socket)){
      /* Flag 3 is negative response*/
      response = build_header(3);
   }else{
      /* Flag 2 is positive response */
      response = build_header(2);
   }

   sendPacket(socket, (uint8_t *)&response, HEADER_LEN);
}


uint16_t build_flag1(uint8_t *buff, char *handle){
   uint8_t handle_len = strlen(handle);
   struct packet_header *header;

   if(handle_len > MAX_HANDLE){
      fprintf(stderr, "Handle too long\n");
      exit(-1);
   }
   header = (struct packet_header *)buff;
   header->length = htons(HEADER_LEN 
                    + sizeof(uint8_t) 
                    + handle_len);
   header->flag = 1;
   put_handle(buff + HEADER_LEN, handle);
   return(ntohs(header->length));
}


struct packet_header build_header(uint8_t flag){
   struct packet_header header;
   header.flag = flag;
   header.length = htons(HEADER_LEN);
   return header;
}


/* Give a starting pointer, calculate handle len
 * Copy handle len followed by the actual handle
 * Exclude Null terminator if it exists (strlen does this)
 */
uint8_t *put_handle(uint8_t *buff, char *handle){
   uint8_t len = strlen(handle);

   /* Add len to buffer */
   smemcpy(buff, &len, sizeof(len));

   /* Add handle to buffer */
   smemcpy(buff + sizeof(len), handle, strlen(handle) * sizeof(uint8_t));

   /* Return new memory location */
   return (buff + sizeof(len) + strlen(handle));
}

/* Gets length of handle from the buffer
 * Fills handle_buff with the handle name
 * Returns pointer to mem location just after the handle
 */
uint8_t *get_handle(uint8_t *buff, char *handle_buff){
   uint8_t len;

   /* Get handle len from buff */
   smemcpy(&len, buff, sizeof(len));

   /* Set handle string buffer to 0 */
   smemset(handle_buff, '0', sizeof(uint8_t) * len);

   /* Copy handle */
   smemcpy(handle_buff, buff, sizeof(uint8_t) * len);

   /* Return memory location */
   return (buff + sizeof(len) + len);
}

void print_buff(uint8_t *buff, int len){
   int i;
   for(i= 0; i < len; i++){
      printf("%02x ", buff[i]);
      if(i == 70){printf("\n");}
   }
   printf("\n");
}