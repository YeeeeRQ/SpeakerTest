#ifndef RECORDWORKER_H
#define RECORDWORKER_H

#include <QObject>
#include <QThread>
#include <QAudioInput>
#include <QIODevice>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QTimer>

#include <aubio/aubio.h>

struct WavFileHead
{
    char RIFFNAME[4];
    unsigned int nRIFFLength;
    char WAVNAME[4];
    char FMTNAME[4];
    unsigned int nFMTLength;
    unsigned short nAudioFormat;
    unsigned short nChannleNumber;
    unsigned int nSampleRate;
    unsigned int nBytesPerSecond;
    unsigned short nBytesPerSample;
    unsigned short   nBitsPerSample;
    char   DATANAME[4];
    unsigned int  nDataLength;
};

qint64 AddWavHeader(QString catheFileName , QString wavFileName);

/*
 * DataSource
 */
class DataSource : public QIODevice
{
    Q_OBJECT
public:
    explicit DataSource(QObject *parent = nullptr);
    ~DataSource();
    void resetStatus();


// 音频保存时长设定
public:
    bool setDuration(quint64 duration);
private:
   quint64 m_duration;

// 音频格式设定
public:
    void setAudioFormat(QAudioFormat fmt);
private:
    QAudioFormat fmt; //缺省录制格式

// 音频文件输出设定
public:
    bool setOutputFile(QString filename);
private:
    QFile* m_outputFile;


// raw -> wav 保存为WAV
private:
    WavFileHead m_wavFileHead;
signals:
    void write2WavFile();
private slots:
    void onWrite2WavFile();

// 需求：仅保存录制期间，侦测到指定频率后的录音 (每100ms检查一次频率)
// 侦测到连续300ms时长的指定频率,即认为侦测到指定频率

// 频率侦测
public:
    void setIntercept(bool open); // 侦听 开关设定
    void setInterceptTimeout(quint64 duration); // 侦听 超时设定
    void setInterceptFreqRange(qint64 freq, quint64 range);

private:
    bool singleIntercept = false; //是否打开侦测
    bool isInterceptDone = false; //侦测是否完成
    bool isInterceptTimeout = false; //侦测是否超时
    qint64 m_freq1 = 0;
    qint64 m_freq2 = 0;

    QTimer interceptCheckTimer; //侦听超时检测

    QByteArray* m_testAudioData; //测试采样


// QIODevice 数据读写
protected:
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
    bool isOK = false;
private:
    QByteArray* m_audioData;

signals:
    void getFrequency(qint64 freq); //频率获取
    void interceptDone(bool done);   //侦听是否完成
    void interceptTimeout(bool timeout);   //侦听超时
    void recordDone();
private slots:
    void onInterceptTimeout(bool timeout);
};


/*
 * RecordWorker
 */
class RecordWorker : public QObject
{
    Q_OBJECT
public:
    explicit RecordWorker(QObject *parent = nullptr);
    ~RecordWorker();

public:
    bool setOutputFile(QString filename); // 录制输出文件

    void setIntercept(bool open); // 侦听 开关设定
    void setInterceptTimeout(quint64 duration); // 侦听 超时设定
    void setInterceptFreqRange(qint64 freq, quint64 range); // 侦听 频率范围设定


signals:
    void recordDone(); // 工作流程结束
    void interceptTimeout();
    void getFrequency(qint64 freq);

public slots:
    void startRecord(quint64 duration);
    bool setMic(quint64 idx); // 输入麦克风
    void onRecordDone();

    void onInterceptTimeout(); //侦测超时处理
    void onGetFrequency(qint64 freq);

private:
    bool isRecording = false;
    quint64 duration = 0;

    QAudioFormat fmt; //缺省录制格式
    QAudioInput* audioInput = nullptr;//音频输入设备
    QAudioDeviceInfo curDevice;//当前输入设备
    QList<QAudioDeviceInfo> deviceList;  //音频录入设备列表

    DataSource* ds;

    QFile m_outputFile;
//    bool setAudioFormat();
};

#endif // RECORDWORKER_H
