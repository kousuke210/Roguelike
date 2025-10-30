#pragma once

class Player{
public:
	Player();
	~Player();
	void Update();
	void Draw();
private:
	bool isDash; //true:�_�b�V���ł��� false:�_�b�V���ł��Ȃ�
	bool isDodge; //true:����ł��� false:����ł��Ȃ��i�N�[���^�C��or�X�^�~�i�j
	bool isMoving; //true:�ړ��� false:��~��

	float Stamina;
	float DashSpeed;
	float moveSpeed;
	float CoolTime;
	float DashTime;

	void Dash();  //�_�b�V��
	void Dodge(); //���
};