#pragma once

class Player {
public:
	Player();
	~Player();
	void Update();
	void Draw();
private:
	//プレイヤーの位置座標
	float x;
	float y;

	//状態管理
	bool isDash; // true:ダッシュ中 false:通常状態
	bool isDodge; // true:回避中 false:回避中でない
	bool isMoving; // true:移動キーが押されている false:停止

	//パラメータ
	float Stamina; // 現在のスタミナ
	static constexpr float MAX_STAMINA = 100.0f; // スタミナの最大値
	const float STAMINA_CONSUME_DASH = 1.0f; // 1フレームあたりのダッシュ消費スタミナ
	const float STAMINA_CONSUME_DODGE = 30.0f; // 回避で消費するスタミナ
	const float STAMINA_RECOVER_RATE = 0.5f; // 1フレームあたりのスタミナ回復量

	//移動速度
	const float moveSpeed = 3.0f;    // 通常の移動速度
	const float DashSpeed = 6.0f;    // ダッシュ時の移動速度

	int dashCoolTime;        // ダッシュ後のクールタイム（フレーム）
	int dodgeDuration;       // 回避の持続時間（フレーム）
	const int MAX_COOLTIME = 60; // ダッシュ・回避後のクールタイム最大値 (60フレーム=1秒)
	const int MAX_DODGETIME = 5; // 【修正】回避の持続時間最大値 (20フレーム -> 5フレームに変更)

	const float DODGE_SPEED = 15.0f; // 回避時の1フレームあたりの移動速度

	void Dash();
	void Dodge();
	void Move();
};