// Item.h
#pragma once
#include <vector>
#include "DxLib.h"

// yC³z#include "Stage.h" ‚ğÁ‚µ‚ÄA‰º‚Ì1s‚ğ’Ç‰Á
class Stage;

enum E_ITEM_TYPE {
    ITEM_POTION_HEAL,
    ITEM_POTION_ATK,
    ITEM_MAX
};

struct ItemData {
    int map_x;
    int map_y;
    E_ITEM_TYPE type;
};

class ItemManager {
public:
    ItemManager();
    ~ItemManager();

    // yC³zstruct Stage::Room ‚Å‚Í‚È‚­ Stage::Room ‚É‚·‚é
    void SpawnItems(const std::vector<class Stage::Room>& rooms);

    // class Stage* stage ‚Æ‘‚­‚±‚Æ‚ÅAÚ×‚ğ’m‚ç‚È‚­‚Ä‚àƒRƒ“ƒpƒCƒ‹‚ğ’Ê‚µ‚Ü‚·
    void Draw(class Stage* stage);

    void PickUpItem(int x, int y);

private:
    std::vector<ItemData> items;
};