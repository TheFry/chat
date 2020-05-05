#ifndef PACKET_H
#define PACKET_H

#include "networks.h"
void server_parse_packet(uint8_t *buff, int socket);
void client_parse_packet(uint8_t *buff, int socket);

uint8_t get_type(uint8_t *packet);
void print_buff(uint8_t *buff);
struct packet_header build_header(uint8_t flag);

void parse_flag1(uint8_t *buff, int socket);
uint16_t build_flag1(uint8_t *buff, char *handle);

void build_flag5(uint8_t *buff,
                 char *my_handle,
                 char handles[MAX_NUM_HANDLES][MAX_HANDLE],
                 uint8_t num_handles,
                 char *msg);

uint8_t *put_data(uint8_t *buff, char *handle);
uint8_t *packet_get_handle(uint8_t *buff, char *handle_buff);

#endif