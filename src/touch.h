#pragma once

#include <QtCore>
#include "smbus.h"
#include <vector>

class Touch : public QObject{
    Q_OBJECT
    public:
        struct Regeon{
            int start;
            int end;
            float center;
        };
        Touch(QObject* parent = 0);

        void wait();
        void stop();
    
    protected:
        QThread* thread;
        smbus* i2c;
        const char *device = "/dev/i2c-1";
        bool is_run;

        void update();

        void init_sensor();
        std::vector<Regeon> regs;

        bool is_pressed;

    signals:
        void onMove(float new_pos);
        void onPress(float new_pos);
        void onRelease();


    private slots:
        void doWork();
};