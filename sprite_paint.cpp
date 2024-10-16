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
    Vector2 selected = {23.f, 23.f};
    void set_selected(Vector2 new_pos) {
	std::cout << "new pos = " << new_pos.x << ", " << new_pos.y << "\n";
	//assert(new_pos.x < img.width && new_pos.x >= 0.f);
	//assert(new_pos.y < img.height && new_pos.y >= 0.f);
	selected = new_pos;
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
	DrawText("HIIIII", window.width / 2 + 200, window.height / 2 + 200, 5, WHITE);
	Vector2 new_pos = {floor(mouse_pos.x), floor(mouse_pos.y)};
	new_pos = Vector2Divide(new_pos, {(float)window.width, (float)window.height});	
	float cell_size = window.sprite_boundary.width / sprite.tex.width;
	new_pos = Vector2Multiply(new_pos, {(float)sprite.img.width, (float)sprite.img.height});
	new_pos = Vector2Add(new_pos, {cell_size / 2.f, cell_size / 2.f});
	sprite.set_selected(new_pos);
    }
}

int main() {
    Window window = {.width = 1000, .height = 1000, .title = "Sprite Paint"};
    init_window(window);
    Sprite sprite;
    sprite.img = GenImageColor(24, 24, BLACK);
    sprite.tex = LoadTextureFromImage(sprite.img);
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
