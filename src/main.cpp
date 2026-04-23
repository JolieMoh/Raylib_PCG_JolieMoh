#include "HexGenerator.h"
#include <raylib.h>
#include <iostream>

int main() {
    // Window setup
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "The Construct - Hex Level Generator");
    SetTargetFPS(60);

    // Camera setup
    Camera3D camera = { 0 };
    camera.position = { 15.0f, 12.0f, 15.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Initialize generator
    HexGridGenerator generator;
    generator.InitializeGrid(15, 15);
    generator.GeneratePerlinNoiseMap();

    // Paint mode state 
    /*
    enum PaintMode { PLATFORM, ERASE };
    PaintMode currentPaintMode = PLATFORM; */

    // Camera control
    Vector3 lastMousePos = { 0 };
    bool rotating = false;

    // Main game loop
    while (!WindowShouldClose()) {
        // Camera controls
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            camera.position.x += delta.x * 0.05f;
            camera.position.z += delta.y * 0.05f;
        }

        if (IsKeyDown(KEY_LEFT_ALT)) {
            Vector2 delta = GetMouseDelta();
            camera.target.x += delta.x * 0.02f;
            camera.target.y += delta.y * 0.02f;
        }

        // Zoom with scroll wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector3 direction = Vector3Subtract(camera.position, camera.target);
            float distance = Vector3Length(direction);
            distance -= wheel * 1.0f;
            direction = Vector3Normalize(direction);
            camera.position = Vector3Add(camera.target, Vector3Scale(direction, distance));
        }

        // Handle painting on hex tiles
        /*
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), camera);

            // Simple collision detection (check each hex)
            for (int r = 0; r < generator.GetHeight(); r++) {
                for (int q = 0; q < generator.GetWidth(); q++) {
                    Vector3 tilePos = { 0 };
                    // Simplified: check bounding sphere
                    Vector3 center = {
                        (q + r * 0.5f) * 1.8f,
                        generator.GetTile(q, r) == TileType::PLATFORM ? 0.0f : -3.0f,
                        r * 1.56f
                    };

                    float radius = 1.2f;
                    if (CheckCollisionRaySphere(ray, center, radius)) {
                        // Apply paint based on current mode
                        switch (currentPaintMode) {
                        case PLATFORM:
                            generator.SetTile(q, r, TileType::PLATFORM);
                            break;
                        case ERASE:
                            generator.SetTile(q, r, TileType::EMPTY);
                            break;
                        }
                        break;
                    }
                }
            }
        }

        // Update paint mode with number keys
        if (IsKeyPressed(KEY_ONE)) currentPaintMode = PLATFORM;
        if (IsKeyPressed(KEY_THREE)) currentPaintMode = ERASE;
        */

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        generator.Render3D(camera);
        generator.RenderUI();

        // Paint mode indicator
        /*const char* modeText = "";
        Color modeColor;
        switch (currentPaintMode) {
        case PLATFORM: modeText = "Paint: PLATFORM (1)"; modeColor = GREEN; break;
        case ERASE: modeText = "Paint: ERASE (3)"; modeColor = GRAY; break;
        }
        DrawText(modeText, screenWidth - 200, 10, 16, modeColor);
        */

        // FPS counter
        DrawFPS(screenWidth - 80, screenHeight - 30);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}