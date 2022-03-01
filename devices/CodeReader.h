#ifndef CODE_READER_H
#define CODE_READER_H

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QDebug>

#include "./log/simplelog.h"

class CodeReader: public QObject
{
    Q_OBJECT
public:
    CodeReader();
    ~CodeReader();
    void setLogger(SimpleLog* log); //for debug
    bool connectDevice(QString port_name, qint32 baud);
    void disconnectDevice();
    bool isConnected();

    QString* code;

signals:
    void connectStatusChanged();
    void receiveBarcode(QString code);

private:
    QSerialPort* serial = nullptr;
    SimpleLog* log = nullptr; //for debug

    CodeReader(const CodeReader&) = delete;
    const CodeReader& operator=(const CodeReader&) = delete;

private slots:
    void onSerialPort_readRead();
};

#endif // CODE_READER_H
