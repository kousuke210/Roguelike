#include "DxLib.h"
#include "Player.h"
#include <cmath>

Player::Player()
	: 
	x(1280.0f / 2.0f),  // ��ʒ����ɏ����z�u
	y(720.0f / 2.0f),
	isDash(false),
	isDodge(false),
	isMoving(false),
	Stamina(MAX_STAMINA),
	dashCoolTime(0),
	dodgeDuration(0)
{
}

Player::~Player()
{
}

void Player::Move()
{
	float speed = isDash ? DashSpeed : moveSpeed;
	float dx = 0.0f;
	float dy = 0.0f;
	isMoving = false;

	if (CheckHitKey(KEY_INPUT_W)) // ��
	{
		dy -= speed; isMoving = true;
	} 
	if (CheckHitKey(KEY_INPUT_S)) // ��
	{ 
		dy += speed; isMoving = true; 
	}
	if (CheckHitKey(KEY_INPUT_A)) // ��
	{ 
		dx -= speed; isMoving = true; 
	} 
	if (CheckHitKey(KEY_INPUT_D)) // �E
	{ 
		dx += speed; isMoving = true; 
	} 

	// �΂߈ړ��̑��x����
	if (dx != 0.0f && dy != 0.0f) {
		float diag_speed_rate = 0.7071f; 
		dx *= diag_speed_rate;
		dy *= diag_speed_rate;
	}

	x += dx;
	y += dy;

	// �X�^�~�i�̉�
	if (!isDash && dashCoolTime == 0 && Stamina < MAX_STAMINA) {
		Stamina += STAMINA_RECOVER_RATE;
		if (Stamina > MAX_STAMINA) Stamina = MAX_STAMINA;
	}
}

void Player::Dash()
{
	// �N�[���^�C�����̓_�b�V���s��
	if (dashCoolTime > 0)
	{
		dashCoolTime--;
		isDash = false;
		return;
	}

	// LShift��������Ă��āA�ړ����A���X�^�~�i�����邩�`�F�b�N
	if (CheckHitKey(KEY_INPUT_LSHIFT) && isMoving && Stamina > 0.0f)
	{
		isDash = true;
		Stamina -= STAMINA_CONSUME_DASH;

		if (Stamina < 0.0f)
		{
			Stamina = 0.0f;
		}

		// �X�^�~�i���s������N�[���^�C���J�n
		if (Stamina <= 0.0f)
		{
			// �X�^�~�i���s�����ꍇ���A�_�b�V���𑱂��ăN�[���^�C���Ɉڍs
			Stamina = 0.0f;
			isDash = false; // �_�b�V���������I��
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
	// 1. ����̎������Ԓ��̏����i�ړ��𑱂���j
	if (dodgeDuration > 0)
	{
		dodgeDuration--;
		isDodge = true;

		// �y�ǉ��z��𒆂̈ړ�����
		float dx = 0.0f;
		float dy = 0.0f;

		// W/A/S/D �L�[��������Ă���������擾
		if (CheckHitKey(KEY_INPUT_W)) dy -= 1.0f;
		if (CheckHitKey(KEY_INPUT_S)) dy += 1.0f;
		if (CheckHitKey(KEY_INPUT_A)) dx -= 1.0f;
		if (CheckHitKey(KEY_INPUT_D)) dx += 1.0f;

		// �ړ����������͂���Ă����ꍇ
		if (dx != 0.0f || dy != 0.0f) {
			// �ړ��x�N�g���𐳋K��
			float length = sqrtf(dx * dx + dy * dy);
			if (length > 0.0f) {
				dx /= length;
				dy /= length;
			}

			// ���W���X�V
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
		Stamina -= STAMINA_CONSUME_DODGE; // ����ŃX�^�~�i����

		// �����̃N�[���^�C����ݒ�
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
		color = GetColor(255, 0, 0); // �_�b�V�����͐�
	}
	else if (isDodge) {
		color = GetColor(0, 255, 255); // ��𒆂̓V�A��
	}

	DrawCircleAA(x, y, 20.0f, 64, color, TRUE);

	// ��ԂƃX�^�~�i�̕\���i�f�o�b�O�p�j
	DrawFormatString(10, 10, GetColor(255, 255, 255), "Stamina: %.1f / %.1f", Stamina, MAX_STAMINA);
	DrawFormatString(10, 30, GetColor(255, 255, 255), "Dash CoolTime: %d", dashCoolTime);
	DrawFormatString(10, 50, GetColor(255, 255, 255), "Dodge Duration: %d", dodgeDuration);
}