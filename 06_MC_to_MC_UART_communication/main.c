/**
 * File:    main.c
 *
 * Processor to processor communication via UART. The concept is to model a system where there are:
 *      - Unit in charge of movement (motors) and control logic via a FSM
 *      - Unit in charge of data collection that influences that movement (e.g., intensive sensor collection such as radar)
 *
 * Configured pins and peripherals:
 *      - P0: UART6 RX
 *      - P1: UART6 TX
 *      - N1: Internal LED
 *      - Timer 1A: periodic timer used for sleep_ms() and sleep_us()
 * 
 * Nice-to-haves:
 *      - DMA to receive sensor data and store it in a buffer while the FSM is running (allows to not interrupt states and be responsive)
 */

#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*=====================
 * Defines
 *=====================*/

#define SENSOR_PACKET_MAX_CHARS     15
#define SENSOR_PACKET_STORAGE_SIZE  (SENSOR_PACKET_MAX_CHARS + 1)
#define SENSOR_PACKET_BUFFER_COUNT  100

uint32_t packet_index = 0;

/*=====================
 * Enumerations
 *=====================*/

typedef enum {
    STATE_WAIT_FOR_DATA,
    STATE_PARSE_DATA,
    STATE_DECIDE_ACTION,
    STATE_EXECUTE_ACTION,
    STATE_ERROR
} control_state_t;

typedef enum {
    MOVE_FORWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_BACKWARD,
    MOVE_STOP
} robot_movement_t;

/*=====================
 * Globals
 *=====================*/

int wt = 0;
char sensor_packet_storage[SENSOR_PACKET_STORAGE_SIZE];
char sensor_packet_buffer[SENSOR_PACKET_BUFFER_COUNT][SENSOR_PACKET_STORAGE_SIZE];
uint32_t sensor_packet_buffer_index = 0;
uint32_t sensor_packet_buffer_count = 0;

/*=====================
 * Function prototypes
 *=====================*/

void configure_sleep_timer(void);
void configure_led(void);
void configure_uart_pins(void);
void configure_uart(void);

void sleep_ms(uint32_t ms);
void sleep_us(uint32_t us);

void move_forward(void);
void move_backward(void);
void move_left(void);
void move_right(void);
void stop_motors(void);

void receive_array(char* buffer);
void copy_sensor_packet(char* destination, char* source);
void store_sensor_packet(char* packet);
bool sensor_packet_is_valid(char* packet);
uint32_t parse_sensor_packet_distance(char* packet);

void toggle_led(void);
char* receive_fake_sensor_packet(void);

/*=====================
 * Main
 *=====================*/

int main(void) {
    char sensor_packet[SENSOR_PACKET_STORAGE_SIZE];
    int result;

    /* Configure internal LED */
    // configure_led();

    /* Configure Timer 1A for sleep functions */
    configure_sleep_timer();
    
    /* Configure UART6 on P0/P1 */
    configure_uart_pins();
    configure_uart();

    control_state_t state = STATE_WAIT_FOR_DATA;
    robot_movement_t movement = MOVE_STOP;

    receive_array(sensor_packet);
    store_sensor_packet(sensor_packet);

    while(1) {
        
        switch (state) {
            
            case STATE_WAIT_FOR_DATA:
                receive_array(sensor_packet);
                store_sensor_packet(sensor_packet);
                
                if(sensor_packet_storage[0] != '\0') {
                    printf("Received packet: %s", sensor_packet);

                    state = STATE_PARSE_DATA;
                }
                break;
                
            case STATE_PARSE_DATA:
                result = parse_sensor_packet_distance(sensor_packet);
                sensor_packet_storage[0] = '\0';
                
                if(result > 0 && result < 100) {
                    state = STATE_DECIDE_ACTION;
                }
                else { state = STATE_ERROR; }
                break;

            case STATE_DECIDE_ACTION:

                switch (result) {
                    case 0 ... 15:
                        movement = MOVE_STOP;
                        break;
                    case 16 ... 40:
    	        	    movement = MOVE_LEFT;
                        break;
                    case 41 ... 100:
    	        	    movement = MOVE_FORWARD;
                        break;
                }
                state = STATE_EXECUTE_ACTION;
                break;

            case STATE_EXECUTE_ACTION:

                switch (movement) {
                    case MOVE_STOP:
                        stop_motors();
                        break;

                    case MOVE_LEFT:
                        move_left();
                        break;

                    case MOVE_FORWARD:
                        move_forward();
                        break;
                }
                printf("\n");
                state = STATE_WAIT_FOR_DATA;
                break;

            case STATE_ERROR:
                printf("Error detected while parsing -> Message: INVALID \n\n");
                state = STATE_WAIT_FOR_DATA;
                break;
        }
    }
}

/*=====================
 * Initialization
 *=====================*/

void configure_sleep_timer(void) {
    SYSCTL_RCGCTIMER_R   |= (1 << 1);    /* enable Timer 1 */
    while (!(SYSCTL_PRTIMER_R & (1 << 1)));
    TIMER1_CTL_R         &= ~(1 << 0);   /* stop Timer 1A */
    TIMER1_CFG_R         = 0b000;        /* select 32-bit timer mode */
    TIMER1_TAMR_R        |= (1 << 1);    /* Timer 1A periodic mode */
    TIMER1_TAMR_R        &= ~(1 << 4);   /* Timer 1A count down */
    TIMER1_TAMR_R        |= (1 << 5);    /* Timer 1A match interrupt flag enable */
    TIMER1_TAPR_R        = 0x00000000;   /* prescaler value = 1 - 1 */
    TIMER1_ICR_R         = (1 << 4);     /* clear match flag */
}

void configure_led(void) {
    SYSCTL_RCGCGPIO_R       |= (0x1 << 12);  /* switch on clock for Port N */
    while (!(SYSCTL_PRGPIO_R & 0x1000));      /* wait for clock to stabilize */
    GPIO_PORTN_DEN_R        |= (0x1 << 1);   /* digital I/O enable pin N1 */
    GPIO_PORTN_DIR_R        |= 0x02;         /* set PortN 1 Output */
    GPIO_PORTN_DATA_R        = 0x00;         /* set PortN Output to zero */
}

/*=====================
 * Sleep functions
 *=====================*/

void sleep_ms(uint32_t ms) {
    uint32_t steps;
    uint32_t match_value;

    steps       = ms * 16000;
    match_value = 0xFFFFFFFF - steps;

    TIMER1_CTL_R         &= ~(1 << 0);   /* stop Timer 1A */
    TIMER1_TAILR_R       = 0xFFFFFFFF;   /* load maximum value */
    TIMER1_TAMATCHR_R    = match_value;  /* match value */

    TIMER1_CTL_R         |= (1 << 0);    /* start Timer 1A */
    while (!(TIMER1_RIS_R & (1 << 4)));

    TIMER1_CTL_R         &= ~(1 << 0);   /* stop Timer 1A */
    TIMER1_ICR_R         = (1 << 4);     /* clear match flag */
}

void sleep_us(uint32_t us) {
    uint32_t steps;
    uint32_t match_value;

    steps       = us * 16;
    match_value = 0xFFFFFFFF - steps;

    TIMER1_CTL_R         &= ~(1 << 0);   /* stop Timer 1A */
    TIMER1_TAILR_R       = 0xFFFFFFFF;   /* load maximum value */
    TIMER1_TAMATCHR_R    = match_value;  /* match value */

    TIMER1_CTL_R         |= (1 << 0);    /* start Timer 1A */
    while (!(TIMER1_RIS_R & (1 << 4)));

    TIMER1_CTL_R         &= ~(1 << 0);   /* stop Timer 1A */
    TIMER1_ICR_R         = (1 << 4);     /* clear match flag */
}

/*=====================
 * UART
 *=====================*/

void configure_uart_pins(void) {
    SYSCTL_RCGCGPIO_R       |= (0x1 << 13); /* switch on clock for Port P */
    while (!(SYSCTL_PRGPIO_R & 0x2000));    /* wait for clock to stabilize */
    GPIO_PORTP_DEN_R        |= 0x03;        /* enable digital pin function for P0 and P1 */
    GPIO_PORTP_DIR_R        |= 0x02;        /* set P1 to output, P0 to input */
    GPIO_PORTP_AFSEL_R      |= 0x03;        /* switch to alternate pin function P0 and P1 */
    GPIO_PORTP_PCTL_R       |= 0x11;        /* select alternate pin function P0->U6Rx, P1->U6Tx */
}

void configure_uart(void) {
    SYSCTL_RCGCUART_R        |= (0x1 << 6);  /* switch on clock for UART6 */
    wt++;                                    /* FIXME delay for stable clock */
    UART6_CTL_R              &= ~0x01;       /* disable UART6 for config */
    UART6_IBRD_R             = 104;          /* set DIVINT of BRD floor(16 MHz/16*9600 bps) */
    UART6_FBRD_R             = 11;           /* set DIVFRAC of BRD remaining fraction divider */
    UART6_LCRH_R             = 0x00000060;   /* serial format 8N1 */
    UART6_CTL_R              |= 0x0301;      /* UART transmit and receive on and UART enable */
}

/*=====================
 * Motor blueprints
 *=====================*/

void move_forward(void) {
    printf("Move forward\r\n");
}

void move_backward(void) {
    printf("Move backward\r\n");
}

void move_left(void) {
    printf("Move left\r\n");
}

void move_right(void) {
    printf("Move right\r\n");
}

void stop_motors(void) {
    printf("Stop motors\r\n");
}

/*=====================
 * Utils
 *=====================*/

void toggle_led(void) {
    if ((GPIO_PORTN_DATA_R & 0x02)) {
        GPIO_PORTN_DATA_R &= ~0x02;
    } else {
        GPIO_PORTN_DATA_R |= 0x02;
    }
}

void copy_sensor_packet(char* destination, char* source) {
    uint32_t i = 0;

    while (i < SENSOR_PACKET_MAX_CHARS && source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }

    destination[i] = '\0';
}

void store_sensor_packet(char* packet) {
    copy_sensor_packet(sensor_packet_storage, packet);
    copy_sensor_packet(sensor_packet_buffer[0], packet);

    // sensor_packet_buffer_index++;
    /*
    if (sensor_packet_buffer_index >= SENSOR_PACKET_BUFFER_COUNT) {
        sensor_packet_buffer_index = 0;
    }

    if (sensor_packet_buffer_count < SENSOR_PACKET_BUFFER_COUNT) {
        sensor_packet_buffer_count++;
    }
    */
}

/*=====================
 * Packet parsing
 *=====================*/

void receive_array(char* buffer) {
    int i = 0;
    char receivedChar = 0;

    while((i < SENSOR_PACKET_MAX_CHARS - 1) && (receivedChar != '\n')) {
        while((UART6_FR_R & (1 << 4)) != 0); // Wait for UART RX
        receivedChar = (char)(UART6_DR_R & 0xFF);
        buffer[i] = receivedChar;
        i++;
    }

    buffer[i] = '\0'; // Null-terminate the string
    printf("The incoming array is: %s", buffer);
}

bool sensor_packet_is_valid(char* packet) {
    bool result = false;
    char expected_prefix[] = "DATA:";
    uint32_t i = 0;

    if (packet != NULL) {
        while ((expected_prefix[i] != '\0') && (packet[i] == expected_prefix[i])) {
            i++;
        }

        if ((expected_prefix[i] == '\0') &&
            (packet[i] >= '0') &&
            (packet[i] <= '9')) {

            while ((packet[i] >= '0') && (packet[i] <= '9')) {
                i++;
            }

            if (packet[i] == '\r' && packet[i + 1] == '\n') {
                i += 2;
            } else if (packet[i] == '\n' && packet[i + 1] == '\r') {
                i += 2;
            }

            if (packet[i] == '\0') {
                result = true;
            }
        }
    }

    return result;
}

uint32_t parse_sensor_packet_distance(char* packet) {
    uint32_t distance = 0;
    uint32_t source_index = 5;
    uint32_t destination_index = 0;
    char number[SENSOR_PACKET_STORAGE_SIZE];

    if (sensor_packet_is_valid(packet)) {
        while ((packet[source_index] >= '0') && (packet[source_index] <= '9')) {
            number[destination_index] = packet[source_index];
            source_index++;
            destination_index++;
        }

        number[destination_index] = '\0';
        distance = (uint32_t)atoi(number);
    }

    return distance;
}

