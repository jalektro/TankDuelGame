#include <stdio.h>
#include <string.h>
#include <xstatus.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#include "vdma.h"


//----- AXI VDMA + DDR -----
#define AXI_VDMA_ID            XPAR_AXI_VDMA_0_DEVICE_ID
#define AXI_VDMA_STREAM_WIDTH  24 // 24bit  changed to 16 needed for lvgl

#define DDR_BASE_ADDR    XPAR_PS7_DDR_0_BASEADDRESS
// Use 16 MB offset to avoid overwriting actual code regions at the beginning of the DDR!
#define FRAME_BUFFER_BASE_ADDR   (DDR_BASE_ADDR + 0x01000000)


XAxiVdma axi_vdma;
XAxiVdma_Config *axi_vdma_config;
XAxiVdma_DmaSetup axi_dma_read_config;

UINTPTR frame_buffer_addr[3];   // Using 3 frame buffers to match the hardware (IP) config

volatile int current_display_frame = 0;   // Set by your own logic
volatile int render_frame = 1;



void VDMA(void)
{

    uint8_t *buf = (uint8_t *)frame_buffer_addr[0];

    for (int i = 0; i < FRAME_SIZE_BYTES; i += 3) {
        buf[i + 0] = 0;    // G
        buf[i + 1] = 0;    // B
        buf[i + 2] = 255;  // R
    }
    Xil_DCacheFlushRange((UINTPTR)buf, FRAME_SIZE_BYTES);

    while (1) { /* do nothing */ }
}

void VDMA1(void)
{
   
   // InitFrameBuffers_RedBlue();

    int anim =0;
    while(1)
    {

    	/* Keep running */
        // --- Pick a free buffer ---
       // int index = GetFreeBufferIndex();
        u8 *buf = (u8*)frame_buffer_addr[0]; //index

        // --- Render frame ---
        ClearFrame(buf, 0, 0, 40);              // dark background
        DrawMovingBox(buf, anim );      // moving green box

        // --- Flush cache so VDMA sees it ---
        Xil_DCacheFlushRange((UINTPTR)buf, FRAME_SIZE_BYTES);

        // --- Display it ---
      //  PresentFrame();

        anim++;
    }
}



void InitFrameBuffers(void)
{
    // Set up frame buffer addresses in DDR (with proper spacing)
    frame_buffer_addr[0] = FRAME_BUFFER_BASE_ADDR;
    frame_buffer_addr[1] = FRAME_BUFFER_BASE_ADDR + FRAME_SIZE_BYTES;
    frame_buffer_addr[2] = FRAME_BUFFER_BASE_ADDR + (2 * FRAME_SIZE_BYTES);

    xil_printf("Frame buffer addresses:\r\n");
    xil_printf("  Buffer 0: 0x%08X\r\n", (u32)frame_buffer_addr[0]);
    xil_printf("  Buffer 1: 0x%08X\r\n", (u32)frame_buffer_addr[1]);
    xil_printf("  Buffer 2: 0x%08X\r\n", (u32)frame_buffer_addr[2]);
    xil_printf("Frame size: %d bytes\r\n", FRAME_SIZE_BYTES);

    Xil_DCacheFlushRange(FRAME_BUFFER_BASE_ADDR, 3 * FRAME_SIZE_BYTES);
} 

void InitVDMA(void)
{
	int status;

    InitFrameBuffers();
	// Lookup config
	axi_vdma_config = XAxiVdma_LookupConfig(XPAR_AXI_VDMA_0_BASEADDR);
	if(!axi_vdma_config)
	{
		xil_printf("No VDMA found for ID %d\r\n", XPAR_AXI_VDMA_0_BASEADDR);
	//	return XST_FAILURE;
	}

	xil_printf("VDMA Base Address: 0x%08X\r\n", (u32)axi_vdma_config->BaseAddress);

    status = XAxiVdma_CfgInitialize(&axi_vdma, axi_vdma_config, (UINTPTR)(axi_vdma_config->BaseAddress));
    if(status != XST_SUCCESS)
	{
		xil_printf("VDMA config init failed %d\r\n", status);
	//	return XST_FAILURE;
	}

	status = XAxiVdma_Selftest(&axi_vdma);
	if (status != XST_SUCCESS)
	{
		xil_printf("VDMA selftest failed %d\r\n", status);
	//	return XST_FAILURE;
	}

	xil_printf("VDMA selftest passed\r\n");

	// Configure read channel (MM2S)
	memset(&axi_dma_read_config, 0, sizeof(XAxiVdma_DmaSetup));
	axi_dma_read_config.VertSizeInput      = IMAGE_HEIGHT;
	axi_dma_read_config.HoriSizeInput      = IMAGE_WIDTH * IMAGE_BYTES_PER_PIXEL; // 24-bit RBG stream width
	axi_dma_read_config.Stride             = IMAGE_WIDTH * IMAGE_BYTES_PER_PIXEL; // memory width = 3 bytes
	axi_dma_read_config.FrameDelay         = 0;
	axi_dma_read_config.EnableCircularBuf  = 1;
	axi_dma_read_config.EnableSync         = 0;
	axi_dma_read_config.PointNum           = 0;
	axi_dma_read_config.EnableFrameCounter = 0;

	status = XAxiVdma_DmaConfig(&axi_vdma, XAXIVDMA_READ, &axi_dma_read_config);
	if(status != XST_SUCCESS)
	{
		xil_printf("Read channel config failed %d\r\n", status);
	//	return XST_FAILURE;
	}

	// Set all 3 frame buffer addresses
	status = XAxiVdma_DmaSetBufferAddr(&axi_vdma, XAXIVDMA_READ, frame_buffer_addr);
	if(status != XST_SUCCESS)
	{
		xil_printf("Set buffer addr failed %d\r\n", status);
	//	return XST_FAILURE;
	}
//    return XST_SUCCESS;
}

void StartVDMA(void)
{
	int status;

	status = XAxiVdma_DmaStart(&axi_vdma, XAXIVDMA_READ);
	if(status != XST_SUCCESS)
	{
		xil_printf("VDMA start failed %d\r\n", status);
	///	return XST_FAILURE;
	}

	xil_printf("VDMA started successfully.\r\n");
//    return XST_SUCCESS;
}




void ClearFrame(u8 *buf, u8 r, u8 g, u8 b)
{
    for(int i = 0; i < FRAME_SIZE_BYTES; i += 3) {
        buf[i+0] = r;
        buf[i+1] = g;
        buf[i+2] = b;
    }
}

void DrawMovingBox(u8 *buf, int posX)
{
    for (int y = 200; y < 300; y++) {
        for (int x = posX; x < posX + 100; x++) {

            if(x < 0 || x >= IMAGE_WIDTH) continue;

            int idx = (y * IMAGE_WIDTH + x) * 3;

            buf[idx+0] = 0;  // G
            buf[idx+1] = 255;    // B
            buf[idx+2] = 0;    // R
        }
    }
}

int GetRenderBuffer(void)
{
	/* Pick a buffer different from the one curently displayed */
	 if(current_display_frame == 0) 
	 {
		render_frame = 1;
	 } else if (current_display_frame == 1) {
	 render_frame =2;
	 } else {
	 render_frame = 0;
	 }
return render_frame;
}

void PresentRenderFrame(void)
{
    /* Tell VDMA to park on the new frame (pseudo API call) */
    XAxiVdma_StartParking(&axi_vdma, render_frame, XAXIVDMA_READ);

    current_display_frame = render_frame;
}