#ifndef PACKET_H
#define PACKET_H
void parse_packet(uint8_t *buff);
uint8_t get_type(uint8_t *packet);
void print_buff(uint8_t *buff, int len);
#endif