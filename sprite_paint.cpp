#include <algorithm>
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


const char* mode_as_string(Mouse_Mode mode) {
    switch(mode) {

    case DRAW:
	return "Draw";
    case LINE:
	return "Line";
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

struct Layout {
    bool vertical = true;
    Rectangle boundary = {0, 0, 0, 0};
    u64 slot_count = 0;
    Layout(Rectangle boundary, u64 slot_count, bool vertical):
	boundary(boundary), slot_count(slot_count), vertical(vertical){};
    Rectangle get_slot(u64 slot) const {
	assert(slot < slot_count);
	Rectangle rec; 
	if (vertical) rec = rec_slice_vert(boundary, slot, slot_count);
	else rec = rec_slice_horz(boundary, slot, slot_count);
	return rec;
    }
};

struct Button {
    Rectangle boundary;
    bool down = false;
    const char* text;
    Color color;
    void draw() {
	Rectangle rec = down ? squish_rec(boundary, 5.f) : boundary;
	Color contrast_col = reverse_brightness(color);
	DrawRectangleRec(rec, color);
	DrawRectangleLinesEx(rec, 2, contrast_col);
	float font_size = rec.height - rec.height / 3.f;
	DrawText(text, rec.x + font_size * 2, rec.y + font_size / 3.f, font_size, contrast_col);
    }
};

struct Slider {
    float value;
    Rectangle boundary;
    Rectangle handle_rec;
    bool dragging = false;
    Color bg_color = GRAY;
    Color border_color = BLACK;
    Color handle_color = LIGHTGRAY;
    const char* handle_text = "-";
    void set_value(float value) {
	this->value = Clamp(value, 0.f, 1.f);
	handle_rec.x = boundary.x + boundary.width * value - handle_rec.width / 2.f;
    }
    void draw() {
	//Border
	DrawRectangleRec(boundary, border_color);
	//Inner box
	DrawRectangleRec(squish_rec(boundary, 5.f), bg_color);
	// Handle
	DrawRectangleRec(handle_rec, handle_color);
	DrawRectangleLinesEx(handle_rec, 2.f, BLACK);
	DrawText(handle_text, handle_rec.x + handle_rec.width / 4.f, handle_rec.y + handle_rec.height / 2.f - handle_rec.width / 2.f, handle_rec.width, WHITE);
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
    void setup(Color color, const Layout& layout) {
	boundary = layout.boundary;
	r.value = color.r / 255.f;
	g.value = color.g / 255.f;
	b.value = color.b / 255.f;
	a.value = color.a / 255.f;

	r.bg_color = RED;
	r.handle_text = "R";
	g.bg_color = GREEN;
	g.handle_text = "G";
	b.bg_color = BLUE;
	b.handle_text = "B";
	a.handle_text = "A";

	float max_slot_hor = 20.f;
	r.boundary = layout.get_slot(0);
	r.handle_rec = rec_slice_horz(r.boundary, r.value * (max_slot_hor - 1), max_slot_hor);
	g.boundary = layout.get_slot(1);
	g.handle_rec = rec_slice_horz(g.boundary, g.value * (max_slot_hor - 1), max_slot_hor);
	b.boundary = layout.get_slot(2);
	b.handle_rec = rec_slice_horz(b.boundary, b.value * (max_slot_hor - 1), max_slot_hor);
	a.boundary = layout.get_slot(3);
	a.handle_rec = rec_slice_horz(a.boundary, a.value * (max_slot_hor - 1), max_slot_hor);
	color = to_color();
    }
    void draw() {
	r.draw();
	g.draw();
	b.draw();
	a.draw();
    }
};

struct Layouts {
    Layouts(Layout top, Layout ui, Layout cp, Layout button_layout): 
	top_layout(top), ui_layout(ui), color_picker_layout(cp), button_layout(button_layout) {};
    Layout top_layout;
    Layout ui_layout;
    Layout color_picker_layout;
    Layout button_layout;
};

struct Mouse_Data {
    Vector2 position;
    Vector2 last_click;
    Mouse_Mode mode;
};

struct Window {
    float width;
    float height;
    const char* title;
    u64 fps = 60;
    Color bg_color = {0x18, 0x18, 0x18, 0xff};
    Color_Picker color_picker;
    Button set_bg_button;
    Button set_dc_button;
    Button toggle_mouse_mode_button;
    Layouts layouts;
    Mouse_Data mouse;
    Window(float width, float height, const char* title, Layouts layouts):
	width(width), height(height), title(title), layouts(layouts) {};
    void frame_update() {
	Color cp_color = color_picker.to_color();
	set_bg_button.color = cp_color;
	set_dc_button.color = cp_color;
	color_picker.a.bg_color = {0xff, 0xff, 0xff, cp_color.a};
	color_picker.r.bg_color = {0xff, 0, 0, color_picker.a.bg_color.r};
	color_picker.g.bg_color = {0, 0xff, 0, color_picker.a.bg_color.g};
	color_picker.b.bg_color = {0, 0, 0xff, color_picker.a.bg_color.b};
    }
    void draw() {
	color_picker.draw();
	set_bg_button.draw();
	set_dc_button.draw();
	toggle_mouse_mode_button.draw();
    }
};


struct Sprite {
    Image img;
    Image preview_img;
    Texture tex;
    const char* name = "";
    Color bg_color = BLACK;
    Color draw_color = WHITE;
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
    void set_mode (Mouse_Mode mode) {
	if (mode == DRAW) {
	    UpdateTexture(tex, img.data);
	}
	else if (mode == LINE) {
	    //if (preview_img.data) UnloadImage(preview_img);
	    preview_img = ImageCopy(img);
	    UpdateTexture(tex, preview_img.data);
	}
	else if (mode == FILL) {

	}
    }
    void draw(Window& window) {
	Vector2 first_cell = window.mouse.last_click;
	if (window.mouse.mode == DRAW) {
	    DrawTexturePro(tex, {0.f, 0.f, (float)tex.width, (float)tex.height}, boundary, {0.f, 0.f}, 0.f, WHITE);
	    if (CheckCollisionPointRec(window.mouse.position, boundary)) {
		float cell_size = boundary.width / tex.width;
		Vector2 new_pos = point_to_pixel(window.mouse.position);
		DrawRectangle(new_pos.x * cell_size, new_pos.y * cell_size, cell_size, cell_size, draw_color);
	    }
	}
	else if (window.mouse.mode == LINE) {
	     if (CheckCollisionPointRec(window.mouse.position, boundary)) {
		Vector2 last_cell = point_to_pixel(window.mouse.position);
		if (last_cell.x != first_cell.x && last_cell.y != first_cell.y) {
		    UnloadImage(preview_img);
		    preview_img = ImageCopy(img);
		    ImageDrawLineV(&preview_img, first_cell, last_cell, draw_color);
		    UpdateTexture(tex, preview_img.data);
		}
	    }
	    DrawTexturePro(tex, {0.f, 0.f, (float)tex.width, (float)tex.height}, boundary, {0.f, 0.f}, 0.f, WHITE);
	}
	else if (window.mouse.mode == FILL) {
	    UpdateTexture(tex, img.data);
	    DrawTexturePro(tex, {0.f, 0.f, (float)tex.width, (float)tex.height}, boundary, {0.f, 0.f}, 0.f, WHITE);
	    if (CheckCollisionPointRec(window.mouse.position, boundary)) {
		float cell_size = boundary.width / tex.width;
		Vector2 new_pos = point_to_pixel(window.mouse.position);
		DrawRectangle(new_pos.x * cell_size, new_pos.y * cell_size, cell_size, cell_size, draw_color);
	    }
	}
    }

    Vector2 point_to_pixel(Vector2 point) {
	point = Vector2Divide(point, {boundary.width, boundary.height});	
	point = Vector2Multiply(point, {(float)img.width, (float)img.height});
	point = {floor(point.x), floor(point.y)};
	return point; 
    }

    bool is_point_inside(Vector2 point) {
	return point.x < img.width && point.y < img.height && point.x >= 0.f && point.y >= 0.f;
    }

    void fill_region(Vector2 point) {
	Color empty = GetImageColor(img, point.x, point.y);
	if (ColorIsEqual(empty, draw_color)) return;
	ImageDrawPixel(&img, point.x, point.y, draw_color);
	Vector2 new_point = {point.x + 1, point.y};
	Color new_color; 
	if (is_point_inside(new_point)) {
	    new_color = GetImageColor(img, new_point.x, new_point.y);
	    if (ColorIsEqual(empty, new_color)) fill_region(new_point);
	}
	new_point = {point.x - 1, point.y};
	if (is_point_inside(new_point)) {
	    new_color = GetImageColor(img, new_point.x, new_point.y);
	    if (ColorIsEqual(empty, new_color)) fill_region(new_point);
	}
	new_point = {point.x, point.y + 1};
	if (is_point_inside(new_point)) {
	    new_color = GetImageColor(img, new_point.x, new_point.y);
	    if (ColorIsEqual(empty, new_color)) fill_region(new_point);
	}
	new_point = {point.x, point.y - 1};
	if (is_point_inside(new_point)) {
	    new_color = GetImageColor(img, new_point.x, new_point.y);
	    if (ColorIsEqual(empty, new_color)) fill_region(new_point);
	}
    }
};

Sprite init_sprite(const Window& window) {
    Sprite sprite;
    sprite.bg_color = GRAY;
    sprite.img = GenImageColor(10, 10, sprite.bg_color);
    sprite.tex = LoadTextureFromImage(sprite.img);
    sprite.name = "sprite.png";
    sprite.boundary = {0.f, 0.f, window.width / 2.f, window.height / 2.f};
    sprite.set_mode(window.mouse.mode);
    return sprite;
}
Window init_window(float width, float height, const char* title) {
    Layout top_layout = Layout({0, 0, width, height}, 2, false);
    Layout ui_layout = Layout(top_layout.get_slot(1), 2, true);
    Layout cp_layout = Layout(ui_layout.get_slot(0), 4, true);
    Layout button_layout = Layout(ui_layout.get_slot(1), 10, true);
    Layouts layouts(top_layout, ui_layout, cp_layout, button_layout);
    Window window = Window(width, height, title, layouts);
    window.color_picker.setup(WHITE, window.layouts.color_picker_layout);
    window.set_bg_button.boundary = window.layouts.button_layout.get_slot(0);
    window.set_bg_button.text = "set background color";
    window.set_dc_button.boundary = window.layouts.button_layout.get_slot(1); 
    window.set_dc_button.text = "set draw color";
    window.toggle_mouse_mode_button.boundary = window.layouts.button_layout.get_slot(2); 
    window.toggle_mouse_mode_button.text = mode_as_string(DRAW);
    window.toggle_mouse_mode_button.color = WHITE;
    window.mouse.mode = DRAW;
    InitWindow(window.width, window.height, window.title);
    SetTargetFPS(window.fps);
    return window;
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
    window.mouse.position = GetMousePosition();
    if (CheckCollisionPointRec(window.mouse.position, sprite.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    if (window.mouse.mode == DRAW) {
		sprite.set_pixel(sprite.point_to_pixel(window.mouse.position), sprite.draw_color);
	    }
	    else if (window.mouse.mode == LINE) {
		window.mouse.last_click = sprite.point_to_pixel(window.mouse.position);
	    }
	    else if (window.mouse.mode == FILL) {
		Vector2 cell = sprite.point_to_pixel(window.mouse.position);
		sprite.fill_region(cell);
	    }
	}
    }
    if (IsKeyPressed(KEY_S)) {
	ExportImage(sprite.img, TextFormat("img/%s", sprite.name));
    }
    check_slider(window.color_picker.r, window.mouse.position);
    check_slider(window.color_picker.g, window.mouse.position);
    check_slider(window.color_picker.b, window.mouse.position);
    check_slider(window.color_picker.a, window.mouse.position);
    if (CheckCollisionPointRec(window.mouse.position, window.set_bg_button.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    window.set_bg_button.down = true;
	    sprite.set_bg_col(window.color_picker.to_color());
	}
    }
    if (CheckCollisionPointRec(window.mouse.position, window.set_dc_button.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    window.set_dc_button.down = true;
	    sprite.draw_color = window.color_picker.to_color();
	}
    }
    if (CheckCollisionPointRec(window.mouse.position, window.toggle_mouse_mode_button.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    window.mouse.mode = (Mouse_Mode)((window.mouse.mode + 1) % MOUSE_MODE_MAX);
	    window.toggle_mouse_mode_button.text = mode_as_string(window.mouse.mode);
	    sprite.set_mode(window.mouse.mode);
	}
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
	window.set_bg_button.down = false;
	window.set_dc_button.down = false;
    }

}

int main() {
    Window window = init_window(1000.f, 1000.f, "sprite paint");
    Sprite sprite = init_sprite(window);
    std::cout << "after init\n";
    while(!WindowShouldClose()) {
	controls(window, sprite);
	BeginDrawing();
	ClearBackground(window.bg_color);
	sprite.draw(window);
	window.draw();
	window.frame_update();
	EndDrawing();
    }
    CloseWindow();
    return 0;
}
