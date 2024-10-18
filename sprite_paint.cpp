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

struct Window;
struct Sprite;


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

struct Button {
    Rectangle boundary;
    bool down = false;
    const char* text;
    void draw() {
	Rectangle rec = down ? squish_rec(boundary, 5.f) : boundary;
	DrawRectangleRec(rec, GRAY);
	float font_size = rec.height - rec.height / 3.f;
	DrawText(text, rec.x, rec.y + font_size / 2.f, font_size, BLACK);
    }
};

struct Slider {
    float value;
    Rectangle boundary;
    Rectangle handle_rec;
    bool dragging = false;
    void set_value(float value) {
	this->value = Clamp(value, 0.f, 1.f);
	handle_rec.x = boundary.x + boundary.width * value - handle_rec.width / 2.f;
    }
    void draw() {
	DrawRectangleLinesEx(boundary, 2.f, WHITE);
	DrawRectangleRec(squish_rec(boundary, 10.f), WHITE);
	DrawRectangleRec(handle_rec, GRAY);
    }
};

struct Color_Picker {
    Slider r;
    Slider g;
    Slider b;
    Slider a;
    Rectangle boundary;
    Color to_color() {
	Color color = BLACK;
	color.r = r.value * 255;
	color.g = g.value * 255;
	color.b = b.value * 255;
	color.a = a.value * 255;
	return color;
    }
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
	color = to_color();
    }
    void draw() {
	DrawRectangleRec(rec_slice_vert(boundary, 4, 5), to_color());
	r.draw();
	g.draw();
	b.draw();
	a.draw();
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
    Button set_bg_button;
};

struct Sprite {
    Image img;
    Texture tex;
    const char* name = "";
    Color bg_color = BLACK;
    Rectangle boundary;
    void set_bg_col(Color color) {
	for(u64 y = 0; y < img.height; y++) {
	    for (u64 x = 0; x < img.width; ++x) {
		if (ColorIsEqual(GetImageColor(img, x, y), bg_color)) {
		    ImageDrawPixel(&img, x, y, color);
		}
	    }
	}
	bg_color = color;
	UpdateTexture(tex, img.data);
    }
    void set_pixel(Vector2 pos, Color color) {
	ImageDrawPixel(&img, pos.x, pos.y, color);
	UpdateTexture(tex, img.data);
    }
    void draw(Window& window) {
	DrawTexturePro(tex, {0.f, 0.f, (float)tex.width, (float)tex.height}, boundary, {0.f, 0.f}, 0.f, WHITE);
	if (CheckCollisionPointRec(window.mouse_pos, boundary)) {
	    float cell_size = boundary.width / tex.width;
	    Vector2 new_pos = point_to_pixel(window.mouse_pos);
	    DrawRectangle(new_pos.x * cell_size, new_pos.y * cell_size, cell_size, cell_size, window.draw_color);
	}
    }
    Vector2 point_to_pixel(Vector2 point) {
	point = Vector2Divide(point, {boundary.width, boundary.height});	
	point = Vector2Multiply(point, {(float)img.width, (float)img.height});
	point = {floor(point.x), floor(point.y)};
	return point; 
    }
};

void init_window(Window& window) {
    InitWindow(window.width, window.height, window.title);
    SetTargetFPS(window.fps);
}

void check_slider(Slider& slider, Vector2 mouse_pos) {
    if (CheckCollisionPointRec(mouse_pos, slider.boundary)) {
	if (!slider.dragging && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    slider.dragging = true; 
	}

	if (slider.dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
	    slider.set_value((mouse_pos.x - slider.boundary.x) / slider.boundary.width);
	}
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
	slider.dragging = false;
    }
}

void controls(Window& window, Sprite& sprite) {
    window.mouse_pos = GetMousePosition();
    if (CheckCollisionPointRec(window.mouse_pos, sprite.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    sprite.set_pixel(sprite.point_to_pixel(window.mouse_pos), window.draw_color);
	}
    }
    if (IsKeyPressed(KEY_S)) {
	ExportImage(sprite.img, sprite.name);
    }
    check_slider(window.color_picker.r, window.mouse_pos);
    check_slider(window.color_picker.g, window.mouse_pos);
    check_slider(window.color_picker.b, window.mouse_pos);
    check_slider(window.color_picker.a, window.mouse_pos);
    if (CheckCollisionPointRec(window.mouse_pos, window.set_bg_button.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    window.set_bg_button.down = true;
	    sprite.set_bg_col(window.color_picker.to_color());
	}
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
	window.set_bg_button.down = false;
    }

}

int main() {
    Window window = {.width = 1000, .height = 1000, .title = "Sprite Paint"};
    window.color_picker.setup({window.width / 2.f, 0.f, window.width / 2.f, window.height / 2.f}, window.draw_color);
    window.set_bg_button.boundary = {window.width / 2.f, window.color_picker.boundary.height, window.width / 2.f, window.height / 20.f};
    window.set_bg_button.text = "set background color";
    init_window(window);
    Sprite sprite;
    sprite.img = GenImageColor(10, 10, BLACK);
    sprite.tex = LoadTextureFromImage(sprite.img);
    sprite.name = "sprite.png";
    sprite.boundary = {0.f, 0.f, window.width / 2.f, window.height / 2.f};
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(RAYWHITE);
	sprite.draw(window);
	window.color_picker.draw();
	window.set_bg_button.draw();
	controls(window, sprite);
	window.draw_color = window.color_picker.to_color();
	EndDrawing();
    }
    CloseWindow();
    return 0;
}
