#include "lvgl.h"
#include "misc/lv_color.h"
#include "xil_printf.h"

#include <FreeRTOSConfig.h>
#include <lwip/sys.h>
#include <lwip/init.h>
#include <lwipopts.h>
#include <projdefs.h>

#include "game_abstraction.h"
#include "connection_udp.h"
#include "vdma.h"

#define THREAD_STACKSIZE 2048
#define PRIO_RENDER     1
#define PRIO_GAME       1
#define PRIO_INPUT      1
#define PRIO_LVGL_TICK  1

extern void main_thread(void *p);
void vGameTask(void *pvParameters);
void vRenderLoop(void *pvParameters);
void vInputHandler(void *pvParameters);
static void vLvglTickTask(void *pvParameters);

extern void lv_port_disp_init(void);


int main(void)
{
    Game gamestate = {
        .bullets = NULL,
        .bullets_size = 0,
        .particles = NULL,
        .particles_size = 0,
        .player1 = NULL,
        .player2 = NULL,
        .terrain = NULL,
        .terrain_size = 0
    };

	InitVDMA();
	StartVDMA();
    lv_init();
 
	lv_port_disp_init();

	xil_printf("init game start\r\n");
	init_game(&gamestate);    
	xil_printf("create thread\r\n");
  	sys_thread_new("main_thrd", main_thread, 0, THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

    BaseType_t r;


	xil_printf("create task vRenderLoop\r\n"); 
 	r =xTaskCreate(vRenderLoop, "Renderloop",3000,(void*)&gamestate,PRIO_RENDER ,NULL);
    if (r != pdPASS) xil_printf("vRenderLoop create FAILED\r\n");

    xil_printf("create task vGameTask\r\n");
    r = xTaskCreate(vGameTask, "Game", 1500, (void*)&gamestate, PRIO_GAME, NULL);
    if (r != pdPASS) xil_printf("vGameTask create FAILED\r\n");

    r = xTaskCreate(vInputHandler, "UserInput",1024,(void*)&gamestate,PRIO_INPUT,NULL);
    if (r != pdPASS) xil_printf("vInputHandler create FAILED\r\n");
    r = xTaskCreate(vLvglTickTask, "LVGL_Tick", 512, NULL, PRIO_LVGL_TICK, NULL);
	if (r != pdPASS) xil_printf("LVGL_Tick create FAILED\r\n");

 	xil_printf("Start Scheduler\r\n");
    vTaskStartScheduler();

    xil_printf("Scheduler returned (heap/stack issue)\r\n");


    while (1) {
        /* Doesn't come here */
    }
}


static void vLvglTickTask(void *pvParameters)
{
    (void) pvParameters;
    const TickType_t xDelay = pdMS_TO_TICKS(5); // 5 ms LVGL tick
    for (;;)
    {
        vTaskDelay(xDelay);   // Sleep 5 ms
        lv_tick_inc(5);
    }
}
