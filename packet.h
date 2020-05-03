#ifndef PACKET_H
#define PACKET_H
void server_parse_packet(uint8_t *buff);
uint8_t get_type(uint8_t *packet);
void print_buff(uint8_t *buff, int len);
void parse_flag1(uint8_t *buff);
uint16_t build_flag1(uint8_t *buff, char *handle);
uint8_t *put_handle(uint8_t *buff, char *handle);
#endif