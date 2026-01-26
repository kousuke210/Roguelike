#include "Item.h"
#include "Stage.h"
#include <algorithm>

ItemManager::ItemManager() {}
ItemManager::~ItemManager() {}

void ItemManager::SpawnItems(const std::vector<Stage::Room>& rooms) {
    items.clear();
    for (const auto& room : rooms) {
        // 50%の確率でアイテムを生成
        if (rand() % 100 < 50) {
            ItemData newItem;
            // 部屋の内側に座標を決定
            newItem.map_x = room.x + 1 + (rand() % (room.w - 2));
            newItem.map_y = room.y + 1 + (rand() % (room.h - 2));

            // 【解説した箇所】0か1をランダムに選び、アイテム型に変換
            newItem.type = static_cast<E_ITEM_TYPE>(rand() % ITEM_MAX);

            items.push_back(newItem);
        }
    }
}

void ItemManager::Draw(Stage* stage) {
    float z = stage->GetZoomRate();      // 1.5倍などのズーム率
    float ds = stage->GetTileSize() * z; // 拡大後のタイルサイズ
    int ox = stage->GetCameraX();        // カメラの左上X
    int oy = stage->GetCameraY();        // カメラの左上Y

    for (const auto& item : items) {
        // 【重要】現在プレイヤーから見えているタイルかチェック
        if (!stage->IsTileVisible(item.map_x, item.map_y)) continue;

        // 描画位置の計算
        int lx = (int)(item.map_x * ds - ox * z);
        int ty = (int)(item.map_y * ds - oy * z);

        // アイテム種別ごとに色を変える
        int color = (item.type == ITEM_POTION_HEAL) ? GetColor(0, 255, 0) : GetColor(255, 0, 0);

        // タイルの真ん中に小さな円を描く
        DrawCircle(lx + (int)ds / 2, ty + (int)ds / 2, (int)(ds / 4), color, TRUE);
    }
}

void ItemManager::PickUpItem(int x, int y) {
    // プレイヤーと同じ座標にあるアイテムをリストから消去
    items.erase(std::remove_if(items.begin(), items.end(), [x, y](const ItemData& i) {
        return i.map_x == x && i.map_y == y;
        }), items.end());
}