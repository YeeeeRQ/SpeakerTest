#include "AudioProcess.h"
#include <QDebug>

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
    len[0] = duration1 - range1;
    len[1] = duration1 + range1;
    len2[0] = duration2 - range2;
    len2[1] = duration2 + range2;
}


int AudioProcess::process_wav(const QString& target_dir, const QString& filename)
{

    QString target(target_dir + "/" +filename + ".wav");
    QString target_filtered(target_dir + "/" + filename + "_filtered.wav");

//    if (!exists(target)) {
//        std::cout << target << std::endl;
//        std::cout << "音频测试文件不存在!";
//        return -1;
//    }

    qDebug() << target.toUtf8();
    qDebug() << target.toLocal8Bit();

    aubio_source_t* source = new_aubio_source(target.toUtf8(), samplerate, hop_size);
    if (!source) {
        printf("aubio source create error.");
        del_aubio_source(source);
        return -1;
    }

    // 长度判定 时长不小于 2s

    // 跳过环境音段

    float tick = 0.0;
    uint_t n_frames = 0, read = 0;
    fvec_t* samples = new_fvec(hop_size);
    fvec_t* samples_filtered = new_fvec(hop_size);
    fvec_t* pitch_out = new_fvec(2);

    aubio_pitch_t* o = new_aubio_pitch("default", win_s, hop_size, samplerate);
    aubio_sink_t* sink = new_aubio_sink(target_filtered.toUtf8(), samplerate);
    aubio_sink_preset_channels(sink, 1);

    // 滤除高频+低频
    aubio_filter_t* f_highpass_400 =  get_aubio_filter(filter_type::F_HIGHPASS_400Hz);
    aubio_filter_t* f_lowpass_8000 =  get_aubio_filter(filter_type::F_LOWPASS_8000Hz);

    //
    vector<uint_t> len1_pitch, len2_pitch;
    vector<smpl_t> len1_level, len2_level;

    // 两时段的音频处理
    do {
        // put some fresh data in input vector
        aubio_source_do(source, samples, &read);
        //fvec_print(samples);
        //std::cout << std::endl;


        // Todo:
        // 滤波
        aubio_filter_do_outplace(f_highpass_400, samples, samples_filtered);
        aubio_filter_do(f_lowpass_8000, samples_filtered);
        aubio_sink_do(sink, samples_filtered, read);

//        aubio_sink_do(sink, samples, read);



        // 频率提取
        aubio_pitch_do(o, samples_filtered, pitch_out);
        tick = n_frames / (float)samplerate * 1000;

        // 时段1
        if (tick > len[0] && tick < len[1]) {
            //std::cout << tick << std::endl;

            //fvec_print(pitch_out); 
            //fvec_print(samples_filtered);

            //break;
            len1_pitch.push_back(pitch_out->data[0]);
            smpl_t level = aubio_db_spl(samples_filtered);
            len1_level.push_back(level);
            //std::cout << level << std::endl; 
        }

        // 时段2
        if (tick > len2[0] && tick < len2[1]) {
            len2_pitch.push_back(pitch_out->data[0]);
            smpl_t level = aubio_db_spl(samples_filtered);
            len2_level.push_back(level);
        }
        //std::cout << tick << std::endl;
        //fvec_print(pitch_out);

        n_frames += read;
    } while (read == hop_size);

//    qDebug() << "close sink: "<<aubio_sink_close(sink);

    del_aubio_filter(f_lowpass_8000);
    del_aubio_filter(f_highpass_400);

    del_aubio_sink(sink);
    del_aubio_pitch(o);
    del_fvec(samples);
    del_fvec(samples_filtered);
    del_fvec(pitch_out);
    del_aubio_source(source);
    aubio_cleanup();

    // 
    if (len1_level.size() == 0) return -1;
    if (len1_pitch.size() == 0) return -1;
    if (len2_level.size() == 0) return -1;
    if (len2_pitch.size() == 0) return -1;

    // 振幅(amplitude)  -->  响度(loudness)
    // 频率(frequency)  -->  音调(pitch)


    double len1_level_avg = std::accumulate(len1_level.begin(), len1_level.end(), 0.0) / len1_level.size();
    double len1_pitch_avg = std::accumulate(len1_pitch.begin(), len1_pitch.end(), 0.0) / len1_pitch.size();

    double len2_level_avg = std::accumulate(len2_level.begin(), len2_level.end(), 0.0) / len2_level.size();
    double len2_pitch_avg = std::accumulate(len2_pitch.begin(), len2_pitch.end(), 0.0) / len2_pitch.size();


    std::cout << "## tick 1 " << len[0] << "-" << len[1] << "\n";
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
