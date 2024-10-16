#include <cassert>
#include <cstdlib>
#include <iostream>
#include "includes/raylib.h"
#include "includes/raymath.h"
typedef uint64_t u64;
typedef uint32_t u32;


struct Window {
    u64 width;
    u64 height;
    const char* title;
    u64 fps = 60;
    Rectangle sprite_boundary = {0, 0, (float)width / 2.f, (float)height / 2.f};
};



struct Sprite {
    Image img;
    Texture tex;
    Vector2 selected = {0.f, 0.f};
    const char* name = "";
    void set_selected(Vector2 new_pos) {
	assert(new_pos.x < img.width && new_pos.x >= 0.f);
	assert(new_pos.y < img.height && new_pos.y >= 0.f);
	selected = new_pos;
    }
    void set_pixel(Vector2 pos, Color color) {
	ImageDrawPixel(&img, pos.x, pos.y, color);
	UpdateTexture(tex, img.data);
    }
    void set_selected(Color color) {
	set_pixel(selected, color);
    }
};

void init_window(Window& window) {
    InitWindow(window.width, window.height, window.title);
    SetTargetFPS(window.fps);
}

void draw_sprite(const Sprite& sprite, Rectangle boundary) {
    DrawTexturePro(sprite.tex, {0.f, 0.f, (float)sprite.tex.width, (float)sprite.tex.height}, boundary, {0.f, 0.f}, 0.f, WHITE);
    float cell_size = boundary.width / sprite.tex.width;
    DrawRectangle(sprite.selected.x * cell_size, sprite.selected.y * cell_size, cell_size, cell_size, WHITE);
}

void controls(const Window& window, Sprite& sprite) {
    Vector2 mouse_pos = GetMousePosition();
    if (CheckCollisionPointRec(mouse_pos, window.sprite_boundary)) {
	Vector2 new_pos = mouse_pos;
	new_pos = Vector2Divide(new_pos, {window.sprite_boundary.width, window.sprite_boundary.height});	
	new_pos = Vector2Multiply(new_pos, {(float)sprite.img.width, (float)sprite.img.height});
	new_pos = {floor(new_pos.x), floor(new_pos.y)};
	sprite.set_selected(new_pos);

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    sprite.set_selected(WHITE);
	}
    }
    if (IsKeyPressed(KEY_S)) {
	ExportImage(sprite.img, sprite.name);
	
    }
}

int main() {
    Window window = {.width = 1000, .height = 1000, .title = "Sprite Paint"};
    init_window(window);
    Sprite sprite;
    sprite.img = GenImageColor(10, 10, BLACK);
    sprite.tex = LoadTextureFromImage(sprite.img);
    sprite.name = "sprite.png";
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(RED);
	draw_sprite(sprite, window.sprite_boundary);
	controls(window, sprite);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}
