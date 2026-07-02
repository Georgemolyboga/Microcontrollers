# TM4C1294 UART Data Transmission

A C project for the Texas Instruments Tiva C Series TM4C1294NCPDT microcontroller. This project demonstrates how to configure the Universal Asynchronous Receiver-Transmitter (UART) peripheral to continuously transmit raw binary data to a host PC.

---

## Features
* Bare-Metal Firmware: Configured directly via system registers without any hardware abstraction layers like TivaWare.
* Dedicated Hardware Transmission: Uses UART6 mapped to Port P for dedicated serial communication.
* Custom Frame Configuration: Configured for a specific 7O2 serial format (7 data bits, Odd parity, 2 stop bits) running at 4800 baud.
* Non-blocking FIFO Check: Verifies transmitter FIFO availability before writing to the data register to prevent data overwrites.

---

## Hardware Requirements
* Microcontroller: Tiva C Series TM4C1294NCPDT (e.g., EK-TM4C1294XL LaunchPad)
* Connection: USB-to-UART bridge or onboard ICDI virtual COM port connected to a PC terminal application (e.g., PuTTY, Tera Term).
* Pin Mapping: Pin PP1 is utilized as the UART6 Transmitter (U6Tx).

---

## How It Works

### 1. Port Configuration (`config_port`)
* Enables the clock gating control for GPIO Port P using the `SYSCTL_RCGCGPIO_R` register.
* Includes a brief instruction delay loop to allow the peripheral clock to stabilize.
* Configures pin PP1 as a digital output pin (`GPIO_PORTP_DEN_R` and `GPIO_PORTP_DIR_R`).
* Selects the alternate peripheral function for PP1 (`GPIO_PORTP_AFSEL_R`) and routes it specifically to UART6 Transmit (`U6Tx`) using the port control register (`GPIO_PORTP_PCTL_R`).

### 2. UART Configuration (`config_uart`)
* Activates the clock for the UART6 module via `SYSCTL_RCGCUART_R`.
* Disables UART6 temporarily (`UART6_CTL_R`) to safely modify configuration registers.
* Sets the baud rate generation to 4800 bps based on a 16 MHz system clock:
  * Integer Baud-Rate Divisor (IBRD): 208
  * Fractional Baud-Rate Divisor (FBRD): 22
* Adjusts the Line Control register (`UART6_LCRH_R`) to parse a 7-bit word length, enable odd parity generation, and utilize 2 stop bits (7O2 frame specification).
* Re-enables the UART module along with its transmission hardware blocks.

### 3. Main Transmission Loop
The application runs an infinite loop executing the following routine:
1. Polls the UART6 Flag Register (`UART6_FR_R`) to ensure the Transmit FIFO is not full.
2. Writes the raw hex value `0x3B` into the UART6 Data Register (`UART6_DR_R`).
3. Executes a sequential set of software delay countdowns (`idle`) to space out subsequent data transmissions.