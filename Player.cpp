#include "DxLib.h"
#include "Player.h"
#include <cmath>
#include "Stage.h"
#include "Input.h"

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

	if (Input::IsKeyDown(KEY_INPUT_W)) {
		dy = -1;
	}
	else if (Input::IsKeyDown(KEY_INPUT_S)) {
		dy = 1;
	}
	else if (Input::IsKeyDown(KEY_INPUT_A)) {
		dx = -1;
	}
	else if (Input::IsKeyDown(KEY_INPUT_D)) {
		dx = 1;
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

		return true; // 移動の可否に関わらず、キー入力があったらターン終了
	}

	return false; // キー入力なし
}

void Player::Draw()
{
	if (!stage) return;

	int tileSize = stage->GetTileSize();

	// 【変更点：カメラオフセットを取得】
	int offset_x = stage->GetCameraX();
	int offset_y = stage->GetCameraY();

	// 【変更点：描画座標にオフセットを適用】
	float center_x = (float)(map_x * tileSize + tileSize / 2.0f) - offset_x;
	float center_y = (float)(map_y * tileSize + tileSize / 2.0f) - offset_y;

	int color = GetColor(255, 255, 255);

	DrawCircle((int)center_x, (int)center_y, tileSize / 2 - 5, color, TRUE);

	// デバッグ表示は画面固定でオフセット不要
	DrawFormatString(10, 10, GetColor(255, 255, 255), "Map Position: (%d, %d)", map_x, map_y);
}