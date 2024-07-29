#pragma once
#include "effects/Effect.h"

class SparklesEffect : public Effect
{
public:
    explicit SparklesEffect(Image im,const QString &id);
    void tick() override;
};