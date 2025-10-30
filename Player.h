#pragma once

class Player {
public:
	Player();
	~Player();
	void Update();
	void Draw();
private:
	//�v���C���[�̈ʒu���W
	float x;
	float y;

	//��ԊǗ�
	bool isDash; // true:�_�b�V���� false:�ʏ���
	bool isDodge; // true:��� false:��𒆂łȂ�
	bool isMoving; // true:�ړ��L�[��������Ă��� false:��~

	//�p�����[�^
	float Stamina; // ���݂̃X�^�~�i
	static constexpr float MAX_STAMINA = 100.0f; // �X�^�~�i�̍ő�l
	const float STAMINA_CONSUME_DASH = 1.0f; // 1�t���[��������̃_�b�V������X�^�~�i
	const float STAMINA_CONSUME_DODGE = 30.0f; // ����ŏ����X�^�~�i
	const float STAMINA_RECOVER_RATE = 0.5f; // 1�t���[��������̃X�^�~�i�񕜗�

	//�ړ����x
	const float moveSpeed = 3.0f;    // �ʏ�̈ړ����x
	const float DashSpeed = 6.0f;    // �_�b�V�����̈ړ����x

	int dashCoolTime;        // �_�b�V����̃N�[���^�C���i�t���[���j
	int dodgeDuration;       // ����̎������ԁi�t���[���j
	const int MAX_COOLTIME = 60; // �_�b�V���E�����̃N�[���^�C���ő�l (60�t���[��=1�b)
	const int MAX_DODGETIME = 5; // �y�C���z����̎������ԍő�l (20�t���[�� -> 5�t���[���ɕύX)

	const float DODGE_SPEED = 15.0f; // �������1�t���[��������̈ړ����x

	void Dash();
	void Dodge();
	void Move();
};