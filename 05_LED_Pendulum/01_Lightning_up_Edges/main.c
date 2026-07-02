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

/*******************************************************************************************/

void configure_pins() {
    SYSCTL_RCGCGPIO_R |= (1 << 10) | (1 << 11);
    while((SYSCTL_PRGPIO_R & ((1 << 10) | (1 << 11))) != ((1 << 10) | (1 << 11))) {};
    
    GPIO_PORTM_DIR_R |= (0xFF);              // Set PM 0-7 as output
    GPIO_PORTM_DEN_R |= (0xFF);             // Enable digital on PM 0-7

    GPIO_PORTL_DIR_R &= ~(1 << 0);           // Set PL0 as input
    GPIO_PORTL_DEN_R |= (1 << 0);            // Enable digital on PL0
    GPIO_PORTL_AFSEL_R &= ~(1 << 0);         // Disable alternate function on PL0

    // Configure GPIO Interrupts for PD3
    GPIO_PORTL_IM_R &= ~(1 << 0);            // Mask interrupt during setup
    GPIO_PORTL_IS_R &= ~(1 << 0);            // Edge-sensitive
    GPIO_PORTL_IBE_R |= (1 << 0);            // Trigger on BOTH edges
    GPIO_PORTL_ICR_R = (1 << 0);             // Clear any prior flags
    GPIO_PORTL_IM_R |= (1 << 0);             // Unmask (Enable) interrupt

    NVIC_EN1_R |= (1 << 21);                     // Enable NVIC for IRQ 53 (GPIO Port L)
}


/*******************************************************************************************/

void myInterrupt(void) {
    GPIO_PORTL_ICR_R = (1 << 0);   // Acknowledge and clear the interrupt flag for PL0

    GPIO_PORTM_DATA_R = 0xFF;

    for(volatile uint32_t i = 0; i < 5000; i++) {} 

    GPIO_PORTM_DATA_R = 0x00;
}


/*******************************************************************************************/

int main(void) {
    configure_pins();
    
    while(1) {
        
    }
}