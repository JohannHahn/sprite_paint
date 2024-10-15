#include <iostream>
#include "includes/raylib.h"

int main() {
    std::cout << "hi""\n";
    InitWindow(900, 900, "hi");
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(RED);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}
