#include "led_effects.h"

#include "effects/basic/SparklesEffect.h"
#include "effects/basic/FireEffect.h"

Q_LOGGING_CATEGORY(eff_, "effects")

Effects::Effects(int width, int height, QObject* parent ):
            ws(WS2812()),
            im(Image(width, height, ws.get_buff())),
            width(width), 
            height(height) 
{
    RegisterEffect<FireEffect>("Fire");
    RegisterEffect<SparklesEffect>("Sparkles");
    

    effectsIt = effectsMap.begin();
    effect = effectsIt->second;
    effect->activate();

    is_run = true;
    thread = new QThread(parent);
    connect(thread, SIGNAL(started()), this, SLOT(doWork()));
    this->moveToThread(thread);
    thread->start();
}


void Effects::wait(){
    thread->wait();
}

void Effects::update(){
    effect->Process();
    ws.update();   
}

void Effects::stop(){
    is_run = false;
    emit thread->quit();
}

void Effects::doWork(){
    qCInfo(eff_) << "start";
    while(is_run){
        update();
        QThread::msleep(1);
    }
    im.clean();
    ws.update();
    emit thread->quit();
}

void Effects::next_effect(){
    im.clean();
    effect->deactivate();
    if (effectsIt == effectsMap.end()){
        effectsIt = effectsMap.begin();
    } else {
        ++effectsIt;
    }
    effect = effectsIt->second;
    effect->activate();
    qCInfo(eff_) << "Effect name " << effect->settings.name;
}