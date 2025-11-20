#include "DxLib.h"
#include "Player.h"
#include <cmath>
#include "Stage.h"

Player::Player()
	:
	map_x(0),
	map_y(0)
{
}

Player::~Player()
{
}

void Player::SetPosition(int map_x, int map_y)
{
	this->map_x = map_x;
	this->map_y = map_y;
}

bool Player::CheckCollision(int next_map_x, int next_map_y)
{
	if (!stage) return true; 

	// 移動先のマスが壁（TILE_WALL）であれば衝突
	if (stage->GetTileType(next_map_x, next_map_y) == TILE_WALL)
	{
		return true; // 衝突あり
	}

	return false; // 衝突なし
}

bool Player::Update()
{
	int dx = 0;
	int dy = 0;

	if (CheckHitKey(KEY_INPUT_W) == 1) {
		dy = -1;
	}
	else if (CheckHitKey(KEY_INPUT_S) == 1) {
		dy = 1;
	}

	if (CheckHitKey(KEY_INPUT_W) == 1){
		dx = -1;
	}
	else if (CheckHitKey(KEY_INPUT_D) == 1) {
		dx = 1;
	}


	if (dx != 0 && dy != 0) {
		if (CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_S)) {
			dx = 0;
		}
		else {
			dy = 0;
		}
	}

	if (dx != 0 || dy != 0)
	{
		int next_map_x = map_x + dx;
		int next_map_y = map_y + dy;

		// 衝突判定
		if (!CheckCollision(next_map_x, next_map_y))
		{
			// 移動実行
			map_x = next_map_x;
			map_y = next_map_y;
		}

		return true;
	}

	return false;
}

void Player::Draw()
{
	if (!stage) return;

	int tileSize = stage->GetTileSize();

	// マス座標からピクセル座標の中心を計算
	float center_x = (float)(map_x * tileSize + tileSize / 2.0f);
	float center_y = (float)(map_y * tileSize + tileSize / 2.0f);

	int color = GetColor(255, 255, 255);

	// DrawCircleAAの代わりにDrawCircleで描画（ローグライクではDrawCircleで十分です）
	// タイルサイズに合わせて半径を調整（tileSize / 2 - 5）
	DrawCircle((int)center_x, (int)center_y, tileSize / 2 - 5, color, TRUE);

	DrawFormatString(10, 10, GetColor(255, 255, 255), "Map Position: (%d, %d)", map_x, map_y);
}