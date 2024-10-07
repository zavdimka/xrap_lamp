#include "SunSet.h"


SunSet::SunSet(Image im,const QString &id)
    : Effect(im, id)
{
    if (id.toLower() == "sunset"){
        dir = 0;
        pogress = 1;
    } else {
        dir = 1;
        pogress = 0;
    }
}

void SunSet::tick()
{
    float step = 0.001f;
if (dir) {
    if (pogress < 1) pogress += step;
    //qDebug() << "inc " << pogress;
} else {
    if (pogress > 0) pogress -= step;
   // qDebug() << "dec " << pogress;
}
if (pogress > 1) pogress = 1;
if (pogress < 0.1) pogress = 0.1;

for (uint8_t y = 0; y < im.height; y++) {
        uint8_t val = (uint8_t)std::min(settings.scale * 2.55 * pogress * (y / 16.0) * 0.1, 20.0);
        uint8_t sat = (uint8_t)std::min((1.0f-pogress) * 255.0f , 255.0f);
        
        for (uint8_t x = 0; x < im.width; x++) {
            im(x,im.height - y - 1).setHSV(0, sat, val);
        }   
    }

}

void SunSet::activate(){
    if (dir){
       pogress = 0;
    } else {
        pogress = 1;
    }
}
