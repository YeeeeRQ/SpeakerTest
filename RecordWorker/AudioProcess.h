#pragma once
#include <QObject>
#include <QFile>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <numeric>
#include <stdlib.h>
#include <stdio.h>

#include <aubio/aubio.h>

#include "get_aubio_filter.h"

using namespace std;
using std::string;
using std::to_string;
using std::vector;


class AudioProcess :public QObject
{
    Q_OBJECT

public:
    explicit AudioProcess(QObject *parent = nullptr);
//    AudioProcess(uint_t t1, uint_t t2);
	~AudioProcess();
private:
	AudioProcess(const AudioProcess&) = delete;
	AudioProcess& operator=(const AudioProcess&) = delete;
private:
	uint_t samplerate = 44100;
	uint_t win_s = 1024;
	uint_t hop_size = win_s / 4;

    // 信号处理, 根据时长定位采样点
    uint_t len1[2];  //采样时段1(单位ms)
    uint_t len2[2];  //采样时段2(单位ms)

    quint64 freq1;
    quint64 freq2;

    QString target_dir;
    string out_csv;

public:
    bool setAudioFilePath(const QString& target_dir);
    void setDuration(quint64 duration1, quint64 range1, quint64 duration2, quint64 range2);
    void setFreq(quint64 freq1, quint64 freq2);
	void mainProcess();
private:
    int process_wav(const QString& target_dir, const QString& filename);
    double getFreqStartPos(aubio_source_t* source, quint64 target_freq);

//    double getAudioFrequency();
};


