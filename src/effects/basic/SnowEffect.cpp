#include "SnowEffect.h"

SnowEffect::SnowEffect(Image im,const QString &id)
    : Effect(im, id)
{
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
        im(x, im.width - 1).setColor(settings.color);
    }  else {
        im(x, im.width - 1).setColor(0);
    }
    }

}
