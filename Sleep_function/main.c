/*******************************************************************************************
 * File:    main.c
 * Target:  TM4C1294ncpdt
 * Author:  George Molyboga
 *******************************************************************************************
 * @brief
 * Light up the LED for a sec and turn it off preriodically

/*******************************************************************************************/

#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>
#define SYS_CLOCK_HZ 16000000

void sleep(int ms) {
    SYSCTL_RCGCTIMER_R |= (1 << 0);
    while(!(SYSCTL_PRTIMER_R & (1 << 0)));
    TIMER0_CTL_R &= ~0x01;
    TIMER0_CFG_R |= 0b000;
    TIMER0_TAMR_R = 0x1;

    TIMER0_TAILR_R = (SYS_CLOCK_HZ / 1000) * ms - 1;
    TIMER0_ICR_R = 0x01;
    TIMER0_CTL_R |= 0x01;

    while((TIMER0_RIS_R & (1<<0))==0);
    TIMER0_ICR_R = 0x01;
}

/*******************************************************************************************/

void configure_pins() {
    SYSCTL_RCGCGPIO_R |= 0x08;               // Set Port D to active
    while((SYSCTL_PRGPIO_R & 0x08) == 0) {};
    GPIO_PORTD_AHB_DIR_R |= 0x01;            // Set PD0 as output
    GPIO_PORTD_AHB_DEN_R |= 0x01;            // Enable digital on PD0
}

/*******************************************************************************************/

/*******************************************************************************************/

int main(void) {
    configure_pins();
    while(1) {
        GPIO_PORTD_AHB_DATA_R |= 0x01;    // Set PD0 High
        sleep(500);                       // 500ms pause
        GPIO_PORTD_AHB_DATA_R &= ~0x01;   // Set PD0 Low
        sleep(500);                       // 500ms pause
    }
}