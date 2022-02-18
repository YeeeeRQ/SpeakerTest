#ifndef AUDIOPROCESS_H
#define AUDIOPROCESS_H

#include <QObject>
#include <QFile>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <numeric>

#include <aubio\aubio.h>
#include <stdlib.h>
#include <stdio.h>

//using namespace std::filesystem;
using std::string;
using std::to_string;
using std::vector;
//using std::path;
//using std::filesystem::path;


class AudioProcess:public QObject
{
    Q_OBJECT
public:
    AudioProcess(uint_t t1, uint_t t2);
    ~AudioProcess();
private:
    AudioProcess(const AudioProcess&) = delete;
    AudioProcess& operator=(const AudioProcess&) = delete;
private:
    // 信号处理, 根据时长定位采样点
    uint_t len1[2];  //采样时段1(单位ms)
    uint_t len2[2];  //采样时段2(单位ms)

    uint_t samplerate = 44100;
    uint_t win_s = 1024;
    uint_t hop_size = win_s / 4;

    QString src;
    QString wav_l;
    QString wav_r;

    string out_csv;

public:
    bool setSourcePath(const QString& p);
    void mainProcess();

private:
    bool has_audio_input();
    int process_wav(const QString& wav);
};



#endif // AUDIOPROCESS_H
