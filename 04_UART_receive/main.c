/** UART project for recieving the data from PC through TM4C1294ncpdt
 *
 * Author:  George Molyboga
 * Date:    16.06.2026
 * Version: V1
 *
 * Receives UART payload from the PC, and MCU prints in the console.
 */

#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>

#define IDLETIME 1000                  // waiting time between transmissions
#define MAXSIZE 50
int wt = 0;                            // auxillary variable

/*******************************************************************************************/
void config_port(void){
    // initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000;   // switch on clock for Port P
    wt++;                           // delay for stable clock
    // initialize Port P
      GPIO_PORTP_DEN_R   |= 0x3;    // enable digital pin function for PP0 and PP1
      GPIO_PORTP_DIR_R   |= 0x2;    // set PP1 to output, PP0 to input
      GPIO_PORTP_AFSEL_R |= 0x3;    // switch to alternate pin function PP0 and PP1
      GPIO_PORTP_PCTL_R  |= 0x11;   // select alternate pin function PP0->U6Rx, PP1->U6Tx
}

void config_uart(void){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40;  // switch on clock for UART6
    wt++;                       // delay for stable clock
    UART6_CTL_R &= ~0x01;       // disable UART6 for config

    // initialize bitrate of 9600 bit per second
    UART6_IBRD_R = 104;         // set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = 11;          // set DIVFRAC of BRD remaining fraction divider
    UART6_LCRH_R = 0x00000046;  // serial format 7E1
    UART6_CTL_R |= 0x0301;      // UART transmit and receive on and UART enable
}

void idle()      {                     // simple wait for idle state
   int i;
   for (i=IDLETIME;i>0;i--);           // count down loop for waiting 
}

/*******************************************************************************************/

int main(void) {
    config_port();  // configuration of Port P
    config_uart();  // configuration of UART6
    char incomingArray[MAXSIZE];
    char recievedChar;

    while(1) {

        while((UART6_FR_R & 0x20) != 0); 
        UART6_DR_R = 0x0D;
        idle();
        while((UART6_FR_R & 0x20) != 0); 
        UART6_DR_R = 0x0A;
        idle();
        while((UART6_FR_R & 0x20) != 0); 
        UART6_DR_R = '>';
        idle();

        int i = 0;
        recievedChar = 0;

        while((i < MAXSIZE - 1) && (recievedChar != 0x0D)) {
            while((UART6_FR_R & (1 << 4)) != 0);
            recievedChar = UART6_DR_R;
            incomingArray[i] = recievedChar;
            i++;
        }

        incomingArray[i-1] = 0x00;
        printf("The incoming array is: %s \n", incomingArray);
    }
} 


