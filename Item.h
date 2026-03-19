#pragma once
#include <vector>
#include "DxLib.h"

class Stage;
class Player;

enum E_ITEM_TYPE 
{
    ITEM_POTION_HEAL,   // ‰ñ•œ–ò
    ITEM_POTION_ATK,    // UŒ‚—ÍUP
    ITEM_TORCH,         // ‚µ‚Î‚ç‚­‚ÌŠÔ‚½‚¢‚Ü‚Âi‹ŠEL‚°‚éj
    ITEM_CLAIRVOYANCE,  // ‚µ‚Î‚ç‚­“G‚ÌˆÊ’u‚ª‚í‚©‚é
    ITEM_MAX
};

struct ItemData 
{
    int map_x;
    int map_y;
    E_ITEM_TYPE type;
};

class ItemManager 
{
public:
    ItemManager();
    ~ItemManager();

    void SpawnItems(Stage* stage);
    void Draw(Stage* stage);
    void SpawnSpecificItem(Stage* stage, int x, int y, int type);
    void PickUpItem(int x, int y, Player* player);

private:
    std::vector<ItemData> items;
    int potionAtkGraph;
    int potionHealGraph;
    int torchGraph;
    int potionClaGraph;
};