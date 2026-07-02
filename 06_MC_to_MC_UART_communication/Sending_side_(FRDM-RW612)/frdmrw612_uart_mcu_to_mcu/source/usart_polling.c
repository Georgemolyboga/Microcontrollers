/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * UART wiring for this project:
 *   NXP FRDM-RW612 sends sensor packets on FLEXCOMM3/USART3.
 *   NXP USART3 TXD: GPIO_26 -> TI TM4C1294 UART6 RX: PP0 / P0.
 *   NXP USART3 RXD: GPIO_24 <- TI TM4C1294 UART6 TX: PP1 / P1, only needed for two-way UART.
 *   Connect board GND to board GND.
 *
 * Note: this MCUXpresso project also uses FLEXCOMM3/USART3 for the NXP debug console.
 * PRINTF output therefore shares the same USART unless the debug console is moved.
 */
#include "board.h"
#include "app.h"
#include "fsl_usart.h"
#include "fsl_debug_console.h"
#include "peripherals.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TI_UART_BAUDRATE     9600U
#define TI_UART_PACKET_DELAY 1000000U
#define TERMINAL_PRINT_ENABLED 1U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

char* receive_fake_sensor_packet(void);
static void terminal_print_packet(const char* packet);

/*******************************************************************************
 * Code
 ******************************************************************************/

char* receive_fake_sensor_packet(void)
{
    static char* fake_packets[] = {
        "DATA:80\r\n",      /* clear path      --> move forward */
        "DATA:35\r\n",      /* getting close   --> avoid obstacle (arbitrary left/right) */
        "DATA:10\r\n",      /* too close       --> stop */
        "INVALID\r\n"       /* invalid message --> STATE_ERROR */
    };
    static uint32_t packet_index = 0; /* Keeps value even when variable is out of scope */

    char* packet = fake_packets[packet_index];

    packet_index++;
    if (packet_index >= (sizeof(fake_packets) / sizeof(fake_packets[0])))
    {
        packet_index = 0;
    }

    return packet;
}

static void send_sensor_packet(const char* packet)
{
    size_t length = 0U;

    while (packet[length] != '\0')
    {
        length++;
    }

    USART_WriteBlocking(FLEXCOMM0_PERIPHERAL, (const uint8_t*)packet, length);
    PRINTF("%s\r\n", packet);
}

/*!
 * @brief Main function
 */
int main(void)
{
    char* sensor_packet;
    usart_config_t config;

    BOARD_InitHardware();
    BOARD_InitPeripherals();

//    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);

    while (1)
    {
        /* Does not work correctly yet maybe (at least Console doesnt, but maybe because it uses same USART instance)*/
        sensor_packet = receive_fake_sensor_packet();
        send_sensor_packet(sensor_packet);
        SDK_DelayAtLeastUs(TI_UART_PACKET_DELAY, SystemCoreClock);
    }
}
