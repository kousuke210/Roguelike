#pragma once
#include <vector>
#include <random>
#include <cstring>

class Player;

// マップのタイルの種類
enum E_TILE_TYPE
{
	TILE_WALL,  // 壁
	TILE_FLOOR, // 床（通路・部屋）
	TILE_MAX
};

class Stage
{
public:
	Stage();
	~Stage();

	void GenerateMap(); // ダンジョン生成処理を開始する
	void Draw();        // マップを描画する

	// 指定したマス座標のタイルタイプを取得
	int GetTileType(int x, int y) const;

	struct Room {
		int x; int y; int w; int h;
		int center_x; int center_y;
	};

	const std::vector<Room>& GetRooms() const { return rooms; }
	void SetPlayer(Player* player) { this->player = player; }
	Player* GetPlayer() const { return player; }
	int GetTileSize() const { return TILE_SIZE; }

	// 拡大率を取得
	float GetZoomRate() const { return ZOOM_RATE; }
	// エンティティが現在見えているか確認する
	bool IsTileVisible(int x, int y) const;

	// カメラ座標を更新する
	void UpdateCamera(int player_map_x, int player_map_y);
	int GetCameraX() const { return camera_x; }
	int GetCameraY() const { return camera_y; }

	// マップオーバーレイを描画する関数
	void DrawOverlayMap(int screen_width, int screen_height);

private:
	static const int TILE_SIZE = 50;
	static const int MAP_WIDTH = 64;
	static const int MAP_HEIGHT = 36;
	static const int MAX_ROOMS = 20;
	static const int MIN_ROOM_SIZE = 3;
	static const int MAX_ROOM_SIZE = 8;

	// 描画拡大率 (1.5倍)
	static constexpr float ZOOM_RATE = 1.5f;

	int GroundImage;
	int WallImage;

	int mapData[MAP_HEIGHT][MAP_WIDTH];
	int exploredData[MAP_HEIGHT][MAP_WIDTH] = { 0 }; // 探索済みフラグ
	int visibleData[MAP_HEIGHT][MAP_WIDTH] = { 0 };  // 現在の視界フラグ

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
};