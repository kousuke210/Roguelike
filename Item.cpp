#include "Item.h"
#include "Stage.h"
#include "Player.h"
#include <algorithm>

ItemManager::ItemManager() 
{
    // 画像の読み込み
    potionAtkGraph = LoadGraph("Assets/POTION_ATK.png");
    potionHealGraph = LoadGraph("Assets/POTION_HEAL.png");
    torchGraph = LoadGraph("Assets/TOURCH.png");
    potionClaGraph = LoadGraph("Assets/POTION_CLA.png");
}

ItemManager::~ItemManager() 
{
}

void ItemManager::SpawnItems(Stage* stage)
{
    items.clear();
    const auto& rooms = stage->GetRooms();
    if (rooms.empty()) return;

    // --- デバッグ用：最初の部屋に全アイテムを確定で出す ---
    const auto& startRoom = rooms[0];
    for (int i = 0; i < ITEM_MAX; i++)
    {
        ItemData debugItem;
        // 部屋の左上から右に向かって順番に並べる（重ならないように +i）
        debugItem.map_x = startRoom.x + 1 + i;
        debugItem.map_y = startRoom.y + 1;
        debugItem.type = static_cast<E_ITEM_TYPE>(i);

        // 座標が部屋の範囲内か一応チェック
        if (debugItem.map_x < startRoom.x + startRoom.w - 1)
        {
            items.push_back(debugItem);
        }
    }

    // --- 以下、通常のランダム生成（必要なければコメントアウトしてください） ---
    for (size_t i = 1; i < rooms.size(); i++)
    {
        const auto& room = rooms[i];
        if (rand() % 100 < 50)
        {
            ItemData newItem;
            newItem.map_x = room.x + 1 + (rand() % (room.w - 2));
            newItem.map_y = room.y + 1 + (rand() % (room.h - 2));
            newItem.type = static_cast<E_ITEM_TYPE>(rand() % ITEM_MAX);
            items.push_back(newItem);
        }
    }
}

void ItemManager::Draw(Stage* stage) 
{
    float z = stage->GetZoomRate();
    float ds = stage->GetTileSize() * z;
    int ox = stage->GetCameraX();
    int oy = stage->GetCameraY();

    for (const auto& item : items) 
    {
        if (stage->IsTileVisible(item.map_x, item.map_y))
        {
            int lx = (int)(item.map_x * ds - ox * z);
            int ty = (int)(item.map_y * ds - oy * z);
            stage->SetItemFound(item.map_x, item.map_y, item.type);
            int graph = potionHealGraph;
            if (item.type == ITEM_POTION_ATK) graph = potionAtkGraph;
            else if (item.type == ITEM_TORCH) graph = torchGraph;
            else if (item.type == ITEM_CLAIRVOYANCE) graph = potionClaGraph;

            float aspect = 370.0f / 217.0f;
            int drawW = (int)(ds * 0.6f);
            int drawH = (int)(drawW * aspect);
            int offsetX = ((int)ds - drawW) / 2;
            int drawY = ty + (int)ds - drawH;

            DrawExtendGraph(lx + offsetX, drawY, lx + offsetX + drawW, drawY + drawH, graph, TRUE);
        }
    }
}

void ItemManager::PickUpItem(int x, int y, Player* player) 
{
    auto it = std::find_if(items.begin(), items.end(), [x, y](const ItemData& i) 
    {
        return i.map_x == x && i.map_y == y;
    });

    if (it != items.end()) 
    {
        // 効果音を再生
        extern int itemSEHandle;
        PlaySoundMem(itemSEHandle, DX_PLAYTYPE_BACK);

        if (it->type == ITEM_POTION_HEAL) 
        {
            player->Heal(10);
            player->ShowPickUpMessage("HP +10!");
        }
        else if (it->type == ITEM_POTION_ATK) 
        {
            player->AddAttack(1);
            player->ShowPickUpMessage("ATK +1!");
        }
        if (it->type == ITEM_TORCH)
        {
            player->torchTurn = 25;
            player->ShowPickUpMessage("たいまつを灯した！ (視界アップ)");
        }
        else if (it->type == ITEM_CLAIRVOYANCE)
        {
            player->clairvoyanceTurn = 25;
            player->ShowPickUpMessage("千里眼の薬を飲んだ！ (敵の位置を察知)");
        }
        items.erase(it);
    }
}