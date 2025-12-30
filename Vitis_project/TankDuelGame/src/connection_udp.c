#include "FreeRTOS.h"
#include "task.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "netif/xadapter.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/sockets.h"
#include "connection_udp.h"
#include <lwipopts.h>
#include <stdio.h>
#include "lwip/sys.h"

#define THREAD_STACKSIZE    1024
#define RECVBUFSIZE   2048
#define PORT_PLAYER1  5001
#define PORT_PLAYER2  5002

static struct netif server_netif;

uint8_t g_player1_cmd =0;
uint8_t g_player2_cmd =0;
volatile uint8_t g_player1_cmd_valid =0;
volatile uint8_t g_player2_cmd_valid =0;
uint8_t g_player1_movements[TOTALMOVEMENTS] = {0};
uint8_t g_player2_movements[TOTALMOVEMENTS] = {0};
char g_board_ip_str[16] = "0.0.0.0"; // for display

extern void ui_set_ip_label(const char *ip);


static void set_ip_string(void)
{
    snprintf(g_board_ip_str, sizeof(g_board_ip_str), "%d.%d.%d.%d",
             ip4_addr1(&server_netif.ip_addr),
             ip4_addr2(&server_netif.ip_addr),
             ip4_addr3(&server_netif.ip_addr),
             ip4_addr4(&server_netif.ip_addr));
    ui_set_ip_label(g_board_ip_str);
}
/* ---------- small helpers ---------- */

static void print_ip(char *msg, ip_addr_t *ip)
{
    xil_printf(msg);
    xil_printf("%d.%d.%d.%d\n\r",
               ip4_addr1(ip), ip4_addr2(ip),
               ip4_addr3(ip), ip4_addr4(ip));
}

static void print_ip_settings(ip_addr_t *ip,
                              ip_addr_t *mask,
                              ip_addr_t *gw)
{
    print_ip("Board IP: ", ip);
    print_ip("Netmask : ", mask);
    print_ip("Gateway : ", gw);
}
/*
static void print_udp_data(const char *tag,
                           const char *buf,
                           int len,
                           const struct sockaddr_in *from)
{

    ip4_addr_t addr;
    addr.addr = from->sin_addr.s_addr;

    xil_printf("[%s] %d bytes from %d.%d.%d.%d:%d: ",
               tag, len,
               ip4_addr1(&addr),
               ip4_addr2(&addr),
               ip4_addr3(&addr),
               ip4_addr4(&addr),
               ntohs(from->sin_port));
    for (int i = 0; i < len; i++) {
        xil_printf("%02x",(unsigned char) buf[i]);
    }
    xil_printf("\r\n");
}
*/
static void print_udp_app_header(void)
{
    xil_printf("%20s %6d %s\r\n",
               "UDP server", PORT_PLAYER1,
               "2-player UDP control");
}

/* ---------- UDP server task ---------- */

static void udp_server_thread(void *p)
{
    int sock1 = -1, sock2 = -1;
    struct sockaddr_in addr1, addr2;
    struct sockaddr_in from;
    char recvbuf[RECVBUFSIZE];
    socklen_t fromlen;
    int n;

    LWIP_UNUSED_ARG(p);

    sock1 = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    sock2 = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if ((sock1 < 0) || (sock2 < 0)) {
        xil_printf("Error creating UDP sockets\r\n");
        goto done;
    }

    memset(&addr1, 0, sizeof(addr1));
    addr1.sin_family      = AF_INET;
    addr1.sin_port        = htons(PORT_PLAYER1);
    addr1.sin_addr.s_addr = INADDR_ANY;
    if (lwip_bind(sock1,
                  (struct sockaddr *)&addr1,
                  sizeof(addr1)) < 0) {
        xil_printf("Error binding UDP socket on port %d\r\n",
                   PORT_PLAYER1);
        goto done;
    }

    memset(&addr2, 0, sizeof(addr2));
    addr2.sin_family      = AF_INET;
    addr2.sin_port        = htons(PORT_PLAYER2);
    addr2.sin_addr.s_addr = INADDR_ANY;
    if (lwip_bind(sock2,
                  (struct sockaddr *)&addr2,
                  sizeof(addr2)) < 0) {
        xil_printf("Error binding UDP socket on port %d\r\n",
                   PORT_PLAYER2);
        goto done;
    }

    xil_printf("UDP server ready on ports %d and %d\r\n",
               PORT_PLAYER1, PORT_PLAYER2);

    while (1) {
        fd_set readfds;
        int maxfdp1;

        FD_ZERO(&readfds);
        FD_SET(sock1, &readfds);
        FD_SET(sock2, &readfds);
        maxfdp1 = (sock1 > sock2 ? sock1 : sock2) + 1;

        if (lwip_select(maxfdp1,
                        &readfds, NULL, NULL, NULL) < 0) {
            xil_printf("select error\r\n");
            break;
        }

        if (FD_ISSET(sock1, &readfds)) {
            fromlen = sizeof(from);
            n = lwip_recvfrom(sock1,
                              recvbuf,
                              RECVBUFSIZE,
                              0,
                              (struct sockaddr *)&from,
                              &fromlen);
            if (n > 0) {
               // print_udp_data("P1", recvbuf, n, &from);
                uint8_t bits = (uint8_t)recvbuf[0];
                g_player1_cmd = bits;
                g_player1_cmd_valid = 1;

                if (xSemaphoreTake(keyboard_semaphore, 0) == pdTRUE) {
                    memset(g_player1_movements, 0, TOTALMOVEMENTS);

                    if (bits & (1 << 0)) g_player1_movements[MOVEFORWARD] = 1;
                    if (bits & (1 << 1)) g_player1_movements[MOVEBACK]    = 1;
                    if (bits & (1 << 2)) g_player1_movements[MOVELEFT]    = 1;
                    if (bits & (1 << 3)) g_player1_movements[MOVERIGHT]   = 1;
                    if (bits & (1 << 4)) g_player1_movements[GUNLEFT]     = 1;
                    if (bits & (1 << 5)) g_player1_movements[GUNRIGHT]    = 1;
                    if (bits & (1 << 6)) g_player1_movements[SHOOT]       = 1;
                    xSemaphoreGive(keyboard_semaphore);
                }
            }
        }

        if (FD_ISSET(sock2, &readfds)) {
            fromlen = sizeof(from);
            n = lwip_recvfrom(sock2,
                              recvbuf,
                              RECVBUFSIZE,
                              0,
                              (struct sockaddr *)&from,
                              &fromlen);
            if (n > 0) {
             //   print_udp_data("P2", recvbuf, n, &from);
                                uint8_t bits = (uint8_t)recvbuf[0];
                g_player2_cmd = bits;
                g_player2_cmd_valid = 1;

                if (xSemaphoreTake(keyboard_semaphore, 0) == pdTRUE) {
                    memset(g_player2_movements, 0, TOTALMOVEMENTS);

                    if (bits & (1 << 0)) g_player2_movements[MOVEFORWARD] = 1;
                    if (bits & (1 << 1)) g_player2_movements[MOVEBACK]    = 1;
                    if (bits & (1 << 2)) g_player2_movements[MOVELEFT]    = 1;
                    if (bits & (1 << 3)) g_player2_movements[MOVERIGHT]   = 1;
                    if (bits & (1 << 4)) g_player2_movements[GUNLEFT]     = 1;
                    if (bits & (1 << 5)) g_player2_movements[GUNRIGHT]    = 1;
                    if (bits & (1 << 6)) g_player2_movements[SHOOT]       = 1;

                    xSemaphoreGive(keyboard_semaphore);
                }
            }
        }
    }

done:
    if (sock1 >= 0)
        lwip_close(sock1);
    if (sock2 >= 0)
        lwip_close(sock2);
    vTaskDelete(NULL);
}

/* ---------- network thread ---------- */

static void network_thread(void *p)
{
    xil_printf("Starting DHCP on netif\r\n");
    struct netif *netif;
    unsigned char mac_ethernet_address[] =
        { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };
    ip_addr_t ipaddr, netmask, gw;
    int mscnt = 0;

    LWIP_UNUSED_ARG(p);

    netif = &server_netif;

    xil_printf("\r\n\r\n");
    xil_printf("-----network thread setting ------\r\n");

    ipaddr.addr  = 0;
    gw.addr      = 0;
    netmask.addr = 0;

    if (!xemac_add(netif, &ipaddr, &netmask, &gw,
                   mac_ethernet_address, XPAR_XEMACPS_0_BASEADDR)) {
        xil_printf("Error adding N/W interface\r\n");
        return;
    }

    netif_set_default(netif);
    netif_set_up(netif);

    sys_thread_new("xemacif_input_thread",
                   (void (*)(void *))xemacif_input_thread,
                   netif,
                   THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO);

    dhcp_start(netif);

    while (1) {
        vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
        dhcp_fine_tmr();
        mscnt += DHCP_FINE_TIMER_MSECS;
        if (mscnt >= DHCP_COARSE_TIMER_SECS * 1000) {
            dhcp_coarse_tmr();
            mscnt = 0;
        }
    }
}

/* ---------- main FreeRTOS entry ---------- */

void main_thread(void *p)
{
    int mscnt = 0;
 
    LWIP_UNUSED_ARG(p);

    lwip_init();

    sys_thread_new("NW_THRD",
                   network_thread,
                   NULL,
                   THREAD_STACKSIZE,
                   DEFAULT_THREAD_PRIO);

    while (1) {
        vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);

        if (server_netif.ip_addr.addr) {
            xil_printf("DHCP request success\r\n");
            print_ip_settings(&(server_netif.ip_addr),
                              &(server_netif.netmask),
                              &(server_netif.gw));

        snprintf(g_board_ip_str, sizeof(g_board_ip_str), "%d.%d.%d.%d",
             ip4_addr1(&server_netif.ip_addr),
             ip4_addr2(&server_netif.ip_addr),
             ip4_addr3(&server_netif.ip_addr),
             ip4_addr4(&server_netif.ip_addr)); // this is added to show IP on screen top right corner
             set_ip_string();   
            print_udp_app_header();
            xil_printf("\r\n");

            sys_thread_new("udp_srv",
                           udp_server_thread,
                           0,
                           THREAD_STACKSIZE,
                           DEFAULT_THREAD_PRIO);
            break;
        }

        mscnt += DHCP_FINE_TIMER_MSECS;
        if (mscnt >= DHCP_COARSE_TIMER_SECS * 2000) {
            xil_printf("ERROR: DHCP request timed out\r\n");
            xil_printf("Configuring default IP of 192.168.1.10\r\n");

            IP4_ADDR(&(server_netif.ip_addr),  192, 168, 1, 10);
            IP4_ADDR(&(server_netif.netmask), 255, 255, 255, 0);
            IP4_ADDR(&(server_netif.gw),      192, 168, 1, 1);

            print_ip_settings(&(server_netif.ip_addr),
                              &(server_netif.netmask),
                              &(server_netif.gw));

            xil_printf("\r\n");
            xil_printf("%20s %6s %s\r\n",
                       "Server", "Port", "Connect With..");
            xil_printf("%20s %6s %s\r\n",
                       "--------------------",
                       "------",
                       "--------------------");

            print_udp_app_header();
            xil_printf("\r\n");

            sys_thread_new("udp_srv",
                           udp_server_thread,
                           0,
                           THREAD_STACKSIZE,
                          DEFAULT_THREAD_PRIO);
            break;
        }
    }

    vTaskDelete(NULL);
}
