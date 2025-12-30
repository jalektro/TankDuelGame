#include "game_abstraction.h"
#include "connection_udp.h"
#include "xil_printf.h"


static Polygon terrain[] = {
        { //left bound
            .list = (Pixel_Coordinate[]){
                {0,0},
                {10,0},
                {10,SCREEN_HEIGHT},
                {0,SCREEN_HEIGHT},
            },
            .points = 4
        },
        { //right bound
            .list = (Pixel_Coordinate[]){
                {SCREEN_WIDTH-10,0},
                {SCREEN_WIDTH,0},
                {SCREEN_WIDTH,SCREEN_HEIGHT},
                {SCREEN_WIDTH-10,SCREEN_HEIGHT},
            },
            .points = 4
        },
        { //top bound
            .list = (Pixel_Coordinate[]){
                {0,SCREEN_HEIGHT-10},
                {0,SCREEN_HEIGHT},
                {SCREEN_WIDTH,SCREEN_HEIGHT},
                {SCREEN_WIDTH,SCREEN_HEIGHT-10},
            },
            .points = 4
        },
        { //bottom bound
            .list = (Pixel_Coordinate[]){
                {0,0},
                {0,10},
                {SCREEN_WIDTH,10},
                {SCREEN_WIDTH,0},
            },
            .points = 4
        },
        { //Middle shape
            .list = (Pixel_Coordinate[]){
                {SCREEN_WIDTH/2,(SCREEN_HEIGHT/2)+100},
                {(SCREEN_WIDTH/2)+100,(SCREEN_HEIGHT/2)+10},
                {(SCREEN_WIDTH/2)+60,(SCREEN_HEIGHT/2)-100},
                {(SCREEN_WIDTH/2)-60,(SCREEN_HEIGHT/2)-100},
                {(SCREEN_WIDTH/2)-100,(SCREEN_HEIGHT/2)+10}
            },
            .points = 5
        },
    };

GameError init_game(Game* game){
    //Initialize terrain
    game->terrain = terrain;
    game->terrain_size = 5;

    //Copy terrain into buffer

    //Populate into gamestate

    //Initializze players
    static Pixel_Coordinate p1_hitbox[4];
    static Tank player1 = {
        .id = 0,
        .color = {
            .r = 0xff,
            .g = 0x88,
            .b = 0x00,
        },
        .health = 100,
        .position = {
            .x = 100.0f,
            .y = 100.0f
        },
        .tank_rotation = 0.0f,
        .turret_rotation = 0.0f,
        .hitbox = {
            .points = 4,
            .list = p1_hitbox
        }
    };
    game->player1 = &player1;


    static Pixel_Coordinate p2_hitbox[4];
    static Tank player2 = {
        .id=1,
        .color = {
            .r = 0x00,
            .g = 0x88,
            .b = 0xFF,
        },
        .health = 100,
        .position = {
            .x = 1180.0f,
            .y = 580.0f
        },
        .tank_rotation = 180.0f,
        .turret_rotation = 180.0f,
        .hitbox = {
            .points = 4,
            .list = p2_hitbox
        }
    };
    game->player2 = &player2;

    keyboard_semaphore = xSemaphoreCreateBinary();
    if (keyboard_semaphore == NULL) {
    xil_printf("keyboard_semaphore create FAILED in init_game\r\n");
} else {
    xil_printf("keyboard_semaphore created OK\r\n");
}


    //Initialize array of bullets
    static Bullet bullets[32];
    static Pixel_Coordinate hitboxes[4*32];
    
    memset(bullets,0,32*sizeof(Bullet));
    for(int i=0;i<32;i++){
        bullets[i].hitbox.list = &(hitboxes[4*i]);
    }
    memset(hitboxes,0,32*sizeof(Polygon));

    game->bullets = &bullets;
    game->bullets_size = 0;
    game->max_bullets = 32;

}

SemaphoreHandle_t keyboard_semaphore;
uint8_t Player1KeyboardEvents[TOTALMOVEMENTS] = {0};
uint8_t Player2KeyboardEvents[TOTALMOVEMENTS] = {0};

void get_player_movements(uint8_t* movementArray, Tank* tank){
    xSemaphoreTake(keyboard_semaphore,0);
    
    if (tank->id == 0) {
        memcpy(movementArray, g_player1_movements, TOTALMOVEMENTS);
        memset(g_player1_movements, 0, TOTALMOVEMENTS);
    } else {
        memcpy(movementArray, g_player2_movements, TOTALMOVEMENTS);
        memset(g_player2_movements, 0, TOTALMOVEMENTS);    
    }
    
    /*
    if(tank->id==0){
        memcpy(movementArray,Player1KeyboardEvents,TOTALMOVEMENTS);
        memset(Player1KeyboardEvents,0,TOTALMOVEMENTS);
    } else {
        memcpy(movementArray,Player2KeyboardEvents,TOTALMOVEMENTS);
        memset(Player2KeyboardEvents,0,TOTALMOVEMENTS);
    }*/
    
    xSemaphoreGive(keyboard_semaphore);
}