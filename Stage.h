#pragma once
#include <vector>
#include <random> // 乱数生成のために追加

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
	// 【追加】タイルサイズを取得（プレイヤー初期位置計算に必要）
	int GetTileSize() const { return TILE_SIZE; }


private:
	// マップの定数定義
	static const int TILE_SIZE = 40;       // 1マスのサイズ（ピクセル）
	static const int MAP_WIDTH = 32;       // マップの幅（マス） 1280 / 40 = 32
	static const int MAP_HEIGHT = 18;      // マップの高さ（マス） 720 / 40 = 18
	static const int MAX_ROOMS = 10;       // 生成する部屋の最大数
	static const int MIN_ROOM_SIZE = 3;    // 部屋の最小サイズ（マス）
	static const int MAX_ROOM_SIZE = 8;    // 部屋の最大サイズ（マス）

	// マップデータ（[高さ][幅]）
	int mapData[MAP_HEIGHT][MAP_WIDTH];

	// 部屋の情報を保持する構造体
	// NOTE: 構造体定義をpublicに移動
	// struct Room { ... };

	std::vector<Room> rooms; // 生成された部屋のリスト

	// 乱数生成器
	std::mt19937 mt;

	// プライベートメソッド
	void InitializeMap(); // マップをすべて壁で初期化
	void CreateRooms();   // 部屋を生成し、マップに配置する
	void CreateCorridors(); // 部屋同士を通路で繋ぐ
	void DrawTile(int x, int y, int type); // 1マスを描画する
};