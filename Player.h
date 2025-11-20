#pragma once

class Stage;

class Player
{
public:
	Player();
	~Player();
	// 【変更】Updateの戻り値をboolにして、行動したらtrueを返す（ターン終了を示す）
	bool Update();
	void Draw();
	// 【変更】SetPositionの引数をマス座標（int）に変更
	void SetPosition(int map_x, int map_y);
	void SetStage(Stage* stage) { this->stage = stage; }
	// 【追加】マップ座標を取得できるようにする
	int GetMapX() const { return map_x; }
	int GetMapY() const { return map_y; }
private:
	// 【変更】プレイヤーの位置座標をマス座標（int）に
	int map_x;
	int map_y;

	Stage* stage;

	// 【削除/無効化】リアルタイム移動、スタミナ、ダッシュ、回避関連の定数と変数を削除

	// 【変更】衝突判定をマス座標ベースに簡素化
	bool CheckCollision(int next_map_x, int next_map_y);

	// 【削除】リアルタイム移動/アクション関数はUpdateに統合・無効化
	// void Dash();
	// void Dodge();
	// void Move();
};