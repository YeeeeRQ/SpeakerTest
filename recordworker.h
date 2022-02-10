#ifndef RECORDWORKER_H
#define RECORDWORKER_H

#include <QObject>
#include <QThread>
#include <QAudioRecorder>
#include <QAudioProbe>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

// 指定设备(1. 初始化函数 2. 额外接口函数)
// 录音时长
// 输出目录

class RecordWorker : public QObject
{
    Q_OBJECT
public:
    explicit RecordWorker(QObject *parent = nullptr);

public slots:
    void setOutputFile(const QString& f);
    void setAudioInput(const QString& dev);
    void doWork(quint64 duration);

private:
    QAudioRecorder *recorder;//音频录音
    QString outputFile;
    QString inputDev;
    quint64 recordDuration;

    void record();
    void stop();
    void pause();

private slots:
    void onDurationChanged(qint64 duration);//recorder

signals:
    void resultReady(); // 工作流程结束

};

#endif // RECORDWORKER_H
