#include "Dlg4ChangeValue.h"
#include "ui_Dlg4ChangeValue.h"

Dlg4ChangeValue::Dlg4ChangeValue(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg4ChangeValue)
{
    ui->setupUi(this);
}

Dlg4ChangeValue::~Dlg4ChangeValue()
{
    delete ui;
}
