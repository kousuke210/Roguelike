#include "DxLib.h" 
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
#include "Golem.h"
#include <random>
#include <ctime> 
#include <algorithm>
#include <assert.h>

using std::min;
using std::max;

Stage::Stage() : mt(static_cast<unsigned int>(time(NULL))) 
{
	GroundImage = LoadGraph("Assets/tutidou.png");
	WallImage = LoadGraph("Assets/tutikabe1.png");
	StairImage = LoadGraph("Assets/STAIR.png");
	assert(GroundImage > 0 && WallImage > 0 && StairImage > 0);
	memset(exploredData, 0, sizeof(exploredData));
	memset(visibleData, 0, sizeof(visibleData));
	itemManager = new ItemManager();
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
	if (itemManager != nullptr)
	{
		delete itemManager;
		itemManager = nullptr;
	}
}

void Stage::InitializeMap() 
{
	for (int y = 0; y < MAP_HEIGHT; ++y)
		for (int x = 0; x < MAP_WIDTH; ++x) mapData[y][x] = TILE_WALL;
}

void Stage::SetExplored(int x, int y) 
{
	if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) exploredData[y][x] = 1;
}

void Stage::CalculateVisibleTiles(int player_map_x, int player_map_y) 
{
	memset(visibleData, 0, sizeof(visibleData));

	// 基本の視界を 2 にして、潜るほど狭くなる
	int baseRange = (player && player->torchTurn > 0) ? 5 : 2;

	// 例：10階以降は、たいまつ無しだと視界が 1 になる
	if (currentFloor >= 10 && baseRange == 2) baseRange = 1;

	int sight_range = baseRange;

	for (int dy = -sight_range; dy <= sight_range; ++dy) 
	{

		for (int dx = -sight_range; dx <= sight_range; ++dx) 
		{
			int tx = player_map_x + dx; int ty = player_map_y + dy;
			if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT) visibleData[ty][tx] = 1;
		}
	}
}

bool Stage::IsTileVisible(int x, int y) const 
{
	if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) return visibleData[y][x] == 1;
	return false;
}

void Stage::DrawTile(int x, int y, int type, int offset_x, int offset_y)
{
	const float z = ZOOM_RATE;
	const float ds = TILE_SIZE * z;

	// 画面上の描画範囲（l, t, r, b）
	int l = (int)(x * ds - offset_x * z);
	int t = (int)(y * ds - offset_y * z);
	int r = (int)((x + 1) * ds - offset_x * z);
	int b = (int)((y + 1) * ds - offset_y * z);

	bool isVis = IsTileVisible(x, y);
	bool isExp = (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) && (exploredData[y][x] == 1);

	// 床と階段の描画
	if (type == TILE_FLOOR || type == TILE_STAIRS)
	{
		DrawRectExtendGraph(l, t, r, b, 0, 0, 64, 64, GroundImage, TRUE);

		// 階段の場合はその上に重ねる
		if (type == TILE_STAIRS)
		{
			DrawExtendGraph(l, t, r, b, StairImage, TRUE);
		}
	}
	else if (type == TILE_WALL)
	{
		// 壁の描画
		bool adj = false;
		if (GetTileType(x, y - 1) == TILE_FLOOR || GetTileType(x, y + 1) == TILE_FLOOR ||
			GetTileType(x - 1, y) == TILE_FLOOR || GetTileType(x + 1, y) == TILE_FLOOR ||
			GetTileType(x - 1, y - 1) == TILE_FLOOR || GetTileType(x + 1, y - 1) == TILE_FLOOR ||
			GetTileType(x - 1, y + 1) == TILE_FLOOR || GetTileType(x + 1, y + 1) == TILE_FLOOR ||
			GetTileType(x, y - 1) == TILE_STAIRS || GetTileType(x, y + 1) == TILE_STAIRS) adj = true;

		if (adj) DrawExtendGraph(l, t, r, b, WallImage, TRUE);
		else DrawBox(l, t, r, b, GetColor(0, 0, 0), TRUE);
	}


	// 視界の処理
	if (!isVis)
	{
		if (currentFloor % 5 != 0 && !IsTileVisible(x, y))
		{
			// 探索済みなら少し暗く(140)、未探索ならもっと暗く(210)
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, (exploredData[y][x]) ? 140 : 210);
			DrawBox(l, t, r, b, GetColor(0, 0, 0), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
}

void Stage::UpdateCamera(int px, int py) 
{

	const int SW = 1400;
	const int SH = 700;
	const int DTS = (int)(TILE_SIZE * ZOOM_RATE);
	const int STW = SW / DTS;
	const int STH = SH / DTS;

	int tx = (px / STW) * TILE_SIZE * STW;
	int ty = (py / STH) * TILE_SIZE * STH;

	int MWP = MAP_WIDTH * TILE_SIZE;
	int MHP = MAP_HEIGHT * TILE_SIZE;

	int SWU = (int)(SW / ZOOM_RATE);
	int SHU = (int)(SH / ZOOM_RATE);

	camera_x = max(0, min(tx, MWP - SWU));
	camera_y = max(0, min(ty, MHP - SHU));

	CalculateVisibleTiles(px, py);
	for (int y = 0; y < MAP_HEIGHT; ++y)
		for (int x = 0; x < MAP_WIDTH; ++x) if (visibleData[y][x]) SetExplored(x, y);
}

void Stage::DrawOverlayMap(int sw, int sh) 
{
	const int SCALE = 8;
	int sx = (sw - MAP_WIDTH * SCALE) / 2;
	int sy = (sh - MAP_HEIGHT * SCALE) / 2;

	for (int y = 0; y < MAP_HEIGHT; ++y) 
	{
		for (int x = 0; x < MAP_WIDTH; ++x) 
		{
			// 探索済みの床や壁を描画
			if (exploredData[y][x]) 
			{
				int c = (mapData[y][x] == TILE_FLOOR) ? GetColor(50, 50, 200) : GetColor(100, 100, 100);
				DrawBox(sx + x * SCALE, sy + y * SCALE, sx + (x + 1) * SCALE, sy + (y + 1) * SCALE, c, TRUE);

				if (itemMapData[y][x] == 1) // ATKポーション(赤)
				{
					DrawBox(sx + x * SCALE + 1, sy + y * SCALE + 1, sx + (x + 1) * SCALE - 1, sy + (y + 1) * SCALE - 1, GetColor(255, 0, 0), TRUE);
				}
				else if (itemMapData[y][x] == 2)  // HEALポーション(緑)
				{
					DrawBox(sx + x * SCALE + 1, sy + y * SCALE + 1, sx + (x + 1) * SCALE - 1, sy + (y + 1) * SCALE - 1, GetColor(0, 255, 0), TRUE);
				}
			}
		}
	}
	if (player) DrawBox(sx + player->GetMapX() * SCALE - 1, sy + player->GetMapY() * SCALE - 1, sx + (player->GetMapX() + 1) * SCALE + 1, sy + (player->GetMapY() + 1) * SCALE + 1, GetColor(255, 255, 255), TRUE);
}

void Stage::SpawnEnemies(std::vector<Enemy*>& enemies)
{
	if (rooms.empty()) return;

	if (currentFloor % 5 == 0)
	{
		// ボス部屋の中央にゴーレム召喚
		Golem* boss = new Golem();
		boss->SetStage(this);
		boss->SetPosition(rooms[1].center_x, rooms[1].center_y);
		enemies.push_back(boss);
		return; // ザコは出さない
	}

	// どの部屋をモンスターハウスにするか決める（プレイヤーの初期部屋 [0] 以外）
	int mhRoomIdx = isMonsterHouseFloor ? (1 + rand() % (rooms.size() - 1)) : -1;

	for (size_t i = 1; i < rooms.size(); ++i) {
		if (i == mhRoomIdx) {
			// --- モンスターハウスの処理 ---
			// 部屋の全タイルに敵を敷き詰める勢いで生成
			for (int y = rooms[i].y; y < rooms[i].y + rooms[i].h; ++y) 
			{
				for (int x = rooms[i].x; x < rooms[i].x + rooms[i].w; ++x) 
				{
					// 60%の確率で敵を配置
					if (rand() % 100 < 60) {
						Enemy* e = new Enemy((rand() % 100 < 50) ? ENEMY_SKELTON : ENEMY_SLIME);
						e->SetStage(this);
						e->SetPosition(x, y);
						// 階層強化
						e->SetStatus(e->GetAttack() + (currentFloor * 2), e->GetHP() + (currentFloor * 5));
						enemies.push_back(e);
					}
				}
			}
			// 回復アイテムを2つ置く
			for (int k = 0; k < 2; ++k) {
				itemManager->SpawnSpecificItem(this, rooms[i].center_x + k, rooms[i].center_y, 2); // 2: HEALポーション
			}
		}
		else {
			// 通常の敵生成（既存のロジック）
			int num = 2 + (currentFloor / 5);
			for (int j = 0; j < num; ++j) {
				Enemy* e = new Enemy((rand() % 100 < 50) ? ENEMY_SKELTON : ENEMY_SLIME);
				e->SetStage(this);
				e->SetPosition(rooms[i].center_x + (j % 2), rooms[i].center_y + (j / 2));
				e->SetStatus(e->GetAttack() + (currentFloor * 2), e->GetHP() + (currentFloor * 5));
				enemies.push_back(e);
			}
		}
	}
}



void Stage::GenerateMap() 
{
	InitializeMap();
	rooms.clear();
	memset(exploredData, 0, sizeof(exploredData));
	memset(visibleData, 0, sizeof(visibleData));
	memset(itemMapData, 0, sizeof(itemMapData));

	if (currentFloor > 0 && currentFloor % 5 == 0) 
	{
		CreateBossFloor(); // 5の倍数のボスフロア用の固定マップ
	}
	else 
	{
		CreateRooms();
		CreateCorridors();

		if (!rooms.empty()) 
		{
			const auto& exitRoom = rooms.back();
			stairsX = exitRoom.center_x;
			stairsY = exitRoom.center_y;
			mapData[stairsY][stairsX] = TILE_STAIRS;
		}
	}

	itemManager->SpawnItems(this);
}

void Stage::Draw() {
	for (int y = 0; y < MAP_HEIGHT; ++y)
		for (int x = 0; x < MAP_WIDTH; ++x) DrawTile(x, y, mapData[y][x], camera_x, camera_y);

	itemManager->Draw(this);
}

int Stage::GetTileType(int x, int y) const { if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return TILE_WALL; return mapData[y][x]; }

void Stage::CreateRooms() 
{
	std::uniform_int_distribution<> sd(MIN_ROOM_SIZE, MAX_ROOM_SIZE), xd(1, MAP_WIDTH - 2), yd(1, MAP_HEIGHT - 2);
	for (int i = 0; i < MAX_ROOMS; ++i) 
	{
		Room nr; nr.w = sd(mt) * 2 + 1; nr.h = sd(mt) * 2 + 1; nr.x = xd(mt); nr.y = yd(mt);
		if (nr.x + nr.w >= MAP_WIDTH - 1) nr.x = MAP_WIDTH - 1 - nr.w;
		if (nr.y + nr.h >= MAP_HEIGHT - 1) nr.y = MAP_HEIGHT - 1 - nr.h;
		nr.center_x = nr.x + nr.w / 2; nr.center_y = nr.y + nr.h / 2;
		bool inter = false;
		for (const auto& er : rooms) if (nr.x - 1 < er.x + er.w && nr.x + nr.w + 1 > er.x && nr.y - 1 < er.y + er.h && nr.y + nr.h + 1 > er.y) { inter = true; break; }
		if (!inter) 
		{
			rooms.push_back(nr);
			for (int y = nr.y; y < nr.y + nr.h; ++y) for (int x = nr.x; x < nr.x + nr.w; ++x) mapData[y][x] = TILE_FLOOR;
		}
	}
}

void Stage::CreateCorridors() 
{
	if (rooms.size() < 2)
	{
		return;
	}
	for (size_t i = 0; i < rooms.size() - 1; ++i) 
	{
		int x1 = rooms[i].center_x; int y1 = rooms[i].center_y;
		int x2 = rooms[i + 1].center_x; int y2 = rooms[i + 1].center_y;
		for (int x = min(x1, x2); x <= max(x1, x2); ++x) mapData[y1][x] = TILE_FLOOR;
		for (int y = min(y1, y2); y <= max(y1, y2); ++y) mapData[y][x2] = TILE_FLOOR;
	}
}

void Stage::CreateBossFloor() 
{
	Room prepRoom;
	prepRoom.x = 10;
	prepRoom.y = MAP_HEIGHT / 2 - 3;
	prepRoom.w = 6;
	prepRoom.h = 6;
	prepRoom.center_x = prepRoom.x + prepRoom.w / 2;
	prepRoom.center_y = prepRoom.y + prepRoom.h / 2;
	rooms.push_back(prepRoom);

	Room bossRoom;
	bossRoom.w = 16;
	bossRoom.h = 16;
	bossRoom.x = MAP_WIDTH - bossRoom.w - 10;
	bossRoom.y = MAP_HEIGHT / 2 - bossRoom.h / 2;
	bossRoom.center_x = bossRoom.x + bossRoom.w / 2;
	bossRoom.center_y = bossRoom.y + bossRoom.h / 2;
	rooms.push_back(bossRoom); // これが 1 番

	// マップに床を描く
	for (const auto& r : rooms) 
	{
		for (int y = r.y; y < r.y + r.h; y++) 
		{
			for (int x = r.x; x < r.x + r.w; x++) 
			{
				mapData[y][x] = TILE_FLOOR;
			}
		}
	}

	for (int x = prepRoom.x + prepRoom.w; x < bossRoom.x; x++) 
	{
		mapData[prepRoom.center_y][x] = TILE_FLOOR;
	}

	// 準備室にポーションを置く
	itemManager->SpawnSpecificItem(this, prepRoom.center_x + 1, prepRoom.center_y, 2);

	// 階段の座標をボス部屋の中央に（タイルはまだ置かない）
	stairsX = bossRoom.center_x;
	stairsY = bossRoom.center_y;

	// ボス階は最初から全マップ見えてもいいなら以下を追加（デバッグも楽です）
	for (int y = 0; y < MAP_HEIGHT; y++) 
	{
		for (int x = 0; x < MAP_WIDTH; x++) 
		{
			exploredData[y][x] = 1;
		}
	}

	if (rooms.size() >= 2) 
	{
		const auto& bossRoom = rooms[1];
		for (int y = bossRoom.y; y < bossRoom.y + bossRoom.h; y++) 
		{
			for (int x = bossRoom.x; x < bossRoom.x + bossRoom.w; x++) 
			{
				if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
				{
					exploredData[y][x] = 1; // 探索済みにする
					visibleData[y][x] = 1; 
				}
			}
		}
	}
}