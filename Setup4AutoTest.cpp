#include "Setup4AutoTest.h"
#include "ui_Setup4AutoTest.h"
#include <QFileDialog>

Setup4AutoTest::Setup4AutoTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setup4AutoTest)
{
    ui->setupUi(this);
    loadConfig4AutoTest();
}

Setup4AutoTest::~Setup4AutoTest()
{
    delete ui;
}

void Setup4AutoTest::loadConfig4AutoTest()
{
    m_mainWorkDir= conf.Get("AutoTest", "MainWorkDir").toString();
    m_duration1= conf.Get("AutoTest", "duration1").toUInt();
    m_duration2=conf.Get("AutoTest", "duration2").toUInt();
    m_recordDelay = conf.Get("AutoTest", "RecordDelay").toUInt();

    ui->lineEdit_mainWorkDir->setText(m_mainWorkDir);
    ui->lineEdit_mainWorkDir->setToolTip(m_mainWorkDir);

    ui->lineEdit_duration1->setText(QString::number(m_duration1));
    ui->lineEdit_duration2->setText(QString::number(m_duration2));

    ui->lineEdit_recordDelay->setText(QString::number(m_recordDelay));
}

void Setup4AutoTest::saveConfig4AutoTest()
{

    //    conf.Set("", "", "");
}

void Setup4AutoTest::on_btnMainWorkDirApply_clicked()
{
    m_mainWorkDir = QFileDialog::getExistingDirectory();
    ui->lineEdit_mainWorkDir->setText(m_mainWorkDir);
    ui->lineEdit_mainWorkDir->setToolTip(m_mainWorkDir);
    conf.Set("AutoTest", "MainWorkDir", m_mainWorkDir);
    emit autoTestConfigChanged();
}


void Setup4AutoTest::on_btnRecordDelayApply_clicked()
{
    m_recordDelay = ui->lineEdit_recordDelay->text().toUInt();
    conf.Set("AutoTest", "RecordDelay", m_recordDelay);
    emit autoTestConfigChanged();
}


void Setup4AutoTest::on_btnTestDurationApply1_clicked()
{
    m_duration1= ui->lineEdit_duration1->text().toUInt();
    conf.Set("AutoTest", "duration1", m_duration1);

    quint64 range = ui->lineEdit_durationRange1->text().toUInt();
    QString s = QString::number(m_duration1 - range) + "-" +QString::number(m_duration1+range);
    ui->label_duration1->setText(s);

    emit autoTestConfigChanged();
}


void Setup4AutoTest::on_btnTestDurationApply2_clicked()
{
    m_duration2= ui->lineEdit_duration2->text().toUInt();
    conf.Set("AutoTest", "duration2", m_duration2);

    quint64 range = ui->lineEdit_durationRange2->text().toUInt();
    QString s = QString::number(m_duration2 - range) + "-" +QString::number(m_duration2+range);
    ui->label_duration2->setText(s);

    emit autoTestConfigChanged();
}

