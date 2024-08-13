#pragma once
#include <QtCore>

class WS2812 : public QObject
{
    Q_OBJECT
public:
    WS2812(int num_leds = 16 * 16);
    void update();
    uint8_t* get_buff(){
        return led_buff;
    }
    void set_brightness(uint8_t br);

private:
    int speed = 2400000;
    uint8_t brightness = 255;
    unsigned int bits = 8;
    uint8_t mode = 0;
    const char *device = "/dev/spidev1.0";
    int fd = 0;
    uint8_t *led_buff;
    uint8_t *tx_buff;
    uint8_t *rx_buff;
    unsigned int led_buff_len;
    unsigned int tx_buff_len;
    unsigned int padding = 1;

};