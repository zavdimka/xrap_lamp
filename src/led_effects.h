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
        Effect* effect;

    public:
        Effects(int width, int height, QObject* parent = 0);

        void sparklesRoutine();

        void wait();

        void update();

        template <typename T>
        void RegisterEffect(const QString &id)
        {
            effectsMap[id] = new T(im, id);
        }

        int count()
        {
            return effectsMap.size();
        }

    public slots:
        void stop();
        void next_effect();

    private slots:
        void doWork();


};

