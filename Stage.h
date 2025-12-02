#pragma once
#include <vector>
#include <random>

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

	// 【追加】指定したマス座標のタイルタイプを取得
	int GetTileType(int x, int y) const;

	// 【追加】メインループから部屋の情報を取得するためのパブリックな構造体とメソッド
	struct Room {
		int x; // 左上のX座標（マス）
		int y; // 左上のY座標（マス）
		int w; // 幅（マス）
		int h; // 高さ（マス）
		int center_x; // 中心X座標（マス）
		int center_y; // 中心Y座標（マス）
	};

	// 【追加】生成された部屋のリストを取得
	const std::vector<Room>& GetRooms() const { return rooms; }
	// 【追加】プレイヤーへのポインタを設定する
	void SetPlayer(Player* player) { this->player = player; }
	// 【追加】プレイヤーへのポインタを取得する (Enemy::CheckCollisionで使用)
	Player* GetPlayer() const { return player; }
	// 【追加】タイルサイズを取得（プレイヤー初期位置計算に必要）
	int GetTileSize() const { return TILE_SIZE; }

	// カメラ座標を更新する（プレイヤーの現在地に基づいて）
	void UpdateCamera(int player_map_x, int player_map_y);
	// カメラオフセット座標（ピクセル）を取得する
	int GetCameraX() const { return camera_x; }
	int GetCameraY() const { return camera_y; }

private:
	// マップの定数定義
	static const int TILE_SIZE = 50;       // 1マスのサイズ（ピクセル）
	static const int MAP_WIDTH = 64;       // マップの幅（マス） 32 -> 64
	static const int MAP_HEIGHT = 36;      // マップの高さ（マス） 18 -> 36
	static const int MAX_ROOMS = 20;       // 生成する部屋の最大数 10 -> 20 (最低4部屋生成される機会を増やす)
	static const int MIN_ROOM_SIZE = 3;    // 部屋の最小サイズ（マス）
	static const int MAX_ROOM_SIZE = 8;    // 部屋の最大サイズ（マス）

	int GroundImage;
	int WallImage;


	// マップデータ（[高さ][幅]）
	int mapData[MAP_HEIGHT][MAP_WIDTH];

	std::vector<Room> rooms; // 生成された部屋のリスト

	// 乱数生成器
	std::mt19937 mt;

	Player* player = nullptr; // プレイヤーへのポインタ

	// 【追加】カメラオフセット座標
	int camera_x = 0;
	int camera_y = 0;

	// プライベートメソッド
	void InitializeMap(); // マップをすべて壁で初期化
	void CreateRooms();   // 部屋を生成し、マップに配置する
	void CreateCorridors(); // 部屋同士を通路で繋ぐ

	void DrawTile(int x, int y, int type, int offset_x, int offset_y); // 1マスを描画する
};