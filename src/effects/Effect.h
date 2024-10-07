#pragma once
#include <QtCore>
#include <QJsonObject>
#include "Image.h"

int random(int start, int end);

class Effect
{
protected:
    Image im;
public:
    struct EffectSettings {
        QString id;
        QString name;
        uint8_t speed = 1;
        uint8_t scale = 100;
        uint8_t brightness = 80;
        uint32_t color = 0xffffff;
    };

    Effect(Image im, const QString &id);
    virtual ~Effect();
    void Process();

    virtual void activate() {}
    virtual void deactivate() {}

    virtual void initialize(const QJsonObject &json);
    virtual void update(const QJsonObject &json);

    virtual void tick() = 0;

    EffectSettings settings;
};