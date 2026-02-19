#include "Item.h"
#include "Stage.h"
#include "Player.h"
#include <algorithm>

ItemManager::ItemManager() 
{
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
        if (!stage->IsTileVisible(item.map_x, item.map_y)) continue;
        int lx = (int)(item.map_x * ds - ox * z);
        int ty = (int)(item.map_y * ds - oy * z);
        int color = (item.type == ITEM_POTION_HEAL) ? GetColor(0, 255, 0) : GetColor(255, 0, 0);
        DrawCircle(lx + (int)ds / 2, ty + (int)ds / 2, (int)(ds / 4), color, TRUE);
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
        if (it->type == ITEM_POTION_HEAL) 
        {
            player->Heal(10); // HP‚ğ10‰ñ•œ
        }
        else if (it->type == ITEM_POTION_ATK) 
        {
            player->AddAttack(1); // UŒ‚—Í‚ğ1ƒAƒbƒv
        }
        items.erase(it);
    }
}