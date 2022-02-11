#ifndef AUTOLINE_H
#define AUTOLINE_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>


#include "MyCommonFun.h"

class AutoLine:public QObject
{
    Q_OBJECT
public:
    QString* receive_cmd;
public:
    AutoLine();
    ~AutoLine();

    bool connectDevice(QString port_name, qint32 baud);
    void disconnectDevice();
    bool isConnected();

    void sendCmd(const QString& send_cmd);
    void sendCmd(const QString& send_cmd, quint64 delay, quint64 send_count, quint64 span_time);

signals:
    void connectStatusChanged();
    void receiveCmd(QString code);

private:
    QSerialPort* serial = nullptr;
    AutoLine(const AutoLine&) = delete;
    const AutoLine operator=(const AutoLine&) = delete;

private slots:
    void onSerialPort_readRead();
};

#endif // AUTOLINE_H
