# TM4C1294 LED Pendulum Persistence of Vision Display

A C project for the Texas Instruments Tiva C Series TM4C1294NCPDT microcontroller. This project utilizes the concept of Persistence of Vision (PoV) to render static images or text in mid-air using a physically oscillating mechanical LED array synchronized through hardware edge-triggered interrupts.

---

## Features
* Bare-Metal Firmware: Interfaced directly via low-level hardware registers bypassing third-party middleware or hardware abstraction libraries.
* Spatial Synchronization: Uses an external hardware interrupt loop to anchor time-domain LED data to physical locations in the pendulum's sweep arc.
* Dynamic Character Engine: Implements a columnar matrix font scanner to dynamically translate byte streams into physical text matrices.
* High-Efficiency Delay Blocks: Re-uses an optimized hardware Timer 0 block to control visual slice pixel durations precisely.

---

## Hardware Requirements
* Microcontroller: Tiva C Series TM4C1294NCPDT (e.g., EK-TM4C1294XL LaunchPad)
* [cite_start]Actuator Module: Mechanical swinging LED pendulum array (~8.3 Hz resonance frequency)[cite: 84].
* Pin Configurations:
  * [cite_start]Port M (Pins PM0–PM7): Configured as 8-bit digital outputs routing to the 8 moving LEDs[cite: 74, 78].
  * [cite_start]Port L (Pin PL0): Configured as a digital input tracking the Left/Right swing position synchronization signal[cite: 58, 79].

---

## Architectural Evolution (V1 to V2)

The implementation evolved from a fundamental boundary visualization test into a localized text rendering application:

### 🔹 V1: Edge Boundary Illumination
* [cite_start]Mechanism: Configured pin PL0 to trigger a GPIO interrupt on **both edges** (`GPIO_PORTL_IBE_R |= (1 << 0)`) to lock onto both reversal turnaround boundaries[cite: 85, 86].
* Handling: Whenever the pendulum hit either the left or right apex points, `myInterrupt()` fired instantly, outputting `0xFF` to Port M to briefly strobe all 8 LEDs across a tight software wait loop. [cite_start]This created a visual reference line at each edge boundary of the physical arc[cite: 55].

### 🔹 V2: Synchronized Text Display (Current Production Version)
* [cite_start]Mechanism: Refined pin PL0 to trigger **exclusively on rising edges** (`GPIO_PORTL_IEV_R |= (1 << 0)`) to uniquely capture the left reversal turnaround point[cite: 85].
* Handling: Instead of lighting up boundaries immediately, the interrupt handler introduces a precise hardware phase delay (`for (int i=0; i < 74; i++) wait(light_instance);`) to deliberately bypass the accelerating swing edge and coast until the pendulum reaches the precise geometric center of its forward stroke. [cite_start]At this center spatial window, it sequences back-to-back alphanumeric glyph calls (`CHAR_C`, `SPACE`, `CHAR_A`, etc.)[cite: 60].

---

## How It Works (Current V2 Architecture)

### 1. Spatial Math Definitions
* [cite_start]Swings execute at roughly 8.3 Hz [cite: 84][cite_start], resulting in a full mechanical oscillation cycle of ~120.48ms, meaning a single-direction sweep takes ~60.24ms[cite: 93, 94].
* The physical sweep region is subdivided into 166 discrete time slices. The slice duration variable (`light_instance`) is computed using the following baseline calculation:

$$\text{light\_instance} = \frac{1000000 \text{ \mu s}}{(8.3 \text{ Hz} \times 2) \times 166} \approx 362 \text{ \mu s}$$

### 2. Peripheral Configuration (`configure_pins`)
* [cite_start]Activates and binds GPIO Port M (output array for driving the 8 parallel LEDs) and Port L (input path for the spatial synchronization signal)[cite: 74, 78, 79].
* Configures pin PL0 to generate edge-sensitive interrupts exclusively on its rising edge transition, routing it to the processor core via vector line NVIC IRQ 53.

### 3. Columnar Rendering (`display_character`)
* Characters are structured within an imported lookup grid (`font_table`).
* The driver iterates through 5 consecutive data columns per character. It pushes the vertical slice mask directly to `GPIO_PORTM_DATA_R` and holds the pattern for exactly one `light_instance` delay block before indexing to the next column.
* Inserts a blank 1-column space block (`0x00`) immediately following character operations to prevent adjacent characters from blurring together during active sweeps.

### 4. Interrupt-Driven Alignment (`myInterrupt`)
* [cite_start]The left turnaround marker trips a rising edge condition on PL0[cite: 85].
* The handler acknowledges the hardware event, delays for 74 slices to enter the ideal center field of view, and unrolls the text printing commands sequentially.
* [cite_start]Forces the LED lines completely low (`0x00`) prior to loop termination to prevent residual ghosting artifacts when the pendulum slows or reverses its path[cite: 86].
