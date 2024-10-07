#pragma once
#include <QtCore>
#include <QLoggingCategory>
#include <map>
#include "Image.h"
#include "ws2812.h"
#include "effects/Effect.h"

class Effects  : public QObject{
    Q_OBJECT

    protected:
        WS2812 ws;
        Image im;
        int width;
        int height;
        QThread* thread;
        bool is_run;

        std::map<QString, Effect*> effectsMap;
        std::map<QString, Effect*>::iterator effectsIt;
        int effect_cnt;
        uint8_t  brightness;
        Effect* effect;
        QJsonObject settings;

    public:
        Effects(int width, int height, QObject* parent = 0);

        void sparklesRoutine();

        void wait();

        uint8_t get_brightness() {return brightness;}
  
        template <typename T>
        void RegisterEffect(const QString &id)
        {
            effectsMap[id] = new T(im, id);
        }

        int count()
        {
            return effectsMap.size();
        }

        int effect_num() { return effect_cnt;}

    public slots:
        void stop();
        void next_effect();
        void set_effect(int num);
        void update(QJsonObject json);
        void update_brightness(uint8_t br);

    private slots:
        void doWork();


};

