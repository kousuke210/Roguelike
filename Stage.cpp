#include "DxLib.h" 
#include "Stage.h"
#include <random>
#include <ctime> 
#include <algorithm>
#include "Player.h"
#include <assert.h>

using std::min;
using std::max;

Stage::Stage()
	: mt(static_cast<unsigned int>(time(NULL)))
{
	GroundImage = LoadGraph("Assets/tutidou.png");
	assert(GroundImage > 0);
	WallImage = LoadGraph("Assets/tutikabe1.png");
	assert(WallImage > 0);
}

Stage::~Stage()
{
	if (GroundImage != -1)
	{
		DeleteGraph(GroundImage);
	}
	if (WallImage != -1)
	{
		DeleteGraph(WallImage);
	}
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

void Stage::DrawTile(int x, int y, int type, int offset_x, int offset_y)
{
	// マスのピクセル座標を計算（オフセット適用済み）
	int left = x * TILE_SIZE - offset_x;
	int top = y * TILE_SIZE - offset_y;
	int right = (x + 1) * TILE_SIZE - offset_x;
	int bottom = (y + 1) * TILE_SIZE - offset_y;

	if (type == TILE_FLOOR)
	{
		// 床（TILE_FLOOR）は常に画像タイルで描画
		int tile_src_x = 0;
		int tile_src_y = 0;

		DrawRectGraph(left, top, tile_src_x, tile_src_y, TILE_SIZE, TILE_SIZE, GroundImage, TRUE, FALSE);
	}
	else if (type == TILE_WALL)
	{
		// 【修正】壁タイルの描画判定ロジック

		bool is_adjacent_to_floor = false;

		// 上下左右の4方向をチェック
		// TILE_FLOORが隣接しているか確認する
		if (GetTileType(x, y - 1) == TILE_FLOOR) is_adjacent_to_floor = true; // 上
		if (GetTileType(x, y + 1) == TILE_FLOOR) is_adjacent_to_floor = true; // 下
		if (GetTileType(x - 1, y) == TILE_FLOOR) is_adjacent_to_floor = true; // 左
		if (GetTileType(x + 1, y) == TILE_FLOOR) is_adjacent_to_floor = true; // 右

		if (!is_adjacent_to_floor)
		{
			if (GetTileType(x - 1, y - 1) == TILE_FLOOR) is_adjacent_to_floor = true; // 左上
			if (GetTileType(x + 1, y - 1) == TILE_FLOOR) is_adjacent_to_floor = true; // 右上
			if (GetTileType(x - 1, y + 1) == TILE_FLOOR) is_adjacent_to_floor = true; // 左下
			if (GetTileType(x + 1, y + 1) == TILE_FLOOR) is_adjacent_to_floor = true; // 右下
		}


		if (is_adjacent_to_floor)
		{
			// 床に隣接している壁は画像で描画（通路に面している壁、部屋の角など）
			DrawGraph(left, top, WallImage, TRUE);
		}
		else
		{
			int color = GetColor(0, 0, 0);
			DrawBox(left, top, right, bottom, color, TRUE);
		}
	}
	else
	{
		int color = GetColor(0, 0, 0);
		DrawBox(left, top, right, bottom, color, TRUE);
	}

	// マス目の境界線の描画
	//DrawBox(left, top, right, bottom, GetColor(0, 0, 0), FALSE);
}

void Stage::GenerateMap()
{
	InitializeMap();
	rooms.clear();
	CreateRooms();
	CreateCorridors();
}

// 【変更】Draw関数内でカメラオフセットを取得し、DrawTileに渡す
void Stage::Draw()
{
	for (int y = 0; y < MAP_HEIGHT; ++y)
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			// 【変更点：カメラオフセットを渡す】
			DrawTile(x, y, mapData[y][x], camera_x, camera_y);
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
	// 部屋が2つ以上ある場合に通路を作成
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

void Stage::UpdateCamera(int player_map_x, int player_map_y)
{
	// 画面定数 (1600x900)
	const int SCREEN_WIDTH = 1600;
	const int SCREEN_HEIGHT = 900;

	// 画面サイズをタイル単位で計算
	// (1600 / 50 = 32マス, 900 / 50 = 18マス)
	const int SCREEN_TILE_W = SCREEN_WIDTH / TILE_SIZE;
	const int SCREEN_TILE_H = SCREEN_HEIGHT / TILE_SIZE;

	// 1. プレイヤーが現在いる画面エリア（スクリーン）のインデックスを計算
	// 例: プレイヤーがX=32に移動すると、screen_index_xは0から1に変化する
	int screen_index_x = player_map_x / SCREEN_TILE_W;
	int screen_index_y = player_map_y / SCREEN_TILE_H;

	// 2. カメラの目標マス座標（その画面エリアの左上のマス座標）を計算
	int target_map_x = screen_index_x * SCREEN_TILE_W;
	int target_map_y = screen_index_y * SCREEN_TILE_H;

	// 3. マス座標をピクセル座標に変換
	int target_pixel_x = target_map_x * TILE_SIZE;
	int target_pixel_y = target_map_y * TILE_SIZE;

	// マップのピクセルサイズ
	const int MAP_PIXEL_WIDTH = MAP_WIDTH * TILE_SIZE;
	const int MAP_PIXEL_HEIGHT = MAP_HEIGHT * TILE_SIZE;

	// 4. マップの境界でカメラをクランプ
	// カメラオフセットの最大値は (マップ全体のピクセルサイズ - 画面サイズ)
	if (target_pixel_x > MAP_PIXEL_WIDTH - SCREEN_WIDTH) target_pixel_x = MAP_PIXEL_WIDTH - SCREEN_WIDTH;
	if (target_pixel_x < 0) target_pixel_x = 0;

	if (target_pixel_y > MAP_PIXEL_HEIGHT - SCREEN_HEIGHT) target_pixel_y = MAP_PIXEL_HEIGHT - SCREEN_HEIGHT;
	if (target_pixel_y < 0) target_pixel_y = 0;

	// 5. カメラを更新
	camera_x = target_pixel_x;
	camera_y = target_pixel_y;
}