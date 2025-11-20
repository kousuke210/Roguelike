#include "DxLib.h"
#include "Player.h"
#include "Stage.h"

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
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	DxInit();
	Player* player = new Player();
	Stage* stage = new Stage();

	player->SetStage(stage);

	stage->GenerateMap();

	// 生成された最初の部屋の中心にプレイヤーを移動
	if (!stage->GetRooms().empty())
	{
		const Stage::Room& startRoom = stage->GetRooms()[0];

		// 【変更】SetPositionにマス座標を渡す
		player->SetPosition(startRoom.center_x, startRoom.center_y);
	}

	// 【追加】ターン管理用のフラグを導入
	bool isPlayerTurn = true;

	while (true)
	{
		ClearDrawScreen();

		stage->Draw();
		player->Draw();

		if (isPlayerTurn)
		{
			// プレイヤーの行動。Update()がtrueを返したら行動完了
			if (player->Update())
			{
				isPlayerTurn = false; // プレイヤーのターン終了
			}
		}
		else
		{
			// エネミーなどの行動フェーズ
			//isPlayerTurn = true;
		}

		ScreenFlip();
		WaitTimer(16);

		if (ProcessMessage() == -1)
			break;
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)
			break;
	}
	delete player;
	delete stage;
	DxLib_End();
	return 0;
}