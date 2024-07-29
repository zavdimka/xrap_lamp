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

        ef = new Effects(16, 16, this);
        tf = new Touch(this);
        s_i = new Sound_input(this, predictor);
        m701 = new M701("/dev/ttyAS3");


        connect(s_i, &Sound_input::process_sample,
                this, &Application::process_sample);


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
        qCWarning(_main) << "All task finished";

        emit quit();
    }

    void process_sample(float a, float b, float c){
        qCInfo(_main) << "Pred " << a << " " << b << " " << c;

    }

    
};

#include "main.moc"

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    return a.exec();
}