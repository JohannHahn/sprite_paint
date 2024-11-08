#include "common.hpp"
const char* mode_as_string(Draw_Mode mode) {
    switch(mode) {

    case DRAW:
	return "Draw";
    case LINE:
	return "Line Start";
    case FILL:
	return "Fill";
    case MOUSE_MODE_MAX:
	assert(0);
    }
    assert(0);
    return "";
}

Rectangle squish_rec(Rectangle rec, float padding) {
    return {rec.x + padding, rec.y + padding, rec.width - padding * 2.f, rec.height - padding * 2.f};
}

Rectangle rec_slice_vert(Rectangle rec, u64 slot, u64 max_slots) {
    assert(slot < max_slots);
    float height = rec.height / max_slots;
    float y = rec.y + slot * height;
    return {rec.x, y, rec.width, height};
}

Rectangle rec_slice_horz(Rectangle rec, u64 slot, u64 max_slots) {
    assert(slot < max_slots);
    float width = rec.width / max_slots;
    float x = rec.x + width * slot;
    return {x, rec.y, width, rec.height};
}

Color invert_color(Color color) {
    u32 color_int = ColorToInt(color);
    color_int = 0xFFFFFF - color_int | 0xFF000000;
    return *(Color*)(&color_int);
}

Color color_brightness(Color color, float factor) {
    color.r *= factor;
    color.g *= factor;
    color.b *= factor;
    return color; 
}

Color reverse_brightness(Color color) {
    float reverse_brightness = 1.f - ((color.r / 255.f + color.g / 255.f + color.b / 255.f) / 3.f);
    return color_brightness(WHITE, reverse_brightness);
}
