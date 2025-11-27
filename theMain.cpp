#include "DxLib.h"
#include "Player.h"
#include "Stage.h"
#include "Enemy.h"
#include "Input.h"

void DxInit()
{
	ChangeWindowMode(true);
	SetWindowSizeChangeEnableFlag(false, false);
	SetMainWindowText("Roguelike");
	SetGraphMode(1600, 900, 32);
	SetWindowSizeExtendRate(1.0);
	SetBackgroundColor(0, 0, 0);

	// ＤＸライブラリ初期化処理
	if (DxLib_Init() == -1)
	{
		DxLib_End();
	}

	SetDrawScreen(DX_SCREEN_BACK);

	Input::Initialize(GetMainWindowHandle());
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	DxInit();
	Player* player = new Player();
	Enemy* enemy = new Enemy(); // 【追加】Enemyのインスタンス化
	Stage* stage = new Stage();

	player->SetStage(stage);
	// 【追加】StageにPlayerへのポインタを設定 (Enemyの衝突判定に使用)
	stage->SetPlayer(player);

	stage->GenerateMap();

	// 生成された最初の部屋の中心にプレイヤーとエネミーを移動
	if (!stage->GetRooms().empty())
	{
		const Stage::Room& startRoom = stage->GetRooms()[0];

		// 【変更】SetPositionにマス座標を渡す
		player->SetPosition(startRoom.center_x, startRoom.center_y);

		// 【追加】エネミーをプレイヤーと同じ部屋の少しずれた位置に配置
		enemy->SetPosition(startRoom.center_x + 2, startRoom.center_y);
		enemy->SetStage(stage); // StageをEnemyに設定
	}

	bool isPlayerTurn = true; // ターン管理用のフラグ

	while (true)
	{
		// 【追加】毎フレームのInputモジュール更新
		Input::Update();

		ClearDrawScreen();

		stage->Draw();
		player->Draw();
		// 【追加】エネミーの描画
		enemy->Draw();

		if (isPlayerTurn)
		{
			// プレイヤーの行動。Update()がtrueを返したら行動完了
			// Player::Update()内で Input::IsKeyDown() を使用
			if (player->Update())
			{
				isPlayerTurn = false; // プレイヤーのターン終了
			}
		}
		else
		{
			// エネミーの行動フェーズ
			// エネミーの行動。Update()がtrueを返したら行動完了
			if (enemy->Update())
			{
				isPlayerTurn = true; // エネミーのターン終了、プレイヤーのターンへ
			}
		}

		ScreenFlip();
		WaitTimer(16);

		if (ProcessMessage() == -1)
			break;
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)
			break;
	}
	delete player;
	delete enemy;
	delete stage;
	Input::Release();

	DxLib_End();
	return 0;
}