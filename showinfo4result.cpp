#include "showinfo4result.h"
#include "ui_showinfo4result.h"

ShowInfo4Result::ShowInfo4Result(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowInfo4Result)
{
    ui->setupUi(this);

    // UI 界面设定
    QString qlabel_green_style =
                "QLabel{"
                "color:rgb(34, 139, 34)"
                "}";
    QString qlabel_red_style =
                "QLabel{"
                "color:rgb(205, 51, 51)"
                "}";

    ui->label_NG->setStyleSheet(qlabel_red_style);
    ui->label_OK->setStyleSheet(qlabel_green_style);
    ui->label_NG_number->setStyleSheet(qlabel_red_style);
    ui->label_OK_number->setStyleSheet(qlabel_green_style);
    this->clearAll();

    connect(this, &ShowInfo4Result::numberChanged, this, &ShowInfo4Result::onNumberChanged);


    // 秒表 计时
    this->ui->label_Time->setText("00.000s");
    this->timer = new QTimer;
    connect(this->timer, &QTimer::timeout, this, &ShowInfo4Result::freshenTime);


//    QTime::QTime
}

ShowInfo4Result::~ShowInfo4Result()
{
    delete ui;
}

void ShowInfo4Result::on_btnClear_clicked()
{
    this->clearAll();
}

void ShowInfo4Result::clearAll()
{
    ng_num = 0;
    ok_num = 0;
    total_num = 0;
    this->freshen();
}

void ShowInfo4Result::freshen()
{
    ui->label_NG_number->setText(QString::number(ng_num));
    ui->label_OK_number->setText(QString::number(ok_num));
    ui->label_Total_number->setText(QString::number(total_num));
}

void ShowInfo4Result::freshenTime()
{
    QTime  currTime = QTime::currentTime();
    int t= this->baseTime.msecsTo(currTime);
    QTime  showTime(0,0,0,0);
    showTime = showTime.addMSecs(t);
    this->timeStr = showTime.toString("ss.zzz")+"s";
    this->ui->label_Time->setText(timeStr);

}

void ShowInfo4Result::startTimer()
{
    this->baseTime = this->baseTime.currentTime();
    this->timer->start(1);
}

void ShowInfo4Result::stopTimer()
{
    if(timer->isActive()){
        timer->stop();
    }
}

void ShowInfo4Result::pointTimer()
{

}

void ShowInfo4Result::onNumberChanged()
{
    total_num = ok_num + ng_num;
    this->freshen();
}


void ShowInfo4Result::changeStatus2Waiting()
{
    ui->label_Status->setText("Waiting");
    ui->label_Status->setStyleSheet(
                "QLabel{"
                "color:rgb(238,238,0)"
                "}"
                );
}

void ShowInfo4Result::changeStatus2Testing()
{
    ui->label_Status->setText("Testing");
    ui->label_Status->setStyleSheet(
                "QLabel{"
                "color:rgb(255, 165, 0)"
                "}"
                );
}

void ShowInfo4Result::changeStatus2Pass()
{
    ui->label_Status->setText("Pass");
    ui->label_Status->setStyleSheet(
                "QLabel{"
                "color:rgb(34, 139, 34)"
                "}"
                );
}

void ShowInfo4Result::changeStatus2Fail()
{
    ui->label_Status->setText("Fail");
    ui->label_Status->setStyleSheet(
                "QLabel{"
                "color:rgb(205, 51, 51)"
                "}"
                );
}

void ShowInfo4Result::ngNumPlusOne()
{
    ng_num++;
    emit numberChanged();
}

void ShowInfo4Result::okNumPlusOne()
{
    ok_num++;
    emit numberChanged();
}
