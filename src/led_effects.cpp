#include "led_effects.h"

#include "effects/basic/SparklesEffect.h"
#include "effects/basic/FireEffect.h"
#include "effects/basic/MatrixEffect.h"
#include "effects/basic/SnowEffect.h"
#include "effects/basic/SunSet.h"
#include "effects/basic/JustLamp.h"

Q_LOGGING_CATEGORY(eff_, "effects")

Effects::Effects(int width, int height, QObject* parent ):
            ws(WS2812()),
            im(Image(width, height, ws.get_buff())),
            width(width), 
            height(height),
            settings(QJsonObject())
{
    
    RegisterEffect<SunSet>("SunRise");
    RegisterEffect<SunSet>("SunSet");
    RegisterEffect<SparklesEffect>("Sparkles");
    RegisterEffect<MatrixEffect>("Matrix");
    RegisterEffect<SnowEffect>("Snow");
    RegisterEffect<FireEffect>("Fire");
    RegisterEffect<JustLamp>("0JustLamp");
    
    qCInfo(eff_) << "Number of effects : " << count();
    for(auto i = effectsMap.begin(); i!= effectsMap.end(); i++)
        qCInfo(eff_) << " : " << i->first;

    effectsIt = effectsMap.begin();
    effect = effectsIt->second;
    effect_cnt = 0;
    effect->activate();
    brightness = 255;

    is_run = true;
    thread = new QThread(parent);
    connect(thread, SIGNAL(started()), this, SLOT(doWork()));
    this->moveToThread(thread);
    thread->start();
}


void Effects::wait(){
    thread->wait();
}

void Effects::stop(){
    is_run = false;
    emit thread->quit();
}

void Effects::doWork(){
    qCInfo(eff_) << "start";
    while(is_run){
        effect->Process();
        ws.update();   
        int speed = effect->settings.speed;
        QThread::msleep((speed > 100) ? 100 : speed);
    }
    im.clean();
    ws.update();
    emit thread->quit();
}

void Effects::next_effect(){
    im.clean();
    effect->deactivate();
    ++effectsIt;
    effect_cnt++;
    if (effectsIt == effectsMap.end()){
        effectsIt = effectsMap.begin();
        effect_cnt = 0;
    }
    effect = effectsIt->second;
    effect->activate();
    effect->update(settings);
    qCInfo(eff_) << "Effect name " << effect->settings.name;
}

void Effects::set_effect(int num){
    if (num > count() - 1) return;
    im.clean();
    effect->deactivate();
    effectsIt = effectsMap.begin();
    effect_cnt = num;
    while(num--) ++effectsIt;
    effect = effectsIt->second;
    effect->activate();
    effect->update(settings);
    qCInfo(eff_) << "Effect name " << effect->settings.name;
}

void Effects::update(QJsonObject json){
    effect->update(json);
}


void Effects::update_brightness(uint8_t br){
    ws.set_brightness(br);
    brightness = br;
}