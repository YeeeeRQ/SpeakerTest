#ifndef DATASOURCE_H
#define DATASOURCE_H
#include <QObject>
#include <QThread>
#include <QAudioInput>
#include <QIODevice>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include <aubio/aubio.h>

struct WAVFILEHEADER
{
    // RIFF 头
    char RiffName[4];
    unsigned long nRiffLength;
    // 数据类型标识符
    char WavName[4];
    // 格式块中的块头
    char FmtName[4];
    unsigned long nFmtLength;
    // 格式块中的块数据
    unsigned short nAudioFormat;
    unsigned short nChannleNumber;
    unsigned long nSampleRate;
    unsigned long nBytesPerSecond;
    unsigned short nBytesPerSample;
    unsigned short nBitsPerSample;
    // 数据块中的块头
    char    DATANAME[4];
    unsigned long   nDataLength;
};

enum class RecordStatus:uint8_t
{
    IdleMode,
    InterceptMode,
    RecordingMode
};

class DataSource : public QIODevice
{
    Q_OBJECT
public:
    explicit DataSource(QObject *parent = nullptr);
    ~DataSource();

// 设定
public:
    // 状态
    bool isIdle();
    RecordStatus getRecordStatus();
    bool changeRecordStatus(RecordStatus status);

    // 音频录制
    bool setDuration(quint64 duration); // 音频保存时长设定
    void setAudioFormat(QAudioFormat fmt); // 音频格式设定
    bool setOutputFile(QString filename); // 音频文件输出设定

    // 频率侦测
    void setInterceptTimeout(quint64 duration); // 侦听 超时设定
    void setInterceptFreqRange(quint64 freq, quint64 range);

// QIODevice 数据读写
protected:
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);

private:
    RecordStatus m_recordStatus;

    quint64 m_duration = 0;
    quint64 m_duration4Intercept = 0;
//    quint64 m_size4timeout = 0;
    QTimer timer;
    bool m_isTimeout = false;

    QAudioFormat m_fmt; //缺省录制格式
    quint64 m_freq1 = 0;
    quint64 m_freq2 = 0;

    QByteArray* m_audioData;     //录制采样
    QByteArray* m_testAudioData; //侦听测试采样
    QFile* m_outputFile;

    void save2WAV();
    double getAudioFrequency();

signals:
    void getFrequency(double freq); //侦听状态下频率获取
    void interceptDone(bool done);   //侦听是否完成 true侦听完成 false侦听超时
    void recordDone();
private slots:
    void onInterceptTimeout();
};

#endif // DATASOURCE_H
