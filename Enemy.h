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
private:
	int map_x;
	int map_y;

	Stage* stage;

	bool CheckCollision(int next_map_x, int next_map_y);
};

