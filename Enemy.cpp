#include "DxLib.h"
#include "Enemy.h"
#include "Player.h"
#include "Stage.h"
#include <cmath>
#include <random>
#include <ctime>

Enemy::Enemy(E_ENEMY_TYPE type) : type(type), map_x(0), map_y(0), stage(nullptr), hp(10), EnemyGraph(-1), atk(5) {
    if (type == ENEMY_SKELTON)
    {
        EnemyGraph = LoadGraph("Assets/skelton_1.png");
        hp = 15;
        atk = 5;
    }
    else if (type == ENEMY_SLIME)
    {
        EnemyGraph = LoadGraph("Assets/slime.png");
        hp = 8;
        atk = 3;
    }
}

Enemy::~Enemy()
{
    if (EnemyGraph != -1)
    {
        DeleteGraph(EnemyGraph);
    }
}

void Enemy::SetPosition(int map_x, int map_y)
{
    this->map_x = map_x;
    this->map_y = map_y;
}

bool Enemy::CheckCollision(int next_map_x, int next_map_y)
{
    if (!stage) return true;

    if (stage->GetTileType(next_map_x, next_map_y) == TILE_WALL) return true;

    // 敵同士の重なり判定
    for (auto other : stage->GetEnemies())
    {
        if (other == this) continue;
        if (other->GetHP() <= 0) continue;

        if (other->GetMapX() == next_map_x && other->GetMapY() == next_map_y)
        {
            return true;
        }
    }

    return false;
}

bool Enemy::Update()
{
    if (!stage) return false;
    Player* player = stage->GetPlayer();
    if (!player) return false;

    int px = player->GetMapX();
    int py = player->GetMapY();
    int floor = stage->GetCurrentFloor();
    bool acted = false; // このターンすでに行動したか

    int diffX = px - map_x;
    int diffY = py - map_y;
    int dist = abs(diffX) + abs(diffY);

    if (dist <= 8)
    {
        bool canAttack = false;

        if (floor % 5 == 0)
        {
            float bCX = (float)map_x + 0.5f;
            float bCY = (float)map_y + 0.5f;
            float pCX = (float)px + 0.5f;
            float pCY = (float)py + 0.5f;

            float diffX = abs(bCX - pCX);
            float diffY = abs(bCY - pCY);

            // 攻撃リーチを3.0fに設定。2.5fで止まったゴーレムの攻撃が、しっかりプレイヤーに届きます
            if (diffX <= 3.0f && diffY <= 2.0f)
            {
                canAttack = true;
            }
        }
        else
        {
            if (dist == 1) canAttack = true;
        }

        if (canAttack)
        {
            player->Heal(-atk);
            acted = true;
        }
        else
        {
            int dx = 0;
            int dy = 0;

            if (abs(diffX) > abs(diffY)) dx = (diffX > 0) ? 1 : -1;
            else                        dy = (diffY > 0) ? 1 : -1;

            if (!CheckCollision(map_x + dx, map_y + dy))
            {
                map_x += dx;
                map_y += dy;
                acted = true;
            }
            else
            {
                if (dx != 0) { dx = 0; dy = (diffY > 0) ? 1 : -1; }
                else { dy = 0; dx = (diffX > 0) ? 1 : -1; }

                if (!CheckCollision(map_x + dx, map_y + dy))
                {
                    map_x += dx;
                    map_y += dy;
                    acted = true;
                }
            }
        }
    }

    if (!acted)
    {
        static std::mt19937 mt(static_cast<unsigned int>(time(NULL)));
        std::uniform_int_distribution<int> dir_dist(0, 4); // 0は待機
        int dir = dir_dist(mt);

        int dx = 0, dy = 0;
        if (dir == 1) dy = -1;
        else if (dir == 2) dy = 1;
        else if (dir == 3) dx = -1;
        else if (dir == 4) dx = 1;

        if ((dx != 0 || dy != 0) && !CheckCollision(map_x + dx, map_y + dy))
        {
            map_x += dx;
            map_y += dy;
            acted = true;
        }
    }

    return acted;
}

void Enemy::Draw()
{
    if (!stage) return;

    int floor = stage->GetCurrentFloor();

    if (floor > 0 && floor % 5 == 0) return;

    Player* p = stage->GetPlayer();
    bool isClairvoyance = (p && p->clairvoyanceTurn > 0);
    if (!stage->IsTileVisible(map_x, map_y) && !isClairvoyance) return;

    if (EnemyGraph == -1) return;

    const float z = stage->GetZoomRate();
    float ds = stage->GetTileSize() * z;
    int ox = stage->GetCameraX();
    int oy = stage->GetCameraY();

    int lx = (int)(map_x * ds - ox * z);
    int ty = (int)(map_y * ds - oy * z);

    DrawExtendGraph(lx, ty, lx + (int)ds, ty + (int)ds, EnemyGraph, TRUE);

    // 赤い枠線を描画 (thickness を 2 にして少し太く)
    //DrawBox(lx, ty, lx + (int)ds, ty + (int)ds, GetColor(255, 0, 0), FALSE);
}