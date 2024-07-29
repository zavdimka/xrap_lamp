#include "M701.h"

Q_LOGGING_CATEGORY(m7_, "M701")

M701::M701(QString dev, QObject* parent){
    dev_name = dev;
    thread = new QThread(parent);
    is_run = true;
    connect(thread, SIGNAL(started()), this, SLOT(doWork()));
    this->moveToThread(thread);
    thread->start();
}

void M701::doWork(){
    QSerialPort serial;
    serial.setPortName(dev_name);
    serial.setBaudRate(9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    if (!serial.open(QIODevice::ReadWrite)) {
        qCWarning(m7_) << tr("Can't open %1, error code %2")
                           .arg(dev_name).arg(serial.error());
        return;
    }

    QByteArray d; 

    while(is_run){
        if (serial.waitForReadyRead(100)){
            d.append(serial.readAll());
            if (d.size() > 2) {
                if ((d[0] != (char)0x3C) || (d[1] != (char)0x02))
                    d.clear();
            }
            if (d.size() >= 17){
                int crc = 0;
                for(int i=0; i < 16; i++) crc += d[i];
                crc &= 0xff;
                if (d[16] == (char)crc){
                    data.eCO2 = d[2] << 8 | d[3];
                    data.eCH2O = d[4] << 8 | d[5];
                    data.TVOC = d[6] << 8 | d[7];
                    data.PM2_5 = d[8] << 8 | d[9];
                    data.PM10 = d[10] << 8 | d[11];
                    data.temperature = d[12] << 8 | d[13];
                    data.humidity = d[14] << 8 | d[15];
                    qCInfo(m7_) << QString("CO2 %1, CH2O %2, TVOC %3, PM2.5 %4, PM10 %5, Temp %6, Humidity %7").
                    arg(data.eCO2).arg(data.eCH2O).arg(data.TVOC).arg(data.PM2_5).arg(data.PM10).
                    arg(data.temperature / 256.0f, 0, 'f', 1).arg(data.humidity / 256.0f, 0, 'f', 1);
                }
                d.clear();
            }
        }
    }

}

void M701::stop(){
    is_run = false;
    emit thread->quit();
}

void M701::wait(){
    thread->wait();
}