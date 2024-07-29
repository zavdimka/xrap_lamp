#include "ws2812.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QString>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

Q_LOGGING_CATEGORY(_ws2812, "ws2812")


WS2812::WS2812(int num_leds){
    fd = open(device, O_RDWR);
	if (fd < 0) {
        qCInfo(_ws2812).noquote()  << QString("can't open device %1").arg(device);
        return;
    }
    int ret = 0;
    /*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1){
        qCCritical(_ws2812) << "can't set spi mode";
        return;
    }
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1){
		qCCritical(_ws2812) << "can't get spi mode";
        return;
    }
    /*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1){
		qCCritical(_ws2812) << "can't set max speed hz";
        return;
    }

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1){
		qCCritical(_ws2812) << "can't get max speed hz";
        return;
    }
    qCInfo(_ws2812).noquote()  << QString("spi mode: %1").arg(mode);
	qCInfo(_ws2812).noquote()  << QString("bits per word: %1").arg(bits);
	qCInfo(_ws2812).noquote()  << QString("speed: %1 Hz (%2 KHz)").arg(speed).arg(int(speed/1000));

    led_buff_len = num_leds * 3;
    led_buff = (uint8_t*)malloc(led_buff_len);
    tx_buff_len = (led_buff_len * 8) / 2.666667f + 1 + padding;
    qCInfo(_ws2812).noquote() << QString("led_buff_len : %1, tx_buff_len : %2").arg(led_buff_len).arg(tx_buff_len);
    tx_buff = (uint8_t*)malloc(tx_buff_len);
    rx_buff = (uint8_t*)malloc(tx_buff_len);
    memset(led_buff, 0, led_buff_len);
    memset(tx_buff, 0, tx_buff_len);

}

void WS2812::update(){
    memset(tx_buff, 0, tx_buff_len);
    unsigned int bit = 0, byte = padding;
    for(unsigned int i = 0; i < led_buff_len - 1; i++){
        uint8_t v = led_buff[i];
        for(unsigned int j = 0; j < 8; j++){
            if (v & 0x80) {
                tx_buff[byte] |= 1 << (7 - bit);
                bit += 1; if (bit > 7) {bit -= 8; byte++;}
                tx_buff[byte] |= 1 << (7 - bit);
                bit += 2; if (bit > 7) {bit -= 8; byte++;}
            } else {
                tx_buff[byte] |= 1 << (7 - bit);
                bit += 3; if (bit > 7) {bit -= 8; byte++;}
            }
            v <<= 1;
        }
    }
    struct spi_ioc_transfer tr;
    memset((void*)&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)tx_buff;
    tr.rx_buf = (unsigned long)rx_buff;
	tr.len = tx_buff_len;
	tr.delay_usecs = 0;
	tr.speed_hz = speed;
	tr.bits_per_word = bits;
    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
        qCCritical(_ws2812) << "can't send spi message";
    }

}