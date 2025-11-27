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
	Enemy* enemy = new Enemy();
	Stage* stage = new Stage();

	player->SetStage(stage);
	// StageにPlayerへのポインタを設定 (Enemyの衝突判定に使用)
	stage->SetPlayer(player);

	stage->GenerateMap();

	// 生成された最初の部屋の中心にプレイヤーとエネミーを移動
	if (!stage->GetRooms().empty())
	{
		const Stage::Room& startRoom = stage->GetRooms()[0];

		// SetPositionにマス座標を渡す
		player->SetPosition(startRoom.center_x, startRoom.center_y);

		// エネミーをプレイヤーと同じ部屋の少しずれた位置に配置
		enemy->SetPosition(startRoom.center_x + 2, startRoom.center_y);
		enemy->SetStage(stage);

		// 【追加】初期配置後、一度カメラを更新してプレイヤーの初期位置に画面を合わせる
		stage->UpdateCamera(player->GetMapX(), player->GetMapY());
	}

	bool isPlayerTurn = true; // ターン管理用のフラグ

	while (true)
	{
		// 毎フレームのInputモジュール更新
		Input::Update();

		ClearDrawScreen();

		stage->Draw();
		player->Draw();
		// エネミーの描画
		enemy->Draw();

		if (isPlayerTurn)
		{
			// プレイヤーの行動。Update()がtrueを返したら行動完了
			if (player->Update())
			{
				// 【追加】プレイヤーの移動後、カメラを更新
				stage->UpdateCamera(player->GetMapX(), player->GetMapY());

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