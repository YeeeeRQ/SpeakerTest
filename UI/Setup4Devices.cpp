#include "Setup4Devices.h"
#include "ui_Setup4Devices.h"

Setup4Devices::Setup4Devices(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setup4Devices)
{
    ui->setupUi(this);
}

Setup4Devices::~Setup4Devices()
{
    delete ui;
}
