#include "Golem.h"
#include "DxLib.h"

Golem::Golem()
{
    bossImage = LoadGraph("Assets/Golem.png");
    sizeRatio = 2.5f;

    // ゴーレムの初期ステータス
    hp = 150;
    maxHp = 150;
    atk = 25;
}

Golem::~Golem()
{
}

bool Golem::Update()
{
    return Boss::Update();
}