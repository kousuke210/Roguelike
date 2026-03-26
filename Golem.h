#include "Boss.h"

class Golem : public Boss
{
public:
    Golem();
    virtual ~Golem();
    void Draw() override;
};