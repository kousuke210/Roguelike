#include "DxLib.h"
#include "Player.h"
#include <cmath>
#include "Stage.h"

Player::Player()
	: 
	x(1280.0f / 2.0f),  
	y(720.0f / 2.0f),
	isDash(false),
	isDodge(false),
	isMoving(false),
	Stamina(MAX_STAMINA),
	dashCoolTime(0),
	dodgeDuration(0), 
	staminaDelayCounter(STAMINA_RECOVER_DELAY_FRAMES)
{
}

Player::~Player()
{
}

void Player::SetPosition(float x, float y)
{
	this->x = x;
	this->y = y;
}

// 【追加】移動先の座標に壁があるかチェックする
bool Player::CheckCollision(float next_x, float next_y)
{
	if (!stage) return false; // ステージが設定されていなければ常に衝突なしとする

	// プレイヤーの円の中心座標と半径
	float radius = 20.0f; // Player::Draw() で使われている半径
	int tileSize = stage->GetTileSize();

	// プレイヤーの移動先座標が壁と重なるかどうかをチェックするための4隅の点
	// 衝突判定をシンプルにするため、円の中心から外側に向かって4点の位置を計算します
	float check_points[4][2] = {
		{next_x + radius - 1.0f, next_y}, // 右端 (-1.0f は丸め誤差対策)
		{next_x - radius + 1.0f, next_y}, // 左端
		{next_x, next_y + radius - 1.0f}, // 下端
		{next_x, next_y - radius + 1.0f}  // 上端
	};

	for (int i = 0; i < 4; ++i)
	{
		float check_x = check_points[i][0];
		float check_y = check_points[i][1];

		// ピクセル座標をマス座標に変換
		int tile_x = (int)(check_x / tileSize);
		int tile_y = (int)(check_y / tileSize);

		// マス座標がマップの範囲内かチェックし、壁（TILE_WALL）であれば衝突
		if (stage->GetTileType(tile_x, tile_y) == TILE_WALL)
		{
			return true; // 衝突あり
		}
	}

	return false; // 衝突なし
}

void Player::Move()
{
	float speed = isDash ? DashSpeed : moveSpeed;
	float dx = 0.0f;
	float dy = 0.0f;
	isMoving = false;

	if (CheckHitKey(KEY_INPUT_W)) // 上
	{
		dy -= speed; isMoving = true;
	} 
	if (CheckHitKey(KEY_INPUT_S)) // 下
	{ 
		dy += speed; isMoving = true; 
	}
	if (CheckHitKey(KEY_INPUT_A)) // 左
	{ 
		dx -= speed; isMoving = true; 
	} 
	if (CheckHitKey(KEY_INPUT_D)) // 右
	{ 
		dx += speed; isMoving = true; 
	} 

	// 斜め移動の速度調整
	if (dx != 0.0f && dy != 0.0f) {
		float diag_speed_rate = 0.7071f;
		dx *= diag_speed_rate;
		dy *= diag_speed_rate;
	}

	// 【修正】衝突判定を適用しながら座標を更新

	// X軸の移動チェック
	if (dx != 0.0f)
	{
		if (!CheckCollision(x + dx, y))
		{
			x += dx;
		}
	}

	// Y軸の移動チェック
	if (dy != 0.0f)
	{
		if (!CheckCollision(x, y + dy))
		{
			y += dy;
		}
	}

	// スタミナの回復
	if (!isDash && dashCoolTime == 0 && Stamina < MAX_STAMINA) {
		Stamina += STAMINA_RECOVER_RATE;
		if (Stamina > MAX_STAMINA) Stamina = MAX_STAMINA;
	}
}

void Player::Dash()
{
	// クールタイム中はダッシュ不可
	if (dashCoolTime > 0)
	{
		dashCoolTime--;
		isDash = false;
		return;
	}

	// LShiftが押されていて、移動中、かつスタミナがあるかチェック
	if (CheckHitKey(KEY_INPUT_LSHIFT) && isMoving && Stamina > 0.0f)
	{
		isDash = true;
		Stamina -= STAMINA_CONSUME_DASH;

		if (Stamina < 0.0f)
		{
			Stamina = 0.0f;
		}

		// スタミナが尽きたらクールタイム開始
		if (Stamina <= 0.0f)
		{
			// スタミナが尽きた場合も、ダッシュを続けてクールタイムに移行
			Stamina = 0.0f;
			isDash = false; // ダッシュを強制終了
			dashCoolTime = MAX_COOLTIME;
		}
	}
	else
	{
		isDash = false;
	}
}

void Player::Dodge()
{
	// 1. 回避の持続時間中の処理（移動を続ける）
	if (dodgeDuration > 0)
	{
		dodgeDuration--;
		isDodge = true;

		// 【追加】回避中の移動処理
		float dx = 0.0f;
		float dy = 0.0f;

		// W/A/S/D キーが押されている方向を取得
		if (CheckHitKey(KEY_INPUT_W)) dy -= 1.0f;
		if (CheckHitKey(KEY_INPUT_S)) dy += 1.0f;
		if (CheckHitKey(KEY_INPUT_A)) dx -= 1.0f;
		if (CheckHitKey(KEY_INPUT_D)) dx += 1.0f;

		// 移動方向が入力されていた場合
		if (dx != 0.0f || dy != 0.0f) {
			// 移動ベクトルを正規化
			float length = sqrtf(dx * dx + dy * dy);
			if (length > 0.0f) {
				dx /= length;
				dy /= length;
			}

			// 座標を更新
			x += dx * DODGE_SPEED;
			y += dy * DODGE_SPEED;
		}

		return;
	}

	isDodge = false;

	if (CheckHitKey(KEY_INPUT_SPACE) == 1 && dashCoolTime == 0 && Stamina >= STAMINA_CONSUME_DODGE)
	{
		isDodge = true;
		dodgeDuration = MAX_DODGETIME;
		Stamina -= STAMINA_CONSUME_DODGE; // 回避でスタミナ消費

		// 回避後のクールタイムを設定
		dashCoolTime = MAX_COOLTIME;
	}
}

void Player::Update()
{
	Dodge();

	if (!isDodge)
	{
		Dash();
		Move();
	}
}
void Player::Draw()
{
	int color = GetColor(255, 255, 255); 
	if (isDash) {
		color = GetColor(255, 0, 0); // ダッシュ中は赤
	}
	else if (isDodge) {
		color = GetColor(0, 255, 255); // 回避中はシアン
	}

	DrawCircleAA(x, y, 20.0f, 64, color, TRUE);

	// 状態とスタミナの表示（デバッグ用）
	DrawFormatString(10, 10, GetColor(255, 255, 255), "Stamina: %.1f / %.1f", Stamina, MAX_STAMINA);
	DrawFormatString(10, 30, GetColor(255, 255, 255), "Dash CoolTime: %d", dashCoolTime);
	DrawFormatString(10, 50, GetColor(255, 255, 255), "Dodge Duration: %d", dodgeDuration);
}