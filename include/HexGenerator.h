#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include <random>

enum class TileType {
    EMPTY = 0,      // Abyss - instant death
    PLATFORM = 1,   // Solid platform
};

struct HexTile {
    TileType type;
    bool isSelected;
    Vector3 worldPosition;
    int level;  // Track which level this tile is on (for stacking)

    HexTile() : type(TileType::PLATFORM), isSelected(false), worldPosition({ 0,0,0 }), level(0) {}
};

class HexGridGenerator {
private:
    // 3D Grid storage for stacked platforms
    std::vector<std::vector<std::vector<HexTile>>> grid;  // [level][row][col]
    int gridRadius;  // Radius of circular arena
    int maxLevels;    // Maximum number of stacked levels
    float hexRadius;
    float hexSpacing;

    // PCG parameters
    float noiseScale;
    float platformThreshold;
    int levelsBeforeAbyss;  // modifiable feature

    // Random generation
    std::mt19937 rng;

    // Helper functions
    Vector3 HexToWorld(int q, int r, float yOffset = 0.0f);
    bool IsWithinCircle(int q, int r);
    void CalculateHexNeighbors(int q, int r, std::vector<std::pair<int, int>>& neighbors);

public:
    HexGridGenerator();
    ~HexGridGenerator();

    // Core generation
    void InitializeGrid(int width, int height);
    void GeneratePerlinNoiseMap();
    void GenerateRandomWalk(int steps);
    void GenerateCellularAutomata(int iterations = 5);

    // Editor functions
    void SetTile(int x, int y, TileType type);
    TileType GetTile(int x, int y) const;
    void ClearGrid();

    // Level control - Your requested feature!
    void SetLevelsBeforeAbyss(int levels);
    int GetLevelsBeforeAbyss() const { return levelsBeforeAbyss; }

    // Rendering
    void Render3D(const Camera3D& camera);
    void RenderUI();

    // File I/O - The Pipeline
    bool SaveToFile(const std::string& filename);
    bool LoadFromFile(const std::string& filename);

    // Getter for grid dimensions
    int GetWidth() const { return gridWidth; }
    int GetHeight() const { return gridHeight; }

    // Regenerate with current settings
    void Regenerate();
};