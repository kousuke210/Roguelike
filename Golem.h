#include "Boss.h"

class Golem : public Boss
{
public:
    Golem();
    virtual ~Golem();

    // ƒS[ƒŒƒ€“Æ©‚ÌUŒ‚ƒ‹[ƒ`ƒ“‚ğ‚±‚±‚É‘‚­
    bool Update() override;
};