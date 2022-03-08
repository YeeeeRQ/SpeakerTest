#ifndef SETUP4AUTOTEST_H
#define SETUP4AUTOTEST_H

#include <QWidget>
#include "./config/config.h"

namespace Ui {
class Setup4AutoTest;
}

class Setup4AutoTest : public QWidget
{
    Q_OBJECT

public:
    explicit Setup4AutoTest(QWidget *parent = nullptr);
    ~Setup4AutoTest();

private:
    Ui::Setup4AutoTest *ui;

private: // config
    Config& conf = Config::getInstance();
    void loadConfig4AutoTest();  // 载入配置文件
    void saveConfig4AutoTest();  // 保存配置文件



public:
    QString m_mainWorkDir;

    QString m_firstSpeaker;

    quint64 m_recordDelay = 0; //录音延时

    quint64 m_duration1 = 0; // 测试时段1
    quint64 m_duration1range = 0;
    quint64 m_duration1freq= 0;// 测试频率1
    int m_frequency1Idx = 0;
    quint64 m_duration1freqRange= 0;

    quint64 m_duration2 = 0; // 测试时段2
    quint64 m_duration2range = 0;
    quint64 m_duration2freq= 0;// 测试频率2
    int m_frequency2Idx = 0;
    quint64 m_duration2freqRange= 0;


    int m_interceptFreqIdx = 0;
    quint64 m_interceptFreq= 0; // 侦测频率
    quint64 m_interceptFreqRange = 0;
    quint64 m_interceptTimeout= 0; //侦测超时


signals:
    void autoTestConfigChanged();

private slots:
    void on_btnMainWorkDirApply_clicked();
    void on_btnApplyAll_clicked();
};

#endif // SETUP4AUTOTEST_H
