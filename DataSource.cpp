#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTime>
#include <vector>

#include "DataSource.h"

// !!!仅闲置时才可以设定属性

// Todo:

// 侦听设定
    // 侦听超时  XXXms
    // 侦听频率范围 1K(±100Hz)

// 录制设定
    // 输出文件
    // 录制时长
    // 默认格式

DataSource::DataSource( QObject *parent) :
    QIODevice(parent)
{
    m_fmt.setSampleRate(44100);
    m_fmt.setChannelCount(1);
    m_fmt.setSampleSize(16);
    m_fmt.setCodec("audio/pcm");
    m_fmt.setByteOrder(QAudioFormat::LittleEndian);
    m_fmt.setSampleType(QAudioFormat::UnSignedInt);

    m_outputFile = new QFile(this);
    m_audioData = new QByteArray;
    m_testAudioData = new QByteArray;
}

DataSource::~DataSource()
{
    delete m_outputFile;
    delete m_audioData;
    delete m_testAudioData;
}

void DataSource::setAudioFormat(QAudioFormat fmt)
{
    this->m_fmt = fmt;
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

bool DataSource::isIdle()
{
    return (m_recordStatus == RecordStatus::IdleMode);
}

RecordStatus DataSource::getRecordStatus()
{
    return m_recordStatus;
}

double DataSource::getAudioFrequency()
{
    // 测试音频大小
//        qDebug() << "cache :" << m_testAudioData->size();

        uint_t samplerate = m_fmt.sampleRate();
//        uint_t samplerate = 44100;
        uint_t win_s = 1024;
        uint_t hop_size = win_s / 4;

        fvec_t* samples = new_fvec(hop_size);
        fvec_t* pitch_out = new_fvec(2);

        std::vector<uint_t> v_point;
        std::vector<uint_t> len1_pitch;
        aubio_pitch_t* o = new_aubio_pitch("default", win_s, hop_size, samplerate);

        // 按2字节(16bit)长度读取，256一组， 封装samples
        for(quint64 i = 0; i< m_testAudioData->size() ;i+=2 ){
            quint16 point =((quint16)m_testAudioData->at(i+1) << 8) | ((quint16)m_testAudioData->at(i) &0x00ff);
            v_point.push_back(point);
        }
//            qDebug() << "v_point size:" << v_point.size();

        for(quint64 i = 0 ; i < v_point.size()/hop_size; ++i){
            for(quint64 j =0; j< hop_size; ++j){
                samples->data[j] = v_point.at(i*hop_size+j);
            }

            aubio_pitch_do(o, samples, pitch_out);
            len1_pitch.push_back(pitch_out->data[0]);
        }

        double freq = std::accumulate(len1_pitch.begin(), len1_pitch.end(), 0.0) / len1_pitch.size();

//            qDebug() << "Frequency :" << len1_pitch_avg;

        del_aubio_pitch(o);
        del_fvec(samples);
        del_fvec(pitch_out);
        aubio_cleanup();

        return freq;
}

void DataSource::setInterceptFreqRange(quint64 freq, quint64 range)
{
//    if(range > 300){
//    }
    m_freq1 = freq-range;
    m_freq2 = freq+range;
}

void DataSource::save2WAV()
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
    //    WavFileHeader.nSampleRate = 44100;
    WavFileHeader.nSampleRate = m_fmt.sampleRate();

    // nBytesPerSample 和 nBytesPerSecond这两个值通过设置的参数计算得到;
    // 数据块对齐单位(每个采样需要的字节数 = 通道数 × 每次采样得到的样本数据位数 / 8 )
    WavFileHeader.nBytesPerSample = 2;
    // 波形数据传输速率
    // (每秒平均字节数 = 采样频率 × 通道数 × 每次采样得到的样本数据位数 / 8  = 采样频率 × 每个采样需要的字节数 )
    //    WavFileHeader.nBytesPerSecond = 16000;
    WavFileHeader.nBytesPerSecond = 88200;

    // 每次采样得到的样本数据位数;
    WavFileHeader.nBitsPerSample = 16;

    int nSize = sizeof(WavFileHeader);
    qint64 nFileLen = m_audioData->size();

    WavFileHeader.nRiffLength = nFileLen - 8 + nSize;
    WavFileHeader.nDataLength = nFileLen;

    // 先将wav文件头信息写入，再将音频数据写入;
    m_outputFile->write((char *)&WavFileHeader, nSize);
    m_outputFile->write(*m_audioData);

    m_outputFile->close();
}

// Idle -> Recording | Idle -> Intercept
bool DataSource::changeRecordStatus(RecordStatus status)
{
    if(this->isIdle()){
        // 开始侦听模式
        if(RecordStatus::InterceptMode == status){
            m_testAudioData->clear();
            m_recordStatus = status;
        }
        // 开始录制模式
        if(RecordStatus::RecordingMode == status){
            m_audioData->clear();
            m_recordStatus = status;
        }
    }else{
        return false;
    }
    return true;
}

qint64 DataSource::writeData(const char * data, qint64 maxSize)
{
    if(m_recordStatus == RecordStatus::IdleMode){
        // do nothing
        return maxSize;
    }

    if(m_recordStatus == RecordStatus::InterceptMode){
        // 参数设定检测

        // 超时检测
        static quint64 timeout_size = 0;

        quint64 size4timeout =
                (double)(m_duration4Intercept/1000.0) *
                (double)(m_fmt.sampleRate() * m_fmt.sampleSize()/ 8);

        if(timeout_size > size4timeout){
            //侦听超时
            m_recordStatus = RecordStatus::IdleMode;
            m_testAudioData->clear();
            timeout_size = 0;
            emit interceptDone(false);
        }

        // 测试音频大小
        static const quint64 size_300ms = 0.3 * (m_fmt.sampleRate() * m_fmt.sampleSize()/ 8);
//        qDebug() << m_testAudioData->size();
        if(m_testAudioData->size() >= size_300ms){ //测试数据已达300ms
            // 次数统计
            static quint64 count = 0;
            static bool prevFreqInRange = false;

            double freq  = this->getAudioFrequency();

            emit getFrequency(freq);

//            qDebug() << "Freq : " << m_freq1 << " ~ " << m_freq2;
            if(freq > m_freq1 && freq < m_freq2){ // 满足条件
                qDebug() << "In Range!!";

                // 累计计数
                if(prevFreqInRange){
                    ++count;
                }else{
                    count = 1;
                }
                prevFreqInRange = true;
            }else{
                // 重置次数统计
                count = 0;
                prevFreqInRange = false;
            }

            // 连续X次满足指定频率
            if(count > 2){
                qDebug() << "Intercep Done!!";

                m_recordStatus = RecordStatus::IdleMode;
                m_testAudioData->clear();
                timeout_size = 0;

                emit interceptDone(true);
            }
            // 清空缓存音频数据
            m_testAudioData->clear();
        }

        m_testAudioData->append(data, maxSize); //保存音频数据 for test


        timeout_size += maxSize; //累计测试数据大小
        return maxSize;
    }

    if(m_recordStatus == RecordStatus::RecordingMode){
        // 参数设定检测

        m_audioData->append(data, maxSize); //保存音频数据

        // 1. 录制任务
        //到达指定录制时长(通过pcm流大小测定，而非计时统计)
        quint64 size4record = (double)(m_duration/1000.0) * (double)(m_fmt.sampleRate() * m_fmt.sampleSize()/ 8);
        if(m_audioData->size() > size4record){
            // 到达指定录制时长
            this->save2WAV();

            m_recordStatus = RecordStatus::IdleMode;
            m_audioData->clear();

            emit recordDone();

            // Todo：
            // 2. 测试任务 (ConsoleAudioTest满足需求， 暂不更新)
        }

        return maxSize;
    }

    return maxSize; //返回每次收到数据的大小
}

qint64 DataSource::readData(char * data, qint64 maxSize)
{
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    return -1;
}
