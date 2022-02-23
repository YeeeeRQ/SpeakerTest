#ifndef RECORDWORKER_H
#define RECORDWORKER_H
#include <QObject>
#include <QThread>
#include <QAudioInput>
#include <QIODevice>
#include <QFileInfo>
#include <QFile>
#include <QUrl>
#include <QDir>
#include <QTime>
#include <QTimer>
#include <QDebug>

#include <vector>

#include "DataSource.h"


// !!!默认独占麦克风
// 管理麦克风输入 QAudioInput
// 将麦克风采集数据交给DataSource处理

// 若启用侦听，到达指定频率才会开始录制

// Todo:

class RecordWorker : public QObject
{
    Q_OBJECT
public:
    explicit RecordWorker(QObject *parent = nullptr);
    ~RecordWorker();

private:
    QAudioFormat fmt; //缺省录制格式
    QAudioDeviceInfo curDevice; //当前输入设备
    QList<QAudioDeviceInfo> deviceList;  //音频录入设备列表

    QAudioInput* audioInput = nullptr; //音频输入设备
    DataSource* ds = nullptr; //

    bool m_openIntercept = false;
    bool isRecording = false;

private:
    void setRecordDuration(quint64 duration);
    bool setRecordOutputFile(const QString& filename); // 录制输出文件
    void setInterceptTimeout(quint64 duration); // 侦听 超时设定
    void setInterceptFreqRange(quint64 freq, quint64 range); // 侦听 频率范围设定

public:
    bool setRecord(quint64 duration, const QString& filename);
    void setIntercept(bool open, quint64 duration, quint64 freq, quint64 range);
    void setIntercept(bool open); // 侦听 开关设定

public slots:
    bool setMic(quint64 idx); // 输入麦克风
//    void startRecord(quint64 duration); //开启录制流程
    void startRecord(); //开启录制流程

    void onDSRecordDone();
    void onDSInterceptDone(bool done);
    void onDSGetFrequency(double freq);
    void onDSStatusChanged(RecordStatus status);

signals:
    void getFrequency(double freq); //侦听状态下频率获取
//    void interceptDone(bool done);   //侦听是否完成 true侦听完成 false侦听超时
    void recordDone(bool done, const QString& result); // 录制结束
    void statusChanged(RecordStatus status);
};

#endif // RECORDWORKER_H

//signals:
//    void recordDone(); // 工作流程结束
//    void interceptTimeout(); //
//    void getFrequency(quint64 freq);

