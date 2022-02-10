#include "TempForm.h"
#include "ui_TempForm.h"

TempForm::TempForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TempForm)
{
    ui->setupUi(this);
}

TempForm::~TempForm()
{
    delete ui;
}
