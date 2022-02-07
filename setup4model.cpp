#include "setup4model.h"
#include "ui_setup4model.h"

Setup4Model::Setup4Model(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setup4Model)
{
    ui->setupUi(this);
}

Setup4Model::~Setup4Model()
{
    delete ui;
}
