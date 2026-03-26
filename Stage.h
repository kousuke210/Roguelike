#pragma once
#include <vector>
#include <random>
#include <cstring>
#include "Item.h"
#include "Enemy.h"

class Player;
class ItemManager;

// マップのタイルの種類
enum E_TILE_TYPE
{
	TILE_WALL,
	TILE_FLOOR,
	TILE_MAX,
	TILE_STAIRS
};

class Stage
{
public:
	Stage();
	~Stage();

	void GenerateMap();
	void Draw();

	int GetTileType(int x, int y) const;

	struct Room 
	{
		int x; int y; int w; int h;
		int center_x; int center_y;
	};

	const std::vector<Room>& GetRooms() const { return rooms; }
	void SetPlayer(Player* player) { this->player = player; }
	Player* GetPlayer() const { return player; }
	int GetTileSize() const { return TILE_SIZE; }

	float GetZoomRate() const { return ZOOM_RATE; }
	bool IsTileVisible(int x, int y) const;

	void UpdateCamera(int player_map_x, int player_map_y);
	int GetCameraX() const { return camera_x; }
	int GetCameraY() const { return camera_y; }

	void DrawOverlayMap(int screen_width, int screen_height);
	class ItemManager* GetItemManager() { return itemManager; }

	void AdvanceFloor() { currentFloor++; }
	int GetCurrentFloor() const { return currentFloor; }

	void SpawnEnemies(std::vector<Enemy*>& enemies);
	const std::vector<Enemy*>& GetEnemies() const { return enemies; }

	// プレイヤーの開始位置を返す関数を追加
	int GetStartIdxX() const { return rooms.empty() ? 0 : rooms[0].center_x; }
	int GetStartIdxY() const { return rooms.empty() ? 0 : rooms[0].center_y; }

	void SetItemFound(int x, int y, int type) 
	{
		if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) 
		{
			// ATKなら1(赤)、HEALなら2(緑)として保存
			itemMapData[y][x] = (type == 0) ? 2 : 1; // E_ITEM_TYPEの定義に合わせて調整
		}
	}
	void ClearItemFound(int x, int y) 
	{
		if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) itemMapData[y][x] = 0;
	}

	int GetStairsX() const {
		// roomsの最後の部屋の中央に階段がある前提
		return rooms.empty() ? 0 : rooms.back().center_x;
	}
	int GetStairsY() const {
		return rooms.empty() ? 0 : rooms.back().center_y;
	}

	static const int MAP_WIDTH = 64;
	static const int MAP_HEIGHT = 36;
	int itemMapData[MAP_HEIGHT][MAP_WIDTH] = { 0 };
	void SetExplored(int x, int y);
	void ResetFloor() { currentFloor = 4; }

	bool CanMoveTo(int nx, int ny) const
	{
		if (GetTileType(nx, ny) == TILE_WALL) return false;

		for (auto e : enemies) {
			if (e->GetHP() <= 0) continue;

			if (currentFloor % 5 == 0) 
			{
				float dist_x = abs((e->GetMapX() + 0.5f) - (nx + 0.5f));
				float dist_y = abs((e->GetMapY() + 0.5f) - (ny + 0.5f));

				if (dist_x < 1.8f && dist_y < 1.8f) return false;
			}
			else 
			{
				if (e->GetMapX() == nx && e->GetMapY() == ny) return false;
			}
		}
		return true;
	}

	bool IsOccupied(int x, int y) const;

private:
	static const int TILE_SIZE = 50;
	static const int MAX_ROOMS = 20;
	static const int MIN_ROOM_SIZE = 3;
	static const int MAX_ROOM_SIZE = 8;

	// 描画拡大率 (1.5倍)
	static constexpr float ZOOM_RATE = 1.5f;

	int GroundImage;
	int WallImage;
	int StairImage;

	int stairsX = -1;
	int stairsY = -1;

	int mapData[MAP_HEIGHT][MAP_WIDTH];
	int exploredData[MAP_HEIGHT][MAP_WIDTH] = { 0 }; 
	int visibleData[MAP_HEIGHT][MAP_WIDTH] = { 0 };

	std::vector<Room> rooms;
	std::mt19937 mt;
	Player* player = nullptr;
	std::vector<Enemy*> enemies;

	int camera_x = 0;
	int camera_y = 0;

	int currentFloor = 1;
	bool isMonsterHouseFloor = false;

	void InitializeMap();
	void CreateRooms();
	void CreateCorridors();
	void CreateBossFloor();

	void CalculateVisibleTiles(int player_map_x, int player_map_y);
	void DrawTile(int x, int y, int type, int offset_x, int offset_y);

	class ItemManager* itemManager = nullptr;
};