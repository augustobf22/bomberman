#include "raylib.h"
#define GRID_SIZE 15
#define TILE_SIZE 60

void menu(void){
    //Font
/*    Font fontMecha = LoadFont("mecha.png");
    DrawTextEx(fontMecha, "Jogar", (Vector2){100, 100}, 100, 1, BLACK);
*/
    int choice;

    ClearBackground(RAYWHITE);
    DrawText("Jogar", 100, 100, 100, BLACK);
    DrawText("Continuar", 100, 300, 100, BLACK);
    DrawText("Carregar", 100, 500, 100, BLACK);
    DrawText("Sair", 100, 700, 100, BLACK);
}

int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = GRID_SIZE * TILE_SIZE;
    const int screenHeight = screenWidth;

    InitWindow(screenWidth, screenHeight, "Mini Bomberman");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            menu();
            ClearBackground(RAYWHITE);

            /*for(int posX = 0; posX < GRID_SIZE; posX++){
                for(int posY = 0; posY < GRID_SIZE; posY++){
                    if(posX == 0 || posY == 0 || posX == GRID_SIZE - 1 || posY == GRID_SIZE - 1) {
                        DrawRectangle(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY); // X, Y, LARGURA, ALTURA, COR
                        DrawRectangleLines(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
                    } else if ((posX % 2) == 0 && (posY % 2) == 0) {
                        DrawRectangle(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY); // X, Y, LARGURA, ALTURA, COR
                        DrawRectangleLines(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
                    } else {
                        DrawRectangle(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, WHITE); // X, Y, LARGURA, ALTURA, COR
                        DrawRectangleLines(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
                    }
                }
            }*/


        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
