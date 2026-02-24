#pragma once
#include <vector>
#include "DxLib.h"

class Stage;
class Player;

enum E_ITEM_TYPE 
{
    ITEM_POTION_HEAL,
    ITEM_POTION_ATK,
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

    // Stageのポインタを受け取るように変更
    void SpawnItems(Stage* stage);
    void Draw(Stage* stage);

    // プレイヤーに効果を与えるために Player* を追加
    void PickUpItem(int x, int y, Player* player);

private:
private:
    std::vector<ItemData> items;
    int potionAtkGraph;
    int potionHealGraph;
};