#include "raylib.h"
#include <stdio.h>
#include <string.h>
#define GRID_SIZE 15
#define TILE_SIZE 60
#define MAX_BOMBS 10
#define MAX_ENEMIES 10

typedef struct{
    int posX, posY, bombsAvailable, bombsMax, bombRange, score;
    bool status; // 0 = dead, 1 = alive
} PLAYER;

typedef struct{
    bool indestructible, empty, hasPowerUp, hasExit, pendingDestruction;
    Color color;
} TILE;

typedef struct{
    int posX, posY, timer, range;
    bool isActive, isExploding;
    double lifeTime, explosionTime;
} BOMB;

typedef struct{
    int posX, posY, dir;
    bool status, isStarted;
    double lastTime, currentTime;
} ENEMY;

typedef struct{
    int number;
    int maxEnemies;
    int enemySpeed;
    int maxWalls;
} LEVEL;

void UpdatePlayerPosition(PLAYER *p, TILE grid[][GRID_SIZE], LEVEL *l){
    //grid[15][15], 0 -> 14
    //if new position is empty, go there; else, do nothing
    int pX = p->posX / TILE_SIZE;
    int pY = p->posY / TILE_SIZE;

    //check for powerup
    if(grid[pX][pY].hasPowerUp){
        int type = GetRandomValue(0, 1); //0 = increase range, 1 == increase bombs available
        if(type == 0){
            p->bombRange++;
        } else {
            p->bombsMax++;
            p->bombsAvailable++;
        }
        grid[pX][pY].hasPowerUp = 0;
        grid[pX][pY].color = WHITE;
    }

    //check for exit
    if(grid[pX][pY].hasExit){
        //print level won
        //change level
        //HandleLevelChange(l);
        l->number++;
    }

    //check collision with enemy

    //if new position is empty and key pressed is 'a' or <-, go left
    if (grid[pX - 1][pY].empty && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))) p->posX = p->posX - TILE_SIZE;
    //if new position is empty and key pressed is 'd' or ->, go right
    if (grid[pX + 1][pY].empty && (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))) p->posX = p->posX + TILE_SIZE;
    //if new position is empty and key pressed is 'w' or <-, go up
    if (grid[pX][pY - 1].empty && (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))) p->posY = p->posY - TILE_SIZE;
    //if new position is empty and key pressed is 's' or <-, go down
    if (grid[pX][pY + 1].empty && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))) p->posY = p->posY + TILE_SIZE;
}

void DrawPlayer(PLAYER p){
    int playerSize = TILE_SIZE - 10, posX = p.posX + 5, posY = p.posY + 5;
    Rectangle playerRect = { posX, posY, playerSize, playerSize };
    DrawRectangleRec(playerRect, DARKBLUE);
    DrawText("P", posX + 15, posY + 10, 35, WHITE);
}

void UpdateEnemies(ENEMY *e, TILE grid[][GRID_SIZE], BOMB *b, PLAYER *p, LEVEL l){
    for(int i=0; i<l.maxEnemies; i++){
        int pX = e[i].posX / TILE_SIZE, pY = e[i].posY / TILE_SIZE;

        //start enemy in a random position between 5 - 13, keeping a safe starting area for the player
        if(!e[i].isStarted){
            while(!grid[pX][pY].empty){
                pX = GetRandomValue(5, 13);
                pY = GetRandomValue(5, 13);
            }
            e[i].status = 1;
            e[i].isStarted = 1;
            e[i].posX = pX * TILE_SIZE;
            e[i].posY = pY * TILE_SIZE;
            e[i].dir = GetRandomValue(0, 3);
        }

        //move enemy every 1 sec * speed
        double diffTime = e[i].currentTime - e[i].lastTime;
        e[i].currentTime = GetTime();
        if( diffTime * l.enemySpeed >= 1.0){
            //select random direction
            //move in this direction until next tile is not empty

            //provisory position, first need to check if any collision will happen
            int new_x = e[i].posX;
            int new_y = e[i].posY;

            switch(e[i].dir){
                //up
                case 0: if(grid[pX][pY-1].empty) new_y -= TILE_SIZE; else e[i].dir = GetRandomValue(0, 3); break;
                //down
                case 1: if(grid[pX][pY+1].empty) new_y += TILE_SIZE; else e[i].dir = GetRandomValue(0, 3); break;
                //right
                case 2: if(grid[pX+1][pY].empty) new_x += TILE_SIZE; else e[i].dir = GetRandomValue(0, 3); break;
                //left
                case 3: if(grid[pX-1][pY].empty) new_x -= TILE_SIZE; else e[i].dir = GetRandomValue(0, 3); break;
            }

            //check for collision with explosion
            //if(grid[new_x / TILE_SIZE][new_y / TILE_SIZE].hasExplosion) e[i].status = 0;

            //check for collision with other enemies
            int collided = 0;
            for(int j=0; j<l.maxEnemies; j++) {
                if(i != j && e[j].posX == new_x && e[j].posY == new_y) {
                    collided = 1;
                    break;
                }
            }

            //check collision with bomb
            for(int j=0; j<3; j++){
                int b_x = b[j].posX;
                int b_y = b[j].posY;
                if(b_x == new_x && b_y == new_y) {
                    collided = 1;
                    break;
                }
            }

            //change direction or go to the provisory position
            if(!collided) {
                e[i].posX = new_x;
                e[i].posY = new_y;
            } else {
                e[i].dir = GetRandomValue(0, 3);
            }

            //check for collision with player
            if(p->posX == e[i].posX && p->posY == e[i].posY) {
                p->status = 0;
            }

            //update time
            e[i].lastTime = e[i].currentTime;
        }
    }
}

void DrawEnemy(ENEMY e){
    int enemySize = TILE_SIZE - 10, posX = e.posX + 5, posY = e.posY + 5;
    Rectangle enemyRect = { posX, posY, enemySize, enemySize };
    DrawRectangleRec(enemyRect, BLUE);
    DrawText("E", posX + 15, posY + 10, 35, WHITE);
}

void DrawBomb(BOMB b){
    int r = TILE_SIZE / 2 - 10;
    int drawX = b.posX + (TILE_SIZE * 1 / 2);
    int drawY = b.posY + (TILE_SIZE * 1 / 2);

    DrawCircle(drawX, drawY, r, BLACK);
    DrawText(TextFormat("%d", (int) b.lifeTime+1), drawX - 5, drawY - 10, 25, WHITE);
}

void HandleExplosionsDirection(BOMB *b, PLAYER *p, TILE grid[][GRID_SIZE], int initialX, int initialY, int gridX, int gridY, int dx, int dy, int currentBomb, ENEMY *e, LEVEL l){
    for(int r = 1; r <= p->bombRange; r++){
        int newGridX = gridX + dx*r;
        int newGridY = gridY + dy*r;
        int newX = initialX + dx * r * TILE_SIZE;
        int newY = initialY + dy * r * TILE_SIZE;

        //do not propagate towards indestructible tiles
        if(grid[newGridX][newGridY].indestructible){
            break;
        }
        //if its not empty, its a destructible wall. it should explode but stop propagation
        else if(!grid[newGridX][newGridY].empty){
            DrawRectangle(newX, newY, TILE_SIZE, TILE_SIZE, ORANGE);
            DrawRectangleLines(newX, newY, TILE_SIZE, TILE_SIZE, BLACK);

            grid[newGridX][newGridY].pendingDestruction = 1;
            break;
        }
        //others cases are blank tiles
        else{
            DrawRectangle(newX, newY, TILE_SIZE, TILE_SIZE, ORANGE);
            DrawRectangleLines(newX, newY, TILE_SIZE, TILE_SIZE, BLACK);
            //check if player is hit
            if(p->posX == newX && p->posY == newY) p->status = 0;
            //check if any enemies were hit
            for(int i=0; i<l.maxEnemies; i++){
                if(e[i].posX == newX && e[i].posY == newY) e[i].status = 0;
            }
            //check if any bombs were hit
            for(int i=0; i<p->bombsMax; i++){
                if(i != currentBomb && b[i].isActive && b[i].posX == newX && b[i].posY == newY)
                {
                    p->bombsAvailable++;
                    b[i].isExploding = 1;
                    b[i].explosionTime = 1;
                    b[i].isActive = 0;
                }
            }
        }
    }
}

void HandleExplosions(PLAYER *p, TILE grid[][GRID_SIZE], BOMB b[], ENEMY e[], LEVEL l){
    for(int i=0; i<p->bombsMax; i++){
        if(b[i].isExploding){
            //position = bomb position
            //range = player bombRange
            //explosion hits if tile is not indestructible and is in range
            //if it hits obstacle, stop propagating
            int initialX = b[i].posX;
            int initialY = b[i].posY;
            int gridX = initialX / TILE_SIZE;
            int gridY = initialY / TILE_SIZE;

            //initial position is guaranteed to explode
            DrawRectangle(initialX, initialY, TILE_SIZE, TILE_SIZE, ORANGE);
            //player can be in bomb position, enemies cant
            if(p->posX == initialX && p->posY == initialY) p->status = 0;

            //up, dy = -1
            HandleExplosionsDirection(b, p, grid, initialX, initialY, gridX, gridY, 0, -1, i, e, l);
            //down, dy = 1
            HandleExplosionsDirection(b, p, grid, initialX, initialY, gridX, gridY, 0, 1, i, e, l);
            //right, dx = 1
            HandleExplosionsDirection(b, p, grid, initialX, initialY, gridX, gridY, 1, 0, i, e, l);
            //left, dx = -1
            HandleExplosionsDirection(b, p, grid, initialX, initialY, gridX, gridY, -1, 0, i, e, l);
        }
    }
}

void AfterExplosions(TILE grid[][GRID_SIZE]){
    for(int posX = 0; posX < GRID_SIZE; posX++){
        for(int posY = 0; posY < GRID_SIZE; posY++){
            if(grid[posX][posY].pendingDestruction){
                grid[posX][posY].empty = 1;
                grid[posX][posY].pendingDestruction = 0;
                //if its not the exit or powerup, color the tile WHITE
                if(!grid[posX][posY].hasExit && !grid[posX][posY].hasPowerUp){
                    grid[posX][posY].color = WHITE;
                }
            }
        }
    }
}

void UpdateBombs(PLAYER *p, BOMB *b, TILE grid[][GRID_SIZE]){
    //decrease timers of active bombs
    //when timer is done, set the flag off and increase bombsAvailable
    for(int i=0; i<p->bombsMax; i++){
        if(b[i].isActive) {
            b[i].lifeTime -= GetFrameTime();
            if(b[i].lifeTime <= 0){
                p->bombsAvailable++;
                b[i].isActive = 0;
                b[i].isExploding = 1;
                b[i].explosionTime = 1;
            }
        }
        if(b[i].isExploding) {
            b[i].explosionTime -= GetFrameTime();
            if(b[i].explosionTime <= 0){
                b[i].isExploding = 0;
                AfterExplosions(grid);
            }
        }
    }

    //check if a new bomb is planted
    //set the coordinates of the bomb
    //set the flag to 1
    //decrease bombs available
    if(IsKeyPressed(KEY_SPACE) && p->bombsAvailable > 0){
        for(int i=0; i<p->bombsMax; i++){
            if(!b[i].isActive && !b[i].isExploding){
                b[i].posX = p->posX;
                b[i].posY = p->posY;
                b[i].isActive = 1;
                b[i].lifeTime = 2;
                p->bombsAvailable--;
                break;
            }
        }
    }
}

void HandleDeath(void){
    int width = GRID_SIZE * TILE_SIZE;
    int posX = 0;
    int posY = width / 2 - 60;

    DrawRectangle(posX, posY, width, 120, BLACK);
    DrawText("GAME OVER", posX + 170, posY + 20, 90, WHITE);
}

void Menu(void){
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

void MakeGrid(TILE grid[][GRID_SIZE]){
    for(int posX = 0; posX < GRID_SIZE; posX++){
        for(int posY = 0; posY < GRID_SIZE; posY++){
            //position indestructible walls
            if(posX == 0 || posY == 0 || posX == GRID_SIZE - 1 || posY == GRID_SIZE - 1 || ((posX % 2) == 0 && (posY % 2) == 0))
            {
                grid[posX][posY].indestructible = 1;
                grid[posX][posY].empty = 0;
                grid[posX][posY].color = DARKGRAY;
                grid[posX][posY].hasPowerUp = 0;
                grid[posX][posY].hasExit = 0;
                grid[posX][posY].pendingDestruction = 0;
            }
            else
            {
                grid[posX][posY].indestructible = 0;
                grid[posX][posY].empty = 1;
                grid[posX][posY].color = WHITE;
                grid[posX][posY].hasPowerUp = 0;
                grid[posX][posY].hasExit = 0;
                grid[posX][posY].pendingDestruction = 0;
            }
        }
    }
}

void DrawGridCustom(TILE grid[][GRID_SIZE]){
    ClearBackground(RAYWHITE);

    for(int posX = 0; posX < GRID_SIZE; posX++){
        for(int posY = 0; posY < GRID_SIZE; posY++){
            //first, draw powerup
            if(grid[posX][posY].hasPowerUp && grid[posX][posY].empty){
                DrawRectangle(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, WHITE);
                DrawRectangle(posX * TILE_SIZE + 5, posY*TILE_SIZE + 5, TILE_SIZE - 10, TILE_SIZE - 10, PURPLE);
                DrawRectangleLines(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
                DrawText("B", posX * TILE_SIZE + 20, posY*TILE_SIZE + 15, 35, WHITE);
            }
            //next, draw exit
            else if(grid[posX][posY].hasExit && grid[posX][posY].empty){
                DrawRectangle(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, WHITE);
                DrawRectangle(posX * TILE_SIZE + 5, posY*TILE_SIZE + 5, TILE_SIZE - 10, TILE_SIZE - 10, GREEN);
                DrawRectangleLines(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
            //then, draw the rest
            } else {
                DrawRectangle(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, grid[posX][posY].color);
                DrawRectangleLines(posX * TILE_SIZE, posY*TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
            }
        }
    }
}

void GenerateWallsAndPowerUp(TILE grid[][GRID_SIZE], LEVEL l){
    int posX = 0;
    int posY = 0;

    //then, for each wall, choose a random position that's empty
    for(int w = 0; w < l.maxWalls; w++){
        //in the remaining iterations, just create walls
        while(!grid[posX][posY].empty){
            posX = GetRandomValue(1, 13);
            posY = GetRandomValue(1, 13);
        }
        grid[posX][posY].color = BROWN;
        grid[posX][posY].empty = 0;

        //in the first iteration, place the exit and hide it
        if(w == 0) grid[posX][posY].hasExit = 1;

        //in the second iteration, place powerUp and hide it
        if(w == 1) grid[posX][posY].hasPowerUp = 1;
    }
}

void DrawLevelPass(void){
    int width = GRID_SIZE * TILE_SIZE;
    int posX = 0;
    int posY = width / 2 - 60;

    DrawRectangle(posX, posY, width, 120, GREEN);
    DrawText("Fase completa! Pressione ENTER para continuar", posX + 70, posY + 45, 30, WHITE);
}

void DrawInterface(PLAYER p, LEVEL l){
    int width = GRID_SIZE * TILE_SIZE;
    int height = TILE_SIZE;
    int posX = 0;
    int posY = width;
    char info[50];

    snprintf(info, sizeof(info), "%s %d %s %d %s %d %s %d %s %d", "Fase:", l.number, "   Score: ", p.score, "   Bombas:", p.bombsAvailable, "/", p.bombsMax, "Alcance:", p.bombRange);

    DrawRectangle(posX, posY, width, height, LIGHTGRAY);
    DrawText(info, posX + 60, posY + 10, 30, BLACK);
}

int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = GRID_SIZE * TILE_SIZE;
    const int screenHeight = screenWidth + TILE_SIZE; //extra tile size for the interface below

    InitWindow(screenWidth, screenHeight, "Mini Bomberman");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    PLAYER player1 = {TILE_SIZE, TILE_SIZE, 2, 2, 2, 0, 1};
    LEVEL level = {1, 3, 2, 5};
    int currentLevel = level.number; //maybe change to player attribute
    TILE grid[GRID_SIZE][GRID_SIZE];
    MakeGrid(grid);
    GenerateWallsAndPowerUp(grid, level);
    BOMB bombs[MAX_BOMBS] = {0};
    ENEMY enemies[MAX_ENEMIES] = {0};

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if(currentLevel == level.number && player1.status){
            UpdatePlayerPosition(&player1, grid, &level);
            UpdateBombs(&player1, bombs, grid);
            UpdateEnemies(enemies, grid, bombs, &player1, level);
        }
        //HandleLevelPass(&level, &currentLevel);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            //Menu();
            if(player1.status){
                DrawGridCustom(grid);
                DrawInterface(player1, level);
                for(int i=0; i<player1.bombsMax; i++){
                    if(bombs[i].isActive) DrawBomb(bombs[i]);
                    if(bombs[i].isExploding) HandleExplosions(&player1, grid, bombs, enemies, level);
                }
                //AfterExplosions(grid);
                for(int i=0; i<level.maxEnemies; i++){
                    if(enemies[i].status) DrawEnemy(enemies[i]);
                }
                DrawPlayer(player1);
                if(currentLevel != level.number){
                    DrawLevelPass();
                }
            } else HandleDeath();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
