#include "DxLib.h"
#include "Enemy.h"
#include "Player.h"
#include <cmath>
#include "Stage.h"
#include <random>
#include <ctime>

Enemy::Enemy()
    :
    map_x(0),
    map_y(0)
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
        return true; // 衝突あり
    }

    // 【追加】プレイヤーとの衝突判定
    if (stage->GetPlayer() != nullptr) 
    {
        if (next_map_x == stage->GetPlayer()->GetMapX() && next_map_y == stage->GetPlayer()->GetMapY()) {
            return true; // プレイヤーがいるマスへは移動しない（衝突あり）
        }
    }

    return false; // 衝突なし
}

bool Enemy::Update()
{
    // 移動ロジック（既存のランダム移動）
    bool acted = false;
    const int num_directions = 5;
    const int dir_coords[num_directions][2] = { {0,0}, {0,-1}, {0,1}, {-1,0}, {1,0} };
    static std::mt19937 mt(static_cast<unsigned int>(time(NULL)));
    std::uniform_int_distribution<> dir_dist(0, num_directions - 1);

    int random_dir_index = dir_dist(mt);
    int dx = dir_coords[random_dir_index][0];
    int dy = dir_coords[random_dir_index][1];

    if (dx != 0 || dy != 0) {
        int next_map_x = map_x + dx;
        int next_map_y = map_y + dy;
        if (!CheckCollision(next_map_x, next_map_y)) {
            map_x = next_map_x;
            map_y = next_map_y;
        }
    }
    return true;
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

    float aspect = 154.0f / 96.0f; // 元画像の比率 (縦/横)
    int drawW = (int)(ds * SIZE_RATE); // 縮小後の横幅
    int drawH = (int)(drawW * aspect); // 横幅に比率を掛けた高さ

    // キャラクターがタイルの左右中央に来るようにずらす
    int offsetX = ((int)ds - drawW) / 2;

    // 足元がマスの底辺に合うように調整
    int drawY = ty - (drawH - (int)ds);

    DrawExtendGraph(lx + offsetX, drawY, lx + offsetX + drawW, drawY + drawH, EnemyGraph, TRUE);
}