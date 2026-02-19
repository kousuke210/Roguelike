#include "DxLib.h"
#include "Enemy.h"
#include "Player.h"
#include <cmath>
#include "Stage.h"
#include <random>
#include <ctime>

Enemy::Enemy()
    : map_x(0), map_y(0), stage(nullptr), hp(10) 
{
    EnemyGraph = LoadGraph("Assets/skelton_1.png");
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

    // 移動先のマスが壁（TILE_WALL）であれば衝突
    if (stage->GetTileType(next_map_x, next_map_y) == TILE_WALL)
    {
        return true; // 衝突
    }

    // プレイヤーとの衝突判定
    if (stage->GetPlayer() != nullptr) 
    {
        if (next_map_x == stage->GetPlayer()->GetMapX() && next_map_y == stage->GetPlayer()->GetMapY()) 
        {
            return true; // プレイヤーがいるマスへは移動しない
        }
    }

    return false; // 衝突なし
}

bool Enemy::Update()
{
    if (hp <= 0 || map_x < 0)
    {
        return true;
    }
    bool acted = false;
    const int num_directions = 5;
    const int dir_coords[num_directions][2] = { {0,0}, {0,-1}, {0,1}, {-1,0}, {1,0} };
    static std::mt19937 mt(static_cast<unsigned int>(time(NULL)));
    std::uniform_int_distribution<> dir_dist(0, num_directions - 1);

    int random_dir_index = dir_dist(mt);
    int dx = dir_coords[random_dir_index][0];
    int dy = dir_coords[random_dir_index][1];

    if (dx != 0 || dy != 0)
    {
        int next_map_x = map_x + dx;
        int next_map_y = map_y + dy;

        Player* player = stage->GetPlayer();
        if (player && next_map_x == player->GetMapX() && next_map_y == player->GetMapY())
        {
            player->Heal(-5);
            acted = true;
        }
        else if (!CheckCollision(next_map_x, next_map_y))
        {
            map_x = next_map_x;
            map_y = next_map_y;
            acted = true;
        }
    }
    return acted;
}

void Enemy::Draw()
{
    if (!stage || !stage->IsTileVisible(map_x, map_y) || EnemyGraph == -1) return;

    const float z = stage->GetZoomRate();
    float ds = stage->GetTileSize() * z;

    int lx = (int)(map_x * ds - stage->GetCameraX() * z);
    int ty = (int)(map_y * ds - stage->GetCameraY() * z);

    // 何%にするか (0.8f = 80%)
    const float SIZE_RATE = 0.7f;

    float aspect = 154.0f / 96.0f;
    int drawW = (int)(ds * SIZE_RATE);
    int drawH = (int)(drawW * aspect);

    int offsetX = ((int)ds - drawW) / 2;
    int drawY = ty - (drawH - (int)ds);

    DrawExtendGraph(lx + offsetX, drawY, lx + offsetX + drawW, drawY + drawH, EnemyGraph, TRUE);
}