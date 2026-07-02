# TM4C1294 Ultrasonic Sonar Distance Measurement

A C project for the Texas Instruments Tiva C Series **TM4C1294NCPDT** microcontroller. This project interfaces with an HC-SR04 (or similar) Ultrasonic Sonar sensor to measure distances accurately using hardware timers and interrupt-driven edge capture.

---

## Features
* **Bare-Metal Firmware:** Written purely in C without relying on heavy abstraction layers like TivaWare.
* **Precise Time-of-Flight Tracking:** Tracks sonic echo return periods using high-resolution hardware timers.
* **Architectural Evolution:** Designed across three iterations (Polling $\rightarrow$ Timer Interrupt $\rightarrow$ GPIO Interrupt) to minimize CPU blocking and optimize peripheral layout.
* **Efficient Math:** Converted raw clock ticks directly to centimeters via an optimized integer division shortcut.

---

## Hardware Requirements
* **Microcontroller:** Tiva C Series TM4C1294NCPDT (e.g., EK-TM4C1294XL LaunchPad)
* **Ultrasonic Sensor:** HC-SR04 or equivalent
* **Pin Connections:**
  * **PD2:** Trigger Pin (`Output`)
  * **PD3:** Echo Pin (`Input`)

---

## Architectural Evolution (V1 to V3)

This project was developed through three versions to improve efficiency and peripheral utilization:

### 🔹 V1: Polling Mode (Initial Proof-of-Concept)
* **Mechanism:** The CPU explicitly polled the input pin state in a tight loop to detect when the Echo signal went high and low.
* **Drawback:** Highly inefficient. The CPU was completely blocked during the measurement window, rendering it unable to handle other background tasks.

### 🔹 V2: Timer Input Capture Interrupt Mode
* **Mechanism:** Configured **Timer 1B** in Input Capture Mode (`TIMER1_TBMR_R = 0x17`) to track edge events automatically. Pin **PD3** was designated with its alternate function (`AFSEL |= (1 << 3)`) to route directly to Timer 1B. 
* **Handling:** The Timer 1B peripheral issued an interrupt (IRQ 22) on edge captures, fetching timestamps directly from the hardware counter `TIMER1_TBR_R` inside `myTimer_Handler()`.

### 🔹 V3: GPIO Interrupt Mode (Current Production Version)
* **Mechanism:** Ported to a pure GPIO-driven interrupt architecture to free up Timer 1B's specialized capture capabilities. **Timer 1** was reverted back to standard **Periodic Up-Counter Mode** (`TIMER1_TBMR_R = 0x12`), serving as a free-running clock.
* **Handling:** Pin **PD3** was stripped of its alternate function and configured as a standard digital input triggering interrupts on **both edges** (`GPIO_PORTD_AHB_IBE_R |= (1 << 3)`). The GPIO Port D Interrupt (IRQ 3) fires `myInterrupt()`, capturing timestamps instantly from the free-running `TIMER1_TBR_R` register.

---

## How It Works (Current V3 Architecture)

### 1. Peripherals Initialization (`configure_pins` & `configure_timer`)
* **GPIO Port D (AHB Enabled):** PD2 is output (Trigger) and PD3 is input (Echo). PD3 is configured to interrupt on both rising and falling edges, mapped via the NVIC controller on IRQ 3.
* **Timer 0:** Configured in One-Shot mode to provide an accurate 10µs pulse window for triggering the sensor.
* **Timer 1:** Acts as a 24-bit free-running system timestamp generator counting upwards.

### 2. Sensor Triggering (`trigger_sensor`)
* Asserts PD2 High, invokes `wait(10)` to hold for exactly 10 microseconds, then pulls PD2 Low. This signals the ultrasonic module to send out its sonic burst.

### 3. Edge Capture State Machine
A simple `capture_state` machine coordinates the GPIO edge interrupts:
* **State 1 (Waiting for Rising Edge):** The sonic pulse rebounds back; the echo line jumps high. The GPIO interrupt fires, captures `start_time`, and sets state to 2.
* **State 2 (Waiting for Falling Edge):** The echo line goes low. The interrupt fires, captures `end_time`, calculates total `pulse_ticks`, and returns state to 0 (Idle).

### 4. Distance Calculation (`calculate_distance_cm`)
Given a system clock speed of 16 MHz ($16,000,000\text{ Hz}$), the distance is derived using the speed of sound ($34,300\text{ cm/s}$):

$$\text{Distance (cm)} = \frac{\text{Ticks} \times 34300}{16000000 \times 2} = \frac{\text{Ticks}}{932.94}$$

The firmware uses an integer calculation shortcut by dividing total clock ticks by **933**.