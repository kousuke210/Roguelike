#include "DxLib.h"
#include "Player.h"
#include "Stage.h"
#include "Enemy.h"
#include "Input.h"

void DxInit() 
{
	ChangeWindowMode(true);
	SetGraphMode(1400, 700, 32);
	SetBackgroundColor(0, 0, 0);
	if (DxLib_Init() == -1) DxLib_End();
	SetDrawScreen(DX_SCREEN_BACK);
	Input::Initialize(GetMainWindowHandle());
}

int WINAPI WinMain(_In_ HINSTANCE h, _In_opt_ HINSTANCE hp, _In_ LPSTR l, _In_ int n) 
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
		auto& r = stage->GetRooms()[0];
		player->SetPosition(r.center_x, r.center_y);
		enemy->SetPosition(r.center_x + 2, r.center_y); enemy->SetStage(stage);
		stage->UpdateCamera(player->GetMapX(), player->GetMapY());
	}

	bool isPlayerTurn = true;
	bool isMapOverlayVisible = false;

	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0) 
	{
		Input::Update();
		if (Input::IsKeyDown(KEY_INPUT_TAB)) isMapOverlayVisible = !isMapOverlayVisible;

		ClearDrawScreen();
		stage->Draw(); player->Draw(); enemy->Draw();

		if (!isMapOverlayVisible) 
		{
			if (isPlayerTurn) 
			{ 
				if (player->Update()) 
				{ 
					stage->UpdateCamera(player->GetMapX(), player->GetMapY()); isPlayerTurn = false; 
				}
			}
			else 
			{ 
				if (enemy->Update()) isPlayerTurn = true;
			}
		}
		else 
		{
			stage->DrawOverlayMap(1400, 700);
		}

		ScreenFlip();
		WaitTimer(16);
	}
	delete player; delete enemy; delete stage;
	Input::Release(); DxLib_End(); return 0;
}