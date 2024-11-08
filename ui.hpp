#pragma once
#include "common.hpp"

struct Layout {
    bool vertical = true;
    Rectangle boundary = {0, 0, 0, 0};
    u64 slot_count = 0;
    Layout(){};
    Layout(Rectangle boundary, u64 slot_count, bool vertical):
	boundary(boundary), slot_count(slot_count), vertical(vertical){};
    Rectangle get_slot(u64 slot) const;
};

struct Button {
    Rectangle boundary = {0, 0, 10, 10};
    bool down = false;
    const char* text = "no text";
    Color color = GRAY;
    void draw() const ;
};

struct Slider {
    float value = 0;
    Rectangle boundary = {0};
    Rectangle handle_rec = {0};
    bool dragging = false;
    Color bg_color = GRAY;
    Color border_color = BLACK;
    Color handle_color = LIGHTGRAY;
    const char* handle_text = "-";
    void set_value(float value);
    void draw();
};

struct Color_Picker {
    Slider r = {0};
    Slider g = {0};
    Slider b = {0};
    Slider a = {0};
    Rectangle boundary = {0};
    void init(Rectangle boundary, Color color = WHITE);
    Color to_color();
    void draw();
};

struct UI {
    void init(Layout layout);
    Rectangle boundary;
    u64 fps = 60;
    Color bg_color = {0x18, 0x18, 0x18, 0xff};
    Color_Picker color_picker = {0};
    Button buttons[2] = {{0}, {0}};
    void frame_update();
    void draw();
};

struct Sprite_Window {
    void init(Rectangle boundary, Color bg_col);
    Image sprite_img = {0};    
    Image preview_img = {0};
    Image undo_img = {0};
    Texture tex = {0};
    Rectangle boundary = {0};
    Draw_Mode mode = DRAW;
    bool line_dragging = false;
    Vector2 line_first_cell = {-1, -1};
    Color draw_color = WHITE;
    const char* sprite_name = "sprite.png";
    void set_pixel(Vector2 pos, Color color);
    Vector2 point_to_pixel(Vector2 point);
    bool is_point_inside(Vector2 point);
    void fill_region(Vector2 point);
    void draw(Vector2 mouse_position);
    void draw_preview(Vector2 mouse_position);
    void draw_preview_line(Vector2 mouse_position);
};
