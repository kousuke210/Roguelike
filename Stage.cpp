#include "DxLib.h" 
#include "Stage.h"
#include <random>
#include <ctime> 
#include <algorithm>
#include "Player.h"
#include <assert.h>

using std::min;
using std::max;

Stage::Stage() : mt(static_cast<unsigned int>(time(NULL))) 
{
	GroundImage = LoadGraph("Assets/tutidou.png");
	WallImage = LoadGraph("Assets/tutikabe1.png");
	assert(GroundImage > 0 && WallImage > 0);
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
	const int sight_range = 2; // Ž‹ŠE‚ÌL‚³
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
	const int FOG_ALPHA = 128; // ˆÃ‚³‚Ì“x‡‚¢ (0-255)

	int l = (int)(x * ds - offset_x * z);
	int t = (int)(y * ds - offset_y * z);
	int r = (int)((x + 1) * ds - offset_x * z);
	int b = (int)((y + 1) * ds - offset_y * z);

	bool isVis = IsTileVisible(x, y);
	bool isExp = (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) && (exploredData[y][x] == 1);

	if (!isExp) 
	{ // –¢’Tõ‚Í^‚Á•
		DrawBox(l, t, r, b, GetColor(0, 0, 0), TRUE);
		return;
	}

	if (type == TILE_FLOOR) 
	{
		DrawRectGraph(l, t, 0, 0, (int)ds, (int)ds, GroundImage, TRUE, FALSE);
		if (!isVis) 
		{ // Ž‹ŠEŠO‚È‚ç”¼“§–¾‚Ì•‚ðd‚Ë‚é
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, FOG_ALPHA);
			DrawBox(l, t, r, b, GetColor(0, 0, 0), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}
	else if (type == TILE_WALL) 
	{
		bool adj = false;
		if (GetTileType(x, y - 1) == TILE_FLOOR || GetTileType(x, y + 1) == TILE_FLOOR ||
			GetTileType(x - 1, y) == TILE_FLOOR || GetTileType(x + 1, y) == TILE_FLOOR ||
			GetTileType(x - 1, y - 1) == TILE_FLOOR || GetTileType(x + 1, y - 1) == TILE_FLOOR ||
			GetTileType(x - 1, y + 1) == TILE_FLOOR || GetTileType(x + 1, y + 1) == TILE_FLOOR) adj = true;

		if (adj) 
		{
			DrawExtendGraph(l, t, r, b, WallImage, TRUE);
			if (!isVis) 
			{
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, FOG_ALPHA);
				DrawBox(l, t, r, b, GetColor(0, 0, 0), TRUE);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
		}
		else 
		{
			DrawBox(l, t, r, b, GetColor(0, 0, 0), TRUE);
		}
	}
}

void Stage::UpdateCamera(int px, int py) 
{
	// yC³zV‚µ‚¢‰æ–ÊƒTƒCƒY (1400x700)
	const int SW = 1400;
	const int SH = 700;
	const int DTS = (int)(TILE_SIZE * ZOOM_RATE);
	const int STW = SW / DTS;
	const int STH = SH / DTS;

	int tx = (px / STW) * TILE_SIZE * STW;
	int ty = (py / STH) * TILE_SIZE * STH;

	int MWP = MAP_WIDTH * TILE_SIZE;
	int MHP = MAP_HEIGHT * TILE_SIZE;
	// yC³zŠg‘å—¦‚ðl—¶‚µ‚½”ñƒY[ƒ€Žž‚Ì‰æ–Ê•
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
	// yC³zˆø” sw(1400), sh(700) ‚ÉŠî‚Ã‚¢‚Ä’†‰›ˆÊ’u‚ðŒvŽZ
	int sx = (sw - MAP_WIDTH * SCALE) / 2;
	int sy = (sh - MAP_HEIGHT * SCALE) / 2;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(0, 0, sw, sh, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	for (int y = 0; y < MAP_HEIGHT; ++y) 
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			if (exploredData[y][x]) 
			{
				int c = (mapData[y][x] == TILE_FLOOR) ? GetColor(50, 50, 200) : GetColor(100, 100, 100);
				DrawBox(sx + x * SCALE, sy + y * SCALE, sx + (x + 1) * SCALE, sy + (y + 1) * SCALE, c, TRUE);
			}
		}
	}
	if (player) DrawBox(sx + player->GetMapX() * SCALE - 1, sy + player->GetMapY() * SCALE - 1, sx + (player->GetMapX() + 1) * SCALE + 1, sy + (player->GetMapY() + 1) * SCALE + 1, GetColor(255, 255, 255), TRUE);
}

void Stage::GenerateMap() 
{ 
	InitializeMap(); 
	rooms.clear(); 
	CreateRooms(); 
	CreateCorridors(); 
	itemManager->SpawnItems(rooms);
}

void Stage::Draw() {
	// 1. ’n–Ê‚Ì•`‰æ
	for (int y = 0; y < MAP_HEIGHT; ++y)
		for (int x = 0; x < MAP_WIDTH; ++x) DrawTile(x, y, mapData[y][x], camera_x, camera_y);

	// 2. ƒAƒCƒeƒ€‚Ì•`‰æ (’n–Ê‚Ìã‚Éd‚Ë‚é)
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
	if (rooms.size() < 2) return;
	for (size_t i = 0; i < rooms.size() - 1; ++i) 
	{
		int x1 = rooms[i].center_x; int y1 = rooms[i].center_y;
		int x2 = rooms[i + 1].center_x; int y2 = rooms[i + 1].center_y;
		for (int x = min(x1, x2); x <= max(x1, x2); ++x) mapData[y1][x] = TILE_FLOOR;
		for (int y = min(y1, y2); y <= max(y1, y2); ++y) mapData[y][x2] = TILE_FLOOR;
	}
}