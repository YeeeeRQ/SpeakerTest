#include "AutoLine.h"

AutoLine::AutoLine()
{
    serial = new QSerialPort();
    receive_cmd = new QString();
    connect(serial, &QSerialPort::readyRead, this, &AutoLine::onSerialPort_readRead);
}

AutoLine::~AutoLine()
{
}

void AutoLine::sendCmd(const QString &s_cmd)
{
    if(!serial->isOpen()){
        qDebug() << "AutoLine 未连接!";
        return;
    }
    serial->write(s_cmd.toLatin1());
}

void AutoLine::sendCmd(const QString &send_cmd, quint64 delay, quint64 send_count, quint64 span_time)
{
    if(!serial->isOpen()){
        qDebug() << "AutoLine 未连接!";
        return;
    }

    if(delay > 500) delay = 500;
    if(send_count > 5) send_count = 5;
    if(span_time > 2000) span_time = 2000;

    delaymsec(delay);
    for(quint64 i = 0; i< send_count; ++i){

        serial->write(send_cmd.toLatin1());

        delaymsec(span_time);
    }
}

void AutoLine::onSerialPort_readRead()
{
    QByteArray buffer = serial->readAll();

    QString temp;
    qDebug() << "buffer size : " << buffer.size();
    for(int i = 0; i< buffer.size(); ++i){
        quint8 c = (quint8)buffer.at(i);
        if(c < 0x0F) temp.append("0");
        temp.append(QString::number(c, 16).toUpper());
        temp.append(" ");
    }
    temp.remove(temp.size()-1, 1);
    qDebug() << buffer;
    qDebug() << temp;

    //拼接收到的字符
    receive_cmd->append(buffer);

    // 判断后两个字节是否为 0x0D 0x0A
    if((quint8)buffer.at(buffer.size()-1) == 0x0A &&
       (quint8)buffer.at(buffer.size()-2) == 0x0D)
    {
        qDebug() << "CodeReader: " << *receive_cmd;

        receive_cmd->chop(2);
        emit receiveCmd(*receive_cmd);

        this->receive_cmd->clear();
    }
}

bool AutoLine::connectDevice(QString port_name, QSerialPort::BaudRate baud)
{
    if(serial->isOpen()) return true;

    serial->setPortName(port_name);
    serial->setBaudRate(baud);

    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);


    if(serial->open(QIODevice::ReadWrite)){
        emit connectStatusChanged();
        qDebug() << "打开成功";
    }else{
        qDebug() << "打开失败";
    }
    return serial->isOpen();
}

void AutoLine::disconnectDevice()
{
    if(serial->isOpen()){
       serial->close();
       emit connectStatusChanged();
    }
}

bool AutoLine::isConnected()
{
    return serial->isOpen();
}
