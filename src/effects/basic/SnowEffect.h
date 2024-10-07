#pragma once
#include "effects/Effect.h"

class SnowEffect : public Effect
{
public:
    explicit SnowEffect(Image im,const QString &id);
    void tick() override;
};