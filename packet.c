#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "headers/safemem.h"
#include "headers/networks.h"
#include "headers/table.h"
#include "headers/packet.h"

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


void client_parse_packet(uint8_t *buff, int socket){
   uint8_t flag = get_type(buff);

   switch(flag){
      case 2:
         break;
      case 3:
         fprintf(stderr, "Handle Taken!\n");
         exit(-1);
      default:
         fprintf(stderr, "Not defined\n");
         exit(-1);
   }
}


uint8_t get_type(uint8_t *buff){
   struct packet_header *header;
   header = (struct packet_header *)buff;
   return(header->flag);
}


/* Builds basic packet header. This is useful
 * for flags that only require the header
 */
struct packet_header build_header(uint8_t flag){
   struct packet_header header;
   header.flag = flag;
   header.length = htons(HEADER_LEN);
   return header;
}


/* Try to add handle to table.
 * Respond with flag 2 on success
 * Respond with flag 3 on error (handle exists)
 */
void parse_flag1(uint8_t *buff, int socket){
   char handle[MAX_HANDLE] = "";
   struct packet_header response;

   packet_get_handle(buff + HEADER_LEN, handle);

   if(add_entry(handle, socket)){
      /* Flag 3 is negative response*/
      response = build_header(3);
   }else{
      /* Flag 2 is positive response */
      response = build_header(2);
   }

   sendPacket(socket, (uint8_t *)&response, HEADER_LEN);
}


/* Used by client to create init packet */
uint16_t build_flag1(uint8_t *buff, char *handle){
   uint8_t handle_len = strlen(handle);
   struct packet_header *header;

   if(handle_len > MAX_HANDLE){
      fprintf(stderr, "Handle too long\n");
      exit(-1);
   }

   /*Overlay a structure on the packet to add info */
   header = (struct packet_header *)buff;

   header->length = htons(HEADER_LEN 
                    + sizeof(uint8_t) 
                    + handle_len);
   header->flag = 1;
   put_handle(buff + HEADER_LEN, handle);
   return(ntohs(header->length));
}


void build_flag5(uint8_t *buff, 
                 char *my_handle,
                 char handles[MAX_NUM_HANDLES][MAX_HANDLE],
                 uint8_t num_handles,
                 char *msg){

   struct packet_header header = build_header(5);
   struct packet_header *overlay;
   uint8_t *current = buff + HEADER_LEN;
   int i;

   smemset(buff, '\0', MAX_PACKET);
   smemcpy(buff, &header, sizeof(header));
   current = put_handle(current, my_handle);
   smemcpy(current, &num_handles, sizeof(num_handles));
   current += sizeof(num_handles);

   for(i = 0; i < num_handles; i++){
      current = put_handle(current, handles[i]);
   }
   //current = put_handle(current, msg);
   overlay = (struct packet_header *)buff;
   overlay->length = htons(current - buff);
   print_buff(buff);
}


/* Give a starting pointer, calculate handle len
 * Copy handle len followed by the actual handle
 * Exclude Null terminator if it exists (strlen does this)
 */
uint8_t *put_handle(uint8_t *buff, char *handle){
   uint8_t len = sstrlen(handle);

   /* Add len to buffer */
   smemcpy(buff, &len, sizeof(len));

   /* Add handle to buffer */
   smemcpy(buff + sizeof(len), handle, sstrlen(handle) * sizeof(uint8_t));

   /* Return new memory location */
   return (buff + sizeof(len) + sstrlen(handle));
}


/* Gets length of handle from the buffer
 * Fills handle_buff with the handle name
 * Returns pointer to mem location just after the handle
 */
uint8_t *packet_get_handle(uint8_t *buff, char *handle_buff){
   uint8_t len;

   /* Get handle len from buff */
   smemcpy(&len, buff, sizeof(len));

   /* Copy handle */
   smemcpy(handle_buff, buff + sizeof(len), sizeof(uint8_t) * len);

   /* Return memory location */
   return (buff + sizeof(len) + len);
}


/* Print the buffer in hex byte by byte
 * Packet len is in network order
 */
void print_buff(uint8_t *buff){
   struct packet_header *header;
   uint16_t len;
   int line_break = 70;
   int i;

   header = (struct packet_header *)buff;
   len = ntohs(header->length);

   printf("Buffer Data Length: %u\n", len);;
   for(i= 0; i < len; i++){
      printf("%02x ", buff[i]);
      if(i == line_break){
         printf("\n");
         line_break += line_break;
      }
   }
   printf("\n");
}