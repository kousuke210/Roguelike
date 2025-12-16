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
	stage->SetPlayer(player);

	stage->GenerateMap();

	if (!stage->GetRooms().empty())
	{
		const Stage::Room& startRoom = stage->GetRooms()[0];

		// SetPositionにマス座標を渡す
		player->SetPosition(startRoom.center_x, startRoom.center_y);

		// エネミーをプレイヤーと同じ部屋の少しずれた位置に配置
		enemy->SetPosition(startRoom.center_x + 2, startRoom.center_y);
		enemy->SetStage(stage);

		// 【修正】初期配置後、一度カメラを更新してプレイヤーの初期位置に画面を合わせる (FoVの初期化も兼ねる)
		stage->UpdateCamera(player->GetMapX(), player->GetMapY());
	}

	bool isPlayerTurn = true; // ターン管理用のフラグ
	// 【追加】マップオーバーレイ表示フラグ
	bool isMapOverlayVisible = false;

	while (true)
	{
		// 毎フレームのInputモジュール更新
		Input::Update();

		// 【追加】Tabキーでマップ表示をトグルする
		if (Input::IsKeyDown(KEY_INPUT_TAB))
		{
			isMapOverlayVisible = !isMapOverlayVisible;
		}

		ClearDrawScreen();

		// ゲーム画面の描画
		stage->Draw();
		player->Draw();
		enemy->Draw();

		if (!isMapOverlayVisible) // マップオーバーレイが表示されていない場合のみゲームを進行
		{
			if (isPlayerTurn)
			{
				// プレイヤーの行動。Update()がtrueを返したら行動完了
				if (player->Update())
				{
					// 【修正】プレイヤーの移動後、カメラを更新 (FoV更新も含む)
					stage->UpdateCamera(player->GetMapX(), player->GetMapY());

					isPlayerTurn = false; // プレイヤーのターン終了
				}
			}
			else
			{
				// エネミーの行動フェーズ
				if (enemy->Update())
				{
					isPlayerTurn = true; // エネミーのターン終了、プレイヤーのターンへ
				}
			}
		}

		// 【追加】マップオーバーレイの描画
		if (isMapOverlayVisible)
		{
			const int SCREEN_WIDTH = 1600;
			const int SCREEN_HEIGHT = 900;
			stage->DrawOverlayMap(SCREEN_WIDTH, SCREEN_HEIGHT);
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