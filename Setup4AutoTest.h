#ifndef SETUP4AUTOTEST_H
#define SETUP4AUTOTEST_H

#include <QWidget>
#include "config.h"

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

    quint64 m_recordDelay = 0; //录音延时

    quint64 m_duration1 = 0; // 测试时段1
    quint64 m_duration1range = 0;
    quint64 m_duration1freq= 0;

    quint64 m_duration2 = 0; // 测试时段2
    quint64 m_duration2range = 0;
    quint64 m_duration2freq= 0;

    qint64 m_firstFreq = 0; // 侦测频率
    quint64 m_firstFreqRange = 0;
    quint64 m_interceptTimeout= 0; //侦测超时

signals:
    void autoTestConfigChanged();

private slots:
    void on_btnMainWorkDirApply_clicked();
    void on_btnApplyAll_clicked();
};

#endif // SETUP4AUTOTEST_H
