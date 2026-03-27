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
                // プレイヤーのHPと状態をリセット
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
            // BGMのループ再生
            if (CheckSoundMem(bgmHandle) == 0) PlaySoundMem(bgmHandle, DX_PLAYTYPE_LOOP);

            if (Input::IsKeyDown(KEY_INPUT_TAB)) isMapOverlayVisible = !isMapOverlayVisible;
            if (Input::IsKeyDown(KEY_INPUT_V)) scene.SetScene(SCENE_GAMEOVER);
            if (Input::IsKeyDown(KEY_INPUT_C)) scene.SetScene(SCENE_GAMECLEAR);

            stage->Draw();
            for (auto e : enemies)
            {
                if (stage->GetCurrentFloor() % 5 == 0)
                {
                    player->clairvoyanceTurn = 2;
                }
                // 千里眼の効果中なら、視界に関係なく敵を描画する
                if (stage->IsTileVisible(e->GetMapX(), e->GetMapY()) || player->clairvoyanceTurn > 0)
                {
                    e->Draw();
                }
            }
            player->Draw();
            player->DrawMessage();

            if (!isMapOverlayVisible)
            {
                if (isPlayerTurn)
                {
                    // Eキーで攻撃
                    if (Input::IsKeyDown(KEY_INPUT_E))
                    {
                        bool acted = false;
                        for (auto it = enemies.begin(); it != enemies.end(); )
                        {
                            Enemy* e = *it;
                            if (e && e->GetHP() > 0)
                            {
                                int ex = e->GetMapX();
                                int ey = e->GetMapY();
                                int px = player->GetMapX();
                                int py = player->GetMapY();

                                bool isHit = false;

                                // ボス階なら少し広い判定（2.5マス以内）
                                if (stage->GetCurrentFloor() % 5 == 0)
                                {
                                    float dx = abs((ex + 0.5f) - (px + 0.5f));
                                    float dy = abs((ey + 0.5f) - (py + 0.5f));
                                    if (dx <= 2.5f && dy <= 2.5f) isHit = true;
                                }
                                else
                                {
                                    // 通常の敵は隣接1マス以内
                                    int dist = abs(ex - px) + abs(ey - py);
                                    if (dist <= 1) isHit = true;
                                }

                                if (isHit)
                                {
                                    if (e->TakeDamage(player->GetAttack()))
                                    {
                                        // 撃破処理
                                        int floor = stage->GetCurrentFloor();
                                        int gainExp = 10 + (floor * 5);
                                        player->AddExp(gainExp);
                                        player->ShowPickUpMessage((std::to_string(gainExp) + " EXP獲得").c_str());

                                        delete e;
                                        it = enemies.erase(it); // リストから削除
                                        acted = true;
                                        continue;
                                    }
                                    acted = true; // ダメージを与えたら行動成立
                                }
                            }
                            it++;
                        }

                        if (acted)
                        {
                            isPlayerTurn = false; // 攻撃したらターン終了
                            player->UpdateTurn(); // バフのターン更新
                        }
                    }
                    int dx = 0, dy = 0;
                    if (Input::IsKeyDown(KEY_INPUT_W)) dy = -1;
                    else if (Input::IsKeyDown(KEY_INPUT_S)) dy = 1;
                    else if (Input::IsKeyDown(KEY_INPUT_A)) dx = -1;
                    else if (Input::IsKeyDown(KEY_INPUT_D)) dx = 1;

                    if (CheckHitKey(KEY_INPUT_B) == 1)
                    {
                        int sx = stage->GetStairsX();
                        int sy = stage->GetStairsY();

                        // 階段の地点を「探索済み」フラグに書き換える
                        stage->SetExplored(sx, sy);

                        // 階段を発見したフラグを立てる
                        stairsFoundMsg = true;
                    }

                    if (isPlayerTurn)
                    {
                        if (Input::IsKeyDown(KEY_INPUT_E))
                        {
                            bool hitAny = false;
                            for (auto it = enemies.begin(); it != enemies.end(); )
                            {
                                Enemy* e = *it;
                                if (e && e->GetHP() > 0)
                                {
                                    float dx = abs((e->GetMapX() + 0.5f) - (player->GetMapX() + 0.5f));
                                    float dy = abs((e->GetMapY() + 0.5f) - (player->GetMapY() + 0.5f));

                                    bool isHit = false;
                                    if (stage->GetCurrentFloor() % 5 == 0) 
                                    {
                                        if (dx <= 2.5f && dy <= 2.5f) isHit = true;
                                    }
                                    else 
                                    { 
                                        if (dx + dy <= 1.5f) isHit = true;
                                    }

                                    if (isHit) {
                                        hitAny = true;
                                        if (e->TakeDamage(player->GetAttack())) 
                                        {
                                            int gainExp = 10 + (stage->GetCurrentFloor() * 5);
                                            player->AddExp(gainExp);
                                            player->ShowPickUpMessage((std::to_string(gainExp) + " EXP獲得").c_str());
                                            delete e;
                                            it = enemies.erase(it);
                                            continue;
                                        }
                                    }
                                }
                                it++;
                            }
                            if (hitAny) {
                                player->UpdateTurn();
                                isPlayerTurn = false;
                            }
                        }

                        int dx = 0, dy = 0;
                        if (Input::IsKeyDown(KEY_INPUT_W))      dy = -1;
                        else if (Input::IsKeyDown(KEY_INPUT_S)) dy = 1;
                        else if (Input::IsKeyDown(KEY_INPUT_A)) dx = -1;
                        else if (Input::IsKeyDown(KEY_INPUT_D)) dx = 1;

                        if (dx != 0 || dy != 0) 
                        {
                            int nx = player->GetMapX() + dx;
                            int ny = player->GetMapY() + dy;

                            if (stage->CanMoveTo(nx, ny)) 
                            {
                                player->SetPosition(nx, ny);
                                stage->UpdateCamera(nx, ny);
                                stage->GetItemManager()->PickUpItem(nx, ny, player);

                                if (stage->GetTileType(nx, ny) == TILE_STAIRS) 
                                {
                                    stage->AdvanceFloor();
                                    stage->GenerateMap();
                                    for (auto e : enemies) delete e;
                                    enemies.clear();
                                    stage->SpawnEnemies(enemies);
                                    player->SetPosition(stage->GetStartIdxX(), stage->GetStartIdxY());
                                    player->ShowPickUpMessage("階段を下りた...");
                                }
                                player->UpdateTurn();
                                isPlayerTurn = false;
                            }
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

                bool anyEnemyAlive = false;
                for (auto e : enemies)
                {
                    if (e->GetHP() > 0)
                    {
                        anyEnemyAlive = true;
                        break;
                    }
                }

                if (!anyEnemyAlive) {
                    if (!stairsFoundMsg) 
                    { // まだメッセージを出していないなら
                        player->ShowPickUpMessage("敵を殲滅した！階段の場所が判明！");

                        int sx = stage->GetStairsX();
                        int sy = stage->GetStairsY();
                        stage->SetExplored(sx, sy);

                        stairsFoundMsg = true;
                        enemyClearTimer = 180; // 3秒タイマー
                    }
                }

                if (enemyClearTimer > 0)
                {
                    enemyClearTimer--;
                    if (enemyClearTimer == 0)
                    {
                        // 3秒経ったら、表示されているログを消す
                        player->ShowPickUpMessage("");
                    }
                }
            }
            else
            {
                stage->DrawOverlayMap(1400, 700);
            }

            // ステータス関連表示
            if (stage->GetCurrentFloor() % 5 == 0)
            {
                for (auto e : enemies)
                {
                    if (e && e->GetHP() > 0)
                    {
                        int curHp = e->GetHP();
                        int maxHp = 100; //後でボスごとの変数に

                        int barWidth = 800;
                        int barHeight = 30;
                        int startX = (1400 - barWidth) / 2;
                        int startY = 50;

                        DrawBox(startX, startY, startX + barWidth, startY + barHeight, GetColor(50, 50, 50), TRUE);

                        float hpRate = (float)curHp / maxHp;
                        int redWidth = (int)(barWidth * hpRate);
                        DrawBox(startX, startY, startX + redWidth, startY + barHeight, GetColor(255, 0, 0), TRUE);

                        //SetFontSize(20);
                        //DrawString(startX, startY - 25, "GOLEM", GetColor(255, 255, 255));
                        break;
                    }
                }
            }
            SetFontSize(24);
            DrawFormatString(300, 10, GetColor(255, 150, 200), "HP %d / %d", player->GetHP(), player->GetMaxHP());
            DrawFormatString(550, 10, white, "ATK %d", player->GetAttack());
            DrawFormatString(700, 10, GetColor(255, 255, 0), "LV %d (EXP %d/%d)",
                player->GetLevel(), player->GetExp(), player->GetNextExp());
            DrawFormatString(10, 10, GetColor(255, 255, 255), "B%dF", stage->GetCurrentFloor());
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