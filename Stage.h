#pragma once
#include <vector>
#include <random>
#include <cstring>
#include "Item.h"

class Player;
class ItemManager;

// ƒ}ƒbƒv‚Ìƒ^ƒCƒ‹‚ÌŽí—Þ
enum E_TILE_TYPE
{
	TILE_WALL,
	TILE_FLOOR,
	TILE_MAX
};

class Stage
{
public:
	Stage();
	~Stage();

	void GenerateMap();
	void Draw();

	int GetTileType(int x, int y) const;

	struct Room {
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

private:
	static const int TILE_SIZE = 50;
	static const int MAP_WIDTH = 64;
	static const int MAP_HEIGHT = 36;
	static const int MAX_ROOMS = 20;
	static const int MIN_ROOM_SIZE = 3;
	static const int MAX_ROOM_SIZE = 8;

	// •`‰æŠg‘å—¦ (1.5”{)
	static constexpr float ZOOM_RATE = 1.5f;

	int GroundImage;
	int WallImage;

	int mapData[MAP_HEIGHT][MAP_WIDTH];
	int exploredData[MAP_HEIGHT][MAP_WIDTH] = { 0 }; 
	int visibleData[MAP_HEIGHT][MAP_WIDTH] = { 0 };

	std::vector<Room> rooms;
	std::mt19937 mt;
	Player* player = nullptr;

	int camera_x = 0;
	int camera_y = 0;

	void InitializeMap();
	void CreateRooms();
	void CreateCorridors();
	void SetExplored(int x, int y);
	void CalculateVisibleTiles(int player_map_x, int player_map_y);
	void DrawTile(int x, int y, int type, int offset_x, int offset_y);

	class ItemManager* itemManager = nullptr;
};