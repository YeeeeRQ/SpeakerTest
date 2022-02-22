#include "RecordWorker.h"
// Todo:
//RecordWorker

//设备设定 //设备切换

//录制参数
//	时长
//  输出目录

//侦听参数
//  侦听超时
//  频率设定


//DataSource
// 设定 侦听超时  XXXms
// 设定 侦听频率范围 1K(±100)

// ---------------------------------------------------------------------------
// RecordWorker
// ---------------------------------------------------------------------------


// setMic -> set
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

    ds = new DataSource();

    connect(ds, &DataSource::recordDone, this, &RecordWorker::onRecordDone);
    connect(ds, &DataSource::interceptTimeout, this, &RecordWorker::onInterceptTimeout);
    connect(ds, &DataSource::getFrequency,this, &RecordWorker::onGetFrequency);
}

RecordWorker::~RecordWorker()
{
    if(ds) delete ds;
    if(audioInput) delete audioInput;
}

bool RecordWorker::setExclusive()
{
    if(!ds)return false;
    if(!audioInput) return false;

    if(ds->open(QIODevice::WriteOnly)){
        audioInput->start(ds);
    }else{
        return false;
    }
    return true;
}

void RecordWorker::startRecord(quint64 duration)
{
    if(record_status == RecordStatus::RecordingMode) return;

    if(!ds->setDuration(duration)){
        qDebug() << "设定输出文件时长失败!";
    }
    //    curDevice = QAudioDeviceInfo::defaultInputDevice(); // 选择缺省设备
    if (!curDevice.isFormatSupported(fmt))
    {
        qDebug() << "Dev is Null: " <<curDevice.isNull();
        qDebug() << "测试失败，输入设备不支持此设置";
        return;
    }

//    isRecording = true;
    record_status = RecordStatus::RecordingMode;
}

void RecordWorker::onRecordDone()
{
    audioInput->stop(); //关闭麦克风输入
    ds->resetStatus(); //ds状态重置
//    isRecording = false;

    emit recordDone(); //告知上游
    record_status = RecordStatus::IdleMode;
}

void RecordWorker::onInterceptTimeout()
{
    audioInput->stop(); //关闭麦克风输入
    ds->resetStatus(); //ds状态重置
//    isRecording = false;

    emit interceptTimeout(); // 告知上游, 侦测超时， 录制失败
    record_status = RecordStatus::IdleMode;
}

void RecordWorker::onGetFrequency(quint64 freq)
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

    if(audioInput){
        delete audioInput;
        audioInput = nullptr;
    }
    audioInput = new QAudioInput(curDevice, fmt, this);
    audioInput->setBufferSize(4000);
    return true;
}

bool RecordWorker::setOutputFile(QString filename)
{
    if(!ds) return false;
    return ds->setOutputFile(filename);
}

void RecordWorker::setIntercept(bool open)
{
    if(!ds) return ;
    ds->setIntercept(open);
}

void RecordWorker::setInterceptTimeout(quint64 duration)
{
    if(!ds) return ;
    ds->setInterceptTimeout(duration);
}

void RecordWorker::setInterceptFreqRange(quint64 freq, quint64 range)
{
    if(!ds) return ;
    ds->setInterceptFreqRange(freq, range);
}


