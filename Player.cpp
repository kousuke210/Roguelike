#include "DxLib.h"
#include "Player.h"
#include "Stage.h"
#include "Input.h"
#include <cmath>

Player::Player()
	: map_x(0), map_y(0), hp(35), maxHP(35), attack(5), stage(nullptr)
{
	PlayerGraph = LoadGraph("Assets/Chara_1.png");
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
	if (stage->GetTileType(next_map_x, next_map_y) == TILE_WALL) return true;
	return false;
}

bool Player::Update() {
	int dx = 0, dy = 0;
	if (Input::IsKeyDown(KEY_INPUT_W))
	{
		dy = -1;
	}
	else if (Input::IsKeyDown(KEY_INPUT_S))
	{
		dy = 1;
	}
	else if (Input::IsKeyDown(KEY_INPUT_A))
	{
		dx = -1;
	}
	else if (Input::IsKeyDown(KEY_INPUT_D))
	{
		dx = 1;
	}

	if (Input::IsKeyDown(KEY_INPUT_Q))
	{

	}

	if (dx != 0 || dy != 0) {
		int next_x = map_x + dx;
		int next_y = map_y + dy;
		if (!CheckCollision(next_x, next_y)) 
		{
			map_x = next_x;
			map_y = next_y;
		}
		return true;
	}
	return false;
}

void Player::Draw() 
{
	if (!stage || !stage->IsTileVisible(map_x, map_y) || PlayerGraph == -1) return;

	const float z = stage->GetZoomRate();
	float ds = stage->GetTileSize() * z;

	int lx = (int)(map_x * ds - stage->GetCameraX() * z);
	int ty = (int)(map_y * ds - stage->GetCameraY() * z);

	// ƒ^ƒCƒ‹‚Ì•‚Ì‰½%‚É‚·‚é‚© (0.8f = 80%)
	const float SIZE_RATE = 0.75f;
	float aspect = 163.0f / 109.0f;
	int drawW = (int)(ds * SIZE_RATE);
	int drawH = (int)(drawW * aspect);
	int offsetX = ((int)ds - drawW) / 2;
	int drawY = ty - (drawH - (int)ds);

	DrawExtendGraph(lx + offsetX, drawY, lx + offsetX + drawW, drawY + drawH, PlayerGraph, TRUE);
}