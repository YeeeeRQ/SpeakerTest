#include "AudioProcess.h"
#include <QDebug>

// Todo: 通过onsets主动判断起始点。

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

    // =========================================
    // filter
    // =========================================
    aubio_source_t* source = new_aubio_source(target.toUtf8(), samplerate, hop_size);
    if (!source) {
        printf("aubio source create error.");
        del_aubio_source(source);
        return -1;
    }
    aubio_sink_t* sink = new_aubio_sink(target_filtered.toUtf8(), samplerate);
//    aubio_sink_preset_channels(sink, 1);
    n_frames = 0;
    do {
        // put some fresh data in input vector
        aubio_source_do(source, samples, &read);
        //fvec_print(samples);
        //std::cout << std::endl;

        aubio_filter_do_outplace(f_highpass_400, samples, samples_filtered);
        aubio_filter_do(f_lowpass_8000, samples_filtered);
        aubio_sink_do(sink, samples_filtered, read);

        n_frames += read;
    } while (read == hop_size);

//    aubio_sink_close(sink);
    del_aubio_sink(sink);


    // =========================================
    // 打开filter处理后的音频
    // =========================================
    aubio_source_t* source_filtered = new_aubio_source(target_filtered.toUtf8(), samplerate, hop_size);
    if (!source_filtered) {
        printf("aubio source create error.");
        del_aubio_source(source_filtered);
        return -1;
    }

    vector<double> onset_last_ms;
//    vector<uint_t> onset_last;
//    vector<uint_t> nframes;

    aubio_onset_t* onset = new_aubio_onset("default", win_s, hop_size, samplerate);
    fvec_t* onset_out = new_fvec(2); // output position
    do {
        aubio_source_do(source_filtered, samples, &read);

        aubio_onset_do(onset, samples, onset_out);

        if (onset_out->data[0] != 0) {
//            onset_last.push_back(aubio_onset_get_last(onset));
            onset_last_ms.push_back(aubio_onset_get_last_ms(onset));
//            nframes.push_back(aubio_onset_get_last(onset));
            QString v;
            qDebug() << v.asprintf("onset at %.3fs, frame %d",
                                   aubio_onset_get_last_s(onset),
                                   aubio_onset_get_last(onset));
        }
        n_frames += read;
    } while (read == hop_size);


    double skip_ms= 0.0;
    // 重置起始点
    if(onset_last_ms.at(1) - onset_last_ms.at(0) > 900){
        // 整个第二时段都被录制下来了
        skip_ms = onset_last_ms.at(0);
//        aubio_source_seek(source_filtered, onset_last.at(0));
//        n_frames = onset_last.at(0);
//        n_frames = nframes.at(0);
        qDebug() << "source seek " << onset_last_ms.at(0);
    }else{
        skip_ms = onset_last_ms.at(1);
//        aubio_source_seek(source_filtered, onset_last.at(1));
//        n_frames = nframes.at(1);
//        n_frames = onset_last.at(1);
        qDebug() << "source seek " << onset_last_ms.at(1);
    }

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

        // 时段1
        if (tick > len1[0]+skip_ms && tick < len1[1]+skip_ms) {
//            qDebug() << "Length 1: " << tick;

            len1_pitch.push_back(pitch_out->data[0]);
            smpl_t level = aubio_db_spl(samples);
            len1_level.push_back(level);
        }

        // 时段2
        if (tick > len2[0]+skip_ms && tick < len2[1]+skip_ms) {
//            qDebug() << "Length 1: " << tick;

            len2_pitch.push_back(pitch_out->data[0]);
            smpl_t level = aubio_db_spl(samples);
            len2_level.push_back(level);
        }

        n_frames += read;
    } while (read == hop_size);

    del_aubio_source(source);
    del_aubio_source(source_filtered);

    del_aubio_onset(onset);
    del_aubio_pitch(pitch);

    del_aubio_filter(f_lowpass_8000);
    del_aubio_filter(f_highpass_400);

    del_fvec(samples);
    del_fvec(samples_filtered);
    del_fvec(onset_out);
    del_fvec(pitch_out);

    aubio_cleanup();

    //=====================================
    if (len1_level.size() == 0) return -1;
    if (len1_pitch.size() == 0) return -1;
    if (len2_level.size() == 0) return -1;
    if (len2_pitch.size() == 0) return -1;

    double len1_level_avg = std::accumulate(len1_level.begin(), len1_level.end(), 0.0) / len1_level.size();
    double len1_pitch_avg = std::accumulate(len1_pitch.begin(), len1_pitch.end(), 0.0) / len1_pitch.size();
    double len2_level_avg = std::accumulate(len2_level.begin(), len2_level.end(), 0.0) / len2_level.size();
    double len2_pitch_avg = std::accumulate(len2_pitch.begin(), len2_pitch.end(), 0.0) / len2_pitch.size();

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
