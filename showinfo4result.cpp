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
