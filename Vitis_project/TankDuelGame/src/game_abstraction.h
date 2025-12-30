#pragma once
#include <stdint.h>
#include "FreeRTOS.h"
#include <string.h>
#include <math.h>
#include <semphr.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define MAX_POLYGON_POINTS 32

#define TANK_WIDTH 20
#define TANK_HEIGT 36
#define TANK_TURRET_RADIUS 8
#define TANK_TURRET_LENGTH 18
#define TANK_TURRET_WIDTH 6

#define BULLET_WIDTH 5
#define BULLET_LENGTH 12

typedef enum GameError GameError;
enum GameError{
    SUCCESS,
    GENERIC_FAILURE
};

enum TankMovement{
    MOVEFORWARD,
    MOVEBACK,
    MOVELEFT,
    MOVERIGHT,
    GUNLEFT,
    GUNRIGHT,
    SHOOT,
    TOTALMOVEMENTS
};

typedef struct Screen Screen;
struct Screen{
    int width;
    int height;
};

typedef struct RGB RGB;
struct RGB{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

typedef struct Pixel_Coordinate Pixel_Coordinate;
struct Pixel_Coordinate{
    uint16_t x;
    uint16_t y;
};

typedef struct Game_Coordinate Game_Coordinate;
struct Game_Coordinate{
    float x;
    float y;
};

typedef struct Polygon Polygon;
typedef struct Polygon
{
    uint16_t points;
    Pixel_Coordinate* list;
};

typedef struct TankInstance Tank;
struct TankInstance{
    uint8_t id;
    Game_Coordinate position;
    RGB color;
    int health;
    float tank_rotation;        //Rotation of the tank, 0 degrees points to the top.
    float turret_rotation;      //Rotation of the turret on top, 0 degrees points to the top.
    uint8_t controls[TOTALMOVEMENTS];
    Polygon hitbox;
    uint16_t fire_cooldown_ticks;
};

typedef struct Particle Particle;
struct Particle{
    uint16_t lifetime;            //Time left to live for the particle in number of game ticks
    Game_Coordinate position;
};

typedef struct Bullet Bullet;
struct Bullet{
    Game_Coordinate position;
    uint8_t alive;
    uint16_t bounces_left;
    float rotation;
    float velocity;
    Polygon hitbox;
};

typedef struct Game Game;
struct Game{
    Polygon* terrain;
    uint16_t terrain_size;

    Tank* player1;
    Tank* player2;

    Particle* particles;
    uint16_t particles_size;

    Bullet* bullets;
    uint16_t bullets_size;
    uint16_t max_bullets;
};

//Initialize the screen
GameError init_screen(struct Screen*);

//Initialize game with the right settingss and terrain
GameError init_game(Game* game);

//Draw rectangle on XY position
GameError draw_rectangle(int x, int y, RGB rgb, int width, int height, Screen* screen);

//Draw circle on XY postition
GameError draw_circle(int x, int y, RGB rgb, int r, Screen* screen);

//Draw polygon. A polygon is made up out of coordinates which are connected together
GameError draw_polygon(Pixel_Coordinate* coordlist, uint16_t len, RGB rgb, Screen* screen);

//Checks if user has sent input, fills an array of size TOTALMOVEMENTS with at it's index a bollean value corresponding if enum TankMovement has been pressed
extern SemaphoreHandle_t keyboard_semaphore;
extern uint8_t Player1KeyboardEvents[TOTALMOVEMENTS];
extern uint8_t Player2KeyboardEvents[TOTALMOVEMENTS];

void get_player_movements(uint8_t* movementArray, Tank* tank);