#include "HexGenerator.h"
#include <fstream>
#include <sstream>
#include <iostream>

HexGridGenerator::HexGridGenerator():
    gridWidth(15), gridHeight(15), hexRadius(1.0f), hexSpacing(1.8f),
    noiseScale(0.15f), platformThreshold(0.55f), levelsBeforeAbyss(1),
    rng(std::random_device{}()) {
    InitializeGrid(gridWidth, gridHeight);
}

HexGridGenerator::~HexGridGenerator() {
    // Vector handles cleanup automatically
}

void HexGridGenerator::InitializeGrid(int width, int height) {
    gridWidth = width;
    gridHeight = height;
    grid.assign(gridHeight, std::vector<HexTile>(gridWidth));

    // Initialize positions
    for (int r = 0; r < gridHeight; r++) {
        for (int q = 0; q < gridWidth; q++) {
            grid[r][q].worldPosition = HexToWorld(q, r, 0.0f);
        }
    }
}

Vector3 HexToWorldCube(int q, int r) {
    // Convert axial coordinates to world position
    float x = (q + r * 0.5f) * 1.8f;
    float z = r * 1.56f;
    return { x, 0, z };
}

Vector3 HexGridGenerator::HexToWorld(int q, int r, float yOffset) {
    // Axial coordinates to world space
    float x = (q + r * 0.5f) * hexSpacing;
    float z = r * (hexRadius * 1.732f);  // sqrt(3) * radius
    return { x, yOffset, z };
}

void HexGridGenerator::GeneratePerlinNoiseMap() { //why not use raylib's perlin noise gen?
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int r = 0; r < gridHeight; r++) {
        for (int q = 0; q < gridWidth; q++) {
            // Perlin-like noise using sin/cos combinations
            float noise = 0.0f;
            float freq = noiseScale;

            for (int octave = 0; octave < 3; octave++) {
                noise += sin(q * freq + r * freq * 0.866f) * cos(r * freq - q * freq * 0.5f);
                noise += 0.5f * sin(q * freq * 2.4f + 1.2f) * cos(r * freq * 2.1f);
                freq *= 2.0f;
            }

            noise = (noise + 2.0f) / 4.0f; // Normalize to [0,1]

            // Apply levels before abyss logic
            if (noise > platformThreshold) {
                grid[r][q].type = TileType::PLATFORM;
            }
            else {
                // Multiple levels of platforms before abyss
                // This creates tiers - top platforms, then lower platforms, then abyss
                if (levelsBeforeAbyss > 1 && noise > platformThreshold - 0.3f) {
                    // Intermediate level
                    grid[r][q].type = TileType::PLATFORM;
                    grid[r][q].worldPosition.y = -1.5f; // Lower tier
                }
                else {
                    grid[r][q].type = TileType::EMPTY;
                    grid[r][q].worldPosition.y = -5.0f; // Abyss
                }
            }
        }
    }
}

void HexGridGenerator::GenerateRandomWalk(int steps) {
    // Clear grid first
    for (auto& row : grid) {
        for (auto& tile : row) {
            tile.type = TileType::EMPTY;
        }
    }

    std::uniform_int_distribution<int> dirDist(0, 5);
    int currentX = gridWidth / 2;
    int currentY = gridHeight / 2;

    for (int step = 0; step < steps; step++) {
        if (currentX >= 0 && currentX < gridWidth && currentY >= 0 && currentY < gridHeight) {
            grid[currentY][currentX].type = TileType::PLATFORM;

            // Random direction (hex grid has 6 directions)
            int dir = dirDist(rng);
            switch (dir) {
            case 0: currentX++; break;
            case 1: currentX--; break;
            case 2: currentY++; break;
            case 3: currentY--; break;
            case 4: currentX++; currentY++; break;
            case 5: currentX--; currentY--; break;
            }
        }
        else {
            // Reset to center if out of bounds
            currentX = gridWidth / 2;
            currentY = gridHeight / 2;
        }
    }

    // Apply levels before abyss
    SetLevelsBeforeAbyss(levelsBeforeAbyss);
}

void HexGridGenerator::GenerateCellularAutomata(int iterations) {
    // Random initialization with 45% platform chance
    std::uniform_int_distribution<int> dist(0, 100);

    for (int r = 0; r < gridHeight; r++) {
        for (int q = 0; q < gridWidth; q++) {
            grid[r][q].type = (dist(rng) < 45) ? TileType::PLATFORM : TileType::EMPTY;
        }
    }

    // Apply cellular automata rules
    for (int iter = 0; iter < iterations; iter++) {
        auto newGrid = grid;

        for (int r = 0; r < gridHeight; r++) {
            for (int q = 0; q < gridWidth; q++) {
                int wallCount = 0;

                // Check neighbors
                for (int dr = -1; dr <= 1; dr++) {
                    for (int dq = -1; dq <= 1; dq++) {
                        if (dr == 0 && dq == 0) continue;

                        int nr = r + dr;
                        int nq = q + dq;

                        if (nr >= 0 && nr < gridHeight && nq >= 0 && nq < gridWidth) {
                            if (grid[nr][nq].type == TileType::PLATFORM) {
                                wallCount++;
                            }
                        }
                        else {
                            wallCount++; // Border counts as wall
                        }
                    }
                }

                // Apply rules: birth if exactly 5 neighbors, death if less than 3
                if (grid[r][q].type == TileType::PLATFORM) {
                    newGrid[r][q].type = (wallCount < 3) ? TileType::EMPTY : TileType::PLATFORM;
                }
                else {
                    newGrid[r][q].type = (wallCount > 5) ? TileType::PLATFORM : TileType::EMPTY;
                }
            }
        }

        grid = newGrid;
    }

    SetLevelsBeforeAbyss(levelsBeforeAbyss);
}

void HexGridGenerator::SetLevelsBeforeAbyss(int levels) {
    levelsBeforeAbyss = std::max(1, levels);

    // Adjust platform heights based on levels
    for (int r = 0; r < gridHeight; r++) {
        for (int q = 0; q < gridWidth; q++) {
            if (grid[r][q].type == TileType::PLATFORM) {
                // Distribute platforms across levels
                float levelHeight = 0.0f;
                if (levelsBeforeAbyss == 1) {
                    levelHeight = 0.0f;
                }
                else {
                    // Create terraced platforms
                    int level = (r + q) % levelsBeforeAbyss;
                    levelHeight = -level * 1.2f;
                }
                grid[r][q].worldPosition.y = levelHeight;
            }
            else {
                grid[r][q].worldPosition.y = -3.0f - (levelsBeforeAbyss * 0.5f);
            }
        }
    }
}

void HexGridGenerator::Render3D(const Camera3D& camera) {
    BeginMode3D(camera);

    // Draw grid
    for (int r = 0; r < gridHeight; r++) {
        for (int q = 0; q < gridWidth; q++) {
            const auto& tile = grid[r][q];
            Color color;

            switch (tile.type) {
            case TileType::PLATFORM:
                color = tile.isSelected ? GREEN : YELLOW;
                break; 
            case TileType::EMPTY:
                color = DARKGRAY;
                break;
            }

            // Draw hexagon (using cylinder approximation)
            DrawCylinder(tile.worldPosition, hexRadius, hexRadius, 0.2f, 6, color);

            // Draw outline
            DrawCylinderWires(tile.worldPosition, hexRadius, hexRadius, 0.21f, 6, BLACK);

            // Draw selection indicator
            if (tile.isSelected) {
                DrawCylinder({ tile.worldPosition.x, tile.worldPosition.y + 0.3f, tile.worldPosition.z },
                    hexRadius + 0.05f, hexRadius + 0.05f, 0.05f, 6, { 255,255,0,100 });
            }
        }
    }

    // Draw abyss effect (fog-like)
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), { 0,0,0,50 });

    EndMode3D();
}

void HexGridGenerator::RenderUI() {
    // Panel background
    DrawRectangle(10, 10, 300, 400, { 50,50,50,200 });
    DrawRectangleLines(10, 10, 300, 400, WHITE);

    int yOffset = 30;
    int lineHeight = 35;

    // Title
    DrawText("The Construct - Level Generator", 20, yOffset, 20, WHITE);
    yOffset += lineHeight;

    // Levels before abyss slider - YOUR REQUESTED FEATURE
    DrawText(TextFormat("Levels Before Abyss: %d", levelsBeforeAbyss), 20, yOffset, 16, WHITE);
    yOffset += 20;

    Rectangle sliderRect = { 20, (float)yOffset, 260, 10 };
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();

    DrawRectangleRec(sliderRect, GRAY);
    float sliderPos = (levelsBeforeAbyss - 1) / 4.0f; // Max 5 levels
    DrawRectangle(sliderRect.x + sliderRect.width * sliderPos - 5, sliderRect.y - 5, 10, 20, WHITE);

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, sliderRect)) {
        float newPos = (mouseX - sliderRect.x) / sliderRect.width;
        levelsBeforeAbyss = 1 + (int)(newPos * 4);
        levelsBeforeAbyss = std::clamp(levelsBeforeAbyss, 1, 5);
        Regenerate();
    }

    yOffset += 30;

    // Regenerate button
    Rectangle regenBtn = { 20, (float)yOffset, 120, 30 };
    DrawRectangleRec(regenBtn, BLUE);
    DrawText("REGENERATE", 30, yOffset + 8, 16, WHITE);

    if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, regenBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Regenerate();
    }

    yOffset += 45;

    // Generation algorithms
    DrawText("Generation Algorithms:", 20, yOffset, 16, WHITE);
    yOffset += 25;

    // Perlin Noise button
    Rectangle perlinBtn = { 20, (float)yOffset, 130, 25 };
    DrawRectangleRec(perlinBtn, DARKGREEN);
    DrawText("Perlin Noise", 30, yOffset + 5, 14, WHITE);

    if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, perlinBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GeneratePerlinNoiseMap();
    }

    // Random Walk button
    Rectangle walkBtn = { 160, (float)yOffset, 130, 25 };
    DrawRectangleRec(walkBtn, DARKPURPLE);
    DrawText("Random Walk", 170, yOffset + 5, 14, WHITE);

    if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, walkBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GenerateRandomWalk(200);
    }

    yOffset += 35;

    // Cellular Automata button
    Rectangle cellularBtn = { 20, (float)yOffset, 130, 25 };
    DrawRectangleRec(cellularBtn, MAROON);
    DrawText("Cellular Auto", 30, yOffset + 5, 14, WHITE);

    if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, cellularBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GenerateCellularAutomata(5);
    }

    // Clear button
    Rectangle clearBtn = { 160, (float)yOffset, 130, 25 };
    DrawRectangleRec(clearBtn, DARKGRAY);
    DrawText("Clear All", 170, yOffset + 5, 14, WHITE);

    if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, clearBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ClearGrid();
    }

    yOffset += 45;

    // File I/O buttons
    DrawText("Save/Load:", 20, yOffset, 16, WHITE);
    yOffset += 25;

    Rectangle saveBtn = { 20, (float)yOffset, 120, 25 };
    DrawRectangleRec(saveBtn, GREEN);
    DrawText("SAVE", 60, yOffset + 5, 14, WHITE);

    if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, saveBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        SaveToFile("level_data.bin");
    }

    Rectangle loadBtn = { 150, (float)yOffset, 120, 25 };
    DrawRectangleRec(loadBtn, ORANGE);
    DrawText("LOAD", 190, yOffset + 5, 14, WHITE);

    if (CheckCollisionPointRec({ (float)mouseX, (float)mouseY }, loadBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        LoadFromFile("level_data.bin");
    }

    yOffset += 40;

    // Paint mode NOT SURE IF I WANT IT
    /*
    DrawText("Paint Mode:", 20, yOffset, 16, WHITE);
    yOffset += 25;

    // Platform paint button
    Rectangle platformPaint = { 20, (float)yOffset, 120, 25 };
    DrawRectangleRec(platformPaint, GREEN);
    DrawText("Platform", 55, yOffset + 5, 14, WHITE);

    // Erase button
    Rectangle erasePaint = { 20, (float)yOffset + 35, 120, 25 };
    DrawRectangleRec(erasePaint, DARKGRAY);
    DrawText("Erase", 55, yOffset + 40, 14, WHITE);

    // Instructions
    DrawText("Click on platforms to edit", 20, yOffset + 80, 14, YELLOW); */
    DrawText("Use mouse to rotate camera", 20, yOffset + 100, 14, YELLOW);
}

void HexGridGenerator::SetTile(int x, int y, TileType type) {
    if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
        grid[y][x].type = type;
    }
}

TileType HexGridGenerator::GetTile(int x, int y) const {
    if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
        return grid[y][x].type;
    }
    return TileType::EMPTY;
}

void HexGridGenerator::ClearGrid() {
    for (auto& row : grid) {
        for (auto& tile : row) {
            tile.type = TileType::EMPTY;
            tile.worldPosition.y = -3.0f;
        }
    }
}

void HexGridGenerator::Regenerate() {
    // Default to Perlin noise for regeneration
    GeneratePerlinNoiseMap();
}

bool HexGridGenerator::SaveToFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Write header
    int version = 1;
    file.write(reinterpret_cast<char*>(&version), sizeof(version));
    file.write(reinterpret_cast<char*>(&gridWidth), sizeof(gridWidth));
    file.write(reinterpret_cast<char*>(&gridHeight), sizeof(gridHeight));
    file.write(reinterpret_cast<char*>(&levelsBeforeAbyss), sizeof(levelsBeforeAbyss));

    // Write grid data
    for (int r = 0; r < gridHeight; r++) {
        for (int q = 0; q < gridWidth; q++) {
            int type = static_cast<int>(grid[r][q].type);
            file.write(reinterpret_cast<char*>(&type), sizeof(type));
        }
    }

    file.close();
    std::cout << "Level saved to " << filename << std::endl;
    return true;
}

bool HexGridGenerator::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Read header
    int version, width, height, levels;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    file.read(reinterpret_cast<char*>(&width), sizeof(width));
    file.read(reinterpret_cast<char*>(&height), sizeof(height));
    file.read(reinterpret_cast<char*>(&levels), sizeof(levels));

    if (width != gridWidth || height != gridHeight) {
        InitializeGrid(width, height);
    }

    levelsBeforeAbyss = levels;

    // Read grid data
    for (int r = 0; r < gridHeight; r++) {
        for (int q = 0; q < gridWidth; q++) {
            int type;
            file.read(reinterpret_cast<char*>(&type), sizeof(type));
            grid[r][q].type = static_cast<TileType>(type);
            grid[r][q].worldPosition = HexToWorld(q, r, 0);
        }
    }

    file.close();
    SetLevelsBeforeAbyss(levelsBeforeAbyss);
    std::cout << "Level loaded from " << filename << std::endl;
    return true;
}