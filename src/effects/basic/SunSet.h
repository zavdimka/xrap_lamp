#pragma once
#include "effects/Effect.h"

class SunSet : public Effect
{
public:
    float pogress;
    bool dir; //0 sunset, 1 sunup
    explicit SunSet(Image im,const QString &id);
    void tick() override;
    void activate() override;
};