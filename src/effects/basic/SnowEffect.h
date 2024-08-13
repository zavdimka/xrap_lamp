#pragma once
#include "effects/Effect.h"

class SnowEffect : public Effect
{
public:
    uint32_t myColor;
    explicit SnowEffect(Image im,const QString &id);
    void tick() override;
    void update(const QJsonObject &json) override;
};