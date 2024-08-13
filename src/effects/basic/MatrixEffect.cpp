#include "MatrixEffect.h"

MatrixEffect::MatrixEffect(Image im,const QString &id)
    : Effect(im, id)
{
}

void MatrixEffect::tick()
{
const int mainColor = 0x00FF00;
const int fadeColor = 0x001000;
for (uint8_t x = 0; x < im.height; x++) {
        for (uint8_t y = 0; y < im.width - 1; y++) {
            im(x,y).setColor(im(x,y + 1).getColor());
        }   
        int thisColor = im(x, im.width - 1).getColor();
        if (!thisColor){
            if (random(0, settings.scale) == 0) {
                im(x, im.width - 1).setColor(mainColor);
            }
        } else if (thisColor < fadeColor) {
            im(x, im.width - 1).setColor(0);
        } else {
            im(x, im.width - 1).setColor(thisColor - fadeColor);
        }


    }

}