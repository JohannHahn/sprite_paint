#include "ui.hpp"
#include "includes/raymath.h"
void Button::draw() const {
    Rectangle rec = down ? squish_rec(boundary, 5.f) : boundary;
    Color contrast_col = reverse_brightness(color);
    DrawRectangleRec(rec, color);
    DrawRectangleLinesEx(rec, 2, contrast_col);
    float font_size = rec.height - rec.height / 3.f;
    DrawText(text, rec.x + font_size * 2, rec.y + font_size / 3.f, font_size, contrast_col);
}
Rectangle Layout::get_slot(u64 slot) const {
    assert(slot < slot_count);
    Rectangle rec; 
    if (vertical) rec = rec_slice_vert(boundary, slot, slot_count);
    else rec = rec_slice_horz(boundary, slot, slot_count);
    return rec;
}
void Slider::set_value(float value) {
    this->value = Clamp(value, 0.f, 1.f);
    handle_rec.x = boundary.x + boundary.width * value - handle_rec.width / 2.f;
}
void Slider::draw() {
    //Border
    DrawRectangleRec(boundary, border_color);
    //Inner box
    DrawRectangleRec(squish_rec(boundary, 5.f), bg_color);
    // Handle
    DrawRectangleRec(handle_rec, handle_color);
    DrawRectangleLinesEx(handle_rec, 2.f, BLACK);
    DrawText(handle_text, handle_rec.x + handle_rec.width / 4.f, handle_rec.y + handle_rec.height / 2.f - handle_rec.width / 2.f, handle_rec.width, WHITE);
}
Color Color_Picker::to_color() {
    Color color = BLACK;
    color.r = r.value * 255;
    color.g = g.value * 255;
    color.b = b.value * 255;
    color.a = a.value * 255;
    return color;
}
void Color_Picker::draw() {
    r.draw();
    g.draw();
    b.draw();
    a.draw();
}
void Color_Picker::init(Rectangle boundary, Color color) {
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
void UI::init(Layout layout) {
    boundary = layout.boundary;
    color_picker = {0};
    buttons[0].boundary = layout.get_slot(1); 
    buttons[0].text = "Set Draw Color";
    buttons[1].boundary = layout.get_slot(2); 
    // should be bound to draw mode directly instead!!
    buttons[1].text = "Draw";
}
  
void UI::frame_update() {
    Color cp_color = color_picker.to_color();
    buttons[0].color = cp_color;
    color_picker.a.bg_color = {0xff, 0xff, 0xff, cp_color.a};
    color_picker.r.bg_color = {0xff, 0, 0, color_picker.a.bg_color.r};
    color_picker.g.bg_color = {0, 0xff, 0, color_picker.a.bg_color.g};
    color_picker.b.bg_color = {0, 0, 0xff, color_picker.a.bg_color.b};
}
void UI::draw() {
    color_picker.draw();
    for (const Button& button : buttons) {
	button.draw();
    }
}
void Sprite_Window::set_pixel(Vector2 pos, Color color) {
    ImageDrawPixel(&sprite_img, pos.x, pos.y, color);
    UpdateTexture(tex, sprite_img.data);
}
Vector2 Sprite_Window::point_to_pixel(Vector2 point) {
    point = Vector2Divide(point, {boundary.width, boundary.height});	
    point = Vector2Multiply(point, {(float)sprite_img.width, (float)sprite_img.height});
    point = {floor(point.x), floor(point.y)};
    return point; 
}

bool Sprite_Window::is_point_inside(Vector2 point) {
    return point.x < sprite_img.width && point.y < sprite_img.height && point.x >= 0.f && point.y >= 0.f;
}

void Sprite_Window::fill_region(Vector2 point) {
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
void Sprite_Window::draw(Vector2 mouse_position) {
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
void Sprite_Window::draw_preview(Vector2 mouse_position) {
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

void Sprite_Window::draw_preview_line(Vector2 mouse_position) {
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
void Sprite_Window::init(Rectangle boundary, Color bg_col) {
    std::cout << "before sprite window constructor\n";
    sprite_img = GenImageColor(boundary.width, boundary.height, bg_col);
    preview_img = GenImageColor(boundary.width, boundary.height, bg_col);
    undo_img = GenImageColor(boundary.width, boundary.height, bg_col);
    std::cout << "before texture creation\n";
    tex = LoadTextureFromImage(sprite_img);
    std::cout << "after sprite window constructor\n";
};
