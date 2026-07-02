/*******************************************************************************************
 * File:    main.c
 * Target:  TM4C1294ncpdt
 * Author:  George Molyboga
 *******************************************************************************************
 * @brief
 * Utrasonic Sonar distance measurements

/*******************************************************************************************/

/**
 * @brief TIM0 A (periodic) and B (capture) peripheral configuration
 *          - Activate timer clock          (SYSCTL_RCGCTIMER_R)
 *          - Wait for stabilization        (SYSCTL_PRTIMER_R)
 *          - Stop timer                    (TIMERx_CTL_R)
 *          - Configure amount of bits      (TIMERx_CFG_R)
 *          - Operation mode                (TIMERx_TnMR_R)
 *          - Prescaler                     (TIMERx_TnPR_R)
 *          - Interval load value           (TIMERx_TnILR_R)
 *          - Match value                   (TIMERx_TnMATCHR_R)
 *          - Start timer                   (TIMERx_CTL_R)
 */

#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>
#define SYS_CLOCK_HZ 16000000

volatile uint32_t pulse_ticks;
volatile uint32_t distance_cm;

/*******************************************************************************************/

void wait(int us) {
    SYSCTL_RCGCTIMER_R |= (1 << 0);
    while(!(SYSCTL_PRTIMER_R & (1 << 0)));
    TIMER0_CTL_R &= ~0x01;
    TIMER0_CFG_R |= 0b000;
    TIMER0_TAMR_R = 0x1;

    TIMER0_TAILR_R = (SYS_CLOCK_HZ / 1000000) * us - 1;
    TIMER0_ICR_R = 0x01;
    TIMER0_CTL_R |= 0x01;

    while((TIMER0_RIS_R & (1<<0))==0);
    TIMER0_ICR_R = 0x01;
}

/*******************************************************************************************/

void configure_timer() {
    SYSCTL_RCGCTIMER_R |= (1 << 1);
    while(!(SYSCTL_PRTIMER_R & (1 << 1)));

    TIMER1_CTL_R &= ~(1 << 8);
    TIMER1_CFG_R = 0x04;
    TIMER1_TBMR_R = 0x17;
    TIMER1_CTL_R |= (0x03 << 10);

    TIMER1_TBPR_R = 0xFF;
    TIMER1_TBILR_R = 65535;      // Enabling a full 16-bit range load interval
    TIMER1_CTL_R |= (1 << 8);
}

/*******************************************************************************************/

void configure_pins() {
    SYSCTL_RCGCGPIO_R |= (1 << 0);               // Set Port A to active
    while((SYSCTL_PRGPIO_R & (1 << 0)) == 0) {};
    
    GPIO_PORTA_AHB_DIR_R |= (1 << 2);            // Set PA2 as output
    GPIO_PORTA_AHB_DEN_R |= (1 << 2);            // Enable digital on PA2

    GPIO_PORTA_AHB_DIR_R &= ~(1 << 3);           // Set PA3 as input
    GPIO_PORTA_AHB_DEN_R |= (1 << 3);            // Enable digital on PA3
    GPIO_PORTA_AHB_AFSEL_R |= (1 << 3);          // Enable alternate function on PA3

    GPIO_PORTA_AHB_PCTL_R = (GPIO_PORTA_AHB_PCTL_R & ~0x0000F000) | (3 << 12);
}

/*******************************************************************************************/

void trigger_sensor() {
    GPIO_PORTA_AHB_DATA_R |= (1 << 2);
    wait(10);
    GPIO_PORTA_AHB_DATA_R &= ~(1 << 2);
}

/*******************************************************************************************/
void measure_distance() {
    uint32_t start_time, end_time;

    trigger_sensor();
    TIMER1_ICR_R = (1 << 10);
    while((TIMER1_RIS_R & (1 << 10)) == 0);
    start_time = TIMER1_TBR_R;
    TIMER1_ICR_R = (1 << 10);
    while((TIMER1_RIS_R & (1 << 10)) == 0);
    end_time = TIMER1_TBR_R;

    // The & 0x00FFFFFF masks out the upper 8 bits, fixing any wrap-around math errors!
    pulse_ticks = (end_time - start_time) & 0x00FFFFFF;
}

/*******************************************************************************************/

uint32_t calculate_distance_cm() {
    /*each tick represents 1 / 16,000,000 of a second. 
    To get the distance in centimeters, you multiply the 
    total time by the speed of sound (34,300 cm/s) and divide by 2 
    (since the sound travels out and bounces back).
    
    A helpful shortcut for a 16MHz clock using integer math is dividing the ticks by roughly 933*/

    distance_cm = pulse_ticks / 933; 
    return distance_cm;
}

/*******************************************************************************************/

int main(void) {
    configure_pins();
    configure_timer();
    while(1) {
        measure_distance();
        wait(200000);
        printf("Distance to the sensor: %u cm\n", calculate_distance_cm());
    }
}