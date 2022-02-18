#ifndef SHOWINFO4RESULT_H
#define SHOWINFO4RESULT_H

#include <QWidget>
#include <QTimer>
#include <QTime>

namespace Ui {
class ShowInfo4Result;
}

class ShowInfo4Result : public QWidget
{
    Q_OBJECT

public:
    explicit ShowInfo4Result(QWidget *parent = nullptr);
    ~ShowInfo4Result();

private slots:
    void on_btnClear_clicked();

private:
    Ui::ShowInfo4Result *ui;
private:
    void clearAll();
    void freshen();
    void freshenTime();
    quint64 ng_num = 0;
    quint64 ok_num = 0;
    quint64 total_num = 0;

private:
    QTimer* timer;
    QTime baseTime;
    QString timeStr;

signals:
    void numberChanged();
private slots:
    void onNumberChanged();

public:
    void changeStatus2Start();
    void changeStatus2Waiting();
    void changeStatus2Testing();
    void changeStatus2Recording();
    void changeStatus2Processing();
    void changeStatus2Pass();
    void changeStatus2Fail();
    void changeStatus2Done();

    void ngNumPlusOne();
    void okNumPlusOne();

    void startTimer();
    void stopTimer();
    void pointTimer();
};

#endif // SHOWINFO4RESULT_H
