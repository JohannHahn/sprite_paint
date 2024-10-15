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

int main() {
    std::cout << "sizeof(u32)" << sizeof(u32) << "\n";
    Window window = {.width = 1000, .height = 1000, .title = "Sprite Paint"};
    init_window(window);
    Sprite sprite = Sprite(100, 100, 0xffff00ff);
    Image img = GenImageColor(100, 100, BLACK);
    u32* pixels_img = (u32*)img.data;
    for(int i = 0; i < 100 * 100; ++i) {
	pixels_img[i] = sprite.pixels[i];
    }
    Texture tex = LoadTextureFromImage(img);
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(RED);
	DrawTexture(tex, 0, 0, WHITE);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}
