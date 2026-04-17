// @noryx101

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

#define MAX_BRICKS 20
#define MAX_BULLETS 20

typedef struct Brick {
    Vector2 pos;
    bool active;
} Brick;

typedef struct Player {
    Vector2 pos;
    Brick brick[3];
} Player;

void DrawBrick(Brick* brick)
{
    DrawRectangle(brick->pos.x, brick->pos.y, 12, 12, DARKGRAY);
    DrawRectangleLines(brick->pos.x, brick->pos.y, 15, 15, BLACK);
}

void DrawBrickPos(int x, int y)
{
    DrawRectangle(x, y, 12, 12, DARKGRAY);
    DrawRectangleLines(x, y, 15, 15, BLACK);
}

void DrawGameGrid(int screenWidth, int screenHeight, int cellSize)
{
    // Vertical lines
    for (int x = 0; x <= screenWidth; x += cellSize)
    {
        DrawLine(x, 0, x, screenHeight, Fade(GRAY, 0.3f));
    }

    // Horizontal lines
    for (int y = 0; y <= screenHeight; y += cellSize)
    {
        DrawLine(0, y, screenWidth, y, Fade(GRAY, 0.3f));
    }
}

void DrawPlayer(Player* player, int brickSize, int screenWidth)
{
    if ((IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) && player->pos.x != 0)
    {
        player->pos.x -= brickSize;
    }
    if ((IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) && player->pos.x < screenWidth - brickSize)
    {
        player->pos.x += brickSize;
    }

    DrawBrickPos(player->pos.x - brickSize, player->pos.y);
    DrawBrickPos(player->pos.x,             player->pos.y);
    DrawBrickPos(player->pos.x + brickSize, player->pos.y);
    DrawBrickPos(player->pos.x,             player->pos.y - brickSize);
}

// Spawn one brick
void SpawnBrick(Brick* brick, int screenWidth, int brickSize)
{
    // Snap to brickSizepx grid
    brick->pos.x = GetRandomValue(0, (screenWidth/brickSize) - 1) * brickSize;
    brick->pos.y = -brickSize;
    brick->active = true;
}

void DrawScore(int score) 
{
    char text[50];
    snprintf(text, sizeof(text), "Score: %d", score);

    DrawRectangle(0, 0, GetScreenWidth(), 30, LIGHTGRAY);
    DrawText(text, GetScreenWidth() - 110, 10, 20, WHITE);
}

void DrawPlayAgain(bool* isGameOver, bool* isReset)
{
    Rectangle buttonRect = { 55, 150, 140, 50 };

    if (CheckCollisionPointRec(GetMousePosition(), buttonRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        *isGameOver = !*isGameOver;
        *isReset = !*isReset;
    }

    DrawRectangleRec(buttonRect, DARKGRAY);
    DrawText("Play Again!", 70, 165, 22, BLACK);
}

int main(void)
{
    const int screenWidth = 250;
    const int screenHeight = 450;
    const int brickSize = 15;

    InitWindow(screenWidth, screenHeight, "Brick Game");

    SetTargetFPS(60);
    SetRandomSeed(GetTime());

    // Brick array
    Brick bricks[MAX_BRICKS] = {0};

    // Bullet array
    Brick* bullets[MAX_BULLETS] = {0};

    // Player
    Player player;
    player.pos = (Vector2){ (screenWidth / 2 / brickSize) * brickSize, (screenHeight / brickSize - 1) * brickSize };

    float moveTimer = 0.0f;
    float spawnTimer = 0.0f;

    int score = 0;
    float bulltetSpeed = 2.0f;

    bool isGameOver = false;
    bool isReset = false;

    while (!WindowShouldClose())
    {
        if (!isGameOver)
        {
            float dt = GetFrameTime();

            moveTimer += dt;
            spawnTimer += dt;

            // Spawn new brick every 1 second
            if (spawnTimer >= 1.0f)
            {
                spawnTimer = 0.0f;

                for (int i = 0; i < MAX_BRICKS; i++)
                {
                    if (!bricks[i].active)
                    {
                        SpawnBrick(&bricks[i], screenWidth, brickSize);
                        break;
                    }
                }
            }

            // Move bricks every 1 second (step movement)
            if (moveTimer >= 1.0f)
            {
                moveTimer = 0.0f;

                for (int i = 0; i < MAX_BRICKS; i++)
                {
                    if (bricks[i].active)
                    {
                        bricks[i].pos.y += brickSize;

                        // Remove if off screen
                        if (bricks[i].pos.y > screenHeight)
                        {
                            bricks[i].active = false;
                            isGameOver       = true;
                        }
                    }
                }
            }
        }

        // Check collisions between the bullets and the bricks
        for (int i = 0; i < MAX_BRICKS; i++)
        {
            for (int j = 0; j < MAX_BULLETS; j++)
            {
                if (bullets[j] != NULL && bricks[i].active)
                {
                    Rectangle bulletRect = { bullets[j]->pos.x, bullets[j]->pos.y, brickSize, brickSize };
                    Rectangle brickRect  = { bricks[i].pos.x,   bricks[i].pos.y,   brickSize, brickSize };

                    if (CheckCollisionRecs(bulletRect, brickRect))
                    {
                        // Add score
                        score += 1;

                        MemFree(bullets[j]);
                        bullets[j] = NULL;

                        // optional: mark brick as destroyed
                        bricks[i].active = false;

                        break; // stop checking this brick after hit
                    }
                }
            }
        }

        // Fire a bullet
        if (IsKeyPressed(KEY_SPACE))
        {
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                if (bullets[i] == NULL)
                {
                    Vector2 spawnPos = { player.pos.x, player.pos.y - brickSize };
                    bullets[i] = (Brick*)MemAlloc(sizeof(Brick));
                    bullets[i]->pos = spawnPos;
                    break;
                }
            }
        }
        
        // Destroy the bullet when out off screen
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (bullets[i] != NULL)
            {
                bullets[i]->pos.y -= bulltetSpeed;
                if (bullets[i]->pos.y < 0)
                {
                    MemFree(bullets[i]);
                    bullets[i] = NULL;
                    continue;  
                }
            }
        }

        // Reset the gameplay 
        if (isReset)
        {
            score = 0;

            for (int i = 0; i < MAX_BRICKS; i++)
            {
                for (int j = 0; j < MAX_BULLETS; j++)
                {
                    MemFree(bullets[j]);
                    bullets[j] = NULL;

                    // optional: mark brick as destroyed
                    bricks[i].active = false;
                }
            }

            player.pos = (Vector2){ (screenWidth / 2 / brickSize) * brickSize, (screenHeight / brickSize - 1) * brickSize };

            moveTimer = 0.0f;
            spawnTimer = 0.0f;

            isReset = false;
        }

        // Draw
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        // Grid first (background)
        DrawGameGrid(screenWidth, screenHeight, brickSize);

        // Draw all bricks
        for (int i = 0; i < MAX_BRICKS; i++)
        {
            if (bricks[i].active)
            {
                DrawBrick(&bricks[i]);
            }
        }
        
        // Draw the bullets
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (bullets[i] != NULL)
            {
                DrawBrick(bullets[i]);
            }
        }

        DrawPlayer(&player, brickSize, screenWidth);

        if (isGameOver)
        {
            DrawPlayAgain(&isGameOver, &isReset);
        }
        
        DrawScore(score);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
