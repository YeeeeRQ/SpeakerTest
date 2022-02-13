#include "setup4mic.h"
#include "ui_setup4mic.h"
#include <QDebug>


Setup4Mic::Setup4Mic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setup4Mic)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("麦克风设定"));

    // 默认设定
    defaultAudioFormat.setSampleRate(8000);
    defaultAudioFormat.setChannelCount(1);
    defaultAudioFormat.setSampleSize(8);
    defaultAudioFormat.setCodec("audio/pcm");
    defaultAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
    defaultAudioFormat.setSampleType(QAudioFormat::UnSignedInt);

    //载入配置

    //输入设备列表 init
    ui->comboDevicesL->clear();
    ui->comboDevicesR->clear();
    deviceList=QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for(int i=0;i<deviceList.count();i++)
    {
        QAudioDeviceInfo device=deviceList.at(i);
        ui->comboDevicesL->addItem(device.deviceName());
        ui->comboDevicesR->addItem(device.deviceName());
    }

    if (deviceList.size()>0)
    {
        ui->comboDevicesL->setCurrentIndex(0); //触发comboDevices的信号currentIndexChanged()
        ui->comboDevicesR->setCurrentIndex(0); //触发comboDevices的信号currentIndexChanged()
        curDevice0 =deviceList.at(0);
        curDevice1 =deviceList.at(0);
    }
    if (deviceList.size()>=2)
    {
        ui->comboDevicesL->setCurrentIndex(0); //触发comboDevices的信号currentIndexChanged()
        ui->comboDevicesR->setCurrentIndex(1); //触发comboDevices的信号currentIndexChanged()
        curDevice0 =deviceList.at(0);
        curDevice1 =deviceList.at(1);
    }

    if (deviceList.size()<=0)
    {
        ui->btnStartL->setEnabled(false);
        ui->btnStopL->setEnabled(false);
        ui->btnStartR->setEnabled(false);
        ui->btnStopR->setEnabled(false);
    }


    //创建显示图表 L
    QChart *chart = new QChart;
    ui->chartViewL->setChart(chart);

    lineSeries0= new QLineSeries(); //序列
    chart->addSeries(lineSeries0);

    QValueAxis *axisX = new QValueAxis;  //坐标轴
    axisX->setRange(0, displayPointsCount); //chart显示4000个采样点数据
    axisX->setLabelFormat("%g");

    QValueAxis *axisY = new QValueAxis;  //坐标轴
    axisY->setRange(0, 256); // UnSingedInt采样，数据范围0-255

    chart->addAxis(axisX,Qt::AlignBottom);
    chart->addAxis(axisY,Qt::AlignLeft);
    lineSeries0->attachAxis(axisX);
    lineSeries0->attachAxis(axisY);

    chart->legend()->hide();

    //创建显示图表 R
    QChart *chart1 = new QChart;
    ui->chartViewR->setChart(chart1);

    lineSeries1= new QLineSeries(); //序列
    chart1->addSeries(lineSeries1);

    QValueAxis *axisX1 = new QValueAxis;  //坐标轴
    axisX1->setRange(0, displayPointsCount); //chart显示4000个采样点数据
    axisX1->setLabelFormat("%g");

    QValueAxis *axisY1 = new QValueAxis;  //坐标轴
    axisY1->setRange(0, 256); // UnSingedInt采样，数据范围0-255

    chart1->addAxis(axisX1,Qt::AlignBottom);
    chart1->addAxis(axisY1,Qt::AlignLeft);
    lineSeries1->attachAxis(axisX1);
    lineSeries1->attachAxis(axisY1);

    chart1->legend()->hide();
}

Setup4Mic::~Setup4Mic()
{
    delete ui;
}

void Setup4Mic::onLoadDeviceConf(int l_idx, int r_idx)
{
    if(l_idx < deviceList.size()){
        ui->comboDevicesL->setCurrentIndex(l_idx); //触发comboDevices的信号currentIndexChanged()
        curDevice0 =deviceList.at(l_idx);
    }
    if(r_idx < deviceList.size()){
        ui->comboDevicesR->setCurrentIndex(r_idx); //触发comboDevices的信号currentIndexChanged()
        curDevice1 =deviceList.at(r_idx);
    }

    this->on_btnSave_clicked();
}

void Setup4Mic::on_btnExit_clicked()
{
    this->close();
}


void Setup4Mic::on_btnSave_clicked()
{
    QString LMic = ui->comboDevicesL->currentText();
    QString RMic = ui->comboDevicesR->currentText();

    int lidx = ui->comboDevicesL->currentIndex();
    int ridx = ui->comboDevicesR->currentIndex();

    if(LMic.isEmpty()||RMic.isEmpty()){
        QMessageBox::warning(this, "设定失败", "系统没有麦克风设备");
        return;
    }
    if(LMic == RMic){
        QMessageBox::warning(this, "设定失败", "左右麦克风输入不能相同");
        return;
    }

    qDebug() << lidx << " : " << LMic;
    qDebug() << ridx << " : " << RMic;

    emit setupMic(lidx, LMic, ridx, RMic);
}



void Setup4Mic::on_btnStartL_clicked()
{//开始音频输入
    if (!curDevice0.isFormatSupported(defaultAudioFormat))
    {
        QMessageBox::critical(this,"音频输入设置测试","测试失败，输入设备不支持此设置");
        return;
    }
    audioInput0 = new QAudioInput(curDevice0,defaultAudioFormat, this);
    audioInput0->setBufferSize(displayPointsCount);

    // 接收音频输入数据的流设备
    displayDevice0 = new QmyDisplayDevice(lineSeries0, displayPointsCount,this);
    displayDevice0->open(QIODevice::WriteOnly); //必须以写方式打开

    audioInput0->start(displayDevice0); //以流设备作为参数，开始录入音频输入数据

    ui->btnStartL->setEnabled(false);
    ui->btnStopL->setEnabled(true);
}


void Setup4Mic::on_btnStartR_clicked()
{//开始音频输入
    if (!curDevice1.isFormatSupported(defaultAudioFormat))
    {
        QMessageBox::critical(this,"音频输入设置测试","测试失败，输入设备不支持此设置");
        return;
    }
    audioInput1 = new QAudioInput(curDevice1,defaultAudioFormat, this);
    audioInput1->setBufferSize(displayPointsCount);

    // 接收音频输入数据的流设备
    displayDevice1 = new QmyDisplayDevice(lineSeries1, displayPointsCount,this);
    displayDevice1->open(QIODevice::WriteOnly); //必须以写方式打开

    audioInput1->start(displayDevice1); //以流设备作为参数，开始录入音频输入数据

    ui->btnStartR->setEnabled(false);
    ui->btnStopR->setEnabled(true);
}


void Setup4Mic::on_btnStopL_clicked()
{
    audioInput0->stop();
    audioInput0->deleteLater();
    displayDevice0->close();
    displayDevice0->deleteLater();

    ui->btnStartL->setEnabled(true);
    ui->btnStopL->setEnabled(false);
}

void Setup4Mic::on_btnStopR_clicked()
{
    audioInput1->stop();
    audioInput1->deleteLater();
    displayDevice1->close();
    displayDevice1->deleteLater();

    ui->btnStartR->setEnabled(true);
    ui->btnStopR->setEnabled(false);
}

void Setup4Mic::on_comboDevicesL_currentIndexChanged(int index)
{

    curDevice0 = deviceList.at(index);
}


void Setup4Mic::on_comboDevicesR_currentIndexChanged(int index)
{
    curDevice1 = deviceList.at(index);
}
