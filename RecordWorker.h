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

qint64 AddWavHeader(QString catheFileName , QString wavFileName);

class RecordWorker : public QObject
{
    Q_OBJECT
public:
    explicit RecordWorker(QObject *parent = nullptr);

signals:
    void recordDone(); // 工作流程结束

public slots:
    bool setMic(quint64 idx);
    void startRecord(quint64 duration);
    void setOutputFile(QString filename);
    void micInRecording(QAudio::State s);

private:
    bool isRecording = false;
    quint64 duration = 0;

    QFile m_outputFile;
    QAudioFormat fmt; //缺省录制格式
    QAudioInput* audioInput = nullptr;//音频输入设备
    QAudioDeviceInfo curDevice;//当前输入设备
    QList<QAudioDeviceInfo> deviceList;  //音频录入设备列表
//    bool setAudioFormat();
};

#endif // RECORDWORKER_H
