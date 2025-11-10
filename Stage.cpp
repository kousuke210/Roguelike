#include "DxLib.h" 
#include "Stage.h"
#include <random>
#include <ctime> 

Stage::Stage()
	: mt(static_cast<unsigned int>(time(NULL)))
{
	InitializeMap();
}

Stage::~Stage()
{
}

void Stage::InitializeMap()
{
	for (int y = 0; y < MAP_HEIGHT; ++y)
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			mapData[y][x] = TILE_WALL;
		}
	}
}

void Stage::DrawTile(int x, int y, int type)
{
	int color = GetColor(0, 0, 0); // デフォルト色は黒

	if (type == TILE_WALL)
	{
		// 壁: 明るい灰色
		color = GetColor(150, 150, 150);
	}
	else if (type == TILE_FLOOR)
	{
		// 【最終修正】床: 非常に目立つ青色
		color = GetColor(0, 0, 255);
	}
	else // 【追加】デバッグ用: どちらでもない場合は赤で塗りつぶす
	{
		color = GetColor(255, 0, 0);
	}

	int left = x * TILE_SIZE;
	int top = y * TILE_SIZE;
	int right = (x + 1) * TILE_SIZE;
	int bottom = (y + 1) * TILE_SIZE;

	// タイル全体を塗りつぶし
	DrawBox(left, top, right, bottom, color, TRUE);

	// 【追加】デバッグ用: マス目の境界線を黒で描画（描画が正しく行われているかの視覚化）
	DrawBox(left, top, right, bottom, GetColor(0, 0, 0), FALSE);
}


void Stage::GenerateMap()
{
	InitializeMap(); // 1. すべて壁で初期化
	CreateRooms();   // 2. 部屋の作成
	CreateCorridors(); // 3. 通路の作成
}

void Stage::Draw()
{
	for (int y = 0; y < MAP_HEIGHT; ++y)
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			DrawTile(x, y, mapData[y][x]);
		}
	}
}

void Stage::CreateRooms()
{
	// 部屋の座標とサイズを設定
	Room startRoom;
	startRoom.w = 12; // 【修正】幅を大きく (10 -> 12)
	startRoom.h = 8;  // 【修正】高さを大きく (5 -> 8)
	startRoom.x = MAP_WIDTH / 2 - startRoom.w / 2;  // マップ中央X
	startRoom.y = MAP_HEIGHT / 2 - startRoom.h / 2; // マップ中央Y
	startRoom.center_x = startRoom.x + startRoom.w / 2;
	startRoom.center_y = startRoom.y + startRoom.h / 2;

	rooms.push_back(startRoom);

	// 部屋の内部を床にする
	for (int y = startRoom.y; y < startRoom.y + startRoom.h; ++y)
	{
		for (int x = startRoom.x; x < startRoom.x + startRoom.w; ++x)
		{
			// マップの範囲内であれば床に設定
			if (y >= 0 && y < MAP_HEIGHT && x >= 0 && x < MAP_WIDTH)
			{
				mapData[y][x] = TILE_FLOOR;
			}
		}
	}
}

void Stage::CreateCorridors()
{

}