#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QThread>
#include <QFile>
#include <QTime>
#include <QDate>
#include <ActiveQt/QAxObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QAudioRecorder>
#include <QKeyEvent>
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

QString mkMutiDir(const QString path);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent *event); //
    void keyPressEvent(QKeyEvent *ev);

private:
    QFileInfo fi;

// -- 音频测试流程
private:
    // 工作目录
    QString default_WorkDir;
    QString default_AudioTestDir;

    QString m_workDir;   // 音频录制存放(主目录)
    QString m_audioTestDir;      // 录制文件保存目录 (主目录->机种->日期时间)

    // 底部状态栏
    QLabel * label_startUpTime; //记录启动时间

    // Mic Device
    QSet<QString> m_audioInputs;
    RecordWorker * m_pRecWorkerL;
    RecordWorker * m_pRecWorkerR;

    QThread m_recordThread4L; //左侧录制线程
    QThread m_recordThread4R; //右侧录制线程

    QString m_micL;        // 左麦克风
    QString m_micR;        // 右麦克风
    int m_micIndexL;       // 左麦克风序号
    int m_micIndexR;       // 右麦克风序号

    // 左右麦克风录制完毕标记 值为1时表示录制结束
    bool m_recordCount[2]; // 0->L | 1->R

// 手动测试
    quint64 m_wavDuration; // 手动测试下单一录制时长

// 自动测试
    // 自动测试 首次发声麦克风
    QString m_firstSpeaker; //首次发声的麦克风

    quint64 m_accept_pitch1[2]; // 接受频率范围
    quint64 m_accept_pitch2[2]; // 接受频率范围
    quint64 m_testTime1[2]; // 测试时段
    quint64 m_testTime2[2]; // 测试时段

    void clearFileList();

    void startTestAudio();
    void startTestAudioInAutoMode();

private:
    bool m_isRecording = false;
signals:
   void recordStatusChanged(bool is_recording);
private slots:
   void onRecordStatusChanged(bool is_recording);


// 启动时载入自定义流程 关联文件:process.xlsx
private:
    void printResult(bool isOk, const QString& msg);

    bool r_record_done = false;
    bool l_record_done = false;
    QString r_record_reason;
    QString l_record_reason;

signals:
    void checkAllRecordOver();
    void allRecordOver();

private slots:
    void onCheckAllRecordOver();
    void onInterceptTimeout(); //侦测超时

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
    QString m_cmd_start; // 接收到则开始自动测试流程
    QString m_cmd_pass;  // 测试成功发送
    QString m_cmd_fail;  // 测试失败发送

    CodeReader* m_CodeReader; // CodeReader
//    AutoLine* m_SigGenerator; // SigGenerator


signals:
    void sig_startAutoTest(); //已接收到读卡器开始指令 自动测试流程开始信号
//    void sig_startRecording(quint64 duration); // 目录+时长以确定，开始录音
    void sig_startRecording(); // 开始录制信号
    void sig_stopRecording(); // 停止录制信号！！！！！！！！！！！！！！！自动模式下不要使用

//    void sig_startRecording(quint64 duration, QFile& file); // 目录+时长以确定，开始录音
    void sig_startTestAudio(); // 手动模式 录音文件已就绪， 开始测试
    void sig_audioTestFinished();

    void sig_setRecordInputL(quint64 idx); //麦克风输入设定 L
    void sig_setRecordInputR(quint64 idx); //麦克风输入设定 R

    void sig_loadMicConf(int l_idx, int r_idx);
    void sig_loadModel(const QString& dbfile);

    void setup4model_loadDB(QString dbfile);

private slots:
    void slot_onSetupMic(int l_idx, const QString& lmic, int r_idx, const QString& rmic);
    void slot_onLMicRecordingOver(bool done, const QString& result); //左侧Mic录制结束
    void slot_onRMicRecordingOver(bool done, const QString& result); //右侧Mic录制结束

    void slot_onCodeReaderReceiveBarcode(QString barcode);
    void slot_onCodeReaderConnectStatusChanged();
    void slot_onAutoLineReceiveCmd(QString rev_cmd);
    void slot_onAutoLineConnectStatusChanged();
    void slot_onSigGeneratorConnectStatusChanged();
    void slot_onAutoTestConfigChanged();

    void slot_startAutoTest();  // 自动测试流程
//    void slot_startCustomAutoTest();  // 自动测试流程
    void slot_testAudio();    // for startTestAudio
    void slot_onAudioTestFinished(); //音频检测后根据csv判断结果

    void loadModel(QString dbfile);

    void slot_onGetRFrequency(qint64 freq);
    void slot_onGetLFrequency(qint64 freq);

public slots:
    void onRecordWorkerStatusChanged(RecordStatus status);

private:
    void fordebug();
// -- 主界面日志输出
private:
    SimpleLog& log;
    QPlainTextEdit textedit4log;

// -- 配置相关
private:
    Config& conf = Config::getInstance();
    void initConfig();  // 初始化配置文件
    void loadConfig();  // 载入配置文件
//    void saveConfig();  // 保存配置文件
//    void resetConfig(); // 恢复默认配置

private: // UI
    Ui::MainWindow *ui;
    Setup4Mic * setup4mic = nullptr;
    Setup4AutoTest* setup4autotest = nullptr;
    Setup4Model* setup4model= nullptr;

    void Setting4Theme();
    void Setting4Path();
    void Setting4Config();
    void Setting4Devices();
    void setting4Mic();
    void Setting4MainWindow(); // 界面载入后的设定

    void setWavDir4UI(const QString &dir);

// -- 界面载入后，进行初始化
signals:
    void MainWindowLoaded();
public:
    void emitMainWindowLoaded();
private slots:
    void onMainWindowLoaded();

// -- UI界面 槽函数
private slots:
    void on_btnSetting4Mic_clicked();
    void on_btnLoadWavDir_clicked();
    void on_btnStartRecord_clicked();
    void on_btnSetting4Model_clicked();
    void on_btnSetting4AutoTest_clicked();
    void on_btnLoadTempWavDir_clicked();
    void on_btnSwitchRunningMode_clicked();
    void on_btnTest_clicked();
    void on_btnLockOption4Model_clicked();
    void on_btnOpenWithExplorer_clicked();
    void on_btnStopRecord_clicked();
};
#endif // MAINWINDOW_H

