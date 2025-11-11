#include "DxLib.h"
#include "Player.h"
#include "Stage.h" // 【追記】Stageクラスを使うためにインクルード

void DxInit()
{
	ChangeWindowMode(true);
	SetWindowSizeChangeEnableFlag(false, false);
	SetMainWindowText("Roguelike");
	SetGraphMode(1280, 720, 32);
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
	Stage* stage = new Stage(); // 【追記】Stageオブジェクトを作成

	// 【追加】ダンジョン生成とプレイヤー初期位置設定
	stage->GenerateMap();

	// 生成された最初の部屋の中心にプレイヤーを移動
	if (!stage->GetRooms().empty())
	{
		const Stage::Room& startRoom = stage->GetRooms()[0];

		// マス座標 * タイルサイズ + タイルサイズの半分 でピクセル座標を計算
		float player_x = (float)(startRoom.center_x * stage->GetTileSize() + stage->GetTileSize() / 2.0f);
		float player_y = (float)(startRoom.center_y * stage->GetTileSize() + stage->GetTileSize() / 2.0f);

		player->SetPosition(player_x, player_y);
	}

	while (true)
	{
		ClearDrawScreen();
		//ここにやりたい処理を書く
		stage->Draw(); // 【追記】マップを描画
		player->Update();
		player->Draw();

		ScreenFlip();
		WaitTimer(16);

		if (ProcessMessage() == -1)
			break;
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)
			break;
	}
	delete player;
	delete stage; // 【追記】Stageオブジェクトを解放
	DxLib_End();
	return 0;
}