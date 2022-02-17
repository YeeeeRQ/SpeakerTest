#include "RecordWorker.h"

// 设定设备
// 检测设定设备 (存在且支持设置)

RecordWorker::RecordWorker(QObject *parent)
    : QObject{parent}
{
    fmt.setSampleRate(441000);
    fmt.setChannelCount(1);
    fmt.setSampleSize(16);
    fmt.setCodec("audio/pcm");
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::UnSignedInt);

//    connect(&timer, &QTimer::timeout, this, &RecordWorker::stopRecord);

//    timer.setSingleShot(false); // 单次触发

//    file.setFileName("D:\temp\test.raw");
//    file.open(QIODevice::WriteOnly|QIODevice::Truncate);

    audioInput = new QAudioInput(curDevice, fmt, this);
}

void RecordWorker::startRecord(quint64 duration, QFile& file)
{
    curDevice = QAudioDeviceInfo::defaultInputDevice(); // 选择缺省设备
    if (!curDevice.isFormatSupported(fmt))
    {
//        QMessageBox::critical(this,"音频输入设置测试","测试失败，输入设备不支持此设置");
        return;
    }
    audioInput->start(&file);
    QTimer timer;
    timer.singleShot(duration, this, [=](){
        audioInput->stop();
    });
}


//void RecordWorker::record()
//{
/*
    if (recorder->state() == QMediaRecorder::StoppedState) //已停止，重新设置
    {
        if (outputFile.isEmpty())
        {
            qDebug() << "RecordWorkder: 错误 << " << "请先设置录音输出文件" << Qt::endl;
            return;
        }

        if (QFile::exists(outputFile))
         if (!QFile::remove(outputFile))
         {
            qDebug() << "RecordWorkder: 错误 << " << "所设置录音输出文件被占用，无法删除" << Qt::endl;
            return;
         }
        recorder->setOutputLocation(QUrl::fromLocalFile(outputFile));//设置输出文件
    }
    //        ui->comboDevices->addItem(device); //音频录入设备列表
//    recorder->setAudioInput(recorder->defaultAudioInput()); //设置录入设备

    //Todo: 检测设备是否存在

    qDebug()<<inputDev;
    recorder->setAudioInput(inputDev); //设置录入设备


    QAudioEncoderSettings settings; //音频编码设置
    settings.setCodec("audio/cpm");
    settings.setSampleRate(44100);
    settings.setBitRate(128000);
    settings.setChannelCount(1);
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
    recorder->setAudioSettings(settings); //音频设置

    recorder->record()*/;
//}

bool RecordWorker::setAudioFormat()
{
    fmt.setCodec("audio/pcm");
    fmt.setChannelCount(1);
    fmt.setSampleRate(441000);
    fmt.setSampleSize(16);
    fmt.setSampleType(QAudioFormat::UnSignedInt);
    fmt.setByteOrder(QAudioFormat::LittleEndian);

    if (curDevice.isFormatSupported(fmt)){
        return true;
    }else{
        return false;
    }
}

//void RecordWorker::setAudioInput(const QString &dev)
//{
//    qDebug() << "Set AudioInput: "<<dev;
//    inputDev = dev;

////    qDebug() << "--------------";
////    qDebug() << "1. "<<dev;
////    qDebug() << "2. "<<dev.toUtf8();
////    qDebug() << "3. "<<dev.toLocal8Bit();
////    qDebug() << "--------------";
////    foreach (const QString &device, recorder->audioInputs()){
////        qDebug() << device;
////    }
////    recorder->setAudioInput(dev.toUtf8()); //设置录入设备
//}




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
    // 这里具体的数据代表什么含义请看上一篇文章（Qt之WAV文件解析）中对wav文件头的介绍
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
