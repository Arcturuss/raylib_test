#include "raylib.h"
#include <math.h>

#define TARGETS_LEN 30

struct Target {
    int state; // 0 = inactive, 1 = active, 2 = clicked, 3 = fading
    float t;
    Vector2 pos;
    float r;
};

bool _debug = false;

float rt_max = 4.0f;
float rt_fade = 0.3f;
float r_max = 70.0f;

Target targets[TARGETS_LEN] = {0};
int tg_count = 0;
int tg_active = 0;

int score, hit, missed, total = 0;
int health = 5;
int max_targets = 4;
int min_targets = 1;

// ms
float min_delay = 200;
float max_delay = 1500;
float delay = 0;

// seconds
float last_target_created = 0;

float getRadiusK(float t, float tMax) {
    // return 1 - (fabsf((2.0f * t / tMax) - 1));
    return sin(t * (M_PI / tMax));
}

void drawTargets(Target *targets) {
    for (int i = tg_count - 1; i >= 0; i--) {
        switch (targets[i].state) {
            case 0:
                continue; 
                // break;
            case 1: {
                // DrawCircleV(targets[i].pos, targets[i].r, RED);
                DrawRing(targets[i].pos, targets[i].r, fmax(0, targets[i].r - 5), 0, 360, 32, RED);
                DrawCircleV(targets[i].pos, fmax(0, targets[i].r - 5), WHITE);
                break;
            }
            case 2: {
                // DrawCircleV(targets[i].pos, targets[i].r, GREEN);
                DrawRing(targets[i].pos, targets[i].r, fmax(0, targets[i].r - 5), 0, 360, 32, GREEN);
                DrawCircleV(targets[i].pos, fmax(0, targets[i].r - 5), WHITE);
                break;
            }
            case 3: {
                unsigned char alpha = 255 - (255 * (targets[i].t / rt_fade)); // transparent at t = rt_fade
                // DrawCircleV(targets[i].pos, targets[i].r, (Color){230, 41, 55, alpha});
                DrawRing(targets[i].pos, targets[i].r, fmax(0, targets[i].r - 5), 0, 360, 32, (Color){0, 228, 48, alpha});
                DrawCircleV(targets[i].pos, fmax(0, targets[i].r - 5), (Color){255, 255, 255, alpha});
            }
        }
    }
}

void processTargets(Target *targets) 
{
    bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Vector2 mouse_pos = GetMousePosition();
    float ft = GetFrameTime();

    for (int i = 0; i < tg_count; i++) {
        switch (targets[i].state) {
            case 0:
                continue; 
                break;
            case 1: {

                if (click) {
                    if (CheckCollisionPointCircle(mouse_pos, targets[i].pos, targets[i].r)) {
                        targets[i].state = 2;
                        tg_active -= 1;
                        score += 1;
                        hit += 1;
                        total += 1;
                        click = false;
                        continue;
                    }
                }

                targets[i].t += ft;
                targets[i].r = r_max * getRadiusK(targets[i].t, rt_max);

                if (targets[i].t >= rt_max) {
                    targets[i].state = 0;
                    tg_active -= 1;
                    score -= 1;
                    missed += 1;
                    total += 1;
                    continue;
                }
                break;
            }
            case 2: {
                targets[i].state = 3;
                targets[i].t = 0;
                break;
            }
            case 3: {
                targets[i].t += ft;
                if (targets[i].t >= rt_fade) {
                    targets[i].state = 0;
                    continue;
                }
                break;
            }
        }
    }
}

void DrawTextCentered(const char *text, int posX, int posY, int fontSize, Color color)
{
    int w = MeasureText(text, fontSize);
    DrawText(text, posX - (w / 2), posY - (fontSize / 2), fontSize, color);
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 700;
    
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    // SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);

    InitWindow(screenWidth, screenHeight, "reflex");
    SetTargetFPS(60);

    Vector2 dpiScale = GetWindowScaleDPI();

    int x_min = (int)r_max;
    int y_min = (int)r_max;
    int x_max = screenWidth - (int)r_max;
    int y_max = screenHeight - (int)r_max;
    
    int gameState = 0;
    float gameTimer = 0;

    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(GRAY);

        float ft = GetFrameTime();
        float t = GetTime();

        if (IsKeyPressed(KEY_D)) {
            _debug = !_debug;
        }

        switch (gameState) {

        case 0: {
            if (total != 0) {
                DrawTextCentered(TextFormat("Score: %d", score), screenWidth / 2, screenHeight / 2 - 125, 50, BLACK);
                DrawTextCentered(TextFormat("hit: %d, missed: %d, total: %d", hit, missed, total), screenWidth / 2, screenHeight / 2 - 75, 30, BLACK);
            }
            if (gameTimer > 0) {
                gameTimer -= ft;
            }
            else {
                DrawTextCentered("Click to start", screenWidth / 2, screenHeight / 2, 50, BLACK);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    
                    for (int i = 0; i < TARGETS_LEN; i++) targets[i] = {0};
                    tg_count = 0;
                    tg_active = 0;

                    score = hit = missed = total = 0;
                    health = 5;
                    max_targets = 4;
                    min_targets = 1;

                    // ms
                    min_delay = 200;
                    max_delay = 1000;
                    delay = 0;

                    rt_max = 4.0f;

                    gameState = 1;
                    gameTimer = 30;
                }
            }
        } break;

        case 1: {
            gameTimer -= ft;
            if (gameTimer <= 0.0f) {
                gameState = 0;
                gameTimer = 5; // freeze period
                continue;
            }

            max_targets = 4 + (30 - gameTimer) / 10;
            min_targets = 1 + (30 - gameTimer) / 10;
            min_delay -= (ft * 2);
            max_delay -= (ft * 20);
            rt_max -= (ft / 15.0);

            // create target
            if (tg_active < max_targets) {
                if (delay == 0) {
                    delay = GetRandomValue(min_delay, max_delay);
                }
                if ((tg_active < min_targets) || ((t - ((float)delay/1000.0f)) > last_target_created)) {
                    if (tg_count < TARGETS_LEN) {
                        targets[tg_count].state = 1;
                        targets[tg_count].t = 0;
                        targets[tg_count].pos = {(float)GetRandomValue(x_min, x_max), (float)GetRandomValue(y_min, y_max)};
                        targets[tg_count].r = 0;

                        last_target_created = t;
                        tg_count += 1;
                        tg_active += 1;
                        delay = 0;
                    }
                }
            }

            processTargets(targets);
            drawTargets(targets);

            // compact array
            int write_idx = 0;
            for (int i = 0; i < tg_count; i++) {
                if (targets[i].state > 0) {
                    if (i != write_idx) {
                        targets[write_idx] = targets[i];
                    }
                    write_idx++;
                }
            }
            tg_count = write_idx;

            DrawText(TextFormat("score: %d (hit %d missed %d)", score, hit, missed), 10, 10, 30, BLACK);
            DrawText(TextFormat("%.1f", gameTimer), screenWidth - 110, 10, 50, BLACK);

            if (_debug) {
                DrawText(TextFormat("min_d: %.0f, max_d: %.0f, ft: %.2f, d: %.0f, tmax: %.1f", 
                    min_delay, max_delay, ft, delay, rt_max), 10, screenHeight - 40, 20, BLACK);
                DrawText(TextFormat("active: %d, cnt: %d, max: %d", tg_active, tg_count, max_targets), 10, screenHeight - 60, 20, BLACK);
            }


        } break;
        } // switch

        if (_debug)
            DrawText(TextFormat("dpi: %.3f, fps: %.1f", dpiScale.x, 1 / ft), 10, screenHeight - 20, 20, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}