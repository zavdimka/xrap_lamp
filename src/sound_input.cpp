#include "sound_input.h"

Q_LOGGING_CATEGORY(si_, "sound_input")

Sound_input::Sound_input(QObject* parent, Predictor *pred){
    thread = new QThread(parent);
    predictor = pred;
    connect(thread, SIGNAL(started()), this, SLOT(doWork()));
    this->moveToThread(thread);
    thread->start();    
}

void Sound_input::processAudioIn(){
    mInputBuffer.seek(0);
    QByteArray ba = mInputBuffer.readAll();
    int n_len = ba.size() / 8;
    std::vector<int32_t> unfiltred(n_len);
    std::vector<float> filtred(n_len / 3);
    for(int i = 0; i < n_len; i++){
        unfiltred[i] = *((int32_t*)(ba.data() + i * 8)) + *((int32_t*)(ba.data() + i * 8 + 4));
    }
    for(int i = 0; i < n_len; i += 3){
        filtred[i / 3] = ((float)unfiltred[i]) / (float)(1ULL<<30);
    }
    float mag = 0;
    for(int i = 0; i < (int)filtred.size() ; i ++){
        mag += filtred[i] * filtred[i];
    }
    mag /= filtred.size();
    mag = sqrt(mag);

    //drwav_write_pcm_frames(&wav, filtred.size(), filtred.data());
    drwav_write_raw(&wav, filtred.size() * 4, filtred.data());
    qCInfo(si_) << " Write: " << filtred.size() << ": Mag " << QString("%1").arg(mag, 0, 'f', 4);

    
    float excp[3] =  {0};
    if (predictor) { 
        std::vector<float> pred = predictor->process(filtred);    
        for(int i=0; i< 3; i++){
            excp[i] = pred[i];
        }
    }
    
    emit process_sample(excp[0], excp[1], excp[2]);

   // int32_t *dat = (int32_t*)ba.data();

    mInputBuffer.buffer().clear();
    mInputBuffer.seek(0);
}

void Sound_input::stateChangeAudioIn(QAudio::State s){
    qCInfo(si_) << "State change: " << s;
    if ( mAudioIn->error() != QAudio::NoError ) {
    qDebug() << "AudioError: " << mAudioIn->error();
    }
}

void Sound_input::wait(){
    thread->wait();
    drwav_uninit(&wav);
}

void Sound_input::doWork(){
    qCInfo(si_) << "start";
    int sel_card = 1;

    QList<QAudioDeviceInfo> inputDevices =
            QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    qCInfo(si_) << "Avalible devices"; int i = 0;
    for (QAudioDeviceInfo d : inputDevices) {
        qCInfo(si_) << i << " : " << d.deviceName() ; i++;
    }

    qCInfo(si_) << "Select " <<inputDevices[sel_card].deviceName();
    qCInfo(si_) << "Codecs " <<inputDevices[sel_card].supportedCodecs();

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setCodec("audio/pcm");
    format.setSampleSize(32);

    drwav_data_format wformat;
    wformat.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    wformat.format = DR_WAVE_FORMAT_IEEE_FLOAT;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    wformat.channels = 1;
    wformat.sampleRate = 16000;
    wformat.bitsPerSample = 32;
    drwav_init_file_write(&wav, "recording.wav", &wformat, NULL);

    if (!inputDevices[sel_card].isFormatSupported(format)) {
        qCWarning(si_) << "Default format not supported, trying to use the nearest.";
        format = inputDevices[sel_card].nearestFormat(format);
    }

    mAudioIn = new QAudioInput(inputDevices[sel_card], format);
    mAudioIn->setBufferSize(38400);
    mAudioIn->setNotifyInterval(1000);

    connect(mAudioIn, SIGNAL(notify()),
            this, SLOT(processAudioIn()));
    connect(mAudioIn, SIGNAL(stateChanged(QAudio::State)),
            this, SLOT(stateChangeAudioIn(QAudio::State)));

    mInputBuffer.open(QBuffer::ReadWrite);
    mAudioIn->start(&mInputBuffer);    
}


void Sound_input::stop(){
    mAudioIn->stop();
    emit thread->quit();
    is_run = false;
}