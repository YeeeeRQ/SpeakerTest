﻿#include "RecordWorker.h"
#include <QFileInfo>
#include <QDir>
#include <QFile>

// Todo:
//RecordWorker
// 手动|自动 模式设定
// 手动不需要侦听频率

//DataSource
// 设定 侦听超时  XXXms
// 设定 侦听频率范围 1K(±100)

// ---------------------------------------------------------------------------
// DataSource
// ---------------------------------------------------------------------------
DataSource::DataSource( QObject *parent) :
    QIODevice(parent)
{
    fmt.setSampleRate(44100);
    fmt.setChannelCount(1);
    fmt.setSampleSize(16);
    fmt.setCodec("audio/pcm");
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::UnSignedInt);

    m_outputFile = new QFile(this);
    m_audioData = new QByteArray;
    m_testAudioData = new QByteArray;
    isOK = false;


    connect(this, &DataSource::write2WavFile, this, &DataSource::onWrite2WavFile);
    connect(this, &DataSource::interceptTimeout, this, &DataSource::onInterceptTimeout);
}

DataSource::~DataSource()
{
    delete m_outputFile;
    delete m_audioData;
    delete m_testAudioData;
}

void DataSource::setAudioFormat(QAudioFormat fmt)
{
    this->fmt = fmt;
}

bool DataSource::setOutputFile(QString filename)
{
    m_outputFile->setFileName(filename);
    return m_outputFile->open(QIODevice::WriteOnly|QIODevice::Truncate);
}

bool DataSource::setDuration(quint64 duration)
{
    //!!!!!!!!!录制时长不能小于侦听测试缓存时长
    //!!!!!!!!!录制时长不能小于侦听测试缓存时长
    if(duration < 500){
        qDebug() << "录制时长不能小于侦听测试缓存时长";
        return false;
    }
    return m_duration = duration;
}

// 到达指定录制时长，保存录音文件
void DataSource::onWrite2WavFile()
{
    // wav文件保存完毕， 录制过程结束。

    m_outputFile->write(m_audioData->data(), m_audioData->size());
    m_outputFile->close();

    qDebug() << "output file : " << m_outputFile->fileName();
    AddWavHeader(m_outputFile->fileName(), m_outputFile->fileName() + ".wav");
    this->close();

    //清空音频数据
    m_audioData->clear();
    isOK=false;

    emit recordDone();
}

void DataSource::setIntercept(bool open)
{
    singleIntercept = open; //打开任务
    isInterceptDone = false;//设定未完成
    isInterceptTimeout = false;
}

void DataSource::setInterceptTimeout(quint64 duration)
{
    interceptCheckTimer.singleShot(duration, this, [=](){
        if(!isInterceptDone){ //到点还未侦测到指定频率
            isInterceptDone = false;
            isInterceptTimeout = true;
            emit interceptTimeout(true); //发送超时信号
        }
    });
}

void DataSource::setInterceptFreqRange(qint64 freq, quint64 range)
{
    m_freq1 = freq-range;
    m_freq2 = freq+range;
}

qint64 DataSource::readData(char * data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return -1;
}

qint64 DataSource::writeData(const char * data, qint64 maxSize)
{

    if(isOK) return maxSize; //录制任务已结束

    //    isInterceptDone = true;  //不进行侦测 for debug

    // 0. 侦听任务

    // 时长过滤器

    // 采集超过100ms ，检测该时段频率
    // 到达指定频率范围，发送 侦听结束信号
    // 超过指定时长， 发送 侦听超时信号

    // 接受到侦听结束信号，开始 录制指定音频并使用wav格式保存至文件

    // 打开任务 | 侦听未完成 | 未超时
    if(singleIntercept && !isInterceptDone && !isInterceptTimeout){ // 打开侦测任务
        static const quint64 size_100ms = 0.1 * (fmt.sampleRate() * fmt.sampleSize()/ 8);
        if(m_testAudioData->size() > size_100ms){ //测试数据已达100ms
            static quint64 count = 0;
            static bool prevFreqInRange = false;
            qint64 freq = 0;

            // 提取频率
            uint_t samplerate = 44100;
            uint_t win_s = 1024;
            uint_t hop_size = win_s / 4;

//            float tick = 0.0;
//            uint_t n_frames = 0, read = 0;
            fvec_t* samples = new_fvec(hop_size);
            fvec_t* pitch_out = new_fvec(2);

            aubio_pitch_t* o = new_aubio_pitch("default", win_s, hop_size, samplerate);

            // 从 m_tetsAudioData 中提取 samples 256
            int cnt = m_testAudioData->size() / samples->length;
            for(int i = 0; i<cnt ; ++i){
                for(int j =0; j<hop_size;++j){
                    samples->data[j] = (smpl_t)m_testAudioData->at(i*hop_size + j);
                }
                aubio_pitch_do(o, samples, pitch_out);

                qDebug() << "Frequency :" << pitch_out->data[0];
                freq = pitch_out->data[0];
            }
            emit getFrequency(freq);

            del_aubio_pitch(o);
            del_fvec(samples);
            del_fvec(pitch_out);
            aubio_cleanup();

            if(freq > m_freq1 && freq < m_freq2){ // 满足条件
                if(prevFreqInRange){
                    ++count;
                }else{
                    count = 1;
                }
                prevFreqInRange = true;
            }else{
                count = 0;
                prevFreqInRange = false;
            }

            // 连续5次满足指定频率 （300ms音频都是指定频率区间）
            if(count > 3){
                // 侦听任务完成
                isInterceptDone = true;
                emit interceptDone(isInterceptDone);
            }
        }
        // 清空缓存音频数据
        m_testAudioData->clear();
        m_audioData->clear();
    }else{// 侦听任务结束
        //        m_testAudioData->append(data, maxSize); //保存音频数据 for test
        m_audioData->append(data, maxSize); //保存音频数据
    }


    // 1. 录制任务
    //到达指定录制时长(通过pcm流大小测定，而非计时统计)
    quint64 size4record = (double)(m_duration/1000.0) * (double)(fmt.sampleRate() * fmt.sampleSize()/ 8);
    if(m_audioData->size() > size4record){
        isOK = true;

        // Todo：
        // 2. 测试任务

        emit write2WavFile();
    }

    return maxSize; //返回每次收到数据的大小
}

void DataSource::onInterceptTimeout(bool timeout)
{//侦测超时，停止录制
    this->close();
    m_outputFile->close();

    m_testAudioData->clear();
    m_audioData->clear();

    isOK = false;

    emit recordDone();
}

// ---------------------------------------------------------------------------
// RecordWorker
// ---------------------------------------------------------------------------
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

    ds = new DataSource();
    connect(ds, &DataSource::recordDone, this, &RecordWorker::onRecordDone);

}

RecordWorker::~RecordWorker()
{
    if(ds) delete ds;
    if(audioInput) delete audioInput;
}

void RecordWorker::startRecord(quint64 duration)
{
    if(isRecording) return;
    if(!ds->setDuration(duration)){
        qDebug() << "设定输出文件时长失败!";
    }
    //    curDevice = QAudioDeviceInfo::defaultInputDevice(); // 选择缺省设备
    if (!curDevice.isFormatSupported(fmt))
    {
        qDebug() << "Dev is Null: " <<curDevice.isNull();
        qDebug() << "测试失败，输入设备不支持此设置";
        return;
    }
    //    audioInput->start(&m_outputFile);
    ds->open(QIODevice::WriteOnly);
    audioInput->start(ds);
    isRecording = true;
}

void RecordWorker::onRecordDone()
{
    audioInput->stop();
    isRecording = false;

    emit recordDone();
}

bool RecordWorker::setMic(quint64 idx)
{
    if(idx >= deviceList.size()){
        return false;
    }
    curDevice = deviceList.at(idx);

    if(audioInput){
        delete audioInput;
        audioInput = nullptr;
    }
    audioInput = new QAudioInput(curDevice, fmt, this);
    audioInput->setBufferSize(4000);
    return true;
}

bool RecordWorker::setOutputFile(QString filename)
{
    if(!ds) return false;
    return ds->setOutputFile(filename);
}

void RecordWorker::setIntercept(bool open)
{
    if(!ds) return ;
    ds->setIntercept(open);
}

void RecordWorker::setInterceptTimeout(quint64 duration)
{
    if(!ds) return ;
    ds->setInterceptTimeout(duration);
}

void RecordWorker::setInterceptFreqRange(qint64 freq, quint64 range)
{
    if(!ds) return ;
    ds->setInterceptFreqRange(freq, range);
}



// ---------------------------------------------------------------------------

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
