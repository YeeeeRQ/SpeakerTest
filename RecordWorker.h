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


// Todo: 要么超时 要么完成

//两个模式: 侦听，录制
/*
 * RecordWorker
 */
enum class RecordStatus:uint8_t;

class RecordWorker : public QObject
{
    Q_OBJECT
public:
    explicit RecordWorker(QObject *parent = nullptr);
    ~RecordWorker();

public:
    bool setExclusive();
	
    bool setOutputFile(QString filename); // 录制输出文件
	
    void setIntercept(bool open); // 侦听 开关设定
    void setInterceptTimeout(quint64 duration); // 侦听 超时设定
    void setInterceptFreqRange(quint64 freq, quint64 range); // 侦听 频率范围设定

signals:
    void recordDone(); // 工作流程结束
    void interceptTimeout(); //
    void getFrequency(quint64 freq);

public slots:
    void startRecord(quint64 duration);
    bool setMic(quint64 idx); // 输入麦克风

    void onRecordDone();
    void onInterceptTimeout(); //侦测超时处理
    void onGetFrequency(quint64 freq);

private:
    bool isExclusive = false; // 独占麦克风

    QAudioFormat fmt; //缺省录制格式
    QAudioDeviceInfo curDevice;//当前输入设备
    QList<QAudioDeviceInfo> deviceList;  //音频录入设备列表

    QAudioInput* audioInput = nullptr;//音频输入设备
    DataSource* ds = nullptr; //

};

#endif // RECORDWORKER_H
