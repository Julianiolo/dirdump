#include <iostream>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui/rlImGui.h"
#include "oneHeaderLibs/VectorOperators.h"

//#include "fromJeffMTestframe/Application/platform_tools.h"

#define println(x) std::cout << x << std::endl;

void setup();
void draw();
void destroy();

int frameCnt = 0;

Vector2 lastMousePos;
Vector2 mouseDelta;

int main(void) {
    setup();

    while (!WindowShouldClose()) {
        draw();
    }

    destroy();

    return 0;
}

void setup() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1200, 800, "TexRip");

    SetWindowResizeDrawCallback(draw);
    SetTargetFPS(60);

    lastMousePos = GetMousePosition();
    mouseDelta = { 0,0 };

    SetupRLImGui(true);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsResizeFromEdges = true;
    io.WantSaveIniSettings = false;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

}
void draw() {
    BeginDrawing();

    mouseDelta = GetMousePosition() - lastMousePos;

    ClearBackground(DARKGRAY);

    BeginRLImGui();


    ImGui::ShowDemoWindow(NULL);
    EndRLImGui();

    //TexRip::TexRipper::debugDraw();

    lastMousePos = GetMousePosition();

    EndDrawing();

    frameCnt++;
}
void destroy() {
    ShutdownRLImGui();
    CloseWindow();
}