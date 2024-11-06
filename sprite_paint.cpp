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

enum Draw_Mode {
    DRAW, LINE, FILL, MOUSE_MODE_MAX
};

enum Layout_Type {
    HORIZONTAL = 0, VERTICAL = 1,
};

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
    Rectangle boundary = {0, 0, 10, 10};
    bool down = false;
    const char* text = "no text";
    Color color = GRAY;
    void draw() const {
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
    Color_Picker(Rectangle boundary, Color color = WHITE) {
	Layout layout = Layout(boundary, 4, false);
	boundary = boundary;
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
    };

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
    void draw() {
	r.draw();
	g.draw();
	b.draw();
	a.draw();
    }
};

struct Mouse_Data {
    Vector2 position;
    Vector2 last_click;
};

struct Sprite_Window {
    Sprite_Window() {};
    Sprite_Window (Rectangle boundary, Color bg_col) {
	std::cout << "before sprite window constructor\n";
	sprite_img = GenImageColor(boundary.width, boundary.height, bg_col);
	preview_img = GenImageColor(boundary.width, boundary.height, bg_col);
	undo_img = GenImageColor(boundary.width, boundary.height, bg_col);
	std::cout << "before texture creation\n";
	tex = LoadTextureFromImage(sprite_img);
	std::cout << "after sprite window constructor\n";
    };
    Image sprite_img;    
    Image preview_img;
    Image undo_img;
    Texture tex;
    Rectangle boundary;
    Draw_Mode mode = DRAW;
    bool line_dragging = false;
    Vector2 line_first_cell = {-1, -1};
    Color draw_color = WHITE;
    const char* sprite_name = "sprite.png";
    void set_pixel(Vector2 pos, Color color) {
	ImageDrawPixel(&sprite_img, pos.x, pos.y, color);
	UpdateTexture(tex, sprite_img.data);
    }
    Vector2 point_to_pixel(Vector2 point) {
	point = Vector2Divide(point, {boundary.width, boundary.height});	
	point = Vector2Multiply(point, {(float)sprite_img.width, (float)sprite_img.height});
	point = {floor(point.x), floor(point.y)};
	return point; 
    }

    bool is_point_inside(Vector2 point) {
	return point.x < sprite_img.width && point.y < sprite_img.height && point.x >= 0.f && point.y >= 0.f;
    }

    void fill_region(Vector2 point) {
	Color empty = GetImageColor(sprite_img, point.x, point.y);
	if (ColorIsEqual(empty, draw_color)) return;
	ImageDrawPixel(&sprite_img, point.x, point.y, draw_color);
	Vector2 new_point = {point.x + 1, point.y};
	Color new_color; 
	if (is_point_inside(new_point)) {
	    new_color = GetImageColor(sprite_img, new_point.x, new_point.y);
	    if (ColorIsEqual(empty, new_color)) fill_region(new_point);
	}
	new_point = {point.x - 1, point.y};
	if (is_point_inside(new_point)) {
	    new_color = GetImageColor(sprite_img, new_point.x, new_point.y);
	    if (ColorIsEqual(empty, new_color)) fill_region(new_point);
	}
	new_point = {point.x, point.y + 1};
	if (is_point_inside(new_point)) {
	    new_color = GetImageColor(sprite_img, new_point.x, new_point.y);
	    if (ColorIsEqual(empty, new_color)) fill_region(new_point);
	}
	new_point = {point.x, point.y - 1};
	if (is_point_inside(new_point)) {
	    new_color = GetImageColor(sprite_img, new_point.x, new_point.y);
	    if (ColorIsEqual(empty, new_color)) fill_region(new_point);
	}
    }
    void draw(Vector2 mouse_position) {
	switch (mode) {
        case DRAW:
	    draw_preview(mouse_position);
	    break;
        case LINE:
	    if (!line_dragging) draw_preview(mouse_position);
	    else draw_preview_line(mouse_position); 
	    break;
        case FILL:
	    draw_preview(mouse_position);
	    break;
        case MOUSE_MODE_MAX:
          break;
        }
    }
private:
    void draw_preview(Vector2 mouse_position) {
	if (!line_dragging) {
	    DrawTexturePro(tex, {0.f, 0.f, (float)tex.width, (float)tex.height}, boundary, {0.f, 0.f}, 0.f, WHITE);
	    if (CheckCollisionPointRec(mouse_position, boundary)) {
		float cell_size = boundary.width / tex.width;
		Vector2 new_pos = point_to_pixel(mouse_position);
		DrawRectangle(new_pos.x * cell_size, new_pos.y * cell_size, cell_size, cell_size, draw_color);
	    }
	    return;
	}
    }
    
    void draw_preview_line(Vector2 mouse_position) {
	Vector2 last_cell = point_to_pixel(mouse_position);
	float cell_size = boundary.width / tex.width;
	 if (CheckCollisionPointRec(mouse_position, boundary)) {
	    if (last_cell.x != line_first_cell.x && last_cell.y != line_first_cell.y) {
		UnloadImage(preview_img);
		preview_img = ImageCopy(sprite_img);
		ImageDrawLineV(&preview_img, line_first_cell, last_cell, draw_color);
		UpdateTexture(tex, preview_img.data);
	    }
	}
	DrawTexturePro(tex, {0.f, 0.f, (float)tex.width, (float)tex.height}, boundary, {0.f, 0.f}, 0.f, WHITE);
	DrawRectangle(last_cell.x * cell_size, last_cell.y * cell_size, cell_size, cell_size, MAGENTA);
    }
};

struct UI {
    UI(Layout layout) 
    :boundary(layout.boundary), color_picker(Color_Picker(layout.get_slot(0))) {
	buttons[0].boundary = layout.get_slot(1); 
	buttons[0].text = "Set Draw Color";
	buttons[1].boundary = layout.get_slot(2); 
	// should be bound to draw mode directly instead!!
	buttons[1].text = "Draw";
    }
    Rectangle boundary;
    u64 fps = 60;
    Color bg_color = {0x18, 0x18, 0x18, 0xff};
    Color_Picker color_picker;
    Button buttons[2];
    void frame_update() {
	Color cp_color = color_picker.to_color();
	buttons[0].color = cp_color;
	color_picker.a.bg_color = {0xff, 0xff, 0xff, cp_color.a};
	color_picker.r.bg_color = {0xff, 0, 0, color_picker.a.bg_color.r};
	color_picker.g.bg_color = {0, 0xff, 0, color_picker.a.bg_color.g};
	color_picker.b.bg_color = {0, 0, 0xff, color_picker.a.bg_color.b};
    }
    void draw() {
	color_picker.draw();
	for (const Button& button : buttons) {
	    button.draw();
	}
    }
};

struct App {
    App(float width, float height)
    :	screen_width(width), screen_height(height), 
	layout(Layout({0, 0, width, height} , 2, true)), ui(UI(Layout({width / 2.f, 0, width / 2.f, height}, 4, false))) {
	Rectangle sprite_rec = layout.get_slot(0);
	InitWindow(width, height, name);
	SetTargetFPS(fps);
	sprite_window = Sprite_Window(sprite_rec, BLACK);
	std::cout << "after app constructor\n";
    }
    Sprite_Window sprite_window;   
    UI ui;
    float screen_width;
    float screen_height;
    Layout layout;
    Mouse_Data mouse;
    const char* name = "Sprite Paint";
    float fps = 60;
    void draw() {
	sprite_window.draw(mouse.position);
	ui.draw();
    }
};

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

void controls(App& app) {
    app.mouse.position = GetMousePosition();
    Sprite_Window& sprite = app.sprite_window;
    UI& ui= app.ui;
    if (CheckCollisionPointRec(app.mouse.position, sprite.boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    if (sprite.mode == DRAW) {
		sprite.set_pixel(sprite.point_to_pixel(app.mouse.position), sprite.draw_color);
	    }
	    else if (sprite.mode == LINE) {
		app.mouse.last_click = sprite.point_to_pixel(app.mouse.position);
	    }
	    else if (sprite.mode == FILL) {
		Vector2 cell = sprite.point_to_pixel(app.mouse.position);
		sprite.fill_region(cell);
	    }
	}
    }
    if (IsKeyPressed(KEY_S)) {
	ExportImage(sprite.sprite_img, TextFormat("img/%s", sprite.sprite_name));
    }                         
    check_slider(ui.color_picker.r, app.mouse.position);
    check_slider(ui.color_picker.g, app.mouse.position);
    check_slider(ui.color_picker.b, app.mouse.position);
    check_slider(ui.color_picker.a, app.mouse.position);
    if (CheckCollisionPointRec(app.mouse.position, ui.buttons[0].boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    ui.buttons[0].down = true;
	    sprite.draw_color = ui.color_picker.to_color();
	}
    }
    if (CheckCollisionPointRec(app.mouse.position, ui.buttons[1].boundary)) {
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    sprite.mode = (Draw_Mode)((sprite.mode + 1) % MOUSE_MODE_MAX);
	    ui.buttons[1].text = mode_as_string(sprite.mode);
	    ui.buttons[1].down = true;
	}
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
	for (Button& b: ui.buttons) {
	    b.down = false;
	}
    }

}

int main() {
    float window_width = 1000;
    float window_height = 1000; 
    App app = App(window_width, window_height);
    std::cout << "after app creation\n";
    while(!WindowShouldClose()) {
	controls(app);
	BeginDrawing();
	ClearBackground(BLACK);
	app.draw();
	app.ui.frame_update();
	EndDrawing();
    }
    CloseWindow();
    return 0;
}
