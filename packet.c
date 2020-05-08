#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "headers/safemem.h"
#include "headers/networks.h"
#include "headers/table.h"
#include "headers/packet.h"
#include "headers/pollLib.h"

void server_parse_packet(uint8_t *buff, int socket){
   uint8_t flag = get_type(buff);

   switch(flag){
      case 1:
         parse_flag1(buff, socket);
         break;
      case 5:
         parse_flag5(buff, SERVER);
         break;
      case 8:
         parse_flag8(socket);
         break;
      case 10:
         parse_flag10(socket);
         break;
      default:
         printf("Not defined\n");
         print_buff(buff);
         break;
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
      case 5:
         parse_flag5(buff, CLIENT);
         break;
      case 7:
         parse_flag7(buff);
         break;
      case 9:
         printf("\n");
         exit(-1);
         break;
      case 11:
         parse_flag11(buff);
         break;
      case 12:
         parse_flag12(buff);
         break;
      case 13:
         parse_flag13();
         break;
      default:
         printf("\nNot defined\n");
         print_buff(buff);
         break;
   }
}


/* Get packet flag field */
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

   packet_get_data(buff + HEADER_LEN, handle);

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
   put_data(buff + HEADER_LEN, handle);
   return(ntohs(header->length));
}


/* Build a message packet */
uint16_t build_flag5(uint8_t *buff, 
                 char *my_handle,
                 char handles[MAX_NUM_HANDLES][MAX_HANDLE + 1],
                 uint8_t num_handles,
                 char *msg){

   struct packet_header header = build_header(5);
   struct packet_header *overlay;
   uint8_t *current = buff + HEADER_LEN;
   int i;

   smemset(buff, '\0', MAX_PACKET);
   smemcpy(buff, &header, sizeof(header));
   current = put_data(current, my_handle);
   smemcpy(current, &num_handles, sizeof(num_handles));
   current += sizeof(num_handles);

   for(i = 0; i < num_handles; i++){
      current = put_data(current, handles[i]);
   }
   strcpy((char *)current, msg);
   current += strlen(msg) + 1;
   overlay = (struct packet_header *)buff;
   overlay->length = htons(current - buff);
   return(ntohs(overlay->length));
}


/* Get info from a message packet 
 * The client will print the message,
 * The server will forward or respond with flag 7
 */
void parse_flag5(uint8_t *buff, int process_type){
   char dest_handles[MAX_NUM_HANDLES][MAX_HANDLE + 1];
   char msg[MAX_MSG] = "";
   char sender[MAX_HANDLE + 1] = "";
   uint8_t flag7_buff[MAX_PACKET] = "";
   uint8_t *ptr;
   struct packet_header *header;
   uint16_t packet_len;
   uint16_t flag7_len;
   uint8_t num_handles;
   int socket;
   int i;

   header = (struct packet_header *)buff;
   packet_len = ntohs(header->length);
   ptr = packet_get_data(buff + HEADER_LEN, sender);
   smemcpy(&num_handles, ptr, sizeof(num_handles));
   ptr += sizeof(num_handles);

   for(i = 0; i < num_handles; i++){
      smemset(dest_handles[i], '\0', sizeof(dest_handles[i]));
      ptr = packet_get_data(ptr, dest_handles[i]);
   }
   sstrcpy(msg, (char *)ptr);


   /* Client Print */
   if(process_type == CLIENT){
      printf("\n%s: %s\n$", sender, msg);
      fflush(stdout);
      return;
   }

   /* Server forward */
   for(i = 0; i < num_handles; i++){
      if((socket = table_get_socket(dest_handles[i])) < 0){
         socket = table_get_socket(sender);
         flag7_len = build_flag7(flag7_buff, dest_handles[i]);
         sendPacket(socket, flag7_buff, flag7_len);
         smemset(flag7_buff, '0', sizeof(flag7_buff));
         continue;
      }
      printf("%d\n", socket);
      sendPacket(socket, buff, packet_len);
   }
}


/* Let client know that the handle was invalid */
uint16_t build_flag7(uint8_t *buff, char *handle){
   struct packet_header *header;
   uint8_t *ptr;
   uint16_t plen;

   header = (struct packet_header *)buff;
   header->flag = 7;
   ptr = put_data(buff + HEADER_LEN, handle);
   plen = (uintptr_t)ptr - (uintptr_t)buff;
   header->length = htons(plen);
   return plen;
}



void parse_flag7(uint8_t *buff){
   char handle[MAX_HANDLE + 1] = "";
   packet_get_data(buff + HEADER_LEN, handle);
   printf("\nClient with handle %s does not exist\n$", handle);
   fflush(stdout);
}


/* Remove client from table and send flag 9*/
void parse_flag8(int socket){
   struct packet_header header;
   int exit_response = 9;
   header = build_header(exit_response);
   remove_entry(socket);
   sendPacket(socket, (uint8_t *)&header, ntohs(header.length));
}


/* Send handles to client
 * Build and send flag 11, flag 12(s) and flag 13 */
void parse_flag10(int socket){
   struct packet_header flag13;
   uint8_t buff[MAX_PACKET] = "";
   char handle[MAX_HANDLE + 1] = "";
   uint32_t num_handles = get_num_elements();
   uint16_t send_len;
   int i;

   send_len = build_flag11(buff);
   sendPacket(socket, buff, send_len);


   for(i = 0; i < num_handles; i++){
      smemset(buff, '0', sizeof(buff));
      smemset(handle, '0', sizeof(handle));
      get_entry(i, handle);
      send_len = build_flag12(buff, handle);
      sendPacket(socket, buff, send_len);
   }

   flag13 = build_header(13);
   sendPacket(socket, (uint8_t *)&flag13, HEADER_LEN);
}


/* Stick the number of handles onto a header */
uint16_t build_flag11(uint8_t *buff){
   struct packet_header *header;
   uint32_t num_handles = 0;

   header = (struct packet_header *)buff;
   num_handles = htonl(get_num_elements());

   header->length = htons(HEADER_LEN + sizeof(uint32_t));
   header->flag = 11;

   smemcpy(buff + HEADER_LEN, &num_handles, sizeof(num_handles)); 
   return(ntohs(header->length));
}


void parse_flag11(uint8_t *buff){
   uint32_t *num_handles;

   num_handles = (uint32_t *)((uintptr_t)buff + HEADER_LEN);
   printf("\nNumber of clients: %u\n", ntohl(*num_handles));

   /* Wait to get all handles before continuing anything */
   removeFromPollSet(STDIN_FILENO);
}


uint16_t build_flag12(uint8_t *buff, char *handle){
   struct packet_header *header;
   uint8_t handle_len = strlen(handle);

   header = (struct packet_header *)buff;

   header->flag = 12;
   header->length = htons(HEADER_LEN + sizeof(handle_len) + handle_len);
   
   put_data(buff + HEADER_LEN, handle);
   return(ntohs(header->length));
}


void parse_flag12(uint8_t *buff){
   char handle[MAX_HANDLE + 1] = "";

   packet_get_data(buff + HEADER_LEN, handle);
   printf("  %s\n", handle);
}


void parse_flag13(){
   addToPollSet(STDIN_FILENO);
   printf("$ ");
   fflush(stdout);
}




/* Give a starting pointer, calculate data len using strlen
 * Copy data len followed by the actual data
 * Exclude Null terminator if it exists (strlen does this)
 * Returns pointer to next location for easy looping
 */
uint8_t *put_data(uint8_t *buff, char *handle){
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
uint8_t *packet_get_data(uint8_t *buff, char *handle_buff){
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