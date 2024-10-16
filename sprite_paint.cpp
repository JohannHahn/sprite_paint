#include <cstdlib>
#include <iostream>
#include "includes/raylib.h"
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
    u32* pixels = NULL;
    u64 width;
    u64 height;
    Sprite(u64 width, u64 height, u32 default_value = 0):
	width(width), height(height) {
	pixels = new u32[width * height];
	for (u64 i = 0; i < width * height; ++i) {
	    pixels[i] = default_value;
	}
    }
    ~Sprite() {
	delete[] pixels;
    }
};

void init_window(Window& window) {
    InitWindow(window.width, window.height, window.title);
    SetTargetFPS(window.fps);
}

void draw_sprite(const Image& img, Rectangle boundary) {
}

int main() {
    Window window = {.width = 1000, .height = 1000, .title = "Sprite Paint"};
    init_window(window);
    Image img = GenImageColor(100, 100, BLACK);
    Texture tex = LoadTextureFromImage(img);
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(RED);
	DrawTexturePro(tex, {0.f, 0.f, (float)img.width, (float)img.height}, window.sprite_boundary, {0.f, 0.f}, 0.f, WHITE);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}
