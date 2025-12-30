#include "xil_printf.h"
#include "xil_cache.h"

#include "lvgl/lvgl.h"
#include "vdma.h"

#define BUF_LINES 40

static lv_color_t buf1[IMAGE_WIDTH * BUF_LINES];
static lv_disp_draw_buf_t disp_buf;
extern UINTPTR frame_buffer_addr[3];
extern volatile int current_display_frame;
extern volatile int render_frame;


void my_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    // xil_printf("FLUSH %d,%d -> %d,%d\r\n", area->x1, area->y1, area->x2, area->y2);

    uint8_t *dst_base =(uint8_t *)frame_buffer_addr[render_frame];

    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
        for (int32_t y = 0; y < h; y++) {
            uint8_t *dst = dst_base +
                        (area->y1 + y) * FRAME_STRIDE +
                        area->x1 * IMAGE_BYTES_PER_PIXEL;

            for (int32_t x = 0; x < w; x++) {
                uint32_t c = color_p->full;   // ARGB8888
                color_p++;

                uint8_t r = (c >> 16) & 0xFF;
                uint8_t g = (c >>  8) & 0xFF;
                uint8_t b =  c        & 0xFF;

                dst[0] = g;   // G
                dst[1] = b;   // B
                dst[2] = r;   // R
                dst += 3;
            }
        }

    UINTPTR flush_addr = (UINTPTR)(dst_base + area->y1 * FRAME_STRIDE);
    UINTPTR flush_size = (UINTPTR)(h * FRAME_STRIDE);
    Xil_DCacheFlushRange(flush_addr, flush_size);

    // When LVGL is done drawing the whole frame, switch VDMA to it
    PresentRenderFrame();   // parks VDMA on render_frame and updates current_display_frame

    lv_disp_flush_ready(drv);
}

void lv_port_disp_init(void)
{
    lv_disp_draw_buf_init(&disp_buf,
                          buf1,
                          NULL,
                          IMAGE_WIDTH * BUF_LINES);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = IMAGE_WIDTH;
    disp_drv.ver_res = IMAGE_HEIGHT;
    disp_drv.flush_cb = my_flush_cb;
    disp_drv.draw_buf = &disp_buf;

    lv_disp_drv_register(&disp_drv);
}

