#include "HexGenerator.h"
#include "raygui.h"  // Raylib's UI library (buttons, sliders, etc)
#include <fstream>   // File stream - for reading/writing files
#include <iostream>  // For console output (cout)
#include "raymath.h"     
#include <cmath>     // Math functions (sin, cos, sqrt)

// CONSTRUCTOR - Sets up default values when we create the generator
HexGridGenerator::HexGridGenerator()
    : gridRadius(8),         // 8 hexes from center = ~17 hexes wide total
    maxLevels(5),            // Can stack up to 5 floors
    hexRadius(1.0f),         // Each hex is 1 unit wide
    hexSpacing(1.8f),        // Gives slight gap
    noiseScale(0.15f),       // Noise 15% zoom - creates decent sized clusters
    platformThreshold(0.55f), // 55% chance of platform at any point
    levelsBeforeAbyss(1),    // Start with 1 floors
    rng(std::random_device{}()) {  // Seed the random generator with unpredictable value
    InitializeGrid(gridRadius, maxLevels);  // Create the empty grid structure
}

// DESTRUCTOR - Nothing to clean up because vectors manage their own memory
HexGridGenerator::~HexGridGenerator() {
    // The grid vector will automatically destroy itself
}

// Creates the empty grid structure with the right dimensions
void HexGridGenerator::InitializeGrid(int radius, int maxStackLevels) {
    gridRadius = radius;               // Store for later use
    maxLevels = maxStackLevels;        // Store max floors

    // Calculate size: radius 8 means coordinates from -8 to +8 = 17 possible positions
    int gridSize = 2 * radius + 1;     // Example: 2*8+1 = 17

    // RESIZE creates the 3D structure:
    // grid[FLOORS][ROWS][COLUMNS]
    grid.assign(maxLevels,
        std::vector<std::vector<HexTile>>(gridSize,
            std::vector<HexTile>(gridSize)));

    // Now calculate the world position for every possible hex
    // We do this once at start so we don't recalculate every frame
    for (int level = 0; level < maxLevels; level++) {           // For each floor
        for (int r = -gridRadius; r <= gridRadius; r++) {       // For each row
            for (int q = -gridRadius; q <= gridRadius; q++) {   // For each column
                if (IsWithinCircle(q, r)) {  // Only if inside our circular arena
                    // Convert array indices from world coordinates (-8..8) to array indices (0..16)
                    int rowIdx = r + gridRadius;     // Example: -8 + 8 = 0 (first row)
                    int colIdx = q + gridRadius;     // Example: 8 + 8 = 16 (last column)

                    // Store the 3D world position
                    // Each level is 1.2 units below the level above it
                    grid[level][rowIdx][colIdx].worldPosition = HexToWorld(q, r, -level * 1.2f);
                    grid[level][rowIdx][colIdx].level = level;
                }
            }
        }
    }
}

// Converts axial grid coordinates (q, r) to 3D world coordinates (x, y, z)
Vector3 HexGridGenerator::HexToWorld(int q, int r, float yOffset) {
    // Math for hex grid layout:
    // q moves horizontally, r moves diagonally
    // This formula creates perfectly tessellating hexagons
    float x = (q + r * 0.5f) * hexSpacing;        // X coordinate
    float z = r * (hexRadius * 1.732f);            // Z coordinate (1.732 = sqrt(3))
    return { x, yOffset, z };                        // Return as Vector3 (X,Y,Z)
}

// Checks if a hex coordinate is within our circular arena
// Uses CUBE COORDINATES - a different way to represent hex grids that makes circle math easy
bool HexGridGenerator::IsWithinCircle(int q, int r) {
    // Convert axial (q,r) to cube coordinates (x,y,z)
    // Cube coordinates have the property that x+y+z = 0 always
    // This makes distance calculation simple: distance = max(|x|,|y|,|z|)
    float x = q;                    // X coordinate in cube space
    float z = r;                    // Z coordinate in cube space
    float y = -x - z;               // Y is derived (must sum to zero)

    // Calculate Manhattan distance in cube space
    float distance = std::sqrt(x * x + y * y + z * z);

    // Return TRUE if within radius, FALSE if outside
    return distance <= gridRadius;
}

// ===== ALGORITHM 1: RANDOM WALK =====
void HexGridGenerator::GenerateRandomWalk(int steps) {
    ClearGrid();

    // Generate each level independently
    for (int level = 0; level < levelsBeforeAbyss; level++) {
        // Directions on a hex grid (6 possible moves)
        // Each move changes q and r coordinates
        int currentQ = 0;  // Start at center
        int currentR = 0;  // Start at center

        // Take 'steps' number of steps
        for (int step = 0; step < steps; step++) {
            // Only place tile if we're inside the circle
            if (IsWithinCircle(currentQ, currentR)) {
                int rowIdx = currentR + gridRadius;
                int colIdx = currentQ + gridRadius;
                grid[level][rowIdx][colIdx].type = TileType::PLATFORM;

                // Pick a random direction (0-5) for the next step
                int dir = std::uniform_int_distribution<int>(0, 5)(rng);
                switch (dir) {
                case 0: currentQ++; break;           // Move right
                case 1: currentQ--; break;           // Move left
                case 2: currentR++; break;           // Move down-right
                case 3: currentR--; break;           // Move up-left
                case 4: currentQ++; currentR--; break; // Move up-right
                case 5: currentQ--; currentR++; break; // Move down-left
                }
            }
            else {
                // Fell out of bounds - teleport back to center
                currentQ = 0;
                currentR = 0;
            }
        }
    }
}

// ===== ALGORITHM 2: CELLULAR AUTOMATA (Game of Life) =====
void HexGridGenerator::GenerateCellularAutomata(int iterations) {
    ClearGrid();

    for (int level = 0; level < levelsBeforeAbyss; level++) {
        // First, create a temporary grid for this level
        int gridSize = 2 * gridRadius + 1;
        std::vector<std::vector<TileType>> levelGrid(
            gridSize, std::vector<TileType>(gridSize, TileType::EMPTY));

        // RANDOM INITIALIZATION - 45% chance of being a platform
        for (int r = -gridRadius; r <= gridRadius; r++) {
            for (int q = -gridRadius; q <= gridRadius; q++) {
                if (IsWithinCircle(q, r)) {
                    int rowIdx = r + gridRadius;
                    int colIdx = q + gridRadius;
                    // 45% chance of platform, 55% chance empty
                    bool isPlatform = (std::uniform_int_distribution<int>(0, 100)(rng) < 45);
                    levelGrid[rowIdx][colIdx] = isPlatform ? TileType::PLATFORM : TileType::EMPTY;
                }
            }
        }

        // Apply rules multiple times (iterations)
        for (int iter = 0; iter < iterations; iter++) {
            // Create a copy to store the NEXT generation
            auto newGrid = levelGrid;

            for (int r = -gridRadius; r <= gridRadius; r++) {
                for (int q = -gridRadius; q <= gridRadius; q++) {
                    if (!IsWithinCircle(q, r)) continue;

                    int rowIdx = r + gridRadius;
                    int colIdx = q + gridRadius;

                    // Count how many neighboring tiles are walls
                    int wallCount = 0;

                    // The 6 neighbors on a hex grid
                    // Each pair is (deltaQ, deltaR)
                    std::vector<std::pair<int, int>> neighbors = {
                        {1,0},   // Right
                        {-1,0},  // Left
                        {0,1},   // Down-Right
                        {0,-1},  // Up-Left
                        {1,-1},  // Up-Right
                        {-1,1}   // Down-Left
                    };

                    // Check each neighbor
                    for (auto& [dq, dr] : neighbors) {
                        int nq = q + dq;
                        int nr = r + dr;

                        if (IsWithinCircle(nq, nr)) {
                            int nRowIdx = nr + gridRadius;
                            int nColIdx = nq + gridRadius;
                            if (levelGrid[nRowIdx][nColIdx] == TileType::PLATFORM) {
                                wallCount++;
                            }
                        }
                        else {
                            // Out of bounds counts as a wall (border makes caves feel enclosed)
                            wallCount++;
                        }
                    }

                    // CELLULAR AUTOMATA RULES:
                    // 1. If a wall has less than 2 neighbors, it dies (erosion)
                    // 2. If an empty space has more than 4 neighbors, it becomes a wall (infill)
                    if (levelGrid[rowIdx][colIdx] == TileType::PLATFORM) {
                        newGrid[rowIdx][colIdx] = (wallCount < 2) ? TileType::EMPTY : TileType::PLATFORM;
                    }
                    else {
                        newGrid[rowIdx][colIdx] = (wallCount > 4) ? TileType::PLATFORM : TileType::EMPTY;
                    }
                }
            }

            levelGrid = newGrid;  // Move to next generation
        }

        // Copy the final result to our main grid
        for (int r = -gridRadius; r <= gridRadius; r++) {
            for (int q = -gridRadius; q <= gridRadius; q++) {
                if (IsWithinCircle(q, r)) {
                    int rowIdx = r + gridRadius;
                    int colIdx = q + gridRadius;
                    grid[level][rowIdx][colIdx].type = levelGrid[rowIdx][colIdx];
                }
            }
        }
    }
}

// Changes how many floors exist and regenerates the level
void HexGridGenerator::SetLevelsBeforeAbyss(int levels) {
    levelsBeforeAbyss = std::max(1, std::min(levels, maxLevels));  // Clamp between 1 and max
    Regenerate();  // Create new level with new floor count
}

// Draws the entire 3D scene
void HexGridGenerator::Render3D(const Camera3D& camera) {
    BeginMode3D(camera);  // Start 3D drawing mode

    // ===== DRAW THE BOTTOM ABYSS PLANE =====
    // This is a large flat rectangle that spans the entire arena bottom
    float floorSize = (gridRadius * 2 * hexSpacing) + 5.0f;  // Slightly bigger than the hex grid

    // Purple plane represents the "abyss" - players fall to their death here
    DrawPlane({ 0, -levelsBeforeAbyss * 1.5f - 1.0f, 0 }, { floorSize, floorSize }, DARKPURPLE);
    // Black plane below it creates depth (you can't see past it)
    DrawPlane({ 0, -levelsBeforeAbyss * 1.5f - 1.05f, 0 }, { floorSize, floorSize }, BLACK);

    // Draw only up to levelsBeforeAbyss (fixed!)
    for (int level = 0; level < levelsBeforeAbyss; level++) {
        for (int r = -gridRadius; r <= gridRadius; r++) {
            for (int q = -gridRadius; q <= gridRadius; q++) {
                if (!IsWithinCircle(q, r)) continue;

                int rowIdx = r + gridRadius;
                int colIdx = q + gridRadius;

                const auto& tile = grid[level][rowIdx][colIdx];
                if (tile.type == TileType::EMPTY) continue; // Don't draw empty tiles

                // DrawCylinder draws a 3D cylinder (perfect for hexagons with 6 sides!)
                // Parameters: position, radiusTop, radiusBottom, height, sides, color
                DrawCylinder(tile.worldPosition, hexRadius, hexRadius, 0.2f, 6, YELLOW);

                // Draw wireframe outline so edges are visible
                DrawCylinderWires(tile.worldPosition, hexRadius, hexRadius, 0.21f, 6, BLACK);
            }
        }
    }
    EndMode3D();  // Stop 3D drawing mode
}

// Draws all the UI buttons and sliders using RayGUI
void HexGridGenerator::RenderUI() {
    // GuiPanel creates a semi-transparent background box for our UI
    Rectangle panel = { 10, 10, 320, 500 };
    GuiPanel(panel, "The Construct - Level Generator");

    int yOffset = 50;  // Starting Y position for first UI element

    // LEVELS SLIDER
    GuiLabel({ 20, (float)yOffset, 200, 20 }, TextFormat("Levels: %d / %d", levelsBeforeAbyss, maxLevels));

    // Create a temporary float for the slider
    static float sliderValue = (levelsBeforeAbyss - 1) / (float)(maxLevels - 1);

    // Update slider value based on current levels
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        sliderValue = (levelsBeforeAbyss - 1) / (float)(maxLevels - 1);
    }

    // Draw slider and capture changes
    Rectangle sliderRect = { 20, (float)yOffset, 280, 20 };
    GuiSlider(sliderRect, "1", TextFormat("%d", maxLevels), &sliderValue, 0, 1);

    // Apply changes when slider moves OR when mouse is released
    int newLevels = 1 + (int)(sliderValue * (maxLevels - 1));
    if (newLevels != levelsBeforeAbyss) {
        SetLevelsBeforeAbyss(newLevels);
    }

    yOffset += 40; // Space

    // REGENERATE BUTTON
    if (GuiButton({ 20, (float)yOffset, 280, 30 }, "REGENERATE")) {
        Regenerate();  // Create new level with same settings
    }

    yOffset += 45;

    // GENERATION ALGORITHMS SECTION
    GuiLabel({ 20, (float)yOffset, 200, 20 }, "Generation Algorithms:");
    yOffset += 25;

    // Cellular Automata button
    if (GuiButton({ 20, (float)yOffset, 130, 25 }, "Cellular Auto")) {
        GenerateCellularAutomata(5);
    }

    // Random Walk button
    if (GuiButton({ 160, (float)yOffset, 130, 25 }, "Random Walk")) {
        GenerateRandomWalk(200);
    }

    yOffset += 35;

    // Clear button
    if (GuiButton({ 160, (float)yOffset, 130, 25 }, "Clear All")) {
        ClearGrid();
    }

    yOffset += 45;

    // SAVE/LOAD SECTION
    GuiLabel({ 20, (float)yOffset, 200, 20 }, "Save/Load:");
    yOffset += 25;

    if (GuiButton({ 20, (float)yOffset, 130, 25 }, "SAVE")) {
        SaveToFile("level_data.bin");
    }

    if (GuiButton({ 160, (float)yOffset, 130, 25 }, "LOAD")) {
        LoadFromFile("level_data.bin");
    }

    yOffset += 45;

    // INSTRUCTIONS
    GuiLabel({ 20, (float)yOffset, 280, 20 }, "Controls:");
    yOffset += 20;
    GuiLabel({ 20, (float)yOffset, 280, 20 }, "Alt + Drag: Orbit camera");
    yOffset += 20;
    GuiLabel({ 20, (float)yOffset, 280, 20 }, "Scroll: Zoom in/out");
}

// Manual editing - change a specific tile
void HexGridGenerator::SetTile(int level, int x, int y, TileType type) {
    // Validate coordinates (prevent crashes from bad input)
    if (level >= 0 && level < levelsBeforeAbyss) {
        int rowIdx = y + gridRadius;
        int colIdx = x + gridRadius;
        if (rowIdx >= 0 && rowIdx < 2 * gridRadius + 1 &&
            colIdx >= 0 && colIdx < 2 * gridRadius + 1) {
            grid[level][rowIdx][colIdx].type = type;
        }
    }
}

// Get tile type (with safety checks)
TileType HexGridGenerator::GetTile(int level, int x, int y) const {
    if (level >= 0 && level < levelsBeforeAbyss) {
        int rowIdx = y + gridRadius;
        int colIdx = x + gridRadius;
        if (rowIdx >= 0 && rowIdx < 2 * gridRadius + 1 &&
            colIdx >= 0 && colIdx < 2 * gridRadius + 1) {
            return grid[level][rowIdx][colIdx].type;
        }
    }
    return TileType::EMPTY;  // Return empty if invalid
}

// Erase everything
void HexGridGenerator::ClearGrid() {
    for (int level = 0; level < maxLevels; level++) {
        for (auto& row : grid[level]) {
            for (auto& tile : row) {
                tile.type = TileType::EMPTY;
            }
        }
    }
}

void HexGridGenerator::Regenerate() {
    GenerateCellularAutomata(5);
}

// ===== FILE I/O - Save level to disk =====
bool HexGridGenerator::SaveToFile(const std::string& filename) {
    // Open file for binary writing
    // std::ios::binary means "write raw bytes, not text"
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Write HEADER first (this defines what version of the file format we're using)
    int version = 2;  // Version 2 = format with circular arena and level stacking
    file.write(reinterpret_cast<char*>(&version), sizeof(version));
    file.write(reinterpret_cast<char*>(&gridRadius), sizeof(gridRadius));
    file.write(reinterpret_cast<char*>(&maxLevels), sizeof(maxLevels));
    file.write(reinterpret_cast<char*>(&levelsBeforeAbyss), sizeof(levelsBeforeAbyss));

    // Write the actual GRID DATA
    // Only save tiles that are inside the circle
    for (int level = 0; level < levelsBeforeAbyss; level++) {
        for (int r = 0; r < 2 * gridRadius + 1; r++) {
            for (int q = 0; q < 2 * gridRadius + 1; q++) {
                // Convert array index back to world coordinate to check circle
                int worldQ = q - gridRadius;
                int worldR = r - gridRadius;
                if (IsWithinCircle(worldQ, worldR)) {
                    int type = static_cast<int>(grid[level][r][q].type);
                    file.write(reinterpret_cast<char*>(&type), sizeof(type));
                }
            }
        }
    }

    file.close();
    std::cout << "Level saved to " << filename << std::endl;
    return true;
}

// ===== FILE I/O - Load level from disk =====
bool HexGridGenerator::LoadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Read header
    int version, radius, maxLvls, levels;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    file.read(reinterpret_cast<char*>(&radius), sizeof(radius));
    file.read(reinterpret_cast<char*>(&maxLvls), sizeof(maxLvls));
    file.read(reinterpret_cast<char*>(&levels), sizeof(levels));

    // If saved grid is different size, reinitialize
    if (radius != gridRadius || maxLvls != maxLevels) {
        InitializeGrid(radius, maxLvls);
    }

    levelsBeforeAbyss = levels;

    // Read grid data
    for (int level = 0; level < levelsBeforeAbyss; level++) {
        for (int r = 0; r < 2 * gridRadius + 1; r++) {
            for (int q = 0; q < 2 * gridRadius + 1; q++) {
                int worldQ = q - gridRadius;
                int worldR = r - gridRadius;
                if (IsWithinCircle(worldQ, worldR)) {
                    int type;
                    file.read(reinterpret_cast<char*>(&type), sizeof(type));
                    grid[level][r][q].type = static_cast<TileType>(type);
                }
            }
        }
    }

    file.close();
    std::cout << "Level loaded from " << filename << std::endl;
    return true;
}