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
#include <ActiveQt/QAxObject>

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


QVector<QVector<QString>> loadExcel(QString strSheetName);


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent *event); //

// -- 界面载入后，进行初始化
signals:
    void uiLoaded();
public:
    void emitUILoaded();
private slots:
    void onUILoaded();

// -- 主界面日志输出
private:
    SimpleLog& log;
    QPlainTextEdit textedit4log;

// -- 配置相关
private:
    Config& conf = Config::getInstance();
    void initConfig();
    void saveConfig();  // 保存配置文件
    void loadConfig();  // 载入配置文件
    void resetConfig(); // 恢复默认配置


// -- 音频测试流程
private:
    // 工作目录
    QString default_WorkDir;
    QString default_AudioTestDir;

    QString current_WorkDir;
    QString current_AudioTestDir;

    // Mic Device
    RecordWorker * m_pRecWorkerL;
    RecordWorker * m_pRecWorkerR;

    QThread m_recordThread4L; //左侧录制线程
    QThread m_recordThread4R; //右侧录制线程

    QString m_micL;        // 左麦克风
    QString m_micR;        // 右麦克风
    int m_micIndexL;       // 左麦克风序号
    int m_micIndexR;       // 右麦克风序号

    QString m_outputDir;   //音频录制存放目录
    QString m_wavDir;      // 录制文件保存目录

    // 左右麦克风录制完毕标记 值为1时表示录制结束
    bool m_recordCount[2]; // 0->L | 1->R

// 手动测试
    quint64 m_wavDuration; // 手动测试下单一录制时长

// 自动测试
    // 自动测试 首次发生麦克风
    QString m_firstSpeaker; //首次发生麦克风

    // 自动测试录制时长
    quint64 m_recordDuration1;  //第1次录制时长
    quint64 m_recordDuration2;  //第2次录制时长

    quint64 m_accept_pitch1[2]; // 接受频率范围
    quint64 m_accept_pitch2[2]; // 接受频率范围
    quint64 m_testTime1[2]; // 测试时段
    quint64 m_testTime2[2]; // 测试时段

// 启动时载入自定义流程 关联文件:process.xlsx
private:
    QVector<QVector<QString>> m_processTable;
    int m_processTable_rows;
    int m_processTable_cols;

    bool m_customTestProcessIsOK = false;

    void loadAutoProcess();
    void processCmdParser(QString cmd);

    bool checkCustomTestProcess(); //检测自定义流程
    void startCustomTestAudio();

    void custom_do_sleep(quint64 duration);
    void custom_do_record(quint64 duration);
    void custom_do_sendcmd2pg(const QString& cmd);
    void custom_do_sendcmd2mnt(const QString& cmd);
    void custom_do_set_order(const QString & first_speaker);

    void custom_do_get_audio_info(int order, quint64 tick, quint64 tick_range, quint64 freq,quint64 freq_range);
    void custom_do_autotest_end();

    void printResult(bool isOk, const QString& msg);


signals:
    void checkAllRecordOver();
    void allRecordOver();

    void parseCmd(const QString& cmd, const QList<QString>&cmd_args);
    void custom_cmd_done(bool has_err);

private slots:
    void custom_do_record_done();
    void customTestAudio(bool has_err);
    void customCmdParser(const QString& cmd, const QList<QString>&cmd_args);
    void onCheckAllRecordOver();



// -- 测试模式 (手动|自动)
private:
    bool m_autoMode = false;  // 自己保证安全
    bool setAutoMode(bool mode);
    bool isAutoMode();
signals:
    void sig_autoModeStateChanged(bool mode);
private slots:
    void slot_onAutoModeStateChanged(bool mode);


// -- 外部设备
private:

    // AutoLine
    AutoLine* m_AutoLine;
    quint64 m_AutoTestDelay = 0;
    QString m_cmd_start; // 接收到则开始自动测试流程
    QString m_cmd_pass;  // 测试成功发送
    QString m_cmd_fail;  // 测试失败发送

    // CodeReader
    CodeReader* m_CodeReader;

    // SigGenerator
    AutoLine* m_SigGenerator;


private: // UI
    Ui::MainWindow *ui;
    Setup4Mic * setup4mic = nullptr;
    Setup4AutoTest* setup4autotest = nullptr;

    void themeSetting();
    void pathSetting();
    void configSetting();
    void devicesSetting();
    void uiInit();

    void startTestAudio();

signals:
    void sig_startAutoTest(); //已接收到读卡器开始指令 自动测试流程开始
    void sig_startRecording(quint64 duration); // 目录+时长以确定，开始录音
    void sig_startTestAudio(); //录音文件已就绪， 开始测试
    void sig_audioTestFinished();

    void sig_setRecordOutputL(const QString& out);
    void sig_setRecordInputL(const QString& dev);
    void sig_setRecordOutputR(const QString& out);
    void sig_setRecordInputR(const QString& dev);

    void sig_loadMicConf(int l_idx, int r_idx);

private slots:
    void slot_onSetupMic(int l_idx, const QString& lmic, int r_idx, const QString& rmic);
    void slot_onLMicRecordingOver(); //左侧Mic录制结束
    void slot_onRMicRecordingOver(); //右侧Mic录制结束

    void slot_onCodeReaderReceiveBarcode(QString barcode);
    void slot_onCodeReaderConnectStatusChanged();
    void slot_onAutoLineReceiveCmd(QString rev_cmd);
    void slot_onAutoLineConnectStatusChanged();
    void slot_onSigGeneratorConnectStatusChanged();
    void slot_onAutoTestConfigChanged();

    void slot_startAutoTest();  // 自动测试流程
    void slot_testAudio();    // for startTestAudio
    void slot_onAudioTestFinished(); //音频检测后根据csv判断结果

// -- UI界面 槽函数
private slots:
    void on_btnLockOption4Model_clicked();
    void on_btnSetting4Mic_clicked();
    void on_btnLoadWavDir_clicked();
    void on_btnStartRecord_clicked();
    void on_btnSetting4Model_clicked();
    void on_btnSetting4AutoTest_clicked();
    void on_btnLoadTempWavDir_clicked();
    void on_btnSwitchRunningMode_clicked();
    void on_btnTest_clicked();
};
#endif // MAINWINDOW_H

