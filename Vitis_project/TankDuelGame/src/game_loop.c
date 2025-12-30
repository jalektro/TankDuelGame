#include <stdio.h>
#include "game_abstraction.h"
#include "xil_printf.h"

void lv_timer_handler();

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    int poly;        // 1 or 2 -> which polygon owned the colliding edge
    int edge_index;  // index of edge in that polygon
    Vec2 axis;       // collision normal
} CollisionInfo;

static Vec2 vec_sub(Vec2 a, Vec2 b) {
    Vec2 r = { a.x - b.x, a.y - b.y };
    return r;
}

static float vec_dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float overlap_amount(float min1, float max1, float min2, float max2)
{
    float left  = max2 - min1;
    float right = max1 - min2;
    return (left < right) ? left : right;
}
/*
int polygons_collide_info(const Polygon* p1, const Polygon* p2, CollisionInfo* info)
{
    float smallest_overlap = 1e30f;
    int best_poly = 0;
    int best_edge = -1;
    Vec2 best_axis = {0,0};
} */

static void project_polygon(const Polygon* poly, Vec2 axis, float* min, float* max)
{
    *min = *max = vec_dot((Vec2){poly->list[0].x, poly->list[0].y}, axis);
    for (int i = 1; i < poly->points; i++) {
        float p = vec_dot((Vec2){poly->list[i].x, poly->list[i].y}, axis);
        if (p < *min) *min = p;
        if (p > *max) *max = p;
    }
}

/*
static int overlap_on_axis(const Polygon* p1, const Polygon* p2, Vec2 axis)
{
    float min1, max1, min2, max2;
    project_polygon(p1, axis, &min1, &max1);
    project_polygon(p2, axis, &min2, &max2);
    return !(max1 < min2 || max2 < min1); // true overlap
}*/

//Takes to polygons, if collider is not NULL will be populated with the first point of the colliding wall the wall is formed together with collider + 1. 
//Keep in mind of the edge case where if the last point is collided with it refers to the points at [0]  
//The collider is always returned of the second polygon
int polygons_collide(const Polygon* p1, const Polygon* p2, CollisionInfo* info)
{
    float smallest_overlap = 1e30f;
    int best_poly = 0;
    int best_edge = -1;
    Vec2 best_axis = {0,0};
    // Test edges from p1
    for (int i = 0; i < p1->points; i++) {
        Vec2 a = { p1->list[i].x, p1->list[i].y };
        Vec2 b = { p1->list[(i+1) % p1->points].x,
                p1->list[(i+1) % p1->points].y };

        Vec2 edge = vec_sub(b, a);
        Vec2 axis = { -edge.y, edge.x };   // normal

        float min1, max1, min2, max2;
        project_polygon(p1, axis, &min1, &max1);
        project_polygon(p2, axis, &min2, &max2);

        if (max1 < min2 || max2 < min1)
            return 0;  // SAT exit: no collision

        // float ov = overlap_amount(min1, max1, min2, max2);
        // if (ov < smallest_overlap) {
        //     smallest_overlap = ov;
        //     best_axis = axis;
        //     best_poly = 1;
        //     best_edge = i;   // <-- This is the index of the colliding edge in P1
        // }
    }

    // Test edges from p2
    for (int i = 0; i < p2->points; i++) {
        Vec2 a = { p2->list[i].x, p2->list[i].y };
        Vec2 b = { p2->list[(i+1) % p2->points].x,
                p2->list[(i+1) % p2->points].y };

        Vec2 edge = vec_sub(b, a);
        Vec2 axis = { -edge.y, edge.x };

        float min1, max1, min2, max2;
        project_polygon(p1, axis, &min1, &max1);
        project_polygon(p2, axis, &min2, &max2);

        if (max1 < min2 || max2 < min1)
            return 0;

        float ov = overlap_amount(min1, max1, min2, max2);
        if (ov < smallest_overlap) {
            smallest_overlap = ov;
            best_axis = axis;
            best_poly = 2;
            best_edge = i;   // <-- colliding edge index in P2
        }
    }
    //Populate collider if not null
    if(info!=NULL){
        info->poly       = best_poly;
        info->edge_index = best_edge;
        info->axis       = best_axis;
    }

    return 1; // All axes overlap → collision
}

void get_rotated_rect(Polygon* polygon,float angle_deg, Game_Coordinate pos, float width, float height){
    if(polygon->points!=4) return;
    float rad = angle_deg * 3.14159265f / 180.0f;
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);
    
    float dx[4] = {-width/2,  width/2,  width/2, -width/2};
    float dy[4] = {-height/2, -height/2,  height/2,  height/2};
    for(int i=0; i<4; i++) {
        polygon->list[i].x = (int16_t)(pos.x + dx[i]*cos_a - dy[i]*sin_a);
        polygon->list[i].y = (int16_t)(pos.y + dx[i]*sin_a + dy[i]*cos_a);
    }
}



void vGameTask(void *pvParameters)
{
   // xil_printf("vGameTask STARTED\r\n");
    Game* game = (Game*)pvParameters;
    TickType_t prevTick;

    prevTick = xTaskGetTickCount(); 
       

    get_rotated_rect(&(game->player1->hitbox),game->player1->tank_rotation,game->player1->position,TANK_WIDTH,TANK_HEIGT);
    get_rotated_rect(&(game->player2->hitbox),game->player2->tank_rotation,game->player2->position,TANK_WIDTH,TANK_HEIGT);
    while (1)
    {
        //Make snapshot of player in case of collision
        Tank player1;
        memcpy(&player1, game->player1, sizeof(Tank));

        Tank player2;
        memcpy(&player2, game->player2, sizeof(Tank));

        //Parse movement

        //Player 1
        uint8_t movement_array[TOTALMOVEMENTS];
        get_player_movements(movement_array,game->player1);
        if(movement_array[MOVEFORWARD]){
            float rad = (game->player1->tank_rotation+90.0f) * (M_PI / 180.0f);
            game->player1->position.x -= cos(rad);
            game->player1->position.y -= sin(rad);
            get_rotated_rect(&(game->player1->hitbox),game->player1->tank_rotation,game->player1->position,TANK_WIDTH,TANK_HEIGT);
        }
        if(movement_array[MOVEBACK]){
            float rad = (game->player1->tank_rotation+90.0f) * (M_PI / 180.0f);
            game->player1->position.x += cos(rad);
            game->player1->position.y += sin(rad);
            get_rotated_rect(&(game->player1->hitbox),game->player1->tank_rotation,game->player1->position,TANK_WIDTH,TANK_HEIGT);
        }
        if(movement_array[MOVELEFT]){
            game->player1->tank_rotation -= 1.0f;
            if(game->player1->tank_rotation>360){
                game->player1->tank_rotation -= 360.0f;
            }
            if(game->player1->tank_rotation<0){
                game->player1->tank_rotation += 360.0f;
            }
            get_rotated_rect(&(game->player1->hitbox),game->player1->tank_rotation,game->player1->position,TANK_WIDTH,TANK_HEIGT);
        }
        if(movement_array[MOVERIGHT]){
            game->player1->tank_rotation += 1.0f;
            if(game->player1->tank_rotation>360){
                game->player1->tank_rotation -= 360.0f;
            }
            if(game->player1->tank_rotation<0){
                game->player1->tank_rotation += 360.0f;
            }
            get_rotated_rect(&(game->player1->hitbox),game->player1->tank_rotation,game->player1->position,TANK_WIDTH,TANK_HEIGT);
        }
        if(movement_array[GUNLEFT]){
            game->player1->turret_rotation -= 1.0f;
            if(game->player1->turret_rotation>360){
                game->player1->turret_rotation -= 360.0f;
            }
            if(game->player1->turret_rotation<0){
                game->player1->turret_rotation += 360.0f;
            }
        }
        if(movement_array[GUNRIGHT]){
            game->player1->turret_rotation += 1.0f;
            if(game->player1->turret_rotation>360){
                game->player1->turret_rotation -= 360.0f;
            }
            if(game->player1->turret_rotation<0){
                game->player1->turret_rotation += 360.0f;
            }
        }
        if(movement_array[SHOOT]){            
            if(game->player1->fire_cooldown_ticks==0){
                //Find first vacant bullet slot
                for(int i=0;i<game->max_bullets;i++){
                    if(!(game->bullets[i].alive)){
                        //spawn new bullet
                        game->bullets[i].alive = 1;
                        game->bullets[i].bounces_left = 1;
                        game->bullets[i].rotation = game->player1->turret_rotation;
                        game->bullets[i].velocity = 3.0f;

                        //Get initial bullet position
                        float rad = game->bullets[i].rotation * M_PI / 180.0f;
                        float cos_a = cosf(rad);
                        float sin_a = sinf(rad);
                        game->bullets[i].position.x = game->player1->position.x + cos_a*(TANK_HEIGT-8);
                        game->bullets[i].position.y = game->player1->position.y + sin_a*(TANK_HEIGT-8);

                        //Calculate hitbox
                        game->bullets[i].hitbox.points = 4;
                        get_rotated_rect(&(game->bullets[i].hitbox),game->bullets[i].rotation,game->bullets[i].position,BULLET_LENGTH,BULLET_WIDTH);
                        break;
                    }
                }
                game->player1->fire_cooldown_ticks = 60;
                game->bullets_size++;
            }
            

        }

        //Player2
        get_player_movements(movement_array,game->player2);

        if(movement_array[MOVEFORWARD]){
            float rad = (game->player2->tank_rotation+90.0f) * (M_PI / 180.0f);
            game->player2->position.x -= cos(rad);
            game->player2->position.y -= sin(rad);
            get_rotated_rect(&(game->player2->hitbox),game->player2->tank_rotation,game->player2->position,TANK_WIDTH,TANK_HEIGT);
        }
        if(movement_array[MOVEBACK]){
            float rad = (game->player2->tank_rotation+90.0f) * (M_PI / 180.0f);
            game->player2->position.x += cos(rad);
            game->player2->position.y += sin(rad);
            get_rotated_rect(&(game->player2->hitbox),game->player2->tank_rotation,game->player2->position,TANK_WIDTH,TANK_HEIGT);
        }
        if(movement_array[MOVELEFT]){
            game->player2->tank_rotation -= 1.0f;
            if(game->player2->tank_rotation>360){
                game->player2->tank_rotation -= 360.0f;
            }
            if(game->player2->tank_rotation<0){
                game->player2->tank_rotation += 360.0f;
            }
            get_rotated_rect(&(game->player2->hitbox),game->player2->tank_rotation,game->player2->position,TANK_WIDTH,TANK_HEIGT);
        }
        if(movement_array[MOVERIGHT]){
            game->player2->tank_rotation += 1.0f;
            if(game->player2->tank_rotation>360){
                game->player2->tank_rotation -= 360.0f;
            }
            if(game->player2->tank_rotation<0){
                game->player2->tank_rotation += 360.0f;
            }
            get_rotated_rect(&(game->player2->hitbox),game->player2->tank_rotation,game->player2->position,TANK_WIDTH,TANK_HEIGT);
        }
        if(movement_array[GUNLEFT]){
            game->player2->turret_rotation -= 1.0f;
            if(game->player2->turret_rotation>360){
                game->player2->turret_rotation -= 360.0f;
            }
            if(game->player2->turret_rotation<0){
                game->player2->turret_rotation += 360.0f;
            }
        }
        if(movement_array[GUNRIGHT]){
            game->player2->turret_rotation += 1.0f;
            if(game->player2->turret_rotation>360){
                game->player2->turret_rotation -= 360.0f;
            }
            if(game->player2->turret_rotation<0){
                game->player2->turret_rotation += 360.0f;
            }
        }
        if(movement_array[SHOOT]){
            if(game->player2->fire_cooldown_ticks==0){
                //Find first vacant bullet slot
                for(int i=0;i<game->max_bullets;i++){
                    if(!(game->bullets[i].alive)){
                        //spawn new bullet
                        game->bullets[i].alive = 1;
                        game->bullets[i].bounces_left = 1;
                        game->bullets[i].rotation = game->player2->turret_rotation;
                        game->bullets[i].velocity = 3.0f;

                        //Get initial bullet position
                        float rad = game->bullets[i].rotation * M_PI / 180.0f;
                        float cos_a = cosf(rad);
                        float sin_a = sinf(rad);
                        game->bullets[i].position.x = game->player2->position.x + cos_a*(TANK_HEIGT-8);
                        game->bullets[i].position.y = game->player2->position.y + sin_a*(TANK_HEIGT-8);

                        //Calculate hitbox
                        game->bullets[i].hitbox.points = 4;
                        get_rotated_rect(&(game->bullets[i].hitbox),game->bullets[i].rotation,game->bullets[i].position,BULLET_LENGTH,BULLET_WIDTH);
                        break;
                    }
                }
                game->player2->fire_cooldown_ticks = 60;
                game->bullets_size++;
            }           

        }

        //Bullet movements
        for(int i=0;i<game->max_bullets;i++){
            if(!(game->bullets[i].alive)) continue;
            float rad = game->bullets[i].rotation * M_PI / 180.0f;
            float cos_a = cosf(rad);
            float sin_a = sinf(rad);
            game->bullets[i].position.x += (cos_a*game->bullets[i].velocity);
            game->bullets[i].position.y += (sin_a*game->bullets[i].velocity);

            //Update hitbox
            get_rotated_rect(&(game->bullets[i].hitbox),game->bullets[i].rotation,game->bullets[i].position,BULLET_LENGTH,BULLET_WIDTH);
        }

        //Calculate collisions

        //Player to wall collission
        for(int i=0;i<game->terrain_size;i++){
            if(polygons_collide(&(game->player1->hitbox),&(game->terrain[i]),NULL)){
                game->player1->position = player1.position;
                game->player1->tank_rotation = player1.tank_rotation;
            }

            if(polygons_collide(&(game->player2->hitbox),&(game->terrain[i]),NULL)){
                game->player2->position = player2.position;
                game->player2->tank_rotation = player2.tank_rotation;
            }
        }

        //Player to player collision
        if(polygons_collide(&(game->player1->hitbox),&(game->player2->hitbox),NULL)){
            game->player1->position = player1.position;
            game->player1->tank_rotation = player1.tank_rotation;
            game->player2->position = player2.position;
            game->player2->tank_rotation = player2.tank_rotation;
        }

        //Bullet to player collision
        for(int i=0;i<game->max_bullets;i++){
            if(!(game->bullets[i].alive)) continue;
            if(polygons_collide(&(game->bullets[i].hitbox),&(game->player1->hitbox),NULL)){
                game->player1->health -= 10;
                game->bullets[i].alive = 0;
            }
            if(polygons_collide(&(game->bullets[i].hitbox),&(game->player2->hitbox),NULL)){
                game->player2->health -= 10;
                game->bullets[i].alive = 0;
            }
        }

        //Bullet to wall collision
        for(int i=0;i<game->max_bullets;i++){
            if(!(game->bullets[i].alive)) continue;
            for(int j=0;j<game->terrain_size;j++){
                CollisionInfo collider;
                if(polygons_collide(&(game->bullets[i].hitbox),&(game->terrain[j]),&collider)){
                    //Destroy bullet if no bounces, else bounce bullet
                    if(game->bullets[i].bounces_left==0){
                        game->bullets[i].alive = 0;
                    } else {
                        //Bounce bullet
                        if(collider.poly==2){ //Make sure you are working with the terrain collision
                            //Normalize axis
                            float len = sqrt(collider.axis.x*collider.axis.x + collider.axis.y*collider.axis.y);
                            collider.axis.x = collider.axis.x/len;
                            collider.axis.y = collider.axis.y/len;

                            float rot_radians = (game->bullets[i].rotation*M_PI)/180;

                            Vec2 v = {
                                cosf(rot_radians) * game->bullets[i].velocity,
                                sinf(rot_radians) * game->bullets[i].velocity
                            };

                            float dot = v.x * collider.axis.x + v.y * collider.axis.y;

                            Vec2 reflected = {
                                v.x - 2.0f * dot * collider.axis.x,
                                v.y - 2.0f * dot * collider.axis.y
                            };

                            float reflected_radians = atan2f(reflected.y, reflected.x);
                            float reflected_degrees = (reflected_radians*180)/M_PI;
                            game->bullets[i].rotation = reflected_degrees;

                            game->bullets[i].bounces_left--;
                        }
                    }
                    
                }
            }
        }


        //Timer updates
        if(game->player1->fire_cooldown_ticks>0)game->player1->fire_cooldown_ticks--;
        if(game->player2->fire_cooldown_ticks>0)game->player2->fire_cooldown_ticks--;

     //   lv_timer_handler();
        vTaskDelayUntil(&prevTick,pdMS_TO_TICKS(16));  // was 33
     /*   static int cnt = 0;
        if (++cnt == 30) {
        xil_printf("vGameTask tick\r\n");
        cnt = 0;
        }     */  
    }
}