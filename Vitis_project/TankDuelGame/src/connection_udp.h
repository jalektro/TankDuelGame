#ifndef CONNECTION_UDP_H
#define CONNECTION_UDP_H

#include <stdint.h>
#include "game_abstraction.h"

extern uint8_t g_player1_cmd;
extern uint8_t g_player2_cmd;
extern volatile uint8_t g_player1_cmd_valid;
extern volatile uint8_t g_player2_cmd_valid;
extern uint8_t g_player1_movements[TOTALMOVEMENTS];
extern uint8_t g_player2_movements[TOTALMOVEMENTS];
extern SemaphoreHandle_t keyboard_semaphore;
extern char g_board_ip_str[16];

void main_thread(void *p);



#endif //CONNECTION_UDP_H