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
    quint64 m_duration1 = 0;
    quint64 m_duration2 = 0;
    quint64 m_recordDelay = 0;

signals:
    void autoTestConfigChanged();

private slots:
    void on_btnMainWorkDirApply_clicked();
    void on_btnRecordDelayApply_clicked();
    void on_btnTestDurationApply1_clicked();
    void on_btnTestDurationApply2_clicked();
};

#endif // SETUP4AUTOTEST_H
