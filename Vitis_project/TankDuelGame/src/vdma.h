#ifndef VDMA_H
#define VDMA_H

#include <stdint.h>
#include "xaxivdma.h"

//----- AXI VDMA + DDR -----
#define AXI_VDMA_ID            XPAR_AXI_VDMA_0_DEVICE_ID
#define AXI_VDMA_STREAM_WIDTH  24 // 24bit

#define DDR_BASE_ADDR    XPAR_PS7_DDR_0_BASEADDRESS
// Use 16 MB offset to avoid overwriting actual code regions at the beginning of the DDR!
#define FRAME_BUFFER_BASE_ADDR   (DDR_BASE_ADDR + 0x01000000)

// The Video Timing Controller is configured in 720p mode (1280*720)
#define IMAGE_WIDTH            1280
#define IMAGE_HEIGHT           720

#define IMAGE_BYTES_PER_PIXEL  3   // RBG packing, no padding
#define FRAME_STRIDE         (IMAGE_WIDTH * IMAGE_BYTES_PER_PIXEL)
#define FRAME_SIZE_BYTES       (IMAGE_WIDTH * IMAGE_HEIGHT * IMAGE_BYTES_PER_PIXEL)

extern XAxiVdma axi_vdma;
extern XAxiVdma_Config *axi_vdma_config;
extern XAxiVdma_DmaSetup axi_dma_read_config;

extern UINTPTR frame_buffer_addr[3];   // Using 3 frame buffers to match the hardware (IP) config

extern volatile int current_display_frame;   // Set by your own logic
extern volatile int render_frame;

void InitVDMA(void);
void InitFrameBuffers(void);            // done inside InitVDMA needed for more buffers
void StartVDMA(void);
void ClearFrame(u8 *buf, u8 r, u8 g, u8 b);
void DrawMovingBox(u8 *buf, int posX);
void VDMA(void);
int GetRenderBuffer(void);
void PresentRenderFrame(void);

#endif //VDMA_H