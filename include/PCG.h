#pragma once
#include "raylib.h"

namespace PCG {

    // Screen & Map Dimensions
#define SCREEN_WIDTH 4096
#define SCREEN_HEIGHT 4096
#define TILE_SIZE 64
#define MAP_COLUMNS (SCREEN_WIDTH / TILE_SIZE)
#define MAP_ROWS (SCREEN_HEIGHT / TILE_SIZE)

// Tile Types (Using Enum for readability)
    typedef enum {
        TILE_TYPE_GRASS = 0,
        TILE_TYPE_ROCK = 1,
        TILE_COUNT  // Automatically counts total types
    } TileType;

    // Visual & Character settings
#define GRASS_CHAR '.'
#define ROCK_CHAR '#'
#define GRASS_COLOR {69, 182, 156, 255}
#define ROCK_COLOR {114, 147, 160, 255}
#define UNKNOWN_COLOR WHITE

// Function Declarations
    void PCG_CreateMap(TileType _tileArray[MAP_ROWS][MAP_COLUMNS]);
    void PCG_DrawMap(TileType _tileArray[MAP_ROWS][MAP_COLUMNS]);
    void PCG_PrintMap(TileType _tileArray[MAP_ROWS][MAP_COLUMNS]);
    Color PCG_GetTileColor(TileType tileType);

    // File Names
#define MAP_TEXT_FILENAME "pcg_map_data.txt"

// Helpers
    char GetTileChar(TileType tileType);

    // I/O Functions
    void PCG_SaveMapData(TileType _tileArray[MAP_ROWS][MAP_COLUMNS], const char* filename);
    void PCG_LoadMapData(TileType _tileArray[MAP_ROWS][MAP_COLUMNS], const char* filename);

    // Img file save
#define MAP_IMAGE_FILENAME "pcg_map.png"
    void PCG_SaveMapImage(TileType _tileArray[MAP_ROWS][MAP_COLUMNS], const char* filename);

    // UI variable defines used to position buttons on screen
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50
#define BUTTON_X (SCREEN_WIDTH - BUTTON_WIDTH - 20)
#define BUTTON_Y (SCREEN_HEIGHT - BUTTON_HEIGHT - 20)
#define RESET_BUTTON_BOUNDS { BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT }

// Declare UI drawing function
    void PCG_DrawGUI(TileType tileArray[MAP_ROWS][MAP_COLUMNS]);

}

#endif // PCG_H