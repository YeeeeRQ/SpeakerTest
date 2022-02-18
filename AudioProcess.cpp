﻿#include "AudioProcess.h"

AudioProcess::AudioProcess(uint_t t1, uint_t t2)
{
    len1[0] = t1 - 100;
    len1[1] = t1 + 100;
    len2[0] = t2 - 100;
    len2[1] = t2 + 100;
}

AudioProcess::~AudioProcess()
{
}

bool AudioProcess::setSourcePath(const QString& p) {
    src = p;
    wav_l = (src + "\\L.wav");
    wav_r = (src + "\\R.wav");

    if (has_audio_input()) {
        return true;
    }
    return false;
}


int AudioProcess::process_wav(const QString& wav)
{
    aubio_source_t* source = new_aubio_source(wav.toLocal8Bit(), samplerate, hop_size);
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
    fvec_t* pitch_out = new_fvec(2);

    aubio_pitch_t* o = new_aubio_pitch("default", win_s, hop_size, samplerate);

    //
    vector<uint_t> len1_pitch, len2_pitch;
    vector<smpl_t> len1_level, len2_level;

    // 两时段的音频处理
    do {
        // put some fresh data in input vector
        aubio_source_do(source, samples, &read);
        //fvec_print(samples);
        //std::cout << std::endl;

        aubio_pitch_do(o, samples, pitch_out);
        tick = n_frames / (float)samplerate * 1000;

        // 时段1
        if (tick > len1[0] && tick < len1[1]) {
            //std::cout << tick << std::endl;

            //fvec_print(pitch_out);
            //break;
            len1_pitch.push_back(pitch_out->data[0]);
            smpl_t level = aubio_level_lin(samples);
            len1_level.push_back(level);
            std::cout << level << std::endl;
        }

        // 时段2
        if (tick > len2[0] && tick < len2[1]) {
            len2_pitch.push_back(pitch_out->data[0]);
            smpl_t level = aubio_level_lin(samples);
            len2_level.push_back(level);
        }
        //std::cout << tick << std::endl;
        //fvec_print(pitch_out);

        n_frames += read;
    } while (read == hop_size);

    del_aubio_pitch(o);
    del_fvec(samples);
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

    len1_level_avg *= 1e8;
    len2_level_avg *= 1e8;

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
    outfile.open((src + "\\test.csv").toLocal8Bit());

    printf("# L\n");
    out_csv.clear();
    out_csv += "L,";
    process_wav(wav_l);

    outfile << out_csv << std::endl;

    printf("\n# R\n");
    out_csv.clear();
    out_csv += "R,";
    process_wav(wav_r);

    outfile << out_csv << std::endl;

    outfile.close();
}

bool AudioProcess::has_audio_input()
{
    if (!QFile::exists(src)) {
//        std::cout << src << std::endl;
//        std::cout << "输入路径不存在!";
        return false;
    }

    if (!QFile::exists(wav_l) || !QFile::exists(wav_r)) {
//        std::cout << wav_l << std::endl;
//        std::cout << wav_r << std::endl;
//        std::cout << "音频测试文件不存在!";
        return false;
    }
    return true;
}

