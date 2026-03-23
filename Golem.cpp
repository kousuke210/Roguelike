#include "Golem.h"
#include "DxLib.h"

Golem::Golem()
{
    bossImage = LoadGraph("Assets/Golem.png");
    sizeRatio = 2.5f; // ゴーレムは特にデカい（2.5倍）

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
    // 今は親クラス(Boss)のUpdateを呼ぶだけ
    return Boss::Update();
}