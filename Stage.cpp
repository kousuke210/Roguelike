#include "DxLib.h" 
#include "Stage.h"
#include <random>
#include <ctime> 
#include <algorithm>
#include "Player.h"
#include <assert.h>
#include <cstring> // memsetのために追加

using std::min;
using std::max;

Stage::Stage()
	: mt(static_cast<unsigned int>(time(NULL)))
{
	GroundImage = LoadGraph("Assets/tutidou.png");
	assert(GroundImage > 0);
	WallImage = LoadGraph("Assets/tutikabe1.png");
	assert(WallImage > 0);

	// 【追加】visibleDataとexploredDataを0で初期化
	memset(exploredData, 0, sizeof(exploredData));
	memset(visibleData, 0, sizeof(visibleData));
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

// 【追加】探索済みフラグをセットする関数
void Stage::SetExplored(int x, int y)
{
	// マップ境界内の場合のみ探索済みフラグを立てる
	if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
	{
		exploredData[y][x] = 1;
	}
}

// 【追加】現在見えているマスを計算する関数 (簡易FoV: 5x5マス)
void Stage::CalculateVisibleTiles(int player_map_x, int player_map_y)
{
	// 毎フレーム、visibleData をリセット
	memset(visibleData, 0, sizeof(visibleData));

	// 視界の範囲 (5x5 マス, sight_range=2)
	const int sight_range = 2;

	// プレイヤー中心の範囲を visible に設定
	for (int dy = -sight_range; dy <= sight_range; ++dy) {
		for (int dx = -sight_range; dx <= sight_range; ++dx) {
			int target_x = player_map_x + dx;
			int target_y = player_map_y + dy;

			if (target_x >= 0 && target_x < MAP_WIDTH && target_y >= 0 && target_y < MAP_HEIGHT)
			{
				visibleData[target_y][target_x] = 1;
			}
		}
	}
}

// 【追加】エンティティが現在見えているか確認するパブリックなゲッター
bool Stage::IsTileVisible(int x, int y) const
{
	// マップ境界内の場合のみチェック
	if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
	{
		return visibleData[y][x] == 1;
	}
	return false;
}

void Stage::DrawTile(int x, int y, int type, int offset_x, int offset_y)
{
	const float zoom_rate = ZOOM_RATE;
	const float draw_size = TILE_SIZE * zoom_rate;

	// 【追加】半透明オーバーレイ用の定数
	const int FOG_ALPHA = 128; // 0 (透明) ～ 255 (不透明)

	// マスのピクセル座標を計算（オフセット適用済み）
	int left = static_cast<int>(x * draw_size - offset_x * zoom_rate);
	int top = static_cast<int>(y * draw_size - offset_y * zoom_rate);
	int right = static_cast<int>((x + 1) * draw_size - offset_x * zoom_rate);
	int bottom = static_cast<int>((y + 1) * draw_size - offset_y * zoom_rate);

	int draw_width = static_cast<int>(draw_size);
	int draw_height = static_cast<int>(draw_size);

	// -----------------------------------------------------------
	// 視界ステータス判定
	// -----------------------------------------------------------
	bool isVisible = IsTileVisible(x, y);
	bool isExplored = (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) && (exploredData[y][x] == 1);

	if (!isExplored)
	{
		// 1. 【未探索】: 真っ黒に塗りつぶして終了
		int color = GetColor(0, 0, 0);
		DrawBox(left, top, right, bottom, color, TRUE);
		return;
	}


	if (type == TILE_FLOOR)
	{
		// 2. 【探索済み床】: 常に明るい画像を描画し、見えていない場合のみ暗くする

		// 2A. 床タイル画像をフルカラーで描画
		int tile_src_x = 0;
		int tile_src_y = 0;
		DrawRectGraph(left, top, tile_src_x, tile_src_y, draw_width, draw_height, GroundImage, TRUE, FALSE);

		if (!isVisible)
		{
			// 2B. 【探索済みだが暗い床】: 画像の上に半透明の黒を重ねる
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, FOG_ALPHA);
			DrawBox(left, top, right, bottom, GetColor(0, 0, 0), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
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
			// 3. 【探索済み壁】: 常に明るい画像を描画し、見えていない場合のみ暗くする

			// 3A. 壁タイル画像をフルカラーで描画
			DrawExtendGraph(left, top, right, bottom, WallImage, TRUE);

			if (!isVisible)
			{
				// 3B. 【探索済みだが暗い壁】: 画像の上に半透明の黒を重ねる
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, FOG_ALPHA);
				DrawBox(left, top, right, bottom, GetColor(0, 0, 0), TRUE);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
		}
		else
		{
			// 4. 【隣接していない壁】: 真っ黒 (探索済みでも描画しない)
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

void Stage::Draw()
{
	for (int y = 0; y < MAP_HEIGHT; ++y)
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			DrawTile(x, y, mapData[y][x], camera_x, camera_y);
		}
	}
}

void Stage::DrawOverlayMap(int screen_width, int screen_height)
{
	// ミニマップの設定
	const int MAP_SCALE = 8; // 1タイルあたりのピクセルサイズ (縮小率)
	const int MAP_DRAW_W = MAP_WIDTH * MAP_SCALE;
	const int MAP_DRAW_H = MAP_HEIGHT * MAP_SCALE;

	// マップ全体の描画開始座標 (画面中央に配置)
	int start_x = (screen_width - MAP_DRAW_W) / 2;
	int start_y = (screen_height - MAP_DRAW_H) / 2;

	// 1. 背景を少し暗くするオーバーレイ (画面全体を半透明の黒で覆う)
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180); // 180は透過度 (0:透明, 255:不透明)
	DrawBox(0, 0, screen_width, screen_height, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ブレンドモードを元に戻す

	// 2. ミニマップ本体の描画
	for (int y = 0; y < MAP_HEIGHT; ++y)
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			if (exploredData[y][x] == 1) // 探索済みのみ描画
			{
				int draw_x = start_x + x * MAP_SCALE;
				int draw_y = start_y + y * MAP_SCALE;
				int color = GetColor(0, 0, 0); // デフォルトは黒

				if (mapData[y][x] == TILE_FLOOR)
				{
					// 探索済みの床/通路: 青色で表示
					color = GetColor(50, 50, 200);
				}
				else if (mapData[y][x] == TILE_WALL)
				{
					// 探索済みの壁: 灰色で表示
					color = GetColor(100, 100, 100);
				}

				DrawBox(draw_x, draw_y, draw_x + MAP_SCALE, draw_y + MAP_SCALE, color, TRUE);
			}
		}
	}

	// 3. プレイヤーの現在位置の描画
	if (player)
	{
		int player_x = start_x + player->GetMapX() * MAP_SCALE;
		int player_y = start_y + player->GetMapY() * MAP_SCALE;
		int player_color = GetColor(255, 255, 255); // 白

		// プレイヤーを少し目立つサイズで描画
		DrawBox(player_x - 1, player_y - 1, player_x + MAP_SCALE + 1, player_y + MAP_SCALE + 1, player_color, TRUE);
	}

	// 4. デバッグ情報の表示
	DrawFormatString(start_x, start_y - 30, GetColor(255, 255, 255), "MINI MAP (Press TAB to close)");
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
	const float zoom_rate = ZOOM_RATE;

	// 画面サイズをタイル単位で計算 (表示されるタイル数)
	const int DRAW_TILE_SIZE = static_cast<int>(TILE_SIZE * zoom_rate);
	const int SCREEN_TILE_W = SCREEN_WIDTH / DRAW_TILE_SIZE;
	const int SCREEN_TILE_H = SCREEN_HEIGHT / DRAW_TILE_SIZE;

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

	// 画面サイズを TILE_SIZE=50 単位のピクセルに変換
	const int SCREEN_WIDTH_UNZOOMED = static_cast<int>(SCREEN_WIDTH / zoom_rate);
	const int SCREEN_HEIGHT_UNZOOMED = static_cast<int>(SCREEN_HEIGHT / zoom_rate);


	// 4. マップの境界でカメラをクランプ
	if (target_pixel_x > MAP_PIXEL_WIDTH - SCREEN_WIDTH_UNZOOMED) target_pixel_x = MAP_PIXEL_WIDTH - SCREEN_WIDTH_UNZOOMED;
	if (target_pixel_x < 0) target_pixel_x = 0;

	if (target_pixel_y > MAP_PIXEL_HEIGHT - SCREEN_HEIGHT_UNZOOMED) target_pixel_y = MAP_PIXEL_HEIGHT - SCREEN_HEIGHT_UNZOOMED;
	if (target_pixel_y < 0) target_pixel_y = 0;

	// 5. カメラを更新
	camera_x = target_pixel_x;
	camera_y = target_pixel_y;

	// 【修正】現在の可視範囲を計算
	CalculateVisibleTiles(player_map_x, player_map_y);

	// 【修正】visibleなタイルすべてを explored に設定する
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			if (visibleData[y][x] == 1) {
				SetExplored(x, y);
			}
		}
	}
}