#ifndef MAINWINDOW_H
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
    QString m_cmd_start; // 接收到则开始自动测试流程
    QString m_cmd_pass;  // 测试成功发送
    QString m_cmd_fail;  // 测试失败发送

private: // Test
    QThread recordThread4L;
    QThread recordThread4R;

    QString wavDir;      // 录制文件保存目录
    QString wavDuration; // 录制时长
    int micIndexL;       // 左麦克风设备号
    int micIndexR;       // 右麦克风设备号
    quint64 m_AutoTestDelay = 0;

    QString default_WorkDir;
    QString default_AudioTestDir;

    QString current_WorkDir;
    QString current_AudioTestDir;

private slots:
    //-------------------------------------
    void autoProcess();  // 自动测试流程
    void testAudio();    //
    void onAudioTestFinished();
    //-------------------------------------


private: // device
    CodeReader* m_CodeReader;
    AutoLine* m_AutoLine;

private slots:
    void onCodeReaderReceiveBarcode(QString barcode);
    void onCodeReaderConnectStatusChanged();
    void onAutoLineReceiveCmd(QString rev_cmd);
    void onAutoLineConnectStatusChanged();

signals:
    void startAutoProcess(); //已接收到读卡器开始指令 自动测试流程开始
    void startRecording(quint64 duration); // 目录+时长以确定，开始录音
    void startTestAudio(); //录音文件已就绪， 开始测试
    void audioTestFinished();

private slots:
    void onRecordingOver();
    void onTestStarted();

private:
    Setup4Mic * setup4mic = nullptr;

private:
    Ui::MainWindow *ui;

private slots:
    void on_btnLockOption4Model_clicked();
    void on_btnSetting4Mic_clicked();
    void on_btnLoadWavDir_clicked();
    void on_btnStartRecord_clicked();
    void on_btnStartTest_clicked();
    void on_btnSetting4Model_clicked();
};
#endif // MAINWINDOW_H
