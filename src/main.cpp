#include "HexGenerator.h"
#include <raylib.h>
#include <raygui.h>
#include <iostream>
#include "raymath.h"

int main() {
    // Window setup
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "The Construct - Hex Level Generator");
    SetTargetFPS(60);

    // Initialize GUI style
    GuiLoadStyleDefault();

    // Camera setup - centered on origin
    Camera3D camera = { 0 };
    camera.position = { 12.0f, 15.0f, 12.0f };
    camera.target = { 0.0f, -2.0f, 0.0f };  // Look at center of stacked platforms
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Initialize generator with circular arena
    HexGridGenerator generator;
    generator.InitializeGrid(8, 5);  // Radius 8, max 5 levels

    // Track selected tile for editing
    int selectedLevel = -1;
    int selectedQ = -1;
    int selectedR = -1;

    // Main game loop
    while (!WindowShouldClose()) {
        // Camera controls
        if (IsKeyDown(KEY_LEFT_ALT)) {
            Vector2 delta = GetMouseDelta();
            // Orbit around center
            static float angleX = 0, angleY = 0;
            angleX += delta.x * 0.01f;
            angleY += delta.y * 0.01f;

            float radius = Vector3Distance(camera.position, camera.target);
            camera.position.x = camera.target.x + radius * sin(angleX) * cos(angleY);
            camera.position.z = camera.target.z + radius * cos(angleX) * cos(angleY);
            camera.position.y = camera.target.y + radius * sin(angleY);
        }

        // Zoom with scroll wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector3 direction = Vector3Subtract(camera.position, camera.target);
            float distance = Vector3Length(direction);
            distance -= wheel * 1.0f;
            distance = std::max(5.0f, distance);
            direction = Vector3Normalize(direction);
            camera.position = Vector3Add(camera.target, Vector3Scale(direction, distance));
        }

        // Handle tile selection (click to paint)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), camera);

            // Check collision with all tiles
            float closestHit = FLT_MAX;
            int hitLevel = -1, hitQ = -1, hitR = -1;

            for (int level = 0; level < generator.GetMaxLevels(); level++) {
                for (int r = -generator.GetRadius(); r <= generator.GetRadius(); r++) {
                    for (int q = -generator.GetRadius(); q <= generator.GetRadius(); q++) {
                        Vector3 center = {
                            (q + r * 0.5f) * 1.8f,
                            -level * 1.2f,
                            r * 1.732f
                        };

                        float radius = 1.2f;
                        float hitDistance = 0;
                        if (GetRayCollisionSphere(ray, center, radius).hit) {
                            float dist = Vector3Distance(ray.position, center);
                            if (dist < closestHit) {
                                closestHit = dist;
                                hitLevel = level;
                                hitQ = q;
                                hitR = r;
                            }
                        }
                    }
                }
            }

            if (hitLevel != -1) {
                // Toggle platform on/off when clicked
                if (generator.GetTile(hitLevel, hitQ, hitR) == TileType::PLATFORM) {
                    generator.SetTile(hitLevel, hitQ, hitR, TileType::EMPTY);
                }
                else {
                    generator.SetTile(hitLevel, hitQ, hitR, TileType::PLATFORM);
                }
            }
        }

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        generator.Render3D(camera);
        generator.RenderUI();

        // Instructions overlay
        DrawText("Alt + Drag: Orbit | Scroll: Zoom",
            10, GetScreenHeight() - 25, 16, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}