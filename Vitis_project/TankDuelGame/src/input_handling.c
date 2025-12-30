#include "FreeRTOS.h"
#include "task.h"
#include <portmacro.h>
#include <projdefs.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "game_abstraction.h"
#include "lvgl.h"
#include "connection_udp.h"


void vInputHandler(void *pvParameters)
{
    Game *game = (Game *)pvParameters;
    TickType_t prevTick = xTaskGetTickCount();

    while(1)
    {
        uint8_t cmd1 = 0;
        uint8_t cmd2 = 0;

        // Take a local snapshot of the latest UDP commands
        if(g_player1_cmd_valid)
        {
            cmd1 = g_player1_cmd;
        }
        if (g_player1_cmd_valid)
        {
            cmd2 = g_player2_cmd;
        }

        xSemaphoreTake(keyboard_semaphore, 0);

        memset(Player1KeyboardEvents, 0, TOTALMOVEMENTS);
        memset(Player2KeyboardEvents, 0, TOTALMOVEMENTS);

        if (cmd1 & (1 << 0)) Player1KeyboardEvents[MOVEFORWARD] = 1;
        if (cmd1 & (1 << 1)) Player1KeyboardEvents[MOVEBACK]    = 1;
        if (cmd1 & (1 << 2)) Player1KeyboardEvents[MOVELEFT]    = 1;
        if (cmd1 & (1 << 3)) Player1KeyboardEvents[MOVERIGHT]   = 1;
        if (cmd1 & (1 << 4)) Player1KeyboardEvents[GUNLEFT]     = 1;
        if (cmd1 & (1 << 5)) Player1KeyboardEvents[GUNRIGHT]    = 1;
        if (cmd1 & (1 << 6)) Player1KeyboardEvents[SHOOT]       = 1;

        if (cmd2 & (1 << 0)) Player2KeyboardEvents[MOVEFORWARD] = 1;
        if (cmd2 & (1 << 1)) Player2KeyboardEvents[MOVEBACK]    = 1;
        if (cmd2 & (1 << 2)) Player2KeyboardEvents[MOVELEFT]    = 1;
        if (cmd2 & (1 << 3)) Player2KeyboardEvents[MOVERIGHT]   = 1;
        if (cmd2 & (1 << 4)) Player2KeyboardEvents[GUNLEFT]     = 1;
        if (cmd2 & (1 << 5)) Player2KeyboardEvents[GUNRIGHT]    = 1;
        if (cmd2 & (1 << 6)) Player2KeyboardEvents[SHOOT]       = 1;

        xSemaphoreGive(keyboard_semaphore);

        // Run at ~100Hz (10ms)
        xTaskDelayUntil(&prevTick, pdMS_TO_TICKS(10));
    }
}

/*
void vInputHandler(void *pvParameters)
{
    Game* game = (Game*)pvParameters;

    TickType_t prevTick;
    prevTick = xTaskGetTickCount();


    
    // Key states for continuous checking
    const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);
    while (1)
    {
        uint8_t movement_array[TOTALMOVEMENTS];
        xSemaphoreTake(keyboard_semaphore,0);
        // Or for real-time game input (check current state):
        if(keyboard_state[SDL_SCANCODE_W]) {
            Player1KeyboardEvents[MOVEFORWARD] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_A]) {
            Player1KeyboardEvents[MOVELEFT] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_S]) {
            Player1KeyboardEvents[MOVEBACK] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_D]) {
            Player1KeyboardEvents[MOVERIGHT] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_Q]) {
            Player1KeyboardEvents[GUNLEFT] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_E]) {
            Player1KeyboardEvents[GUNRIGHT] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_SPACE]) {
            Player1KeyboardEvents[SHOOT] = 1;
        }


        if(keyboard_state[SDL_SCANCODE_KP_5]) {
            Player2KeyboardEvents[MOVEFORWARD] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_KP_1]) {
            Player2KeyboardEvents[MOVELEFT] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_KP_2]) {
            Player2KeyboardEvents[MOVEBACK] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_KP_3]) {
            Player2KeyboardEvents[MOVERIGHT] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_KP_4]) {
            Player2KeyboardEvents[GUNLEFT] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_KP_6]) {
            Player2KeyboardEvents[GUNRIGHT] = 1;
        }
        if(keyboard_state[SDL_SCANCODE_KP_8]) {
            Player2KeyboardEvents[SHOOT] = 1;
        }
        xSemaphoreGive(keyboard_semaphore);

        xTaskDelayUntil(&prevTick,pdMS_TO_TICKS(10));
    }
}

*/