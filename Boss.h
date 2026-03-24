#include "Enemy.h"

class Boss : public Enemy
{
public:
    Boss();
    virtual ~Boss();
    bool Update() override;
    void Draw() override;

protected:
    int bossImage;
    float sizeRatio;  // ÉTÉCÉYî{ó¶
};