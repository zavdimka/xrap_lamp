#include "SparklesEffect.h"

SparklesEffect::SparklesEffect(Image im,const QString &id)
    : Effect(im, id)
{
}

void SparklesEffect::tick()
{
    for (int i = 0; i < 1; i++) {
        int x = qrand() % im.width;
        int y = qrand() % im.height;
        if ((im(x,y).getColor() == 0) && (qrand() % 4 == 0))
            im(x,y).setHSV(qrand() % 255, 255, 255);
    }
    uint8_t step = 10;
    for (int i = 0; i < im.width; i++) {
        for (int j = 0; j < im.height; j++) {
            im(i,j).fade(step);
        }
    }
}