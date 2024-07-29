#ifndef SMBUS_H
#define SMBUS_H

#include <QObject>
#include "libi2c/include/i2c/i2c.h"
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(smbus_)

class smbus : public QObject
{
public:
    smbus(const char *devname, unsigned char addr);
    void write_data_byte(unsigned char addr, quint8 data);
    unsigned char read_data_byte(unsigned char addr);
    void write_block_data(unsigned char addr, quint8 *data, ssize_t len);
    void read_block_data(unsigned char addr, quint8 *data, ssize_t len);
    void write_data_int(unsigned char addr, quint16 data);
    quint16 read_data_int(unsigned char addr);

private:
    I2CDevice device;
};

#endif // SMBUS_H
