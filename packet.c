#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "networks.h"
#include "packet.h"

void parse_packet(uint8_t *buff){
   get_type(buff);
}

uint8_t get_type(uint8_t *buff){
   struct packet_header *header;

   header = (struct packet_header *)buff;
   print_buff(buff, ntohs(header->length));
   printf("Type: %u\n", header->flag);
   return(header->flag);
}

void print_buff(uint8_t *buff, int len){
   int i;
   for(i= 0; i < len; i++){
      printf("%02x ", buff[i]);
      if(i == 70){printf("\n");}
   }
   printf("\n");
}