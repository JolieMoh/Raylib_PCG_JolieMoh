/*
===============================================================================
Construct Map Editor
-------------------------------------------------------------------------------
Author:      Griffith Film School
Description: A simple tile-based map editor template using raylib for
			 introductory programming students. Demonstrates 2D arrays,
			 nested loops, and basic graphics rendering.
===============================================================================
*/

#include "raylib.h"
#include "resource_dir.h"
#include "PCG.h" // Import our new module

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Construct Map Editor");

    TileType tileArray[MAP_ROWS][MAP_COLUMNS] = { 0 };
    PCG_CreateMap(tileArray);

    // Save text and image
    PCG_SaveMapData(tileArray, MAP_TEXT_FILENAME);
    PCG_SaveMapImage(tileArray, MAP_IMAGE_FILENAME);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        PCG_DrawMap(tileArray); // Function from PCG.c
        PCG_DrawGUI(tileArray); 
        DrawText("Construct Map Editor", 20, 20, 20, WHITE);
        EndDrawing();
        
    }

    CloseWindow();
    return 0;
}
