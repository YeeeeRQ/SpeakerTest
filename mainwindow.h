﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QThread>
#include <QFile>
#include <QTime>
#include <QDate>

#include "CodeReader.h"
#include "AutoLine.h"

#include "config.h"
#include "simplelog.h"

#include "setup4mic.h"
#include "setup4model.h"
#include "Setup4AutoTest.h"
#include "showinfo4result.h"

#include "RecordWorker.h"
#include "MyCommonFun.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

////////////////////////////////////////////////////////////////////////////////
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void init();

private: // log
    SimpleLog& log;
    QPlainTextEdit textedit4log;

private: // config
    Config& conf = Config::getInstance();
    void initConfig();
    void saveConfig();  // 保存配置文件
    void loadConfig();  // 载入配置文件
    void resetConfig(); // 恢复默认配置
//    QButtonGroup* mRadioGroup_runningmode;
    QString m_outputDir; //音频录制存放目录

    // AutoLine 相关指令
    quint64 m_AutoTestDelay = 0;
    QString m_cmd_start; // 接收到则开始自动测试流程
    QString m_cmd_pass;  // 测试成功发送
    QString m_cmd_fail;  // 测试失败发送

private: // Test
    QThread m_recordThread4L; //左侧录制线程
    QThread m_recordThread4R; //右侧录制线程
    RecordWorker * recWorkerL;
    RecordWorker * recWorkerR;

    int m_micIndexL;       // 左麦克风序号
    int m_micIndexR;       // 右麦克风序号
    QString m_micL;        // 左麦克风
    QString m_micR;        // 右麦克风

    QString wavDir;      // 录制文件保存目录
    QString wavDuration; // 录制时长

    QString default_WorkDir;
    QString default_AudioTestDir;

    QString current_WorkDir;
    QString current_AudioTestDir;

private: // device
    CodeReader* m_CodeReader;
    AutoLine* m_AutoLine;

private slots:
    void onCodeReaderReceiveBarcode(QString barcode);
    void onCodeReaderConnectStatusChanged();
    void onAutoLineReceiveCmd(QString rev_cmd);
    void onAutoLineConnectStatusChanged();

    void onAutoTestConfigChanged();

signals:
    void startAutoProcess(); //已接收到读卡器开始指令 自动测试流程开始
    void startRecording(quint64 duration); // 目录+时长以确定，开始录音
    void startTestAudio(); //录音文件已就绪， 开始测试
    void audioTestFinished();

    void setRecordOutputL(const QString& out);
    void setRecordInputL(const QString& dev);
    void setRecordOutputR(const QString& out);
    void setRecordInputR(const QString& dev);

    void loadMicConf(int l_idx, int r_idx);

private slots:
    void onSetupMic(int l_idx, const QString& lmic, int r_idx, const QString& rmic);

    void onRecordingOver();
    void onTestStarted();
    //-------------------------------------
    void autoProcess();  // 自动测试流程
    void testAudio();    // for startTestAudio
    void onAudioTestFinished(); //音频检测后结果判断输出
    //-------------------------------------

private: // UI
    Setup4Mic * setup4mic = nullptr;
    Setup4AutoTest* setup4autotest = nullptr;


private:
    Ui::MainWindow *ui;

private slots:
    void on_btnLockOption4Model_clicked();
    void on_btnSetting4Mic_clicked();
    void on_btnLoadWavDir_clicked();
    void on_btnStartRecord_clicked();
    void on_btnStartTest_clicked();
    void on_btnSetting4Model_clicked();
    void on_btnSetting4AutoTest_clicked();
    void on_btnLoadTempWavDir_clicked();
    void on_btnSwitchRunningMode_clicked();
};
#endif // MAINWINDOW_H