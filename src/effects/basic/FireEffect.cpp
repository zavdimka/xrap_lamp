#include "FireEffect.h"

namespace{

uint8_t* line = nullptr;
uint8_t pcnt = 0;

bool useSpectrometer = false;

//these values are substracetd from the generated values to give a shape to the animation
const unsigned char valueMask[8][16]  = {
    {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
    {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
    {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
    {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
    {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
    {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192},
    {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255},
    {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
const unsigned char hueMask[8][16]  = {
    {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 },
    {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 },
    {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 },
    {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 },
    {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 },
    {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
    {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
    {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 }
};

uint8_t matrixValue[8][16] = {};

bool sparkles = true;

} // namespace

FireEffect::FireEffect(Image im,const QString &id)
    : Effect(im, id)
{
}

void FireEffect::activate()
{
    line = new uint8_t[im.width]();
    generateLine();
}

void FireEffect::deactivate()
{
    delete[] line;
}

void FireEffect::tick()
{
    if (pcnt >= 100) {
        shiftUp();
        generateLine();
        pcnt = 0;
    }
    drawFrame(pcnt);
    pcnt += 30;
}

void FireEffect::initialize(const QJsonObject &json)
{
    Effect::initialize(json);

}

void FireEffect::generateLine()
{
    for (uint8_t x = 0; x < im.width; x++) {
        line[x] = static_cast<uint8_t>(random(48, 255) );
    }
}

void FireEffect::shiftUp()
{
    for (uint8_t y = im.height - 1; y > 0; y--) {
        for (uint8_t x = 0; x < im.width; x++) {
            if (y > 7) {
                continue;
            }
            matrixValue[y][x] = matrixValue[y - 1][x];
        }
    }

    for (uint8_t x = 0; x < im.width; x++) {
        matrixValue[0][x] = line[x];
    }
}

void FireEffect::drawFrame(uint8_t pcnt)
{
//    Serial.printf("FireEffect::drawFrame %u\n", pcnt);
    int nextv;

    //each row interpolates with the one before it
    for (uint8_t y = im.height - 1; y > 0; y--) {
        for (uint8_t x = 0; x < im.width; x++) {
            if (y < 8) {
                nextv =
                        (((100.0 - pcnt) * matrixValue[y][x]
                          + pcnt * matrixValue[y - 1][x]) / 100.0)
                        - valueMask[y][x];

                uint8_t hue = settings.scale * 2.55;

                im(x, y).setHSV(hue + hueMask[y][x],
                                255,
                                (uint8_t)std::max(0, nextv));

            } else if (y == 8 && sparkles) {
                if (random(0, 20) == 0 && im(x, y - 1).getColor()) {
                    im(x, y).setColor(im(x, y - 1).getColor());
                } else {
                    im(x, y).setColor(0);
                }
            } else if (random(0, 2) == 0 && sparkles) {
                // старая версия для яркости
                if (im(x, y - 1).getColor()) {
                    im(x, y).setColor(im(x, y - 1).getColor());
                } else {
                    im(x, y).setColor(0);
                }

            }
        }
    }

    //first row interpolates with the "next" line
    for (uint8_t x = 0; x < im.width; x++) {
        uint8_t hue =  settings.scale * 2.55;
        
        im(x, 0).setHSV(hue +  hueMask[0][x], 
                        255,
                        (uint8_t)(((100.0 - pcnt) * matrixValue[0][x] + pcnt * line[x]) / 100.0));
    }
}