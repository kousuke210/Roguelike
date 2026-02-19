#pragma once

class Stage;

class Enemy
{
public:
	Enemy();
	~Enemy();
	bool Update();
	void Draw();
	void SetPosition(int map_x, int map_y);
	void SetStage(Stage* stage) { this->stage = stage; }
	int GetMapX() const { return map_x; }
	int GetMapY() const { return map_y; }

	int GetHP() const { return hp; }
	// ダメージを受け、倒されたら true を返す
	bool TakeDamage(int damage) 
	{
		hp -= damage;
		return hp <= 0;
	}

private:
	int map_x;
	int map_y;
	int hp; // 敵の体力

	Stage* stage;
	int EnemyGraph;

	bool CheckCollision(int next_map_x, int next_map_y);
};