#include "RecordWorker.h"
// Todo:
//1. 参数检测

//RecordWorker

//设备设定 //设备切换

//录制参数
//	时长
//  输出目录

//侦听参数
//  侦听超时
//  频率设定

RecordWorker::RecordWorker(QObject *parent)
    : QObject{parent}
{
    fmt.setSampleRate(44100);
    fmt.setChannelCount(1);
    fmt.setSampleSize(16);
    fmt.setCodec("audio/pcm");
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::UnSignedInt);

    deviceList=QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    ds = new DataSource(); //委托关系
    ds->open(QIODevice::WriteOnly);

    connect(ds, &DataSource::recordDone, this, &RecordWorker::onDSRecordDone);
    connect(ds, &DataSource::interceptDone, this, &RecordWorker::onDSInterceptDone);
    connect(ds, &DataSource::getFrequency,this, &RecordWorker::onDSGetFrequency);
}

RecordWorker::~RecordWorker()
{
    if(ds){
        ds->close();
        delete ds;
    }
    if(audioInput){
        delete audioInput;
    }
}

bool RecordWorker::setRecord(quint64 duration, const QString &filename)
{
    this->setRecordDuration(duration);
    return this->setRecordOutputFile(filename);
}

void RecordWorker::setRecordDuration(quint64 duration)
{
    ds->setDuration(duration);
}

bool RecordWorker::setRecordOutputFile(const QString& filename)
{
    return ds->setOutputFile(filename);
}

void RecordWorker::startRecord()
{
    Q_ASSERT(ds->isIdle() == true);

    isRecording = true;
    if(m_openIntercept){
        ds->changeRecordStatus(RecordStatus::InterceptMode);
    }else{
        ds->changeRecordStatus(RecordStatus::RecordingMode);
    }
}

void RecordWorker::onDSRecordDone()
{
    Q_ASSERT(isRecording == true);

    isRecording = false;
    emit recordDone(true, "RecordDone");
}

void RecordWorker::onDSInterceptDone(bool done)
{
    Q_ASSERT(isRecording == true);
    Q_ASSERT(ds->isIdle() == true);

    if(done){
        // 侦听到指定频率
        ds->changeRecordStatus(RecordStatus::RecordingMode);
    }else{
        //超时了
        isRecording = false;
        emit recordDone(done, "侦听超时");
    }
}

void RecordWorker::onDSGetFrequency(double freq)
{
    emit getFrequency(freq);
}

bool RecordWorker::setMic(quint64 idx)
{
    if(deviceList.isEmpty()) return false;

    if(idx >= deviceList.size()){
        return false;
    }
    curDevice = deviceList.at(idx);

    // 释放当前 音频输入
    if(audioInput){
        delete audioInput;
        audioInput = nullptr;
    }

    audioInput = new QAudioInput(curDevice, fmt, this);
    audioInput->setBufferSize(4000);
    audioInput->start(ds);

    return true;
}

void RecordWorker::setIntercept(bool open, quint64 duration, quint64 freq, quint64 range)
{
    this->setIntercept(open);
    this->setInterceptTimeout(duration);
    this->setInterceptFreqRange(freq, range);
}

void RecordWorker::setIntercept(bool open)
{
    m_openIntercept = open;
}

void RecordWorker::setInterceptTimeout(quint64 duration)
{
    ds->setInterceptTimeout(duration);
}

void RecordWorker::setInterceptFreqRange(quint64 freq, quint64 range)
{
    ds->setInterceptFreqRange(freq, range);
}


















//void RecordWorker::onInterceptTimeout()
//{
//    audioInput->stop(); //关闭麦克风输入
//    ds->resetStatus(); //ds状态重置
////    isRecording = false;

//    emit interceptTimeout(); // 告知上游, 侦测超时， 录制失败
//    record_status = RecordStatus::IdleMode;
//}


//bool RecordWorker::setExclusive()
//{
//    if(!ds)return false;
//    if(!audioInput) return false;

//    if(ds->open(QIODevice::WriteOnly)){
//        audioInput->start(ds);
//    }else{
//        return false;
//    }
//    return true;
//}

//    connect(ds, &DataSource::interceptTimeout, this, &RecordWorker::onInterceptTimeout);

//void RecordWorker::startRecord(quint64 duration)

//    if(record_status == RecordStatus::RecordingMode) return;

//    if(!ds->setDuration(duration)){
//        qDebug() << "设定输出文件时长失败!";
//    }
    //    curDevice = QAudioDeviceInfo::defaultInputDevice(); // 选择缺省设备

//    if (!curDevice.isFormatSupported(fmt))
//    {
//        qDebug() << "Dev is Null: " <<curDevice.isNull();
//        qDebug() << "测试失败，输入设备不支持此设置";
//        return;
//    }

//    isRecording = true;
//    record_status = RecordStatus::RecordingMode;
