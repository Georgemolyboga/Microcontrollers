# TM4C1294 Microcontroller and Embedded Systems Laboratory Portfolio

This repository contains a collection of bare-metal firmware applications developed for the Texas Instruments Tiva C Series TM4C1294NCPDT microcontroller. The projects span foundational GPIO control, hardware timer optimization, sensor processing, physical display synchronization using Persistence of Vision (PoV), and multi-processor communication.

---

## Portfolio Overview

The coursework is structured into six progressive projects designed to demonstrate low-level peripheral configuration without the reliance on abstraction layers like TivaWare. All configurations are handled directly via memory-mapped system registers.

| Project | Title | Peripherals / Hardware Used | Core Concepts Covered |
| :--- | :--- | :--- | :--- |
| 1 | Periodic LED Blinker | GPIO Port D, Timer 0 (One-Shot) | Bare-metal clock gating, AHB bus optimization, hardware delays. |
| 2 | Ultrasonic Distance Measurement | GPIO Port D, Timer 1 (Capture) | Time-of-flight tracking, Input Capture mode, input edge polling. |
| 3 | Precise Echo Timing via Interrupts | GPIO Port D, Timer 1 (Capture), NVIC | Interrupt service routines (ISRs), hardware edge-capture timing. |
| 4 | UART Data Transmission | GPIO Port P, UART6 (Tx Only) | Serial frame serialization (7O2), baud-rate generation, FIFO checks. |
| 5 | UART Echo and Console Receiver | GPIO Port P, UART6 (Rx/Tx) | Bi-directional serial handshaking (7E1), string buffering, debug logging. |
| 6 | Processor-to-Processor FSM | TM4C1294, NXP FRDM-RW612, UART6 | Dual-processor topology, Finite State Machines (FSM), parsing pipelines. |

---

## Detailed Project Summaries

### Project 1: Periodic LED Blinker
Demonstrates foundational bare-metal firmware creation. It maps GPIO Port D onto the Advanced High-Performance Bus (AHB) for maximized register performance. Instead of CPU-blocking delay loops, it utilizes **Timer 0A** in a 32-bit standalone One-Shot down-counter mode to yield precise millisecond blink intervals.

### Project 2 & 3: Ultrasonic Sonar Distance Measurement
Focuses on tracking sonic echo return periods using an HC-SR04 ultrasonic sensor. Over successive iterations, this module evolved from basic software input polling into a highly optimized, interrupt-driven edge-capture architecture. By configuring **Timer 1B** to latch counter values on specific echo transitions, raw clock cycles are cleanly captured and translated into physical metric distances.

The tick-to-distance mapping utilizes the following integer shortcut based on a 16 MHz processor clock:

$$\text{Distance (cm)} = \frac{\text{Clock Ticks}}{933}$$

### Project 4 & 5: Universal Asynchronous Receiver-Transmitter (UART) Modules
Explores serial communication architectures. Project 4 establishes a dedicated transmission pipeline running a unique 7O2 frame specification (7 data bits, Odd parity, 2 stop bits) at 4800 baud. Project 5 scales this into an interactive, bi-directional console shell running a 7E1 format at 9600 baud. Both projects actively poll hardware FIFO flag registers to defend against data overwrites and frame corruption.

### Project 6: Cross-Platform Processor-to-Processor Communication
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