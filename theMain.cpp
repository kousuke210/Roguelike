#include "DxLib.h"
#include "Player.h"
#include "Stage.h"
#include "Enemy.h"
#include "Input.h"
#include "Item.h"
#include "SceneManager.h"
#include <vector>

void DxInit()
{
    ChangeWindowMode(true);
    SetGraphMode(1400, 700, 32);
    SetBackgroundColor(0, 0, 0);
    if (DxLib_Init() == -1)
    {
        DxLib_End();
    }
    SetDrawScreen(DX_SCREEN_BACK);
    Input::Initialize(GetMainWindowHandle());
}

int WINAPI WinMain(_In_ HINSTANCE h, _In_opt_ HINSTANCE hp, _In_ LPSTR l, _In_ int n)
{
    DxInit();
    SceneManager scene;
    Player* player = new Player();
    Stage* stage = new Stage();
    std::vector<Enemy*> enemies;

    auto InitGame = [&]()
        {
            for (auto e : enemies) { delete e; }
            enemies.clear();

            player->SetStage(stage);
            stage->SetPlayer(player);
            stage->GenerateMap();

            const auto& rooms = stage->GetRooms();
            for (size_t i = 0; i < rooms.size(); i++)
            {
                if (i == 0)
                {
                    player->SetPosition(rooms[i].center_x, rooms[i].center_y);
                }
                else
                {
                    Enemy* newEnemy = new Enemy();
                    newEnemy->SetStage(stage);
                    newEnemy->SetPosition(rooms[i].center_x, rooms[i].center_y);
                    enemies.push_back(newEnemy);
                }
            }
            stage->UpdateCamera(player->GetMapX(), player->GetMapY());
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

            if (Input::IsKeyDown(KEY_INPUT_G)) scene.SetScene(SCENE_GAMEOVER);
            if (Input::IsKeyDown(KEY_INPUT_C)) scene.SetScene(SCENE_GAMECLEAR);

            stage->Draw();
            for (auto e : enemies) { e->Draw(); }
            player->Draw();

            if (!isMapOverlayVisible)
            {
                if (isPlayerTurn)
                {
                    int dx = 0, dy = 0;
                    if (Input::IsKeyDown(KEY_INPUT_W)) dy = -1;
                    else if (Input::IsKeyDown(KEY_INPUT_S)) dy = 1;
                    else if (Input::IsKeyDown(KEY_INPUT_A)) dx = -1;
                    else if (Input::IsKeyDown(KEY_INPUT_D)) dx = 1;

                    if (dx != 0 || dy != 0) {
                        int nx = player->GetMapX() + dx;
                        int ny = player->GetMapY() + dy;

                        bool attacked = false;
                        for (auto e : enemies)
                        {
                            // ¶‚«‚Ä‚¢‚é“GiÀ•W‚ª—LŒø‚È“Gj‚Æ‚ÌÕ“Ë”»’è‚ðŒµ–§‰»
                            if (e->GetHP() > 0 && nx == e->GetMapX() && ny == e->GetMapY())
                            {
                                if (e->TakeDamage(player->GetAttack()))
                                {
                                    // “|‚µ‚½“G‚Í”»’èŠO‚É”ò‚Î‚·
                                    e->SetPosition(-100, -100);
                                }
                                else
                                {
                                    player->Heal(-5);
                                }
                                attacked = true;
                                isPlayerTurn = false;
                                break;
                            }
                        }

                        if (!attacked && player->Update())
                        {
                            stage->UpdateCamera(player->GetMapX(), player->GetMapY());
                            stage->GetItemManager()->PickUpItem(player->GetMapX(), player->GetMapY(), player);
                            isPlayerTurn = false;
                        }
                    }
                }
                else
                {
                    // yuŠÔˆÚ“®‘Îôz“G‚Ìƒ^[ƒ“‚É‚È‚Á‚½‚çA‘SŒÂ‘Ì‚ð1‰ñ‚¸‚Â‚¾‚¯XV‚µ‚Ä‘¦À‚ÉƒvƒŒƒCƒ„[‚Ìƒ^[ƒ“‚É–ß‚·
                    for (auto e : enemies)
                    {
                        e->Update();
                    }
                    isPlayerTurn = true;
                }

                if (player->GetHP() <= 0) {
                    scene.SetScene(SCENE_GAMEOVER);
                }
            }
            else
            {
                stage->DrawOverlayMap(1400, 700);
            }

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

    delete player;
    for (auto e : enemies) 
    { 
        delete e; 
    }
    enemies.clear();
    delete stage;
    Input::Release();
    DxLib_End();
    return 0;
}