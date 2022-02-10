#ifndef SETUP4MIC_H
#define SETUP4MIC_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QWidget>
#include <QtCharts>
#include <QAudioDeviceInfo>
#include <QAudioInput>

#include "qmydisplaydevice.h"


QString SampleTypeString(QAudioFormat::SampleType sampleType);
QString ByteOrderString(QAudioFormat::Endian endian);

namespace Ui {
class Setup4Mic;
}


class Setup4Mic : public QWidget
{
    Q_OBJECT

private:
    const qint64 displayPointsCount = 4000;
    QList<QAudioDeviceInfo> deviceList;

    QAudioFormat defaultAudioFormat; //缺省格式

    // L Mic
    QAudioDeviceInfo curDevice0;
    QAudioInput *audioInput0;
    QmyDisplayDevice *displayDevice0;
    QLineSeries *lineSeries0;

    // R Mic
    QAudioDeviceInfo curDevice1;
    QAudioInput *audioInput1;
    QmyDisplayDevice *displayDevice1;
    QLineSeries *lineSeries1;



signals:
//    void setupMic(int l_idx, int r_idx);
    void setupMic(int l_idx, const QString& lmic, int r_idx, const QString& rmic);
public slots:
    void onLoadDeviceConf(int l_idx, int r_idx);

public:
    explicit Setup4Mic(QWidget *parent = nullptr);
    ~Setup4Mic();

private slots:

    void on_btnExit_clicked();

    void on_btnSave_clicked();

    void on_btnStartR_clicked();

    void on_btnStopL_clicked();

    void on_btnStartL_clicked();

    void on_btnStopR_clicked();

    void on_comboDevicesR_currentIndexChanged(int index);

    void on_comboDevicesL_currentIndexChanged(int index);

private:
    Ui::Setup4Mic *ui;
};


#endif // SETUP4MIC_H
