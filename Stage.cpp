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
	// 【追加】ズーム率を取得
	const float zoom_rate = ZOOM_RATE;
	const float draw_size = TILE_SIZE * zoom_rate;

	// マスのピクセル座標を計算（オフセット適用済み）
	// 【修正】座標とサイズにズーム率を適用
	int left = static_cast<int>(x * draw_size - offset_x * zoom_rate);
	int top = static_cast<int>(y * draw_size - offset_y * zoom_rate);
	int right = static_cast<int>((x + 1) * draw_size - offset_x * zoom_rate);
	int bottom = static_cast<int>((y + 1) * draw_size - offset_y * zoom_rate);

	int draw_width = static_cast<int>(draw_size);
	int draw_height = static_cast<int>(draw_size);


	if (type == TILE_FLOOR)
	{
		// 床（TILE_FLOOR）は GroundImage の左上 50x50 を切り出し描画
		int tile_src_x = 0;
		int tile_src_y = 0;

		// 【修正】拡大描画
		DrawRectGraph(left, top, tile_src_x, tile_src_y, draw_width, draw_height, GroundImage, TRUE, FALSE);
	}
	else if (type == TILE_WALL)
	{
		// 壁タイルの描画判定ロジック
		bool is_adjacent_to_floor = false;

		// 上下左右と斜め方向のチェック
		if (GetTileType(x, y - 1) == TILE_FLOOR) is_adjacent_to_floor = true;
		if (GetTileType(x, y + 1) == TILE_FLOOR) is_adjacent_to_floor = true;
		if (GetTileType(x - 1, y) == TILE_FLOOR) is_adjacent_to_floor = true;
		if (GetTileType(x + 1, y) == TILE_FLOOR) is_adjacent_to_floor = true;

		if (!is_adjacent_to_floor)
		{
			if (GetTileType(x - 1, y - 1) == TILE_FLOOR) is_adjacent_to_floor = true;
			if (GetTileType(x + 1, y - 1) == TILE_FLOOR) is_adjacent_to_floor = true;
			if (GetTileType(x - 1, y + 1) == TILE_FLOOR) is_adjacent_to_floor = true;
			if (GetTileType(x + 1, y + 1) == TILE_FLOOR) is_adjacent_to_floor = true;
		}


		if (is_adjacent_to_floor)
		{
			// 【修正】拡大描画: DrawExtendGraph を使用
			DrawExtendGraph(left, top, right, bottom, WallImage, TRUE);
		}
		else
		{
			// 真っ黒に塗りつぶす
			int color = GetColor(0, 0, 0);
			DrawBox(left, top, right, bottom, color, TRUE);
		}
	}
	else
	{
		// その他のタイルは真っ黒に塗りつぶす
		int color = GetColor(0, 0, 0);
		DrawBox(left, top, right, bottom, color, TRUE);
	}
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
	const float zoom_rate = ZOOM_RATE; // 【追加】ズーム率を取得

	// 画面サイズをタイル単位で計算 (表示されるタイル数)
	const int DRAW_TILE_SIZE = static_cast<int>(TILE_SIZE * zoom_rate);
	const int SCREEN_TILE_W = SCREEN_WIDTH / DRAW_TILE_SIZE; // 1600 / 100 = 16
	const int SCREEN_TILE_H = SCREEN_HEIGHT / DRAW_TILE_SIZE; // 900 / 100 = 9

	// 1. プレイヤーが現在いる画面エリア（スクリーン）のインデックスを計算
	int screen_index_x = player_map_x / SCREEN_TILE_W;
	int screen_index_y = player_map_y / SCREEN_TILE_H;

	// 2. カメラの目標マス座標（その画面エリアの左上のマス座標）を計算
	int target_map_x = screen_index_x * SCREEN_TILE_W;
	int target_map_y = screen_index_y * SCREEN_TILE_H;

	// 3. マス座標をピクセル座標に変換 (カメラオフセットは TILE_SIZE=50 単位で保持)
	int target_pixel_x = target_map_x * TILE_SIZE;
	int target_pixel_y = target_map_y * TILE_SIZE;

	// マップのピクセルサイズ (TILE_SIZE=50 単位)
	const int MAP_PIXEL_WIDTH = MAP_WIDTH * TILE_SIZE;
	const int MAP_PIXEL_HEIGHT = MAP_HEIGHT * TILE_SIZE;

	// 【修正】画面サイズを TILE_SIZE=50 単位のピクセルに変換
	const int SCREEN_WIDTH_UNZOOMED = static_cast<int>(SCREEN_WIDTH / zoom_rate); // 800
	const int SCREEN_HEIGHT_UNZOOMED = static_cast<int>(SCREEN_HEIGHT / zoom_rate); // 450


	// 4. マップの境界でカメラをクランプ
	// カメラオフセットの最大値は (マップ全体のピクセルサイズ - 画面サイズ[非ズーム])
	if (target_pixel_x > MAP_PIXEL_WIDTH - SCREEN_WIDTH_UNZOOMED) target_pixel_x = MAP_PIXEL_WIDTH - SCREEN_WIDTH_UNZOOMED;
	if (target_pixel_x < 0) target_pixel_x = 0;

	if (target_pixel_y > MAP_PIXEL_HEIGHT - SCREEN_HEIGHT_UNZOOMED) target_pixel_y = MAP_PIXEL_HEIGHT - SCREEN_HEIGHT_UNZOOMED;
	if (target_pixel_y < 0) target_pixel_y = 0;

	// 5. カメラを更新
	camera_x = target_pixel_x;
	camera_y = target_pixel_y;
}