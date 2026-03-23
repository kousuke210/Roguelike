#include "Enemy.h"

class Boss : public Enemy
{
public:
    Boss();
    virtual ~Boss();
    bool Update() override;
    void Draw() override;

protected:
    int bossImage;    // ボス専用の画像
    float sizeRatio;  // サイズ倍率（2.0なら2倍の大きさ）
};