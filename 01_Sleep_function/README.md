# TM4C1294 Periodic LED Blinker

A C project for the Texas Instruments Tiva C Series **TM4C1294NCPDT** microcontroller. This project demonstrates how to configure GPIO pins for digital output and implement a precise hardware delay using **Timer 0** to blink an external or onboard LED periodically.

---

## Features
* **Bare-Metal Firmware:** Written purely in C without relying on heavy abstraction layers like TivaWare.
* **Hardware Timer Delay:** Uses the 16-bit/32-bit GPTM (General-Purpose Timer Module) **Timer 0A** in one-shot/down-counter mode for precise millisecond delays instead of wasteful CPU blocking loops.
* **AHB Optimization:** Accesses Port D via the Advanced High-Performance Bus (AHB) for faster register operations.

---

## Hardware Requirements
* **Microcontroller:** Tiva C Series TM4C1294NCPDT (e.g., EK-TM4C1294XL LaunchPad)
* **LED:** Connected to pin **PD0** (Port D, Pin 0)
* **Current-limiting Resistor:** ~220–330Ω

---

## How It Works

### 1. Clock and GPIO Initialization (`configure_pins`)
* Enables the clock gating control for **GPIO Port D** via `SYSCTL_RCGCGPIO_R`.
* Waits for the peripheral to be ready using `SYSCTL_PRGPIO_R` to prevent fault exceptions.
* Sets **PD0** as a digital output pin by modifying the Direction (`GPIO_PORTD_AHB_DIR_R`) and Digital Enable (`GPIO_PORTD_AHB_DEN_R`) registers.

### 2. Precise Delay Mechanism (`sleep`)
* Activates **Timer 0** and ensures it is ready.
* Configures it to a 32-bit standalone timer operating in **One-Shot Mode** (counts down to zero once and stops).
* Calculation for the Interval Load Register (`TIMER0_TAILR_R`):
  
  $$\text{Ticks} = \left(\frac{\text{SYS\_CLOCK\_HZ}}{1000}\right) \times \text{ms} - 1$$

* Polls the Raw Interrupt Status register (`TIMER0_RIS_R`) until the timeout flag asserts, then clears the flag.

### 3. Main Loop
The application runs an infinite super-loop (`while(1)`) performing the following sequence:
1. Drive **PD0** High (`|= 0x01`) $\rightarrow$ LED Turns ON
2. Delay for **500ms**
3. Drive **PD0** Low (`&= ~0x01`) $\rightarrow$ LED Turns OFF
4. Delay for **500ms**