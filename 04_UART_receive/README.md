# TM4C1294 UART Echo and Console Receiver

A C project for the Texas Instruments Tiva C Series TM4C1294NCPDT microcontroller. This project demonstrates how to configure the Universal Asynchronous Receiver-Transmitter (UART) peripheral to receive arbitrary character strings from a host PC terminal, compile them into an array, and output the resulting payload to a local console debugger.

---

## Features
* Bare-Metal Firmware: Configured directly via system registers without any reliance on hardware abstraction layers like TivaWare.
* Bi-Directional Handshaking: Transmits interactive prompt formatting characters (carriage returns, line feeds, and terminal prompts) to the host terminal before awaiting user text.
* Specialized Serial Frame: Configured for a distinct 7E1 serial format (7 data bits, Even parity, 1 stop bit) running at 9600 baud.
* Non-blocking FIFO Validation: Employs specific hardware flag monitoring to safely gauge when the receiver FIFO contains valid data and when the transmitter FIFO is cleared to send.

---

## Hardware Requirements
* Microcontroller: Tiva C Series TM4C1294NCPDT (e.g., EK-TM4C1294XL LaunchPad)
* Connection: USB-to-UART bridge or onboard ICDI virtual COM port connected to a host PC terminal application (e.g., PuTTY, Tera Term).
* Pin Mapping: 
  * Pin PP0: Configured as the UART6 Receiver (U6Rx)
  * Pin PP1: Configured as the UART6 Transmitter (U6Tx)

---

## How It Works

### 1. Port Configuration (`config_port`)
* Enables the clock gating control for GPIO Port P using the `SYSCTL_RCGCGPIO_R` register.
* Includes a brief operation delay loop to ensure the peripheral clock stabilizes.
* Enables digital functions for both PP0 and PP1 via the digital enable register (`GPIO_PORTP_DEN_R`).
* Configures PP1 to function as an output and PP0 to function as an input (`GPIO_PORTP_DIR_R`).
* Enables the alternate function capabilities on both pins (`GPIO_PORTP_AFSEL_R`) and explicitly routes them to the UART6 peripheral blocks (`U6Rx` and `U6Tx`) using the port control register (`GPIO_PORTP_PCTL_R`).

### 2. UART Configuration (`config_uart`)
* Activates the clock gating control for the UART6 module via the `SYSCTL_RCGCUART_R` register.
* Temporarily clears the UART control register (`UART6_CTL_R`) to disable the module while configuration is ongoing.
* Establishes a 9600 bps baud rate based on a standard 16 MHz system clock using the following divisors:
  * Integer Baud-Rate Divisor (IBRD): 104
  * Fractional Baud-Rate Divisor (FBRD): 11
* Modifies the Line Control register (`UART6_LCRH_R`) to assign a 7-bit word length, enable even parity generation, and utilize 1 stop bit (7E1 frame specification).
* Re-enables the UART module alongside its hardware transmission and reception pathways.

### 3. Main Reception and Interaction Loop
The application runs an infinite super-loop executing the following routine:
1. Terminal Prompt Transmit: Polls the Transmit FIFO Full flag to safely write a Carriage Return (`0x0D`), a Line Feed (`0x0A`), and a terminal chevron prompt (`>`) out to the connected PC user interface.
2. Character Collection: Loops continuously to gather incoming data. It polls the Receive FIFO Empty flag (`UART6_FR_R` bit 4) to detect incoming characters, extracts them from the data register (`UART6_DR_R`), and commits them sequentially into a local array.
3. String Termination: Captures characters up to a predefined maximum size or until the user submits a Carriage Return (`0x0D`). It substitutes the final return key character with a null-terminator (`0x00`) to create a standard C-string.
4. Debug Logging: Utilizes `printf` to output the successfully compiled incoming array string directly into the system's debugger console window.