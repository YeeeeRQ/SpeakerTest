#include "showinfo4result.h"
#include "ui_showinfo4result.h"

ShowInfo4Result::ShowInfo4Result(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShowInfo4Result)
{
    ui->setupUi(this);
}

ShowInfo4Result::~ShowInfo4Result()
{
    delete ui;
}

void ShowInfo4Result::on_btnClear_clicked()
{
    ui->label_NG_number->setText("0");
    ui->label_OK_number->setText("0");
    ui->label_Total_number->setText("0");
}

