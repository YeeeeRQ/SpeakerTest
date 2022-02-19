#ifndef RECORDWORKER_H
#define RECORDWORKER_H

#include <QObject>
#include <QThread>

#include <QAudioInput>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include <QIODevice>

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

class DataSource : public QIODevice
{
    Q_OBJECT
public:
    explicit DataSource(QObject *parent = nullptr);
    ~DataSource();


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
    bool setOutputFile(const QString& filename);
private:
    QFile m_outputFile;

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
private:
    QByteArray* m_testAudioData;
    bool isInterceptDone = false;

    QTimer interceptCheckTimer; //侦听超时检测


// QIODevice 数据读写
protected:
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
    bool isOK = false;
private:
    QByteArray* m_audioData;
};


class AudioProcess:public QObject
{
    Q_OBJECT

public:
    explicit AudioProcess(QObject *parent = nullptr);
    ~AudioProcess();

};

#endif // RECORDWORKER_H
