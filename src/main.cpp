// main.cpp
#include <QtCore>
#include <QThread>
#include <QDebug>
#include <QLoggingCategory>
#include <iostream> 
#include "ws2812.h"
#include "led_effects.h"
#include "touch.h"
#include "SignalHandler.h"
#include "Logger.h"
#include "sound_input.h"

#include "predictionc.h"
#include "M701.h"
#include "lampable.h"


Q_LOGGING_CATEGORY(_main, "main")

class Application : public QCoreApplication, public SignalHandler
{
    Q_OBJECT
private:
    Effects *ef;
    Touch *tf;
    Sound_input *s_i;
    Predictor *predictor;
    M701 *m701;
    LampaBle *bt;

    bool pressed = false;
    float prev_pos = 0;
    float brightness = 255;
    float movement = 0;
    
public:
    Application(int &argc, char **argv) : 
        QCoreApplication(argc, argv),
        SignalHandler(SignalHandler::SIG_INT) { 
            QTimer::singleShot(0, this, SLOT(run()));

    }    
    
    bool handleSignal(int signal)
    {
        std::cout << "Handling signal " << signal << std::endl;
        ef->stop();
        tf->stop();
        s_i->stop();
        m701->stop();
        bt->stop();
        emit finish();
        return true;
    }


private slots:
    void run(){
        InitLogger("Log");
        qsrand(0); 

        qCWarning(_main) << " hello world";

        predictor = new Predictor();
        predictor->load_model("../model_weights/weights.pt");

        qRegisterMetaType<M701::M701_data>("M701::M701_data");
        qRegisterMetaType<uint8_t>("uint8_t");

        bt = new LampaBle(this);
        ef = new Effects(16, 16, this);
        tf = new Touch(this);
        s_i = new Sound_input(this, predictor);
        m701 = new M701("/dev/ttyAS3");


        connect(s_i, &Sound_input::process_sample,
                bt, &LampaBle::update_hrap_prediction);
        
        connect(s_i, &Sound_input::process_sample,
                this, &Application::process_sample);

        connect(m701, &M701::update,
                bt, &LampaBle::update_meteo_data);

        connect(bt, &LampaBle::update_led_control,
                this, &Application::update_led_control);

        connect(tf, &Touch::onPress,
                this, &Application::onPress);

        connect(tf, &Touch::onMove,
                this, &Application::onMove);

        connect(tf, &Touch::onRelease,
                this, &Application::onRelease);


        qCInfo(_main) << "All task started";

    }

    void finish(){
        ef->wait(); 
        qCWarning(_main) << "ef finished";
        tf->wait();
        qCWarning(_main) << "tf finished";
        s_i->wait();
        qCWarning(_main) << "s_i finished";
        m701->wait();
        qCWarning(_main) << "m701 finished";
        bt->wait();
        qCWarning(_main) << "All task finished";

        emit quit();
    }

    void process_sample(float a, float b, float c){
        qCInfo(_main) << "Pred " << a << " " << b << " " << c;

    }

    void onPress(float pos){
        pressed = true;
        prev_pos = pos;
        movement = 0;
    }

    void onMove(float pos){
        float dif = pos - prev_pos;
        if ((pos < 3) && (prev_pos > 9))
            dif = -((pos - prev_pos) + 12);
        if ((pos > 9) && (prev_pos < 3))
            dif = 12 - (pos - prev_pos);
        prev_pos = pos;
        movement += abs(dif);

        brightness += dif * 10;
        if (brightness > 255) brightness = 255;
        if (brightness < 0)  brightness = 0;

        ef->update_brightness(brightness);

        qCInfo(_main) << "brightness is " << brightness;

    }   

    void onRelease(){
        pressed = false;
        qCInfo(_main) << "movement is" << movement;
        if (movement < 4) {
            ef->next_effect();
        }
    } 

    void update_led_control(uint8_t cmd, uint8_t arg1, uint8_t arg2){
        switch(cmd){
            case 0:
                ef->next_effect();
                break;
            
            case 1:
                ef->update_brightness(arg1);
                break;

            case 2:
                QJsonObject  js;
                js["color"] = arg1;
                ef->update(js);
                break;
        }
    }
};

#include "main.moc"

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    return a.exec();
}