#pragma once 
#include <raylib.h>     
#include <vector>       // C++ dynamic arrays
#include <string>       // file names
#include <random>       // random number generation

// This is an ENUM - like a dropdown in Blueprints where you pick from named options
// Each tile on the grid can be one of these types
enum class TileType {
    EMPTY = 0,      // No platform - player falls through
    PLATFORM = 1,   // Solid ground that holds the player
};

// A STRUCT is like a Blueprint Struct - it bundles multiple variables together
// Each hex tile on our grid is a small object with these properties
struct HexTile {
    TileType type;          // What kind of tile is this? (Platform, Empty, etc)
    bool isSelected;        // Is the player hovering over this tile?
    Vector3 worldPosition;  // Where is it in 3D space? (X, Y, Z coordinates)
    int level;              // Which floor/level is this tile on? (0 = top floor, 1 = below, etc)

    // CONSTRUCTOR - runs when a new HexTile is created
    // Sets default values so every tile starts the same way
    HexTile() : type(TileType::PLATFORM), isSelected(false), worldPosition({ 0,0,0 }), level(0) {}
};

// The MAIN CLASS - like a Blueprint Actor that handles everything
class HexGridGenerator {
private:
    // ===== PRIVATE: Only this class can touch these variables =====

    // This is a 3D VECTOR - think of it as a BUILDING with floors, rows, and columns
    // grid[LEVEL][ROW][COLUMN] - each spot holds a HexTile
    // The '&' here is just part of the type syntax, not a reference
    std::vector<std::vector<std::vector<HexTile>>> grid;

    int gridRadius;      // How many hexagons from center to edge? (like a circle radius)
    int maxLevels;       // Maximum number of floors we can have
    float hexRadius;     // Physical size of each hexagon
    float hexSpacing;    // Gap between hexagons (so they don't overlap)

    // PERLIN NOISE settings - this creates natural-looking terrain instead of random chaos
    float noiseScale;        // How "zoomed in" the noise is (smaller = smoother terrain)
    float platformThreshold; // Above this noise value = platform, below = empty

    int levelsBeforeAbyss;   // How many floors exist before bottomless pit

    // Random number generator - like a dice roller that gives consistent results
    std::mt19937 rng;  // Mersenne Twister algorithm - don't worry about the name

    // ===== PRIVATE HELPER FUNCTIONS =====

    // Converts grid coordinates (q=column, r=row) to actual 3D world position
    // For example: grid position (0,0) becomes world position (0, -2.4, 0)
    Vector3 HexToWorld(int q, int r, float yOffset = 0.0f);

    // Checks if a hex coordinate is inside our circular arena
    // Returns TRUE if the tile is within the circle, FALSE if it's outside
    bool IsWithinCircle(int q, int r);

public:
    // ===== PUBLIC: Anyone can use these functions =====

    // CONSTRUCTOR - runs when we create the generator
    HexGridGenerator();

    // DESTRUCTOR - runs when the generator is destroyed (cleans up memory)
    ~HexGridGenerator();

    // Creates the initial empty grid
    // radius = how many hexes from center to edge (8 makes a 17-hex wide circle)
    // maxStackLevels = how many floors we can stack (like 5 floors in a parking garage)
    void InitializeGrid(int radius, int maxStackLevels);

    // ===== PROCEDURAL GENERATION ALGORITHMS =====

    // Generates terrain using Random Walk (start center, randomly move around placing tiles)
    void GenerateRandomWalk(int steps);

    // Generates terrain using Cellular Automata (like Game of Life - tiles live/die based on neighbors)
    void GenerateCellularAutomata(int iterations = 5);

    // ===== EDITOR FUNCTIONS =====

    // Manually set a specific tile's type
    void SetTile(int level, int x, int y, TileType type);

    // Get a specific tile's type (Returns EMPTY if coordinates are invalid)
    TileType GetTile(int level, int x, int y) const;

    // Erase all tiles (turns everything to EMPTY)
    void ClearGrid();

    // ===== LEVEL CONTROL =====

    void SetLevelsBeforeAbyss(int levels);  // Change how many floors exist
    int GetLevelsBeforeAbyss() const { return levelsBeforeAbyss; }

    // ===== RENDERING =====

    void Render3D(const Camera3D& camera);  // Draw the 3D scene
    void RenderUI();                         // Draw buttons and sliders

    // ===== FILE I/O (Save/Load) =====

    bool SaveToFile(const std::string& filename);  // Save level to disk
    bool LoadFromFile(const std::string& filename); // Load level from disk

    // ===== GETTERS (like Blueprint "Get" nodes) =====

    int GetRadius() const { return gridRadius; }
    int GetMaxLevels() const { return maxLevels; }

    void Regenerate();  // Generate a new level with current settings
};