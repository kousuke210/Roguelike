#include "DxLib.h" 
#include "Stage.h"
#include <random>
#include <ctime> 
#include <algorithm>

Stage::Stage()
	: mt(static_cast<unsigned int>(time(NULL)))
{
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
	InitializeMap(); 
	rooms.clear();   
	CreateRooms();   
	CreateCorridors(); 
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

int Stage::GetTileType(int x, int y) const
{
	// マップの境界外であれば、常に「壁」と見なす
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
	{
		return TILE_WALL;
	}

	return mapData[y][x];
}

void Stage::CreateRooms()
{
	// 【変更点: 複数の部屋をランダムに生成するロジックに置き換え】

	// 部屋のサイズと座標のランダム生成範囲を設定
	std::uniform_int_distribution<> size_dist(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
	// マップの端から1マス内側に部屋を作るように調整
	std::uniform_int_distribution<> x_dist(1, MAP_WIDTH - 2);
	std::uniform_int_distribution<> y_dist(1, MAP_HEIGHT - 2);

	for (int i = 0; i < MAX_ROOMS; ++i)
	{
		// 新しい部屋のサイズと座標をランダムに決定
		Room newRoom;
		// 奇数サイズの部屋にすることで中心座標を明確にし、通路が引きやすくなる
		newRoom.w = size_dist(mt) * 2 + 1;
		newRoom.h = size_dist(mt) * 2 + 1;

		// 部屋の左上座標を決定
		newRoom.x = x_dist(mt);
		newRoom.y = y_dist(mt);

		// 部屋がマップの境界からはみ出ないように調整
		if (newRoom.x + newRoom.w >= MAP_WIDTH - 1) newRoom.x = MAP_WIDTH - 1 - newRoom.w;
		if (newRoom.y + newRoom.h >= MAP_HEIGHT - 1) newRoom.y = MAP_HEIGHT - 1 - newRoom.h;

		// 中心座標を計算
		newRoom.center_x = newRoom.x + newRoom.w / 2;
		newRoom.center_y = newRoom.y + newRoom.h / 2;

		bool intersects = false;
		// 既存の部屋と重なっていないかチェック
		for (const auto& existingRoom : rooms)
		{
			// 重なり判定 (部屋間に1マス分の壁を確保するため、境界+1マスでチェック)
			if (newRoom.x - 1 < existingRoom.x + existingRoom.w &&
				newRoom.x + newRoom.w + 1 > existingRoom.x &&
				newRoom.y - 1 < existingRoom.y + existingRoom.h &&
				newRoom.y + newRoom.h + 1 > existingRoom.y)
			{
				intersects = true;
				break;
			}
		}

		if (!intersects)
		{
			// 重なっていない場合のみ部屋を確定し、リストに追加
			rooms.push_back(newRoom);

			// マップの内部を床にする (newRoom.x, newRoom.y を含む)
			for (int y = newRoom.y; y < newRoom.y + newRoom.h; ++y)
			{
				for (int x = newRoom.x; x < newRoom.x + newRoom.w; ++x)
				{
					mapData[y][x] = TILE_FLOOR;
				}
			}
		}
	}
}

void Stage::CreateCorridors()
{
	// 【追加】部屋が2つ以上ある場合に通路を作成
	if (rooms.size() < 2) return;

	// 隣接する部屋同士を順番に繋いでいく
	for (size_t i = 0; i < rooms.size() - 1; ++i)
	{
		Room& r1 = rooms[i];
		Room& r2 = rooms[i + 1];

		// 2つの部屋の中心座標（マス座標）
		int x1 = r1.center_x;
		int y1 = r1.center_y;
		int x2 = r2.center_x;
		int y2 = r2.center_y;

		// 1. 水平方向(X)の通路を作成 (x1 から x2 へ移動し、Y1の行を通路にする)
		for (int x = min(x1, x2); x <= max(x1, x2); ++x)
		{
			// マップの境界チェック
			if (y1 >= 0 && y1 < MAP_HEIGHT && x >= 0 && x < MAP_WIDTH)
			{
				mapData[y1][x] = TILE_FLOOR;
			}
		}

		// 2. 垂直方向(Y)の通路を作成 (y1 から y2 へ移動し、X2の列を通路にする)
		for (int y = min(y1, y2); y <= max(y1, y2); ++y)
		{
			// マップの境界チェック
			if (y >= 0 && y < MAP_HEIGHT && x2 >= 0 && x2 < MAP_WIDTH)
			{
				mapData[y][x2] = TILE_FLOOR;
			}
		}
	}
}