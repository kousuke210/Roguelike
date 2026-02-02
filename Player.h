#pragma once

class Stage;

class Player
{
public:
	Player();
	~Player();
	bool Update();
	void Draw();
	void SetPosition(int map_x, int map_y);
	void SetStage(Stage* stage) { this->stage = stage; }
	int GetMapX() const { return map_x; }
	int GetMapY() const { return map_y; }

	// --- ステータス関連 ---
	int GetHP() const { return hp; }
	int GetMaxHP() const { return maxHP; }
	void Heal(int amount) {
		hp += amount;
		if (hp > maxHP) hp = maxHP; // 最大値を超えないように制限
	}

	int GetAttack() const { return attack; }
	void AddAttack(int amount) { attack += amount; }

private:
	int map_x;
	int map_y;

	// ステータス変数
	int hp;       // 現在の体力
	int maxHP;    // 最大の体力
	int attack;   // 攻撃力

	Stage* stage;

	bool CheckCollision(int next_map_x, int next_map_y);
};