#include "ui.hpp"
#include "includes/raymath.h"
#include <iostream>

struct Mouse_Data {
    Vector2 position;
    Vector2 last_click;
};

struct App {
    float screen_width;
    float screen_height;
    Layout layout;
    Sprite_Window sprite_window;   
    UI ui;
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

App init(float width, float height, const char* title) {
    float fps = 60.f;
    InitWindow(width, height, title);
    SetTargetFPS(fps);
    Layout app_layout = Layout({0, 0, width, height}, 2, false);
    Sprite_Window app_sprite_window; 
    UI app_ui;
    app_ui.init(Layout(app_layout.get_slot(1), 5, true));
    app_sprite_window.init(app_layout.get_slot(0), BLACK);
    App app = { .screen_width = width, 	.screen_height = height, .layout = app_layout, 
	.sprite_window = app_sprite_window, .ui = app_ui, .mouse = {0}, .name = title, .fps = fps};
    return app;
}

int main() {
    std::cout << "after app creation\n";
    App app = init(1000, 1000, "sprite paint");
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
