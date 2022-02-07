#include "Setup4AutoTest.h"
#include "ui_Setup4AutoTest.h"

Setup4AutoTest::Setup4AutoTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setup4AutoTest)
{
    ui->setupUi(this);
}

Setup4AutoTest::~Setup4AutoTest()
{
    delete ui;
}
