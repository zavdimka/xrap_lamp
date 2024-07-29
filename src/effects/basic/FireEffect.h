#pragma once
#include "effects/Effect.h"

class FireEffect : public Effect
{
public:
    explicit FireEffect(Image im,const QString &id);
    void activate() override;
    void deactivate() override;
    void tick() override;
    void initialize(const QJsonObject &json) override;

private:
    void generateLine();
    void shiftUp();
    void drawFrame(uint8_t pcnt);
};