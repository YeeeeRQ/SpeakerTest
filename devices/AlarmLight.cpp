#include "AlarmLight.h"

AlarmLight::AlarmLight(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort();
    //    connect(serial, &QSerialPort::readyRead, this, &AlarmLight::onSerialPort_readRead);
}

AlarmLight::~AlarmLight()
{
    this->log = nullptr;
}

bool AlarmLight::connectDevice(QString port_name)
{
    if(serial->isOpen()) return true;

    serial->setPortName(port_name);
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    // serial->setReadBufferSize(9);


    if(serial->open(QIODevice::ReadWrite)){
        emit connectStatusChanged();

        log->info("打开成功");
    }else{

        log->warn("打开失败");
    }
    return serial->isOpen();
}

void AlarmLight::disconnectDevice()
{
    if(serial->isOpen()){
        serial->close();
        log->warn("关闭报警灯连接.");
        emit connectStatusChanged();
    }
}

void AlarmLight::setLogger(SimpleLog *log)
{
    this->log = log;
}

bool AlarmLight::isConnected()
{
    return serial->isOpen();
}

void AlarmLight::startAlarm()
{
    if(!serial->isOpen()){
        log->warn("警报灯未连接!");
        return;
    }
    QString cmd("FAIL");
    serial->write(cmd.toLocal8Bit());
}

void AlarmLight::stopAlarm()
{
    if(!serial->isOpen()){
        log->warn("警报灯未连接!");
        return;
    }
    QString cmd("PASS");
    serial->write(cmd.toLocal8Bit());
}

