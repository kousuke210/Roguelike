#include "Boss.h"
#include "DxLib.h"
#include "Stage.h"

Boss::Boss() : Enemy(ENEMY_MAX)
{
    sizeRatio = 2.0f;
    bossImage = -1;
}

Boss::~Boss()
{
    if (bossImage != -1) DeleteGraph(bossImage);
}

void Boss::Draw()
{
    if (hp <= 0 || bossImage == -1 || !stage) return;

    if (!stage->IsTileVisible(map_x, map_y)) return;

    const float z = stage->GetZoomRate();
    int size = (int)(50 * sizeRatio * z);

    int draw_x = (int)(map_x * 50 * z - stage->GetCameraX() * z);
    int draw_y = (int)(map_y * 50 * z - stage->GetCameraY() * z);

    DrawExtendGraph(draw_x, draw_y, draw_x + size, draw_y + size, bossImage, TRUE);
}

bool Boss::Update()
{
    return Enemy::Update();
}