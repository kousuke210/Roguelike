#include "Golem.h"
#include "DxLib.h"
#include "Stage.h"

Golem::Golem()
{
    bossImage = LoadGraph("Assets/Golem.png");
    sizeRatio = 2.5f;

    hp = 100;
    maxHp = 100;
    atk = 15;
}

Golem::~Golem()
{
    if (bossImage != -1) DeleteGraph(bossImage);
}

void Golem::Draw()
{
    if (hp <= 0 || bossImage == -1 || !stage) return;

    const float z = stage->GetZoomRate();
    float ds = stage->GetTileSize() * z;
    int ox = stage->GetCameraX();
    int oy = stage->GetCameraY();

    float centerX = (map_x + 0.5f) * ds - ox * z;
    float centerY = (map_y + 0.5f) * ds - oy * z;

    int drawSize = (int)(50 * sizeRatio * z);
    int halfSize = drawSize / 2;
    DrawExtendGraph(centerX - halfSize, centerY - halfSize, centerX + halfSize, centerY + halfSize, bossImage, TRUE);

    float hitRadius = 1.2f * ds;
    DrawBox(centerX - hitRadius, centerY - hitRadius, centerX + hitRadius, centerY + hitRadius, GetColor(255, 0, 0), FALSE);
}