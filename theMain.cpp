#include "DxLib.h"
#include "Player.h"
#include "Stage.h"
#include "Enemy.h"
#include "Input.h"
#include "Item.h"
#include "SceneManager.h"

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
    SceneManager scene;
    Player* player = new Player();
    Enemy* enemy = new Enemy();
    Stage* stage = new Stage();

    auto InitGame = [&]() {
        player->SetStage(stage);
        stage->SetPlayer(player);
        stage->GenerateMap();
        if (!stage->GetRooms().empty()) {
            auto& r = stage->GetRooms()[0];
            player->SetPosition(r.center_x, r.center_y);
            enemy->SetPosition(r.center_x + 2, r.center_y);
            enemy->SetStage(stage);
            stage->UpdateCamera(player->GetMapX(), player->GetMapY());
        }
        };

    InitGame();

    bool isPlayerTurn = true;
    bool isMapOverlayVisible = false;

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        Input::Update();
        ClearDrawScreen();

        int white = GetColor(255, 255, 255);

        switch (scene.GetScene())
        {
        case SCENE_TITLE:
            DrawString(600, 300, "TITLE SCREEN", white);
            DrawString(550, 350, "Press SPACE to Start", white);
            if (Input::IsKeyDown(KEY_INPUT_SPACE)) {
                InitGame();
                scene.SetScene(SCENE_PLAY);
            }
            break;

        case SCENE_PLAY:
            if (Input::IsKeyDown(KEY_INPUT_TAB)) isMapOverlayVisible = !isMapOverlayVisible;

            // 強制遷移デバッグ用
            if (Input::IsKeyDown(KEY_INPUT_G)) scene.SetScene(SCENE_GAMEOVER);
            if (Input::IsKeyDown(KEY_INPUT_C)) scene.SetScene(SCENE_GAMECLEAR);

            stage->Draw();
            player->Draw();
            enemy->Draw();

            if (!isMapOverlayVisible) {
                if (isPlayerTurn) {
                    if (player->Update()) {
                        stage->UpdateCamera(player->GetMapX(), player->GetMapY());
                        stage->GetItemManager()->PickUpItem(player->GetMapX(), player->GetMapY(), player);
                        isPlayerTurn = false;
                    }
                }
                else {
                    if (enemy->Update()) isPlayerTurn = true;
                }
            }
            else {
                stage->DrawOverlayMap(1400, 700);
            }

            // UI
            DrawFormatString(300, 10, GetColor(255, 150, 200), "HP %d / %d", player->GetHP(), player->GetMaxHP());
            DrawFormatString(500, 10, white, "ATK %d", player->GetAttack());
            break;

        case SCENE_GAMEOVER:
            DrawString(600, 300, "GAME OVER", GetColor(255, 0, 0));
            DrawString(550, 350, "Press SPACE to Title", white);
            if (Input::IsKeyDown(KEY_INPUT_SPACE)) scene.SetScene(SCENE_TITLE);
            break;

        case SCENE_GAMECLEAR:
            DrawString(600, 300, "GAME CLEAR!", GetColor(255, 255, 0));
            DrawString(550, 350, "Press SPACE to Title", white);
            if (Input::IsKeyDown(KEY_INPUT_SPACE)) scene.SetScene(SCENE_TITLE);
            break;
        }

        ScreenFlip();
        WaitTimer(16);
    }

    delete player; delete enemy; delete stage;
    Input::Release(); DxLib_End(); return 0;
}