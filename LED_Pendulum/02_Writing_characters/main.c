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
#include "characters.h"
#define SYS_CLOCK_HZ 16000000

// Half period = 1000000 / (8.3 * 2) = ~60240 us
// One instance = 60240 / 166 = 362 us
volatile uint32_t light_instance = (1000000 / (8.3 * 2)) / 166;

/*******************************************************************************************/

void light_tick(void) {
    for(volatile uint32_t i = 0; i < 1000; i++) {} 
}

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

void configure_pins() {
    SYSCTL_RCGCGPIO_R |= (1 << 10) | (1 << 11);
    while((SYSCTL_PRGPIO_R & ((1 << 10) | (1 << 11))) != ((1 << 10) | (1 << 11))) {};
    
    GPIO_PORTM_DIR_R |= (0xFF);              // Set PM 0-7 as output
    GPIO_PORTM_DEN_R |= (0xFF);             // Enable digital on PM 0-7

    GPIO_PORTL_DIR_R &= ~(1 << 0);           // Set PL0 as input
    GPIO_PORTL_DEN_R |= (1 << 0);            // Enable digital on PL0
    GPIO_PORTL_AFSEL_R &= ~(1 << 0);         // Disable alternate function on PL0

    // Configure GPIO Interrupts for PL0
    GPIO_PORTL_IM_R &= ~(1 << 0);            // Mask interrupt during setup
    GPIO_PORTL_IS_R &= ~(1 << 0);            // Edge-sensitive
    GPIO_PORTL_IBE_R &= ~(1 << 0);           // (NOT) Trigger on BOTH edges
    GPIO_PORTL_IEV_R |= (1 << 0);            // Trigger on RISING edge only
    GPIO_PORTL_ICR_R = (1 << 0);             // Clear any prior flags
    GPIO_PORTL_IM_R |= (1 << 0);             // Unmask (Enable) interrupt

    NVIC_EN1_R |= (1 << 21);                     // Enable NVIC for IRQ 53 (GPIO Port L)
}


/*******************************************************************************************/

void display_character(characters requested_char) {
    // Loop through the 5 columns of the selected character
    for(int col = 0; col < 5; col++) {
        // Output the column to the LEDs
        GPIO_PORTM_DATA_R = font_table[requested_char][col];
        
        // Wait for the pendulum to physically move to the next "slice" space
        wait(light_instance); 
    }
    
    // After drawing the character, turn LEDs off and wait one slice 
    // to create a "blank space" before the next letter begins.
    GPIO_PORTM_DATA_R = 0x00;
    wait(light_instance);
}

/*******************************************************************************************/

void myInterrupt(void) {
    GPIO_PORTL_ICR_R = (1 << 0);   // Acknowledge and clear the interrupt flag

    // Wait exactly enough instances to reach the center of the forward swing
    for(int i=0; i < 74; i++) { 
        wait(light_instance); 
    }
    
    // Draw the letters immediately back-to-back
    display_character(CHAR_C);
    display_character(SPACE);
    display_character(CHAR_A);
    display_character(SPACE);
    display_character(CHAR_B);

    // Ensure LEDs are securely turned off before exiting
    GPIO_PORTM_DATA_R = 0x00;
}


/*******************************************************************************************/

int main(void) {
    configure_pins();
    
    while(1) {
    }
}