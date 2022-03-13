#include "AudioProcess.h"
#include <QDebug>

// Todo:
// (废弃)通过onsets主动判断起始点。
// 侦听第二段频率

AudioProcess::AudioProcess( QObject *parent)
{
}

AudioProcess::~AudioProcess()
{
}

bool AudioProcess::setAudioFilePath(const QString& target_dir) {
    this->target_dir = target_dir;
    return true;
}

void AudioProcess::setDuration(quint64 duration1, quint64 range1, quint64 duration2, quint64 range2)
{
    len1[0] = duration1 - range1;
    len1[1] = duration1 + range1;
    len2[0] = duration2 - range2;
    len2[1] = duration2 + range2;
}

void AudioProcess::setFreq(quint64 freq1, quint64 freq2)
{
    this->freq1 = freq1;
    this->freq2 = freq2;
}


int AudioProcess::process_wav(const QString& target_dir, const QString& filename)
{
    QString target(target_dir + "/" +filename + ".wav");
    QString target_filtered(target_dir + "/" + filename + "_filtered.wav");

    float tick = 0.0;
    uint_t n_frames = 0, read = 0;
    fvec_t* samples = new_fvec(hop_size);
    fvec_t* samples_filtered = new_fvec(hop_size);
    fvec_t* pitch_out = new_fvec(2);

    // 滤除高频+低频
    aubio_filter_t* f_highpass_400 =  get_aubio_filter(filter_type::F_HIGHPASS_400Hz);
    aubio_filter_t* f_lowpass_8000 =  get_aubio_filter(filter_type::F_LOWPASS_8000Hz);

    vector<uint_t> len1_pitch, len2_pitch;
    vector<smpl_t> len1_level, len2_level;

    // filter
    aubio_source_t* source = new_aubio_source(target.toUtf8(), samplerate, hop_size);
    if (!source) {
        printf("aubio source create error.");
        del_aubio_source(source);
        return -1;
    }
    aubio_sink_t* sink = new_aubio_sink(target_filtered.toUtf8(), samplerate);
    n_frames = 0;
    do {
        aubio_source_do(source, samples, &read);
        aubio_filter_do_outplace(f_highpass_400, samples, samples_filtered);
        aubio_filter_do(f_lowpass_8000, samples_filtered);
        aubio_sink_do(sink, samples_filtered, read);
        n_frames += read;
    } while (read == hop_size);

    del_aubio_sink(sink);

    // reopen
    aubio_source_t* source_filtered = new_aubio_source(target_filtered.toUtf8(), samplerate, hop_size);
    if (!source_filtered) {
        printf("aubio source create error.");
        del_aubio_source(source_filtered);
        return -1;
    }

    vector<double> onset_last_ms;
    double skip_ms= this->getFreqStartPos(source_filtered, freq2);
    qDebug() << filename << " skip :" << skip_ms << "ms";
    if(skip_ms < 0.0){
        return -1; //未侦测到指定频率
    }

    // 打印侦测时段
    qDebug() << "Length 1: " << len1[0] << "-"<<len1[1];
    qDebug() << "Length 2: " << len2[0] << "-"<<len2[1];

    aubio_source_seek(source_filtered, 0);
    n_frames = 0;
    aubio_pitch_t* pitch = new_aubio_pitch("default", win_s, hop_size, samplerate);
    do {
        aubio_source_do(source_filtered, samples, &read);
        // 频率提取
        aubio_pitch_do(pitch, samples, pitch_out);
        tick = (n_frames / (float)samplerate * 1000);

        if (tick > len1[0]+skip_ms && tick < len1[1]+skip_ms) {
            len1_pitch.push_back(pitch_out->data[0]);
            smpl_t level = aubio_db_spl(samples);
            len1_level.push_back(level);
        }

        if (tick > len2[0]+skip_ms && tick < len2[1]+skip_ms) {
            len2_pitch.push_back(pitch_out->data[0]);
            smpl_t level = aubio_db_spl(samples);
            len2_level.push_back(level);
        }

        n_frames += read;
    } while (read == hop_size);

    del_aubio_source(source);
    del_aubio_source(source_filtered);

    del_aubio_pitch(pitch);

    del_aubio_filter(f_lowpass_8000);
    del_aubio_filter(f_highpass_400);

    del_fvec(samples);
    del_fvec(samples_filtered);
    del_fvec(pitch_out);

    aubio_cleanup();

    //=====================================
    if (len1_level.size() == 0) return -1;
    if (len1_pitch.size() == 0) return -1;
    if (len2_level.size() == 0) return -1;
    if (len2_pitch.size() == 0) return -1;

    // 1 & 2 调换下位置
    double len2_level_avg = std::accumulate(len1_level.begin(), len1_level.end(), 0.0) / len1_level.size();
    double len2_pitch_avg = std::accumulate(len1_pitch.begin(), len1_pitch.end(), 0.0) / len1_pitch.size();
    double len1_level_avg = std::accumulate(len2_level.begin(), len2_level.end(), 0.0) / len2_level.size();
    double len1_pitch_avg = std::accumulate(len2_pitch.begin(), len2_pitch.end(), 0.0) / len2_pitch.size();

    std::cout << "## tick 1 " << len1[0] << "-" << len1[1] << "\n";
    std::cout << "* level :" << len1_level_avg << std::endl;
    std::cout << "* pitch :" << len1_pitch_avg << std::endl;

    std::cout << "## tick 2 " << len2[0] << "-" << len2[1] << "\n";
    std::cout << "* level :" << len2_level_avg << std::endl;
    std::cout << "* pitch :" << len2_pitch_avg << std::endl;

    out_csv += std::to_string(len1_level_avg) + ',';
    out_csv += std::to_string(len1_pitch_avg) + ',';
    out_csv += std::to_string(len2_level_avg) + ',';
    out_csv += std::to_string(len2_pitch_avg);

    return 0;
}

double AudioProcess::getFreqStartPos(aubio_source_t* source, quint64 target_freq)
{
    double start_ms= -1.0;

    uint_t samplerate = 44100;
    uint_t win_s = 1024;
    uint_t hop_size = win_s / 4;
    uint_t n_frames = 0, read = 0;

    aubio_pitch_t* pitch = new_aubio_pitch("default", win_s, hop_size, samplerate);

    std::vector<uint_t> pitchs;

    fvec_t* samples = new_fvec(hop_size);
    fvec_t* pitch_out = new_fvec(2);

    double pos = 0.0;
    int sample_count = 0;
    do {
        if(sample_count == 0){
            pos = n_frames / (double)samplerate * 1000;
        }

        aubio_source_do(source, samples, &read);
        aubio_pitch_do(pitch, samples, pitch_out);
        pitchs.push_back(pitch_out->data[0]);

        sample_count++;

        if(sample_count == 5){
            // 指定频率范围占比 7/10 即可认定为侦听到指定频率
            int count = 0;
            for(auto i: pitchs){
                if(i < (target_freq+50) && i > (target_freq-50))
                {
                    count++;
                }
            }
            if((double)count / (double)pitchs.size() > 7.0/10.0){
                // 起始点确定
                start_ms = pos;
                break;
            }
            pitchs.clear();
            sample_count = 0;
        }
        n_frames += read;
    } while (read == hop_size);

    del_fvec(samples);
    del_fvec(pitch_out);
    del_aubio_pitch(pitch);
    aubio_cleanup();
    aubio_source_seek(source, 0);

    return start_ms;
}

void AudioProcess::mainProcess()
{
    std::ofstream outfile;
    outfile.open(target_dir.toUtf8() + "\\test.csv");


    printf("\n###### L ######\n");
    out_csv.clear();
    out_csv += "L,";
    process_wav(target_dir, "L");

    outfile << out_csv << std::endl;
    
    printf("\n###### R ######\n");
    out_csv.clear();
    out_csv += "R,";
    process_wav(target_dir, "R");

    outfile << out_csv << std::endl;

    outfile.close();
}
