#include "Golem.h"
#include "DxLib.h"

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