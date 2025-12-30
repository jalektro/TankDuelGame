#include <projdefs.h>
#include <stdio.h>
#include <math.h>
#include "xil_printf.h"
#include "lvgl.h"
#include "game_abstraction.h"
#include "connection_udp.h"



// used for vdma buffer
extern UINTPTR frame_buffer_addr[3];
extern volatile int current_display_frame;
extern volatile int next_draw_frame;
static lv_obj_t *ip_label = NULL;

void ui_set_ip_label(const char *ip)
{
    if (ip_label) {
        static char buf[24];
        snprintf(buf, sizeof(buf), "IP: %s", ip);
        lv_label_set_text(ip_label, buf);
    }
}

/* ---------------- UI health widget ---------------- */

void ui_create_health_widget(lv_obj_t *parent,
                             lv_obj_t **bar,
                             lv_color_t color,
                             lv_align_t align)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    if(align == LV_ALIGN_BOTTOM_LEFT) {
        lv_obj_align(cont, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    }
    else if(align == LV_ALIGN_BOTTOM_RIGHT) {
        lv_obj_align(cont, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    }

    lv_obj_t *label = lv_label_create(cont);
    lv_label_set_text(label, "health");
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);

    *bar = lv_bar_create(cont);
    lv_obj_set_size(*bar, 120, 10);
    lv_bar_set_range(*bar, 0, 100);
    lv_bar_set_value(*bar, 75, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(*bar, color, LV_PART_INDICATOR);
    lv_obj_set_style_radius(*bar, 3, 0);

    lv_obj_set_style_pad_row(cont, 6, 0);

}

/* ---------------- Helpers: colors from game RGB ---------------- */

static lv_color_t game_color_to_lv(const RGB *c)
{
    uint32_t rgb =
        ((uint32_t)c->r << 16) |
        ((uint32_t)c->g << 8)  |
        ((uint32_t)c->b);
    return lv_color_hex(rgb);
}

/* ---------------- Rotated rects via canvas polygons ---------------- */

static void draw_rotated_rect(float angle_deg,
                              lv_obj_t *canvas,
                              Game_Coordinate pos,
                              lv_color_t color,
                              float width,
                              float height)
{
    lv_point_t pts[4];
    float rad   = angle_deg * 3.14159265f / 180.0f;
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);

    float dx[4] = { -width / 2,  width / 2,  width / 2, -width / 2 };
    float dy[4] = { -height / 2, -height / 2, height / 2,  height / 2 };

    for(int i = 0; i < 4; i++) {
        pts[i].x = (lv_coord_t)(pos.x + dx[i] * cos_a - dy[i] * sin_a);
        pts[i].y = (lv_coord_t)(pos.y + dx[i] * sin_a + dy[i] * cos_a);
    }

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color;
    dsc.bg_opa   = LV_OPA_COVER;

    lv_canvas_draw_polygon(canvas, pts, 4, &dsc);
}

static void draw_rotated_offset_rect(float angle_deg,
                                     float offset,
                                     lv_obj_t *canvas,
                                     Game_Coordinate pos,
                                     lv_color_t color,
                                     float width,
                                     float height)
{
    lv_point_t pts[4];
    float rad   = angle_deg * 3.14159265f / 180.0f;
    float cos_a = cosf(rad);
    float sin_a = sinf(rad);

    float dx[4] = { -width / 2,  width / 2,  width / 2, -width / 2 };
    float dy[4] = { -height / 2, -height / 2, height / 2,  height / 2 };

    float x_offset = cos_a * offset;
    float y_offset = sin_a * offset;

    for(int i = 0; i < 4; i++) {
        pts[i].x = (lv_coord_t)((pos.x + dx[i] * cos_a - dy[i] * sin_a) + x_offset);
        pts[i].y = (lv_coord_t)((pos.y + dx[i] * sin_a + dy[i] * cos_a) + y_offset);
    }

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color;
    dsc.bg_opa   = LV_OPA_COVER;

    lv_canvas_draw_polygon(canvas, pts, 4, &dsc);
}

/* ---------------- Render loop task ---------------- */
void vRenderLoop(void *pvParameters)
{
    
   
    Game *game = (Game *)pvParameters;
   // xil_printf("vRenderLoop start\r\n");

    
    /* Terrain canvas */
  
    lv_obj_t *canvas = lv_canvas_create(lv_scr_act());
   static lv_color_t terrain_buf[SCREEN_WIDTH * SCREEN_HEIGHT];
    lv_canvas_set_buffer(canvas,
                         terrain_buf,
                         SCREEN_WIDTH,
                         SCREEN_HEIGHT,
                         LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(canvas, lv_color_hex3(0xfff), LV_OPA_COVER);
    lv_obj_center(canvas);

    /* Draw terrain polygons */
    for(int i = 0; i < game->terrain_size; i++) {
        lv_point_t pts[32];
        for(int j = 0; j < game->terrain[i].points; j++) {
            pts[j].x = (lv_coord_t)game->terrain[i].list[j].x;
            pts[j].y = (lv_coord_t)game->terrain[i].list[j].y;
        }

        lv_draw_rect_dsc_t dsc;
        lv_draw_rect_dsc_init(&dsc);
        dsc.bg_color = lv_color_hex(0xFF0000);
        dsc.bg_opa   = LV_OPA_COVER;

        lv_canvas_draw_polygon(canvas, pts, game->terrain[i].points, &dsc);
    }

    /* Foreground canvas */
    
    lv_obj_t *foreground_canvas = lv_canvas_create(lv_scr_act());
   static lv_color_t foreground_buf[SCREEN_WIDTH * SCREEN_HEIGHT];
    lv_canvas_set_buffer(foreground_canvas,
                         foreground_buf,
                         SCREEN_WIDTH,
                         SCREEN_HEIGHT,
                         LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_canvas_fill_bg(foreground_canvas, lv_color_hex3(0x000), LV_OPA_TRANSP);
    lv_obj_center(foreground_canvas);

    /* UI elements */
    lv_obj_t *p1_bar = NULL;
    lv_obj_t *p2_bar = NULL;

    lv_color_t p1_color = game_color_to_lv(&game->player1->color);
    lv_color_t p2_color = game_color_to_lv(&game->player2->color);

    ui_create_health_widget(lv_scr_act(),
                            &p1_bar,
                            p1_color,
                            LV_ALIGN_BOTTOM_LEFT);
    ui_create_health_widget(lv_scr_act(),
                            &p2_bar,
                            p2_color,
                            LV_ALIGN_BOTTOM_RIGHT);

    // Create IP label once
    ip_label = lv_label_create(lv_scr_act());
    lv_label_set_text(ip_label, "IP: 0.0.0.0");
    lv_obj_align(ip_label, LV_ALIGN_TOP_RIGHT, -10, 10);

    TickType_t prevTick = xTaskGetTickCount();
    uint64_t tickcount  = 0;

    for(;;) {
        /* Clear foreground */
        lv_canvas_fill_bg(foreground_canvas, lv_color_hex3(0x000), LV_OPA_TRANSP);

        /* Player 1 tank */
        lv_color_t player1_color = p1_color;
        draw_rotated_rect(game->player1->tank_rotation,
                          foreground_canvas,
                          game->player1->position,
                          player1_color,
                          TANK_WIDTH,
                          TANK_HEIGT);

        /* Player 1 turret */
        lv_draw_rect_dsc_t turret_desc;
        lv_draw_rect_dsc_init(&turret_desc);
        turret_desc.bg_color = lv_color_hex(0x555555);
        turret_desc.bg_opa   = LV_OPA_COVER;
        turret_desc.radius   = LV_RADIUS_CIRCLE;

        lv_area_t turret_area;
       /* lv_area_set(&turret_area,
                    game->player1->position.x - TANK_TURRET_RADIUS,
                    game->player1->position.y - TANK_TURRET_RADIUS,
                    game->player1->position.x + TANK_TURRET_RADIUS,
                    game->player1->position.y + TANK_TURRET_RADIUS); 
        lv_canvas_draw_rect(foreground_canvas, &turret_area, &turret_desc);*/
        lv_coord_t x1 = game->player1->position.x - TANK_TURRET_RADIUS;
        lv_coord_t y1 = game->player1->position.y - TANK_TURRET_RADIUS;
        lv_coord_t w  = 2 * TANK_TURRET_RADIUS;
        lv_coord_t h  = 2 * TANK_TURRET_RADIUS;

        lv_canvas_draw_rect(foreground_canvas, x1, y1, w, h, &turret_desc);

        draw_rotated_offset_rect(game->player1->turret_rotation,
                                 (float)(TANK_TURRET_LENGTH / 2),
                                 foreground_canvas,
                                 game->player1->position,
                                 lv_color_hex3(0x555),
                                 TANK_TURRET_LENGTH,
                                 TANK_TURRET_WIDTH);

        /* Player 2 tank */
        lv_color_t player2_color = p2_color;
        draw_rotated_rect(game->player2->tank_rotation,
                          foreground_canvas,
                          game->player2->position,
                          player2_color,
                          TANK_WIDTH,
                          TANK_HEIGT);

        lv_draw_rect_dsc_init(&turret_desc);
        turret_desc.bg_color = lv_color_hex(0x555555);
        turret_desc.bg_opa   = LV_OPA_COVER;
        turret_desc.radius   = LV_RADIUS_CIRCLE;

        lv_coord_t x2 = game->player2->position.x - TANK_TURRET_RADIUS;
        lv_coord_t y2 = game->player2->position.y - TANK_TURRET_RADIUS;
        lv_coord_t w2 = 2 * TANK_TURRET_RADIUS;
        lv_coord_t h2 = 2 * TANK_TURRET_RADIUS;

        lv_canvas_draw_rect(foreground_canvas, x2, y2, w2, h2, &turret_desc);
    /*
        lv_area_set(&turret_area,
                    game->player2->position.x - TANK_TURRET_RADIUS,
                    game->player2->position.y - TANK_TURRET_RADIUS,
                    game->player2->position.x + TANK_TURRET_RADIUS,
                    game->player2->position.y + TANK_TURRET_RADIUS);
        lv_canvas_draw_rect(foreground_canvas, &turret_area, &turret_desc);
*/
        draw_rotated_offset_rect(game->player2->turret_rotation,
                                 (float)(TANK_TURRET_LENGTH / 2),
                                 foreground_canvas,
                                 game->player2->position,
                                 lv_color_hex3(0x555),
                                 TANK_TURRET_LENGTH,
                                 TANK_TURRET_WIDTH);

        /* Bullets */
        for(int i = 0; i < game->max_bullets; i++) {
            if(!game->bullets[i].alive) continue;

            draw_rotated_rect(game->bullets[i].rotation,
                              foreground_canvas,
                              game->bullets[i].position,
                              lv_color_hex3(0x888),
                              BULLET_LENGTH,
                              BULLET_WIDTH);
        }

        /* Health bars */
        lv_bar_set_value(p1_bar, game->player1->health, LV_ANIM_OFF);
        lv_bar_set_value(p2_bar, game->player2->health, LV_ANIM_OFF);

   
        tickcount++;
        lv_timer_handler();
       // xil_printf("vRenderLoop tick\r\n");
        vTaskDelayUntil(&prevTick, pdMS_TO_TICKS(16)); // was 33
    }
}