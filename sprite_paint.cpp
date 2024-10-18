#include <cassert>
#include <cstdlib>
#include <iostream>
#include "includes/raylib.h"
#include "includes/raymath.h"
#include <stdint.h>
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;

enum Mouse_Mode {
    DRAW, LINE, FILL, MOUSE_MODE_MAX
};

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

struct Slider {
    float value;
    Rectangle boundary;
    Rectangle handle_rec;
};

struct Color_Picker {
    Slider r;
    Slider g;
    Slider b;
    Slider a;
    Color to_color() {
	Color color = BLACK;
	color.r = r.value * 255;
	color.g = g.value * 255;
	color.b = b.value * 255;
	color.a = a.value * 255;
	return color;
    }
    Rectangle boundary;
    void setup(Rectangle boundary, Color color) {
	this->boundary = boundary;
	r.value = color.r / 255.f;
	g.value = color.g / 255.f;
	b.value = color.b / 255.f;
	a.value = color.a / 255.f;
	float max_slot_hor = 20.f;
	r.boundary = rec_slice_vert(boundary, 0, 5);  
	r.handle_rec = rec_slice_horz(r.boundary, r.value * (max_slot_hor - 1), max_slot_hor);
	g.boundary = rec_slice_vert(boundary, 1, 5);  
	g.handle_rec = rec_slice_horz(g.boundary, g.value * (max_slot_hor - 1), max_slot_hor);
	b.boundary = rec_slice_vert(boundary, 2, 5);  
	b.handle_rec = rec_slice_horz(b.boundary, b.value * (max_slot_hor - 1), max_slot_hor);
	a.boundary = rec_slice_vert(boundary, 3, 5);  
	a.handle_rec = rec_slice_horz(a.boundary, a.value * (max_slot_hor - 1), max_slot_hor);
    }
};

struct Window {
    u64 width;
    u64 height;
    const char* title;
    u64 fps = 60;
    Vector2 mouse_pos;
    Mouse_Mode mouse_mode = DRAW;
    Color draw_color = WHITE;
    Color_Picker color_picker;
};

struct Sprite {
    Image img;
    Texture tex;
    const char* name = "";
    Color bg_color = BLACK;
    Rectangle boundary;
    void set_pixel(Vector2 pos, Color color) {
	ImageDrawPixel(&img, pos.x, pos.y, color);
	UpdateTexture(tex, img.data);
    }
};

Rectangle squish_rec(Rectangle rec, float padding) {
    return {rec.x + padding, rec.y + padding, rec.width - padding * 2.f, rec.height - padding * 2.f};
}

Vector2 point_to_cell(Vector2 point, const Sprite& sprite) {
    point = Vector2Divide(point, {sprite.boundary.width, sprite.boundary.height});	
    point = Vector2Multiply(point, {(float)sprite.img.width, (float)sprite.img.height});
    point = {floor(point.x), floor(point.y)};
    return point; 
}

void init_window(Window& window) {
    InitWindow(window.width, window.height, window.title);
    SetTargetFPS(window.fps);
}

void draw_slider(const Slider& slider) {
    DrawRectangleLinesEx(slider.boundary, 2.f, WHITE);
    Rectangle sl_bg_rec = squish_rec(slider.boundary, 10.f);
    DrawRectangleRec(sl_bg_rec, WHITE);
    DrawRectangleRec(slider.handle_rec, GRAY);
}

void draw_color_picker(const Window& window) {
    DrawRectangleRec(rec_slice_vert(window.color_picker.boundary, 4, 5),
		     window.draw_color);
    draw_slider(window.color_picker.r);
    draw_slider(window.color_picker.g);
    draw_slider(window.color_picker.b);
    draw_slider(window.color_picker.a);
}


void draw_sprite(const Sprite& sprite, Window& window) {
    DrawTexturePro(sprite.tex, {0.f, 0.f, (float)sprite.tex.width, (float)sprite.tex.height}, sprite.boundary, {0.f, 0.f}, 0.f, WHITE);
    if (CheckCollisionPointRec(window.mouse_pos, sprite.boundary)) {
	float cell_size = sprite.boundary.width / sprite.tex.width;
	Vector2 new_pos = point_to_cell(window.mouse_pos, sprite);
	DrawRectangle(new_pos.x * cell_size, new_pos.y * cell_size, cell_size, cell_size, window.draw_color);
    }
}

void check_slider(Slider& slider, Vector2 mouse_pos) {
    if (CheckCollisionPointRec(mouse_pos, slider.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    slider.value = (mouse_pos.x - slider.boundary.x) / slider.boundary.width;
	    DrawText(TextFormat("%d", slider.value), 500, 500, 15, WHITE);
	    slider.value = Clamp(slider.value, 0.f, 1.f);
	    slider.handle_rec.x = slider.boundary.x + slider.boundary.width * slider.value - slider.handle_rec.width / 2.f;
	}
    }
}

void controls(Window& window, Sprite& sprite) {
    window.mouse_pos = GetMousePosition();
    if (CheckCollisionPointRec(window.mouse_pos, sprite.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    sprite.set_pixel(point_to_cell(window.mouse_pos, sprite), window.draw_color);
	}
    }
    if (IsKeyPressed(KEY_S)) {
	ExportImage(sprite.img, sprite.name);
    }
    check_slider(window.color_picker.r, window.mouse_pos);
    check_slider(window.color_picker.g, window.mouse_pos);
    check_slider(window.color_picker.b, window.mouse_pos);
    check_slider(window.color_picker.a, window.mouse_pos);
}

int main() {
    Window window = {.width = 1000, .height = 1000, .title = "Sprite Paint"};
    window.color_picker.setup({window.width / 2.f, 0.f, window.width / 2.f, window.height / 2.f}, window.draw_color);
    init_window(window);
    Sprite sprite;
    sprite.img = GenImageColor(10, 10, BLACK);
    sprite.tex = LoadTextureFromImage(sprite.img);
    sprite.name = "sprite.png";
    sprite.boundary = {0.f, 0.f, window.width / 2.f, window.height / 2.f};
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(GRAY);
	draw_sprite(sprite, window);
	draw_color_picker(window);
	controls(window, sprite);
	window.draw_color = window.color_picker.to_color();
	EndDrawing();
    }
    CloseWindow();
    return 0;
}
