#include "touch.h"
#include <QString>


Q_LOGGING_CATEGORY(th_, "touch")

Touch::Touch(QObject* parent){
    i2c = new smbus(device, 0x68);
    
    is_run = true;
    thread = new QThread(parent);
    connect(thread, SIGNAL(started()), this, SLOT(doWork()));
    this->moveToThread(thread);
    thread->start();
}

void Touch::wait(){
    thread->wait();
}

void Touch::stop(){
    is_run = false;
    emit thread->quit();
}

void Touch::init_sensor(){
    i2c->write_data_byte(0x09, 0x0f);
    QThread::msleep(100);
    i2c->write_data_byte(0x09, 0x07);

    //set sensetivity
    uint8_t sens = 0x4 | (1 << 4);
    for(int i = 0x02; i < 0x07; i++)
        i2c->write_data_byte(i, sens | (sens << 4));

    //Enable all channels
    i2c->write_data_byte(0x0C, 0x00);
    i2c->write_data_byte(0x0D, 0x00);
    //Disable regerence
    i2c->write_data_byte(0x0A, 0x00);
    i2c->write_data_byte(0x0B, 0x00);
    QThread::msleep(50);
    //Enable regerence
    // i2c->write_data_byte(0x0A, 0xFF);
    // i2c->write_data_byte(0x0B, 0xFF);
}

void Touch::doWork(){
    qCInfo(th_) << "start";
    init_sensor();    
    
    // uint8_t tp[0x10];
    // i2c->read_block_data(0x2, tp, 0x10);
    // for(int i=0; i < 0x10; i ++){
    //     qCInfo(th_) << QString("%1 : 0x%2")
    //         .arg(2 + i, 2 , 16)            
    //         .arg(tp[i], 2, 16);
    // }
    is_pressed = false;
    while(is_run){
        update();
        QThread::msleep(100);
    }
    emit thread->quit();
}

void Touch::update(){
    uint8_t buff[3];
    i2c->read_block_data(0x10, buff, 3);
    uint8_t sens[12];
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 4; j++)
            sens[i*4 + j] = (buff[i] & (0x3<<(j*2))) >> (j*2);
    }

    Regeon reg;
    regs.clear();
    reg.start = -1;
    reg.end = -1;
    int first_start = -2;
    int cnt = 0;
    while (((first_start != reg.start) || (cnt < 2)) && (cnt < 4))  {   
        for(int i = reg.end + 1; i < 24; i++)
            if ((sens[ i ? ((i-1) % 12) : 11] == 0 ) && (sens[(i) % 12])) {
                reg.start = i % 12;
                cnt++;
                if (first_start < 0) first_start = i % 12;
                break;
            }
        if (reg.start >= 0) {
            for(int i=reg.start; i < 24; i++)
            if (sens[i % 12] == 0) {
                reg.end = (i - 1) % 12; 
                break;
            } 
            if ((first_start != reg.start) || (cnt < 2)){
                int acc = 0; int m = 0;
                for(int i = reg.start; i <= (reg.end >= reg.start ? reg.end : (reg.end + 12)); i++) {
                    acc += sens[i % 12] * i;
                    m += sens[i % 12];
                }
                reg.center = (float)acc / m;
                if (reg.center > 12.0f) reg.center -= 12.0f;
                regs.push_back(reg);
                qCInfo(th_) << QString("Start %1 End %2 mass %3")  
                    .arg(reg.start).arg(reg.end).arg(reg.center);
            }
        } else {
                break;
            }
    } 

    if (regs.size() ) {
        if (!is_pressed){
            is_pressed = true;
            emit  onPress(regs[0].center);
            qCInfo(th_) << "onPress" << regs[0].center;
        } else {
            emit onMove(regs[0].center);
            qCInfo(th_) << "onMove" << regs[0].center;
        }
    } else {
        if (is_pressed){
            is_pressed = false;
            onRelease();
            qCInfo(th_) << "onRelease";
        }
    }

}