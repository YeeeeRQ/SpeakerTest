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
    connect(ds, &DataSource::statusChanged,this, &RecordWorker::onDSStatusChanged);
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
        qDebug() << QTime::currentTime() <<" Intercept Mode";
        ds->changeRecordStatus(RecordStatus::InterceptMode);
    }else{
        qDebug() << QTime::currentTime() <<" Recording Mode";
        ds->changeRecordStatus(RecordStatus::RecordingMode);
    }
}

void RecordWorker::stopRecord()
{
//    Q_ASSERT(ds->isIdle() == false);

    isRecording = false;
    ds->changeRecordStatus(RecordStatus::IdleMode);
}

bool RecordWorker::isIdle()
{
    return ds->isIdle();
}

void RecordWorker::onDSRecordDone()
{
    Q_ASSERT(isRecording == true);

    isRecording = false;
    emit recordDone(true, "RecordDone");
}

void RecordWorker::onDSInterceptDone(bool done)
{
//    Q_ASSERT(isRecording == true); //??
    Q_ASSERT(ds->isIdle() == true);

    if(done){
        // 侦听到指定频率 -> 立即开始录制
        // ds->changeRecordStatus(RecordStatus::RecordingMode);

        // 发射侦听完成信号
        emit interceptDone();


    }else{
        // 侦听超时 ->录制流程直接结束(不发射侦听完成信号)
        isRecording = false;
        emit recordDone(done, "侦听超时");
    }
}

void RecordWorker::onDSGetFrequency(double freq)
{
    emit getFrequency(freq);
}

void RecordWorker::onDSStatusChanged(RecordStatus status)
{
    emit statusChanged(status);
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

void RecordWorker::openIntercept(quint64 duration, quint64 freq, quint64 range)
{
    m_openIntercept = true;
    this->setInterceptTimeout(duration);
    this->setInterceptFreqRange(freq, range);
}

void RecordWorker::closeIntercept()
{
    m_openIntercept = false;
}

void RecordWorker::setInterceptTimeout(quint64 duration)
{
    ds->setInterceptTimeout(duration);
}

void RecordWorker::setInterceptFreqRange(quint64 freq, quint64 range)
{
    ds->setInterceptFreqRange(freq, range);
}
