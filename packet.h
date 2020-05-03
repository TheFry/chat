#ifndef PACKET_H
#define PACKET_H

void server_parse_packet(uint8_t *buff, int socket);
uint8_t get_type(uint8_t *packet);
void print_buff(uint8_t *buff, int len);
struct packet_header build_header(uint8_t flag);

void parse_flag1(uint8_t *buff, int socket);
uint16_t build_flag1(uint8_t *buff, char *handle);

uint8_t *put_handle(uint8_t *buff, char *handle);
uint8_t *get_handle(uint8_t *buff, char *handle_buff);

#endif