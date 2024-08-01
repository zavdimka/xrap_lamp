#pragma once
#include <QtCore>
#include <QLoggingCategory>
#include <QThread>
#include <QSerialPort>
#include <QTime>
#include <iostream>



class M701  : public QObject{
    Q_OBJECT
    protected:
        QThread* thread;
        bool is_run;
        QString dev_name;

    public:
        struct __attribute__((__packed__)) M701_data {
            uint16_t eCO2;
            uint16_t eCH2O;
            uint16_t TVOC;
            uint16_t PM2_5;
            uint16_t PM10;
            int16_t temperature;
            uint16_t humidity;
        };
        M701(QString dev = "/dev/ttyAS3", QObject* parent = 0);

        void wait();
        
        M701_data data;
       
    public slots:
        void stop();

    private slots:
        void doWork();

    signals:
        void update(M701::M701_data data);


};
