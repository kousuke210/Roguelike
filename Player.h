#pragma once

class Player{
public:
	Player();
	~Player();
	void Update();
	void Draw();
private:
	bool isDash; //true:ダッシュできる false:ダッシュできない
	bool isDodge; //true:回避できる false:回避できない（クールタイムorスタミナ）
	bool isMoving; //true:移動中 false:停止中

	float Stamina;
	float DashSpeed;
	float moveSpeed;
	float CoolTime;
	float DashTime;

	void Dash();  //ダッシュ
	void Dodge(); //回避
};