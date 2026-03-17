#include "Item.h"
#include "Stage.h"
#include "Player.h"
#include <algorithm>

ItemManager::ItemManager() 
{
    // ‰æ‘œ‚Ì“Ç‚Ýž‚Ý
    potionAtkGraph = LoadGraph("Assets/POTION_ATK.png");
    potionHealGraph = LoadGraph("Assets/POTION_HEAL.png");
}

ItemManager::~ItemManager() 
{
}

void ItemManager::SpawnItems(Stage* stage)
{
    items.clear();
    const auto& rooms = stage->GetRooms();
    for (const auto& room : rooms) 
    {
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
            int graph = (item.type == ITEM_POTION_HEAL) ? potionHealGraph : potionAtkGraph;

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
        // Œø‰Ê‰¹‚ðÄ¶
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
        items.erase(it);
    }
}