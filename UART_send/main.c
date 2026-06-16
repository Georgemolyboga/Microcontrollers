/** UART project for sending the data through TM4C1294ncpdt to the PC
 *
 * Author:  George Molyboga
 * Date:    16.06.2026
 * Version: V1
 *
 * Sends the binary 0x3B to the PC via UART continuously.
 */

#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>

#define IDLETIME 10000                  // waiting time between transmissions
int wt = 0;                            // auxillary variable

/*******************************************************************************************/
void config_port(void){
    // initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000;   // switch on clock for Port P
    wt++;                           // delay for stable clock
    // initialize Port P
      GPIO_PORTP_DEN_R |= 0x2;   // enable digital pin function for PP1
      GPIO_PORTP_DIR_R |= 0x2;   // set PP1 to output
      GPIO_PORTP_AFSEL_R |= 0x2; // switch to alternate pin function PP1
      GPIO_PORTP_PCTL_R |= 0x10; // select alternate pin function PP1->U6Tx
}

void config_uart(void){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40;  // switch on clock for UART6
    wt++;                       // delay for stable clock
    UART6_CTL_R &= ~0x01;       // disable UART6 for config
    // initialize bitrate of 4800 bit per second
    UART6_IBRD_R = 208;         // set DIVINT of BRD floor(16 MHz/(16*4800bps))
    UART6_FBRD_R = 22;          // set DIVFRAC of BRD remaining fraction divider
    UART6_LCRH_R = 0x0000004A;  // serial format 7O2
    UART6_CTL_R |= 0x0101;      // UART transmit on and UART enable
}

void idle()      {                     // simple wait for idle state
   int i;
   for (i=IDLETIME;i>0;i--);           // count down loop for waiting 
}

/*******************************************************************************************/

int main(void) {
    config_port();  // configuration of  Port P
    config_uart();  // configuration of  UART6 

    while(1) {
        while((UART6_FR_R & 0x20) !=0); // till transmit FIFO not full
        UART6_DR_R = 0x3B;               // send the binary 0x3B
        idle();                         // idle time
        idle();
        idle();
    }
}
