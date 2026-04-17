#include "raylib.h"
#include "resource_dir.h"
#include "PCG.h" // Import our new module

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(PCG::SCREEN_WIDTH, PCG::SCREEN_HEIGHT, "Construct Map Editor");

    //PCG::TileType tileArray[PCG::MAP_ROWS][PCG::MAP_COLUMNS] = { PCG::TileType::TILE_TYPE_ROCK };
    //PCG::CreateMap(tileArray);
    PCG::TileMap tileMap;
    //tileMap.CreateMap();
    //tileMap.SetMapGenerator(new PCG::RandomMapGenerator());
    tileMap.SetMapGenerator(new PCG::NoiseMapGenerator());
    tileMap.GetMapGenerator()->Generate(tileMap.tileArray); // Generate the map using the selected generator

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        //PCG::DrawMap(tileArray); // Function from PCG.c
        tileMap.DrawMap();
        DrawText("Construct Map Editor", 20, 20, 20, WHITE);
        //PCG::PCG_DrawGUI(tileArray);
        tileMap.DrawGUI();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}