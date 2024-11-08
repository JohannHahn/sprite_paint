#include "includes/raylib.h"
#include <stdint.h>
#include <cassert>
#include <iostream>
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;

enum Draw_Mode {
    DRAW, LINE, FILL, MOUSE_MODE_MAX
};

enum Layout_Type {
    HORIZONTAL = 0, VERTICAL = 1,
};

const char* mode_as_string(Draw_Mode mode);
Rectangle squish_rec(Rectangle rec, float padding);
Rectangle rec_slice_vert(Rectangle rec, u64 slot, u64 max_slots);
Rectangle rec_slice_horz(Rectangle rec, u64 slot, u64 max_slots);
Color invert_color(Color color);
Color color_brightness(Color color, float factor);
Color reverse_brightness(Color color);
