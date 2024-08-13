#pragma once
#include "effects/Effect.h"

class MatrixEffect : public Effect
{
public:
    explicit MatrixEffect(Image im,const QString &id);
    void tick() override;
};
