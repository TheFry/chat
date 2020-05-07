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

void parse_flag5(uint8_t *buff, int process_type);
uint16_t build_flag5(uint8_t *buff,
                 char *my_handle,
                 char handles[MAX_NUM_HANDLES][MAX_HANDLE + 1],
                 uint8_t num_handles,
                 char *msg);

void parse_flag7(uint8_t *buff);
uint16_t build_flag7(uint8_t *buff, char *handle);

void parse_flag8(int socket);
void parse_flag10(int socket);
void build_flag11();
void build_flag12(uint8_t *buff, char *handle);

uint8_t *put_data(uint8_t *buff, char *handle);
uint8_t *packet_get_data(uint8_t *buff, char *handle_buff);

#endif