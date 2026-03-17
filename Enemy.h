#pragma once

class Stage;

enum E_ENEMY_TYPE {
	ENEMY_SKELTON,
	ENEMY_SLIME, // V‚µ‚¢“G
	ENEMY_MAX
};

class Enemy
{
public:
	Enemy(E_ENEMY_TYPE type);
	~Enemy();
	bool Update();
	void Draw();
	void SetPosition(int map_x, int map_y);
	void SetStage(Stage* stage) { this->stage = stage; }
	int GetMapX() const { return map_x; }
	int GetMapY() const { return map_y; }

	int GetHP() const { return hp; }
	// ƒ_ƒ[ƒW‚ğó‚¯A“|‚³‚ê‚½‚ç true ‚ğ•Ô‚·
	bool TakeDamage(int damage) 
	{
		hp -= damage;
		return hp <= 0;
	}

private:
	int map_x;
	int map_y;
	int hp; // “G‚Ì‘Ì—Í
	E_ENEMY_TYPE type;

	Stage* stage;
	int EnemyGraph;

	bool CheckCollision(int next_map_x, int next_map_y);
};