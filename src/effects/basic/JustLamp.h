#pragma once
#include "effects/Effect.h"

class JustLamp : public Effect
{
public:
    explicit JustLamp(Image im,const QString &id);
    void tick() override;
};