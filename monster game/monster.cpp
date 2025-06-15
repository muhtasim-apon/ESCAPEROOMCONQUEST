#include "raylib.h"
#include "raymath.h"
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;

typedef struct {
    Vector2 pos;
    int health;
} Entity;

typedef struct {
    Vector2 pos;
    Vector2 speed;
    bool active;
} Bullet;

Entity player = {{121, 0}, 100};   // Y will be set dynamically
Entity monster = {{569, 0}, 100};  // Y will be set dynamically
std::vector<Bullet> playerBullets;
std::vector<Bullet> monsterBullets;

float monsterTimer = 0;
float monsterInterval = 2.0f;

void DrawBar(Vector2 pos, int health, Color color) {
    DrawRectangle(pos.x, pos.y, 100, 10, GRAY);
    DrawRectangle(pos.x, pos.y, health, 10, color);
}

void ShootBullet(std::vector<Bullet> &bullets, Vector2 pos, Vector2 speed) {
    bullets.push_back({pos, speed, true});
}

void UpdateBullets(std::vector<Bullet> &bullets) {
    for (auto &b : bullets) {
        if (b.active) {
            b.pos.x += b.speed.x;
            b.pos.y += b.speed.y;
            if (b.pos.x < 0 || b.pos.x > screenWidth || b.pos.y < 0 || b.pos.y > screenHeight)
                b.active = false;
        }
    }
}

void CheckCollision(std::vector<Bullet> &bullets, Entity &target, float targetW, float targetH) {
    for (auto &b : bullets) {
        Rectangle targetRect = { target.pos.x, target.pos.y, targetW, targetH };
        if (b.active && CheckCollisionCircleRec(b.pos, 5, targetRect)) {
            target.health -= 1;
            b.active = false;
        }
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "MECHANOID: LAST SURVIVOR");
    SetTargetFPS(60);

    Texture2D background = LoadTexture("background.png");
    Texture2D heroTex = LoadTexture("hero.png");
    Texture2D enemyTex = LoadTexture("enemy.png");
    Texture2D bulletTex = LoadTexture("bullet_player.png");
    Texture2D enemyBulletTex = LoadTexture("bullet_enemy.png");

    float targetHeight = 100.0f;
    float heroScale = (targetHeight / (float)heroTex.height) * 2.5f;
    float enemyScale = (targetHeight / (float)enemyTex.height) * 2.5f;

    float heroHeight = heroTex.height * heroScale;
    float heroWidth = heroTex.width * heroScale;
    float enemyHeight = enemyTex.height * enemyScale;
    float enemyWidth = enemyTex.width * enemyScale;

    float baselineY = screenHeight - heroHeight - 100;

    player.pos.y = baselineY;
    monster.pos.y = baselineY;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Movement controls
        if (IsKeyDown(KEY_W)) player.pos.y -= 200 * dt;
        if (IsKeyDown(KEY_S)) player.pos.y += 200 * dt;
        if (IsKeyDown(KEY_A)) player.pos.x -= 200 * dt;
        if (IsKeyDown(KEY_D)) player.pos.x += 200 * dt;

        // Clamp player position to window
        player.pos.x = Clamp(player.pos.x, 0.0f, (float)(screenWidth - heroWidth));
        player.pos.y = Clamp(player.pos.y, 0.0f, (float)(screenHeight - heroHeight));

        if (IsKeyPressed(KEY_SPACE))
            ShootBullet(playerBullets, {318, 367}, {300 * dt, 0});

        monsterTimer += dt;
        if (monsterTimer >= monsterInterval) {
            ShootBullet(monsterBullets, {584, 367}, {-300 * dt, 0});
            monsterTimer = 0;
        }

        UpdateBullets(playerBullets);
        UpdateBullets(monsterBullets);

        CheckCollision(playerBullets, monster, enemyWidth, enemyHeight);
        CheckCollision(monsterBullets, player, heroWidth, heroHeight);

        // WIN CONDITION: close window immediately
        if (monster.health <= 0) {
            CloseWindow();
            break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle src = {0, 0, (float)background.width, (float)background.height};
        Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
        DrawTexturePro(background, src, dest, {0, 0}, 0.0f, WHITE);

        DrawTextureEx(heroTex, player.pos, 0.0f, heroScale, WHITE);
        DrawBar({player.pos.x - 30, player.pos.y - 20}, player.health, GREEN);

        DrawTextureEx(enemyTex, monster.pos, 0.0f, enemyScale, WHITE);
        DrawBar({monster.pos.x - 30, monster.pos.y - 20}, monster.health, ORANGE);

        for (auto &b : playerBullets)
            if (b.active) DrawTextureEx(bulletTex, b.pos, 0.0f, 0.05f, WHITE);

        for (auto &b : monsterBullets)
            if (b.active) DrawTextureEx(enemyBulletTex, b.pos, 0.0f, 0.05f, WHITE);

        if (player.health <= 0)
            DrawText("GAME OVER", 300, 280, 40, RED);

        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(heroTex);
    UnloadTexture(enemyTex);
    UnloadTexture(bulletTex);
    UnloadTexture(enemyBulletTex);

    return 0;
}
