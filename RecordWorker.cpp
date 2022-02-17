#include "RecordWorker.h"

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
}

void RecordWorker::startRecord(quint64 duration)
{
    if(isRecording) return;
    this->duration = duration;
//    curDevice = QAudioDeviceInfo::defaultInputDevice(); // 选择缺省设备
    if (!curDevice.isFormatSupported(fmt))
    {
        qDebug() << "Dev is Null: " <<curDevice.isNull();
        qDebug() << "测试失败，输入设备不支持此设置";
        return;
    }
    audioInput->start(&m_outputFile);
    isRecording = true;
}

bool RecordWorker::setMic(quint64 idx)
{
    if(idx >= deviceList.size()){
        return false;
    }
    curDevice = deviceList.at(idx);

    if(audioInput){
        delete audioInput;
    }
    audioInput = new QAudioInput(curDevice, fmt, this);
    connect(audioInput, &QAudioInput::stateChanged, this, &RecordWorker::micInRecording);
    audioInput->setBufferSize(4000);
    return true;
}

void RecordWorker::setOutputFile(QString filename)
{
    m_outputFile.setFileName(filename);
    m_outputFile.open(QIODevice::WriteOnly|QIODevice::Truncate);
}

void RecordWorker::micInRecording(QAudio::State s)
{
    if(s != QAudio::ActiveState) return;
    static QTimer timer;
    timer.singleShot(duration, this, [=](){
        audioInput->stop();
        m_outputFile.close();

        isRecording = false;
        //Todo:
        // 1. raw -> wav
        // 2. emit record done
        if (AddWavHeader(m_outputFile.fileName(), m_outputFile.fileName() + ".wav") > 0){
            qDebug() << "raw --> wav";
        }

        emit recordDone();
    });
}

const qint64 TIME_TRANSFORM = 1000 * 1000;              // 微妙转秒;

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

qint64 AddWavHeader(QString catheFileName , QString wavFileName)
{
    // 开始设置WAV的文件头
    WAVFILEHEADER WavFileHeader;
    qstrcpy(WavFileHeader.RiffName, "RIFF");
    qstrcpy(WavFileHeader.WavName, "WAVE");
    qstrcpy(WavFileHeader.FmtName, "fmt ");
    qstrcpy(WavFileHeader.DATANAME, "data");

    // 表示 FMT块 的长度
    WavFileHeader.nFmtLength = 16;
    // 表示 按照PCM 编码;
    WavFileHeader.nAudioFormat = 1;
    // 声道数目;
    WavFileHeader.nChannleNumber = 1;
    // 采样频率;
//    WavFileHeader.nSampleRate = 8000;
    WavFileHeader.nSampleRate = 44100;

    // nBytesPerSample 和 nBytesPerSecond这两个值通过设置的参数计算得到;
    // 数据块对齐单位(每个采样需要的字节数 = 通道数 × 每次采样得到的样本数据位数 / 8 )
    WavFileHeader.nBytesPerSample = 2;
    // 波形数据传输速率
    // (每秒平均字节数 = 采样频率 × 通道数 × 每次采样得到的样本数据位数 / 8  = 采样频率 × 每个采样需要的字节数 )
//    WavFileHeader.nBytesPerSecond = 16000;
    WavFileHeader.nBytesPerSecond = 88200;

    // 每次采样得到的样本数据位数;
    WavFileHeader.nBitsPerSample = 16;

    QFile cacheFile(catheFileName);
    QFile wavFile(wavFileName);

    if (!cacheFile.open(QIODevice::ReadWrite))
    {
        return -1;
    }
    if (!wavFile.open(QIODevice::WriteOnly))
    {
        return -2;
    }

    int nSize = sizeof(WavFileHeader);
    qint64 nFileLen = cacheFile.bytesAvailable();

    WavFileHeader.nRiffLength = nFileLen - 8 + nSize;
    WavFileHeader.nDataLength = nFileLen;

    // 先将wav文件头信息写入，再将音频数据写入;
    wavFile.write((char *)&WavFileHeader, nSize);
    wavFile.write(cacheFile.readAll());

    cacheFile.close();
    wavFile.close();

    return nFileLen;
}
