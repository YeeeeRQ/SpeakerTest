#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMainWindow>
#include <QPlainTextEdit>

#include "config.h"
#include "simplelog.h"

#include "setup4mic.h"
#include "setup4model.h"
#include "showinfo4result.h"

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
    void saveConfig();  // 保存配置文件
    void loadConfig();  // 载入配置文件
    void resetConfig(); // 恢复默认配置
//    QButtonGroup* mRadioGroup_runningmode;

private: // Test
    QString wavdir;


private:
    Setup4Mic * setup4mic = nullptr;

private:
    Ui::MainWindow *ui;

private slots:
    void on_btnLockOption4Model_clicked();
    void on_btnSetting4Mic_clicked();
    void on_btnLoadWavDir_clicked();
    void on_btnLoadWavDir4Test_clicked();
    void on_btnStartRecord_clicked();
    void on_btnStopRecord_clicked();
    void on_btnStartTest_clicked();
    void on_btnSetting4Model_clicked();
};
#endif // MAINWINDOW_H
