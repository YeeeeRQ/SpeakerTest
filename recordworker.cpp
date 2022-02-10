#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QTimer>

#include "RecordWorker.h"

RecordWorker::RecordWorker(QObject *parent)
    : QObject{parent}
{

}

void RecordWorker::doWork(quint64 duration)
{
    recorder = new QAudioRecorder(this);
    connect(recorder, SIGNAL(durationChanged(qint64)), this,
            SLOT(onDurationChanged(qint64)));

    recordDuration = duration;

    qDebug() << "RecordWorker doWork.";

//    qDebug() << "Count : " << recorder->audioInputs().count();
//    foreach (const QString &device, recorder->audioInputs())
//        qDebug() << QThread::currentThreadId()<< device; //音频录入设备

    // 检测
    // 1. 设备
    // 2. 输出路径
    // 3. 录制时长

    record();
}


void RecordWorker::onDurationChanged(qint64 duration)
{
    if(duration >= (qint64)recordDuration){
        // 到达指定时长，录制工作完毕
        stop();
        qDebug() << outputFile;
        emit resultReady();
    }
}

void RecordWorker::record()
{

    if (recorder->state() == QMediaRecorder::StoppedState) //已停止，重新设置
    {
        if (outputFile.isEmpty())
        {
            qDebug() << "RecordWorkder: 错误 << " << "请先设置录音输出文件" << Qt::endl;
            return;
        }

        if (QFile::exists(outputFile))
         if (!QFile::remove(outputFile))
         {
            qDebug() << "RecordWorkder: 错误 << " << "所设置录音输出文件被占用，无法删除" << Qt::endl;
            return;
         }
        recorder->setOutputLocation(QUrl::fromLocalFile(outputFile));//设置输出文件
    }
    //        ui->comboDevices->addItem(device); //音频录入设备列表
//    recorder->setAudioInput(recorder->defaultAudioInput()); //设置录入设备

    //Todo: 检测设备是否存在

    recorder->setAudioInput(inputDev); //设置录入设备


    QAudioEncoderSettings settings; //音频编码设置
    settings.setCodec("audio/cpm");
    settings.setSampleRate(44100);
    settings.setBitRate(128000);
    settings.setChannelCount(1);
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
    recorder->setAudioSettings(settings); //音频设置

    recorder->record();
}

void RecordWorker::stop()
{
    recorder->stop();
}

void RecordWorker::pause()
{
    recorder->pause();
}

void RecordWorker::setOutputFile(const QString& f)
{
    outputFile = f;
}

void RecordWorker::setAudioInput(const QString &dev)
{
    inputDev = dev;
//    qDebug() << "--------------";
//    qDebug() << "1. "<<dev;
//    qDebug() << "2. "<<dev.toUtf8();
//    qDebug() << "3. "<<dev.toLocal8Bit();
//    qDebug() << "--------------";
//    foreach (const QString &device, recorder->audioInputs()){
//        qDebug() << device;
//    }
//    recorder->setAudioInput(dev.toUtf8()); //设置录入设备
}
