#include "DxLib.h"
#include "Player.h"
#include "Stage.h"
#include "Enemy.h"
#include "Input.h"
#include "Item.h"
#include "SceneManager.h"
#include <vector>
#include <string>

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
    int currentFloor = 1;
    static bool stairsFoundMsg = false;
    int enemyClearTimer = 0;
    int turnWaitTimer = 0;
    const int TURN_DELAY = 12;

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
            DrawExtendGraph(0, 0, 1400, 700, titleGraph, FALSE);
            if (Input::IsKeyDown(KEY_INPUT_SPACE))
            {
                player->Heal(player->GetMaxHP());
                player->clairvoyanceTurn = 0;
                stage->ResetFloor();
                stage->GenerateMap();
                for (auto e : enemies) delete e;
                enemies.clear();
                stage->SpawnEnemies(enemies);
                player->SetPosition(stage->GetStartIdxX(), stage->GetStartIdxY());
                scene.SetScene(SCENE_PLAY);
                PlaySoundMem(bgmHandle, DX_PLAYTYPE_LOOP);
            }
            break;

        case SCENE_PLAY:
            if (CheckSoundMem(bgmHandle) == 0) PlaySoundMem(bgmHandle, DX_PLAYTYPE_LOOP);

            if (Input::IsKeyDown(KEY_INPUT_TAB)) isMapOverlayVisible = !isMapOverlayVisible;
            if (Input::IsKeyDown(KEY_INPUT_V)) scene.SetScene(SCENE_GAMEOVER);
            if (Input::IsKeyDown(KEY_INPUT_C)) scene.SetScene(SCENE_GAMECLEAR);

            stage->Draw();
            for (auto e : enemies)
            {
                if (stage->GetCurrentFloor() % 5 == 0) player->clairvoyanceTurn = 2;
                if (stage->IsTileVisible(e->GetMapX(), e->GetMapY()) || player->clairvoyanceTurn > 0)
                {
                    e->Draw();
                }
            }
            player->Draw();
            player->DrawMessage();

            if (!isMapOverlayVisible)
            {
                if (turnWaitTimer > 0)
                {
                    turnWaitTimer--;
                }
                else if (isPlayerTurn)
                {
                    if (player->ProcessInput(enemies))
                    {
                        isPlayerTurn = false;
                        turnWaitTimer = TURN_DELAY;
                    }
                }
                else
                {
                    for (auto e : enemies) { e->Update(); }
                    isPlayerTurn = true;
                    turnWaitTimer = TURN_DELAY;
                }

                // ゲームオーバー判定
                if (player->GetHP() <= 0) scene.SetScene(SCENE_GAMEOVER);

                bool anyEnemyAlive = false;
                for (auto e : enemies) if (e->GetHP() > 0) { anyEnemyAlive = true; break; }

                if (!anyEnemyAlive && !stairsFoundMsg) {
                    player->ShowPickUpMessage("敵を殲滅した！階段の場所が判明！");
                    stage->SetExplored(stage->GetStairsX(), stage->GetStairsY());
                    stairsFoundMsg = true;
                    enemyClearTimer = 180;
                }

                if (enemyClearTimer > 0) {
                    if (--enemyClearTimer == 0) player->ShowPickUpMessage("");
                }
            }
            else
            {
                stage->DrawOverlayMap(1400, 700);
            }

            // --- ボスのHPバー表示 ---
            if (stage->GetCurrentFloor() % 5 == 0)
            {
                for (auto e : enemies)
                {
                    if (e && e->GetHP() > 0)
                    {
                        int barWidth = 800;
                        int startX = (1400 - barWidth) / 2;
                        DrawBox(startX, 50, startX + barWidth, 80, GetColor(50, 50, 50), TRUE);
                        float hpRate = (float)e->GetHP() / 100.0f; // 100はボスの最大HP
                        DrawBox(startX, 50, startX + (int)(barWidth * hpRate), 80, GetColor(255, 0, 0), TRUE);
                        break;
                    }
                }
            }

            // ステータス表示
            SetFontSize(24);
            DrawFormatString(300, 10, GetColor(255, 150, 200), "HP %d / %d", player->GetHP(), player->GetMaxHP());
            DrawFormatString(550, 10, white, "ATK %d", player->GetAttack());
            DrawFormatString(700, 10, GetColor(255, 255, 0), "LV %d (EXP %d/%d)", player->GetLevel(), player->GetExp(), player->GetNextExp());
            DrawFormatString(10, 10, GetColor(255, 255, 255), "B%dF", stage->GetCurrentFloor());
            SetFontSize(16);
            break;

        case SCENE_GAMEOVER:
        case SCENE_GAMECLEAR:
            if (CheckSoundMem(bgmHandle) == 1) StopSoundMem(bgmHandle);
            DrawExtendGraph(0, 0, 1400, 700, (scene.GetScene() == SCENE_GAMEOVER ? overGraph : clearGraph), FALSE);
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