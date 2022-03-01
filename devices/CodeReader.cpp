#include "CodeReader.h"

CodeReader::CodeReader()
{
    serial = new QSerialPort();
    code = new QString();
    connect(serial, &QSerialPort::readyRead, this, &CodeReader::onSerialPort_readRead);
}

CodeReader::~CodeReader()
{
    this->log = nullptr;
}

void CodeReader::setLogger(SimpleLog *log)
{
    this->log = log;
}

bool CodeReader::connectDevice(QString port_name, qint32 baud)
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
//        log->info("打开成功");
    }else{
//        log->warn("打开失败");
    }
    return serial->isOpen();
}

void CodeReader::disconnectDevice()
{
    if(serial->isOpen()){
       serial->close();
       emit connectStatusChanged();
    }
    log->warn("关闭读码器连接.");
}

bool CodeReader::isConnected()
{
    return serial->isOpen();
}

void CodeReader::onSerialPort_readRead()
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

    emit receiveBarcode(buffer);

//    QByteArray buffer = serial->readAll();

//    QString temp;
//    qDebug() << "buffer size : " << buffer.size();
//    for(int i = 0; i< buffer.size(); ++i){
//        quint8 c = (quint8)buffer.at(i);
//        if(c < 0x0F) temp.append("0");
//        temp.append(QString::number(c, 16).toUpper());
//        temp.append(" ");
//    }
//    temp.remove(temp.size()-1, 1);
//    log->info(buffer);
//    log->info(temp);

//    //拼接收到的字符
//    code->append(buffer);

//    // 判断后两个字节是否为 0x0D 0x0A
//    if((quint8)buffer.at(buffer.size()-1) == 0x0A &&
//       (quint8)buffer.at(buffer.size()-2) == 0x0D)
//    {
//        qDebug() << "CodeReader: " << *code;

//        code->chop(2);
//        emit receiveBarcode(*code);

//        this->code->clear();
//    }
}
