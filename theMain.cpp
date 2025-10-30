#include "DxLib.h"
#include "Player.h"

void DxInit()
{
	ChangeWindowMode(true);
	SetWindowSizeChangeEnableFlag(false, false);
	SetMainWindowText("Roguelike");
	SetGraphMode(1280, 720, 32);
	SetWindowSizeExtendRate(1.0);
	SetBackgroundColor(0, 0, 0);

	// ‚c‚wƒ‰ƒCƒuƒ‰ƒŠ‰Šú‰»ˆ—
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

	while (true)
	{
		ClearDrawScreen();
		//‚±‚±‚É‚â‚è‚½‚¢ˆ—‚ð‘‚­
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

	DxLib_End();
	return 0;
}