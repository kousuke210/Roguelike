#include "DxLib.h" 
#include "Stage.h"
#include "Player.h"
#include "Enemy.h"
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

	// Šî–{‚ÌŽ‹ŠE‚ð 2 ‚É‚µ‚ÄAö‚é‚Ù‚Ç‹·‚­‚È‚é
	int baseRange = (player && player->torchTurn > 0) ? 5 : 2;

	// —áF10ŠKˆÈ~‚ÍA‚½‚¢‚Ü‚Â–³‚µ‚¾‚ÆŽ‹ŠE‚ª 1 ‚É‚È‚é
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

	// ‰æ–Êã‚Ì•`‰æ”ÍˆÍil, t, r, bj
	int l = (int)(x * ds - offset_x * z);
	int t = (int)(y * ds - offset_y * z);
	int r = (int)((x + 1) * ds - offset_x * z);
	int b = (int)((y + 1) * ds - offset_y * z);

	bool isVis = IsTileVisible(x, y);
	bool isExp = (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) && (exploredData[y][x] == 1);

	// °‚ÆŠK’i‚Ì•`‰æ
	if (type == TILE_FLOOR || type == TILE_STAIRS)
	{
		DrawRectExtendGraph(l, t, r, b, 0, 0, 64, 64, GroundImage, TRUE);

		// ŠK’i‚Ìê‡‚Í‚»‚Ìã‚Éd‚Ë‚é
		if (type == TILE_STAIRS)
		{
			DrawExtendGraph(l, t, r, b, StairImage, TRUE);
		}
	}
	else if (type == TILE_WALL)
	{
		// •Ç‚Ì•`‰æ
		bool adj = false;
		if (GetTileType(x, y - 1) == TILE_FLOOR || GetTileType(x, y + 1) == TILE_FLOOR ||
			GetTileType(x - 1, y) == TILE_FLOOR || GetTileType(x + 1, y) == TILE_FLOOR ||
			GetTileType(x - 1, y - 1) == TILE_FLOOR || GetTileType(x + 1, y - 1) == TILE_FLOOR ||
			GetTileType(x - 1, y + 1) == TILE_FLOOR || GetTileType(x + 1, y + 1) == TILE_FLOOR ||
			GetTileType(x, y - 1) == TILE_STAIRS || GetTileType(x, y + 1) == TILE_STAIRS) adj = true;

		if (adj) DrawExtendGraph(l, t, r, b, WallImage, TRUE);
		else DrawBox(l, t, r, b, GetColor(0, 0, 0), TRUE);
	}

	// Ž‹ŠE‚Ìˆ—
	if (!isVis)
	{
		if (isExp)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
			DrawBox(l, t, r, b, GetColor(0, 0, 0), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		else
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);
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
			// ’TõÏ‚Ý‚Ì°‚â•Ç‚ð•`‰æ
			if (exploredData[y][x]) 
			{
				int c = (mapData[y][x] == TILE_FLOOR) ? GetColor(50, 50, 200) : GetColor(100, 100, 100);
				DrawBox(sx + x * SCALE, sy + y * SCALE, sx + (x + 1) * SCALE, sy + (y + 1) * SCALE, c, TRUE);

				if (itemMapData[y][x] == 1) // ATKƒ|[ƒVƒ‡ƒ“(Ô)
				{
					DrawBox(sx + x * SCALE + 1, sy + y * SCALE + 1, sx + (x + 1) * SCALE - 1, sy + (y + 1) * SCALE - 1, GetColor(255, 0, 0), TRUE);
				}
				else if (itemMapData[y][x] == 2)  // HEALƒ|[ƒVƒ‡ƒ“(—Î)
				{
					DrawBox(sx + x * SCALE + 1, sy + y * SCALE + 1, sx + (x + 1) * SCALE - 1, sy + (y + 1) * SCALE - 1, GetColor(0, 255, 0), TRUE);
				}
			}
		}
	}
	if (player) DrawBox(sx + player->GetMapX() * SCALE - 1, sy + player->GetMapY() * SCALE - 1, sx + (player->GetMapX() + 1) * SCALE + 1, sy + (player->GetMapY() + 1) * SCALE + 1, GetColor(255, 255, 255), TRUE);
}

void Stage::SpawnEnemies(std::vector<Enemy*>& enemies, int floor) {
	if (rooms.empty()) return;

	for (size_t i = 1; i < rooms.size(); ++i) {
		// ŠK‘w‚ª[‚¢‚Ù‚ÇA“G‚ÌoŒ»”‚ð­‚µ‚¸‚Â‘‚â‚· (—á: 5ŠK‚²‚Æ‚É+1‘Ì)
		int numEnemies = 2 + (floor / 5);

		for (int j = 0; j < numEnemies; j++) {
			E_ENEMY_TYPE type = (rand() % 100 < 50) ? ENEMY_SKELTON : ENEMY_SLIME;
			Enemy* newEnemy = new Enemy(type);
			newEnemy->SetStage(this);

			// ŠK‘w‚É‰ž‚¶‚Ä“G‚ÌƒXƒe[ƒ^ƒX‚ð‹­‰»
			// Šî‘bUŒ‚—Í + (Œ»Ý‚ÌŠK‘w * 2) ‚È‚Ç
			int bonusAtk = floor * 2;
			int bonusHp = floor * 5;
			newEnemy->SetStatus(newEnemy->GetAttack() + bonusAtk, newEnemy->GetHP() + bonusHp);

			newEnemy->SetPosition(rooms[i].center_x + (j % 2), rooms[i].center_y + (j / 2));
			enemies.push_back(newEnemy);
		}
	}
}

void Stage::GenerateMap()
{
	InitializeMap();
	rooms.clear();

	memset(exploredData, 0, sizeof(exploredData)); // ’TõÏ‚Ý‚Ì•‚¢‰e‚ðÁ‚·
	memset(visibleData, 0, sizeof(visibleData));   // Œ»Ý‚ÌŽ‹ŠE‚ðƒŠƒZƒbƒg
	memset(itemMapData, 0, sizeof(itemMapData));   // ƒ~ƒjƒ}ƒbƒvã‚ÌƒAƒCƒeƒ€“_‚ðÁ‚·

	CreateRooms();     // V‚µ‚¢•”‰®‚ðì¬
	CreateCorridors(); // V‚µ‚¢’Ê˜H‚ðì¬

	if (!rooms.empty())
	{
		// ÅŒã‚Ì•”‰®‚Ì’†‰›‚ÉŠK’i‚ðÝ’u
		const auto& exitRoom = rooms.back();
		mapData[exitRoom.center_y][exitRoom.center_x] = TILE_STAIRS;
	}

	itemManager->SpawnItems(this); // ƒAƒCƒeƒ€‚ð”z’u
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