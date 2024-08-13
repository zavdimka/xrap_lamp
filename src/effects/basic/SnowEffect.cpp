#include "SnowEffect.h"

SnowEffect::SnowEffect(Image im,const QString &id)
    : Effect(im, id)
{
    myColor = 0xffffff;
}

void SnowEffect::tick()
{
for (uint8_t x = 0; x < im.height; x++) {
        for (uint8_t y = 0; y < im.width - 1; y++) {
            im(x,y).setColor(im(x,y + 1).getColor());
        }   
    }

for (uint8_t x = 0; x < im.height; x++) {
    if (!im(x, im.width - 2).getColor() && (random(0, settings.scale) == 0)){
        im(x, im.width - 1).setColor(myColor);
    }  else {
        im(x, im.width - 1).setColor(0);
    }
    }

}

void SnowEffect::update(const QJsonObject &json){
    Effect::update(json);
    if (json.contains("color")) {
        myColor = json["color"].toInt();
    }
}