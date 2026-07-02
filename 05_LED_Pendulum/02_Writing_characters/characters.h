#ifndef CHARACTERS_H_
#define CHARACTERS_H_

#include <stdint.h>

// 1. The Enum (Notice the name matches what you used in main.c)
typedef enum {
    CHAR_A = 0,
    CHAR_B,
    CHAR_C,
    NUM_1,
    NUM_2,
    SPACE
} characters;

// 2. The Font Array
// Using 'static const' is a best practice in C headers to prevent "Multiple Definition" 
// linker errors if you ever include this header in more than one .c file.
static const uint8_t font_table[][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // CHAR_A (Index 0)
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // CHAR_B (Index 1)
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // CHAR_C (Index 2)
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // NUM_1  (Index 3)
    {0x42, 0x61, 0x51, 0x49, 0x46}, // NUM_2  (Index 4)
    {0x00, 0x00, 0x00, 0x00, 0x00}  // SPACE  (Index 5)
};

#endif /* CHARACTERS_H_ */