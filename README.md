# TM4C1294 Microcontroller and Embedded Systems Laboratory Portfolio

This repository contains a collection of bare-metal firmware applications developed for the Texas Instruments Tiva C Series TM4C1294NCPDT microcontroller. The projects span foundational GPIO control, hardware timer optimization, sensor processing, physical display synchronization using Persistence of Vision (PoV), and multi-processor communication.

---

## Portfolio Overview

The coursework is structured into six progressive project directories designed to demonstrate low-level peripheral configuration without the reliance on abstraction layers like TivaWare. All configurations are handled directly via memory-mapped system registers.

| Directory Name | Project Title | Peripherals / Hardware Used | Core Concepts Covered |
| :--- | :--- | :--- | :--- |
| `01_Sleep_function` | Periodic LED Blinker | GPIO Port D, Timer 0 (One-Shot) | Bare-metal clock gating, AHB bus optimization, hardware delays. |
| `02_Ultrasonic_Sonar_distance_measurement` | Ultrasonic Distance Tracking | GPIO Port D, Timer 1, NVIC | Time-of-flight tracking, evolution from polling to hardware capture/GPIO interrupts across 3 versions. |
| `03_UART_send` | UART Data Transmission | GPIO Port P, UART6 (Tx Only) | Serial frame serialization (7O2), baud-rate generation, FIFO checks. |
| `04_UART_receive` | UART Echo and Console Receiver | GPIO Port P, UART6 (Rx/Tx) | Bi-directional serial handshaking (7E1), string buffering, debug logging. |
| `05_LED_Pendulum` | LED Pendulum PoV Display | GPIO Ports M & L, Timer 0, NVIC | Persistence of Vision (PoV), spatial-to-temporal synchronization, font matrix scanning. |
| `06_MC_to_MC_UART_communication` | Processor-to-Processor FSM | TM4C1294, NXP FRDM-RW612, UART6 | Dual-processor topology, Finite State Machines (FSM), packet parsing pipelines. |

---

## Detailed Project Summaries

### 01. Sleep Function (Periodic LED Blinker)
Demonstrates foundational bare-metal firmware creation. It maps GPIO Port D onto the Advanced High-Performance Bus (AHB) for maximized register performance. Instead of CPU-blocking delay loops, it utilizes Timer 0A in a 32-bit standalone One-Shot down-counter mode to yield precise millisecond blink intervals.

### 02. Ultrasonic Sonar Distance Measurement
Focuses on tracking sonic echo return periods using an HC-SR04 ultrasonic sensor. This project folder captures the complete engineering evolution of a timing subsystem across three distinct architectural variants:
* **V1_polling:** Basic software input polling where the CPU tightly waits for signal edge state changes.
* **V2_Timer_Interrupt:** Moves edge-detection to Timer 1B Input Capture mode to timestamp transitions via dedicated timer interrupts.
* **V3_GPIO_Interrupt:** Optimizes timer usage by turning Timer 1 into a free-running clock, allowing edge-triggered GPIO interrupts on Port D to handle the timestamps.

The tick-to-distance mapping utilizes the following integer shortcut based on a 16 MHz processor clock:

$$\text{Distance (cm)} = \frac{\text{Clock Ticks}}{933}$$

### 03. UART Send (Data Transmission)
Explores basic serial transmitter configuration. It establishes a dedicated unidirectional transmission pipeline running a unique 7O2 frame specification (7 data bits, Odd parity, 2 stop bits) at 4800 baud while checking the transmission FIFO flags to avoid data degradation.

### 04. UART Receive (Echo and Console Receiver)
Scales serial communications into an interactive, bi-directional console shell running a 7E1 format at 9600 baud. It prompts the host user, handles interactive control characters, gathers raw keyboard arrays into text buffers, and outputs the resulting data strings to a diagnostic system debugger.

### 05. LED Pendulum (Persistence of Vision Display)
Utilizes the concept of Persistence of Vision (PoV) to render static graphics and character text in mid-air using a physically oscillating mechanical array. A rising-edge GPIO interrupt on Port L catches the pendulum's turnaround limit, applies a precise spatial phase delay using hardware Timer 0, and unrolls an 8-bit columnar matrix font engine onto Port M exactly across the center swing axis.

### 06. MC to MC UART Communication (Processor-to-Processor FSM)
Implements a distributed embedded topology mimicking a real-world autonomous robotic control loop. An NXP FRDM-RW612 evaluation board processes high-frequency sensor streams and forwards categorized data arrays over a hardware serial interconnect. The TM4C1294 receives these packets, validates the string formatting signatures via a multivariable parsing pipeline, and feeds the resulting distance tokens into an interrupt-free navigation Finite State Machine (FSM).

---

## General Technical Specifications
* **Core Processor:** ARM Cortex-M4F operating at a baseline clock speed of 16 MHz.
* **Development Environment:** Bare-metal C development without runtime operating systems.
* **Register Architecture:** Direct memory-mapped register manipulation matching standard vendor documentation layout constraints.

---

## Authors & Contributors
* **George Molyboga** - Lead Firmware Engineer
* **lcarricart** - Co-developer and Git Collaborator