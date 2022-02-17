#ifndef RECORDWORKER_H
#define RECORDWORKER_H

#include <QObject>
#include <QThread>

#include <QAudioRecorder>
#include <QAudioProbe>
#include <QAudioInput>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QTimer>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

// 指定设备(1. 初始化函数 2. 额外接口函数)
// 录音时长
// 输出目录

qint64 AddWavHeader(QString catheFileName , QString wavFileName);

class RecordWorker : public QObject
{
    Q_OBJECT
public:
    explicit RecordWorker(QObject *parent = nullptr);

signals:
    void resultReady(); // 工作流程结束

public slots:
//    void doWork(quint64 duration);
    void startRecord(quint64 duration, QFile& file);
    void setAudioInput(const QString& dev);

private:

    QList<QAudioDeviceInfo> deviceList;  //音频录入设备列表
    QAudioDeviceInfo curDevice;//当前输入设备
    QAudioInput* audioInput;//音频输入设备
    QAudioFormat fmt; //缺省录制格式

    bool setAudioFormat();

};

#endif // RECORDWORKER_H
