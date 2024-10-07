#include "JustLamp.h"

JustLamp::JustLamp(Image im,const QString &id)
    : Effect(im, id)
{
}

void JustLamp::tick()
{
for (uint8_t x = 0; x < im.height; x++) {
        for (uint8_t y = 0; y < 8; y++) {
            uint8_t color[3] = {
                ((settings.color & 0xFF) * y) / 32, 
                (((settings.color>>8) & 0xFF) * y) / 32, 
                (((settings.color>>16) & 0xFF) * y) / 32
                };
            int c = color[0] | (color[1]<< 8) | (color[2] << 16);

            im(x,y).setColor(c);
        }   
    }

}