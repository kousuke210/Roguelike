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

	// ステータス関連
	int GetHP() const { return hp; }
	int GetMaxHP() const { return maxHP; }
	void Heal(int amount) {
		hp += amount;
		if (hp > maxHP) hp = maxHP; // 最大値を超えないように
	}

	int GetLevel() const { return level; }
	int GetExp() const { return exp; }
	int GetNextExp() const { return nextExp; }
	// 経験値を獲得し、レベルアップ判定する
	void AddExp(int amount) {
		exp += amount;
		while (exp >= nextExp) {
			LevelUp();
		}
	}

	int GetAttack() const { return attack; }
	void AddAttack(int amount) { attack += amount; }

	int torchTurn = 0;          // たいまつの残りターン
	int clairvoyanceTurn = 0;   // 千里眼の残りターン
	void UpdateTurn() 
	{
		if (torchTurn > 0) torchTurn--;
		if (clairvoyanceTurn > 0) clairvoyanceTurn--;
	}

	void ShowPickUpMessage(const char* text);
	void DrawMessage();

private:
	int map_x;
	int map_y;

	// ステータス
	int hp;       // 現在の体力
	int maxHP;    // 最大の体力
	int attack;   // 攻撃力
	int PlayerGraph;
	char pickUpText[64] = "";
	int messageTimer = 0;
	int level = 1;
	int exp = 0;
	int nextExp = 20;

	Stage* stage;

	bool CheckCollision(int next_map_x, int next_map_y);

	//レベルアップ関連処理
	void LevelUp() 
	{
		exp -= nextExp;
		level++;
		nextExp = (int)(nextExp * 1.5f);

		int hpUp = 5;
		int atkUp = 2;

		maxHP += hpUp;
		hp = maxHP;
		attack += atkUp;

		char msg[128];
		sprintf_s(msg, "LEVEL UP!  最大HP+%d  ATK+%d", hpUp, atkUp);
		ShowPickUpMessage(msg);
	}
};