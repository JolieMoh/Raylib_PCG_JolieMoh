#include "PCG.h"
#include <stdio.h>


// ============================================= 
// PCG_CreateMap
// ============================================= 
void PCG::PCG_CreateMap(TileType _tileArray[MAP_ROWS][MAP_COLUMNS])
{
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLUMNS; x++) {
            _tileArray[y][x] = (TileType)GetRandomValue(0, TILE_COUNT - 1);
        }
    }
}

// ============================================= 
// PCG_GetTileColor
// ============================================= 
Color PCG::PCG_GetTileColor(TileType tileType)
{
    switch (tileType) {
    case TILE_TYPE_GRASS: return GRASS_COLOR;
    case TILE_TYPE_ROCK: return ROCK_COLOR;
    default: return UNKNOWN_COLOR;
    }
}

// ============================================= 
// PCG_DrawMap
// ============================================= 
void PCG::PCG_DrawMap(TileType _tileArray[MAP_ROWS][MAP_COLUMNS])
{
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLUMNS; x++) {
            DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, PCG_GetTileColor(_tileArray[y][x]));
        }
    }
}

// ============================================= 
// PCG_PrintMap
// ============================================= 
void PCG::PCG_PrintMap(TileType _tileArray[MAP_ROWS][MAP_COLUMNS])
{
    printf("\n-------Map Layout:--------\n");
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLUMNS; x++) {
            if (_tileArray[y][x] == TILE_TYPE_GRASS) {
                printf("%c", GRASS_CHAR);
            }
            else {
                printf("%c", ROCK_CHAR);
            }
        }
        printf("\n");
    }
    printf("--------------------------\n");
}
char PCG::GetTileChar(TileType tileType) {
    switch (tileType) {
    case TILE_TYPE_GRASS: return GRASS_CHAR;
    case TILE_TYPE_ROCK: return ROCK_CHAR;
    default: return '?';
    }
}

void PCG::PCG_SaveMapData(TileType _tileArray[MAP_ROWS][MAP_COLUMNS], const char* _filename) {
    FILE* file = fopen(_filename, "w"); // Write mode
    if (file == NULL) return;

    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLUMNS; x++) {
            fputc(GetTileChar(_tileArray[y][x]), file);
        }
        fputc('\n', file);
    }
    fclose(file);
}

void PCG::PCG_LoadMapData(TileType _tileArray[MAP_ROWS][MAP_COLUMNS], const char* _filename) {
    FILE* file = fopen(_filename, "r"); // Read mode
    if (file == NULL) return;

    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLUMNS; x++) {
            int ch = fgetc(file);
            // Skip invisible newline characters
            while (ch == '\n' || ch == '\r') { ch = fgetc(file); }

            if (ch == GRASS_CHAR) _tileArray[y][x] = TILE_TYPE_GRASS;
            else if (ch == ROCK_CHAR) _tileArray[y][x] = TILE_TYPE_ROCK;
        }
    }
    fclose(file);
}
// Img file save
void PCG::PCG_SaveMapImage(TileType _tileArray[MAP_ROWS][MAP_COLUMNS], const char* filename) {
    Image mapImage = GenImageColor(MAP_COLUMNS, MAP_ROWS, BLACK);

    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLUMNS; x++) {
            Color c = PCG_GetTileColor(_tileArray[y][x]);
            ImageDrawPixel(&mapImage, x, y, c);
        }
    }

    if (ExportImage(mapImage, filename)) {
        printf("Image saved successfully!\n");
    }
    UnloadImage(mapImage); // Free heap memory
}

// Required to call Raylib gui buttons. Add this near the top of PCG.c
#define RAYGUI_IMPLEMENTATION
#include "raygui.h" 

// ============================================= 
// PCG_DrawGUI
// ============================================= 
void PCG::PCG_DrawGUI(TileType tileArray[MAP_ROWS][MAP_COLUMNS]) {
    // Reset Button
    if (GuiButton(RESET_BUTTON_BOUNDS, "Reset Map")) {
        PCG_CreateMap(tileArray);
    }

    // Save Data Button
    saveRect = { BUTTON_X, BUTTON_Y - 70, BUTTON_WIDTH, BUTTON_HEIGHT };
    if (GuiButton(saveRect, "Save Map Data")) {
        PCG_SaveMapData(tileArray, MAP_TEXT_FILENAME);
    }

    // Load Data Button
    loadRect = { BUTTON_X, BUTTON_Y - 140, BUTTON_WIDTH, BUTTON_HEIGHT };
    if (GuiButton(loadRect, "Load Map Data")) {
        PCG_LoadMapData(tileArray, MAP_TEXT_FILENAME);
    }

    // Save Image Button
    imgRect = { BUTTON_X, BUTTON_Y - 210, BUTTON_WIDTH, BUTTON_HEIGHT };
    if (GuiButton(imgRect, "Save Map PNG")) {
        PCG_SaveMapImage(tileArray, MAP_IMAGE_FILENAME);
    }
}