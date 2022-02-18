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

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

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
    void setAudioFormat(QAudioFormat fmt);

signals:
    void write2WavFile();
private:
    void onWrite2WavFile();

protected:
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
    bool isOK = false;

private:
    QByteArray* m_audioData;
    QAudioFormat fmt; //缺省录制格式
    WavFileHead m_wavFileHead;
    quint64 curDuration=0;
    quint64 recDuration=0;
};
#endif // RECORDWORKER_H
