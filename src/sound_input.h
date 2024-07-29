#include <QtCore>
#include <QThread>
#include <QtMultimedia/QAudioInput>
#include <QBuffer>
#include <QLoggingCategory>
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "predictionc.h"

class Sound_input  : public QObject{
    Q_OBJECT

    protected:
        QThread* thread;
        bool is_run;
        QAudioInput *mAudioIn;
        QBuffer  mInputBuffer;
        drwav wav;

        Predictor *predictor;

    public:
        Sound_input(QObject* parent = 0, Predictor *pred = NULL);

        void wait();
        void stop();

    signals:
        void process_sample(float a, float b, float c);

    private slots:
        void doWork();
        void processAudioIn();
        void stateChangeAudioIn(QAudio::State s);

};