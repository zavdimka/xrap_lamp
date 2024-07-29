#include "smbus.h"

Q_LOGGING_CATEGORY(smbus_, "driver.smbus")

smbus::smbus(const char *devname, unsigned char addr) 
{
    memset(&device, 0, sizeof(device));
    int bus;
    if ((bus = i2c_open(devname)) == -1) {
       QString err = QString("Can't open %1").arg(devname);
       qCCritical(smbus_) << err;
       throw err.toStdString();
       return;
    }
    i2c_init_device(&device);
    device.bus = bus;
    device.addr = addr;

    qCInfo(smbus_) << QString("Open smbus %1").arg(devname);
}

void smbus::write_data_byte(unsigned char addr, quint8 data)
{
    quint8 buff;
    ssize_t size = sizeof(buff);
    buff = data;
    if ((i2c_write(&device, addr, &buff, size)) != size) {
        char cbuff[128];
        QString err = QString("Can't write dev %1").arg(i2c_get_device_desc(&device, cbuff, sizeof(cbuff)));
        qCCritical(smbus_) << err;
        throw err.toStdString();
    }
  // qCInfo(smbus_) << QString("Write data_byte dev 0x%3 : 0x%1 -> 0x%2").arg(addr,2,16,QLatin1Char('0')).arg(data,2,16,QLatin1Char('0')).arg(device.addr,2,16,QLatin1Char('0'));
}

unsigned char smbus::read_data_byte(unsigned char addr)
{
    quint8 buff;
    ssize_t size = sizeof(buff);
    if ((i2c_read(&device, addr, &buff, size)) != size) {
        char cbuff[128];
        QString err = QString("Can't read dev %1").arg(i2c_get_device_desc(&device, cbuff, sizeof(cbuff)));
        qCCritical(smbus_) << err;
        throw err.toStdString();
    }

    //qCInfo(smbus_) << QString("Read data_byte dev 0x%3 : 0x%1 -> 0x%2").arg(addr,2,16,QLatin1Char('0')).arg(buff,2,16,QLatin1Char('0')).arg(device.addr,2,16,QLatin1Char('0'));
    return buff;
}

void smbus::write_block_data(unsigned char addr,quint8 *data, ssize_t len)
{
    if ((i2c_write(&device, addr, data, len)) != len) {
        char cbuff[128];
        QString err = QString("Can't write dev %1").arg(i2c_get_device_desc(&device, cbuff, sizeof(cbuff)));
        qCCritical(smbus_) << err;
        throw err.toStdString();
    }
    QString log;
    for(int i=0;i<len;i++)
        log += QString("0x%1 ").arg(data[i],2,16, QLatin1Char('0'));
   // qCInfo(smbus_) << QString("Write block_data dev 0x%3 : 0x%1 -> %2").arg(addr,2,16,QLatin1Char('0')).arg(log).arg(device.addr,2,16,QLatin1Char('0'));


}

void smbus::read_block_data(unsigned char addr, quint8 *data, ssize_t len)
{
    if ((i2c_read(&device, addr, data, len)) != len) {
        char cbuff[128];
        QString err = QString("Can't read dev %1").arg(i2c_get_device_desc(&device, cbuff, sizeof(cbuff)));
        qCCritical(smbus_) << err;
        throw err.toStdString();
    }
    QString log;
    for(int i=0;i<len;i++)
        log += QString("0x%1 ").arg(data[i],2,16, QLatin1Char('0'));
   // qCInfo(smbus_) << QString("Read block_data dev 0x%3 : 0x%1 -> %2").arg(addr,2,16,QLatin1Char('0')).arg(log).arg(device.addr,2,16,QLatin1Char('0'));
}

void smbus::write_data_int(unsigned char addr, quint16 data)
{
    quint8 buff[2];
    buff[0] = data & 0xFF;
    buff[1] = (data>>8) & 0xFF;
    write_block_data(addr,buff,2);
    //qCInfo(smbus_) << QString("Write int %1").arg(data,4,16,QLatin1Char('0'));
}

quint16 smbus::read_data_int(unsigned char addr)
{
    quint8 buff[2];
    read_block_data(addr, buff, 2);
    unsigned int res = buff[0] | (buff[1]<<8);
    //qCInfo(smbus_) << QString("Read int %1").arg(res,4,16);
    return res;
}
