#include "DxLib.h"
#include "Player.h"
#include "Stage.h"
#include "Enemy.h"
#include "Input.h"
#include "Item.h"
#include "SceneManager.h"
#include <vector>

int bgmHandle;
int itemSEHandle;

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
    bgmHandle = LoadSoundMem("BGM/8bit21.mp3");
    ChangeVolumeSoundMem(150, bgmHandle);

    itemSEHandle = LoadSoundMem("BGM/drink01.mp3");
    ChangeVolumeSoundMem(255, itemSEHandle);


}

int WINAPI WinMain(_In_ HINSTANCE h, _In_opt_ HINSTANCE hp, _In_ LPSTR l, _In_ int n)
{
    DxInit();
    SceneManager scene;
    Player* player = new Player();
    Stage* stage = new Stage();
    std::vector<Enemy*> enemies;

    int titleGraph = LoadGraph("Assets/TITLE.png");
    int clearGraph = LoadGraph("Assets/CLEAR.png");
    int overGraph = LoadGraph("Assets/OVER.png");

    auto InitGame = [&]()
    {
            player->GetMaxHP();

            for (auto e : enemies)
            {
                delete e;
            }
            enemies.clear();

            player->SetStage(stage);
            stage->SetPlayer(player);
            stage->GenerateMap();

            if (!stage->GetRooms().empty())
            {
                auto& r = stage->GetRooms()[0];
                player->SetPosition(r.center_x, r.center_y);

                const auto& rooms = stage->GetRooms();
                for (size_t i = 1; i < rooms.size(); i++)
                {
                    for (int j = 0; j < 4; j++) 
                    {
                        E_ENEMY_TYPE randomType = (GetRand(100) < 50) ? ENEMY_SKELTON : ENEMY_SLIME;
                        Enemy* newEnemy = new Enemy(randomType);
                        newEnemy->SetStage(stage);
                        newEnemy->SetPosition(rooms[i].center_x + (j % 2), rooms[i].center_y + (j / 2));
                        enemies.push_back(newEnemy);
                    }
                }
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
            if (CheckSoundMem(bgmHandle) == 1)
            {
                StopSoundMem(bgmHandle);
            }
            DrawExtendGraph(0, 0, 1400, 700, titleGraph, FALSE);
            if (Input::IsKeyDown(KEY_INPUT_SPACE)) {
                InitGame();
                scene.SetScene(SCENE_PLAY);
            }
            break;

        case SCENE_PLAY:
            // BGMのループ再生
            if (CheckSoundMem(bgmHandle) == 0) PlaySoundMem(bgmHandle, DX_PLAYTYPE_LOOP);

            if (Input::IsKeyDown(KEY_INPUT_TAB)) isMapOverlayVisible = !isMapOverlayVisible;
            if (Input::IsKeyDown(KEY_INPUT_V)) scene.SetScene(SCENE_GAMEOVER);
            if (Input::IsKeyDown(KEY_INPUT_C)) scene.SetScene(SCENE_GAMECLEAR);

            stage->Draw();
            for (auto e : enemies) { e->Draw(); }
            player->Draw();
            player->DrawMessage();

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
                            if (e->GetHP() > 0 && nx == e->GetMapX() && ny == e->GetMapY())
                            {
                                if (e->TakeDamage(player->GetAttack())) 
                                {
                                    e->SetPosition(-100, -100);
                                    player->AddExp(10); // 敵を倒したら経験値 10 を獲得
                                }
                                else {
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
                    for (auto e : enemies) { e->Update(); }
                    isPlayerTurn = true;
                }

                //ゲームオーバー：プレイヤーのHPが0以下
                if (player->GetHP() <= 0) 
                {
                    scene.SetScene(SCENE_GAMEOVER);
                }

                //ゲームクリア：生きている敵が0
                int aliveEnemyCount = 0;
                for (auto e : enemies) 
                {
                    if (e->GetHP() > 0) aliveEnemyCount++;
                }

                if (aliveEnemyCount == 0) 
                {
                    scene.SetScene(SCENE_GAMECLEAR);
                }
            }
            else
            {
                stage->DrawOverlayMap(1400, 700);
            }

            // ステータス表示
            SetFontSize(24);
            DrawFormatString(300, 10, GetColor(255, 150, 200), "HP %d / %d", player->GetHP(), player->GetMaxHP());
            DrawFormatString(550, 10, white, "ATK %d", player->GetAttack());
            DrawFormatString(700, 10, GetColor(255, 255, 0), "LV %d (EXP %d/%d)",
                player->GetLevel(), player->GetExp(), player->GetNextExp());
            SetFontSize(16); 
            break;

        case SCENE_GAMEOVER:
            if (CheckSoundMem(bgmHandle) == 1)
            {
                StopSoundMem(bgmHandle);
            }    
            DrawExtendGraph(0, 0, 1400, 700, overGraph, FALSE);
            if (Input::IsKeyDown(KEY_INPUT_SPACE)) scene.SetScene(SCENE_TITLE);
            break;

        case SCENE_GAMECLEAR:
            if (CheckSoundMem(bgmHandle) == 1)
            {
                StopSoundMem(bgmHandle);
            }
            DrawExtendGraph(0, 0, 1400, 700, clearGraph, FALSE);
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