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
}

Enemy::~Enemy()
{
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
    if (stage->GetPlayer() != nullptr) {
        if (next_map_x == stage->GetPlayer()->GetMapX() && next_map_y == stage->GetPlayer()->GetMapY()) {
            return true; // プレイヤーがいるマスへは移動しない（衝突あり）
        }
    }

    return false; // 衝突なし
}

bool Enemy::Update()
{
    // 行動（移動）を実行したかどうか
    bool acted = false;

    // 【ターンごとのランダム移動のロジック】

    // 移動方向の配列 {dx, dy} (停止、上下左右の5方向)
    const int num_directions = 5;
    const int dir_coords[num_directions][2] = {
        {0, 0}, // 停止
        {0, -1}, // 上
        {0, 1},  // 下
        {-1, 0}, // 左
        {1, 0}   // 右
    };

    // 乱数エンジンと分布
    // 静的な乱数エンジンを使用し、Updateが呼び出されるたびに異なる結果を得る
    static std::mt19937 mt(static_cast<unsigned int>(time(NULL)));
    std::uniform_int_distribution<> dir_dist(0, num_directions - 1);

    // ランダムな移動方向を選択し、移動可能であれば実行
    // (ここでは、ランダムに1つの方向を選択して、移動を試みる)
    int random_dir_index = dir_dist(mt);

    int dx = dir_coords[random_dir_index][0];
    int dy = dir_coords[random_dir_index][1];

    if (dx != 0 || dy != 0) // 停止 (0,0) 以外の場合
    {
        int next_map_x = map_x + dx;
        int next_map_y = map_y + dy;

        // 衝突判定
        if (!CheckCollision(next_map_x, next_map_y))
        {
            // 移動実行
            map_x = next_map_x;
            map_y = next_map_y;
            acted = true;
        }
        else
        {
            // 移動できなかった場合でも、行動を試みたのでターンを終了
            acted = true;
        }
    }
    else // 停止 (0,0) を選んだ場合も行動完了とみなす
    {
        acted = true;
    }

    return acted; // 行動を実行したかどうかを返す
}

void Enemy::Draw()
{
    if (!stage) return;

    int tileSize = stage->GetTileSize();

    float center_x = (float)(map_x * tileSize + tileSize / 2.0f);
    float center_y = (float)(map_y * tileSize + tileSize / 2.0f);

    // 敵は赤色で描画
    int color = GetColor(255, 0, 0);

    // プレイヤーと同様に円で描画
    DrawCircle((int)center_x, (int)center_y, tileSize / 2 - 5, color, TRUE);

    // デバッグ表示をプレイヤーと被らないように調整
    DrawFormatString(10, 30, GetColor(255, 255, 255), "Enemy Position: (%d, %d)", map_x, map_y);
}