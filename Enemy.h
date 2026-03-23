#pragma once

class Stage;

enum E_ENEMY_TYPE {
	ENEMY_SKELTON,
	ENEMY_SLIME,
	ENEMY_MAX
};

class Enemy
{
public:
	Enemy(E_ENEMY_TYPE type);
	virtual ~Enemy();
	virtual bool Update();
	virtual void Draw();
	void SetPosition(int map_x, int map_y);
	void SetStage(Stage* stage) { this->stage = stage; }
	int GetMapX() const { return map_x; }
	int GetMapY() const { return map_y; }

	int GetAttack() const { return atk; }
	int GetHP() const { return hp; }

	// ステータスを設定する（階層強化用）
	void SetStatus(int newAtk, int newHp)
	{
		this->atk = newAtk;
		this->hp = newHp;
		this->maxHp = newHp;
	}
	// ダメージを受け、倒されたら true を返す
	bool TakeDamage(int damage) 
	{
		hp -= damage;
		return hp <= 0;
	}

	int GetExpValue(int floor) const 
	{
		// 基礎10 + (階層 * x)
		return 10 + (floor * 2);
	}


protected:
	int map_x;
	int map_y;
	int hp;
	int maxHp;
	int atk;
	E_ENEMY_TYPE type;
	Stage* stage;

private:
	int EnemyGraph;
	bool CheckCollision(int next_map_x, int next_map_y);
};