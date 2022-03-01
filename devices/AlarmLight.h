#ifndef ALARMLIGHT_H
#define ALARMLIGHT_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QObject>
#include <QDebug>

#include "simplelog.h"

class AlarmLight : public QObject
{
    Q_OBJECT
public:
    explicit AlarmLight(QObject *parent = nullptr);
    ~AlarmLight();

    bool connectDevice(QString port_name);
    void disconnectDevice();

    void setLogger(SimpleLog* log); //for debug
    bool isConnected();
    void startAlarm();
    void stopAlarm();

private:
    void checkConnect();

signals:
    void connectStatusChanged();

private:
    QSerialPort* serial = nullptr;
    SimpleLog* log = nullptr; //for debug

    AlarmLight(const AlarmLight&) = delete;
    const AlarmLight& operator=(const AlarmLight&) = delete;

private slots:
//    void onSerialPort_readRead();
};

#endif // ALARMLIGHT_H
